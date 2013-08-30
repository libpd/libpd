
package provide wheredoesthisgo 0.1

# a place to temporarily store things until they find a home or go away

proc open_file {filename} {
    set directory [file normalize [file dirname $filename]]
    set basename [file tail $filename]
    # if a Max binary patch, convert to a text patch using cyclist
    if {[regexp -nocase -- "\.(pat|mxb|help)$" $basename]} {
        switch -- $::windowingsystem {
            "x11" {
                # on GNU/Linux, cyclist is installed into /usr/bin usually
                set cyclist "/usr/bin/cyclist"
                set tmpdir "/tmp"
            }
            "win32" {
                set cyclist "$::sys_libdir/bin/cyclist"
                set tmpdir [file normalize $::env(TMP)]
            }
            "aqua" {
                set cyclist "$::sys_libdir/bin/cyclist"
                set tmpdir "/tmp"
            }
        }
        ::pdwindow::debug [format [_ "Running: '%s %s'"] $cyclist $filename]\n
        # convert Max binary to text .pat
        set binport [open "| \"$cyclist\" \"$filename\""]
        set convertedtext [read $binport]
        if {$convertedtext ne "" && ! [catch {close $binport} err]} {
            if {! [file writable $directory]} { set directory tmpdir }
            set basename "$basename.mxt"
            # override $filename so that it loads with the logic in the next if {}
            set filename [file join $directory $basename]
            set textpatfile [open "$filename" w]
            puts $textpatfile $convertedtext
            close $textpatfile
            ::pdwindow::post [concat [_ "converted Max binary to text format:"] \
                                  "'$filename'"]\n
        }
    }
    if {
        [file exists $filename]
        && [regexp -nocase -- "\.(pd|pat|mxt)$" $filename]
    } then {
        ::pdtk_canvas::started_loading_file [format "%s/%s" $basename $filename]
        pdsend "pd open [enquote_path $basename] [enquote_path $directory]"
        # now this is done in pd_guiprefs
        ::pd_guiprefs::update_recentfiles $filename
    } {
        ::pdwindow::post [format [_ "Ignoring '%s': doesn't look like a Pd-file"]\n $filename]
    }
}

# ------------------------------------------------------------------------------
# procs for panels (openpanel, savepanel)

proc pdtk_openpanel {target localdir} {
    if {! [file isdirectory $localdir]} {
        if { ! [file isdirectory $::fileopendir]} {
            set ::fileopendir $::env(HOME)
        }
        set localdir $::fileopendir
    }
    set filename [tk_getOpenFile -initialdir $localdir]
    if {$filename ne ""} {
        set ::fileopendir [file dirname $filename]
        pdsend "$target callback [enquote_path $filename]"
    }
}

proc pdtk_savepanel {target localdir} {
    if {! [file isdirectory $localdir]} {
        if { ! [file isdirectory $::filenewdir]} {
            set ::filenewdir $::env(HOME)
        }
        set localdir $::filenewdir
    }
    set filename [tk_getSaveFile -initialdir $localdir]
    if {$filename ne ""} {
        pdsend "$target callback [enquote_path $filename]"
    }
}

# ------------------------------------------------------------------------------
# window info (name, path, parents, children, etc.)

proc lookup_windowname {mytoplevel} {
    set window [array get ::windowname $mytoplevel]
    if { $window ne ""} {
        return [lindex $window 1]
    } else {
        return ERROR
    }
}

proc tkcanvas_name {mytoplevel} {
    return "$mytoplevel.c"
}

# ------------------------------------------------------------------------------
# quoting functions

# enquote a string for find, path, and startup dialog panels, to be decoded by
# sys_decodedialog()
proc pdtk_encodedialog {x} {
    concat +[string map {" " "+_" "$" "+d" ";" "+s" "," "+c" "+" "++"} $x]
}

# encode a list with pdtk_encodedialog
proc pdtk_encode { listdata } {
    set outlist {}
    foreach this_path $listdata {
        if {0==[string match "" $this_path]} {
            lappend outlist [pdtk_encodedialog $this_path]
        }
    }
    return $outlist
}

# TODO enquote a filename to send it to pd, " isn't handled properly tho...
proc enquote_path {message} {
    string map {"," "\\," ";" "\\;" " " "\\ "} $message
}

#enquote a string to send it to Pd.  Blow off semi and comma; alias spaces
#we also blow off "{", "}", "\" because they'll just cause bad trouble later.
proc unspace_text {x} {
    set y [string map {" " "_" ";" "" "," "" "{" "" "}" "" "\\" ""} $x]
    if {$y eq ""} {set y "empty"}
    concat $y
}

# ------------------------------------------------------------------------------
# watchdog functions

proc pdtk_watchdog {} {
   pdsend "pd watchdog"
   after 2000 {pdtk_watchdog}
}

proc pdtk_ping {} {
    pdsend "pd ping"
}

# ------------------------------------------------------------------------------
# crazy kludges to work around stdout being redirected to tclpd on Mac OS X

# On Mac OS X, when you try to use jack as the audio device, and the
# jack server is not started, then you'll get the cmd line help dump
# from the 'jackdmp' program.  Since stdout is being redirectly into
# tclpd for execution, it tries to execute the jackdmp help message.
# The first part of that message is "jackdmp 1.9.9", so we just make a
# proc called "jackdmp", which tclpd will execute in this condition.

proc jackdmp {args} {
    if {[file exists "/usr/local/bin/jackdmp"]} {
        if {[file exists "/Applications/Jack/JackPilot.app"]} {
            set a [tk_messageBox \
                       -title [_ "Jack not running!"] \
                       -detail [_ "Jack does not seem to be running, so Pd-extended cannot connect to it."] \
                       -message [_ "Would you like to open JackPilot to start Jack?"] \
                       -icon question -type yesno -parent .pdwindow]
            if {$a eq "yes"} {
                ::pd_menucommands::menu_openfile "/Applications/Jack/JackPilot.app"
            }
        } else {
            set a [tk_messageBox \
                       -title [_ "Jack not running!"] \
                       -detail [_ "Jack does not seem to be running, so Pd-extended cannot connect to it."] \
                       -message [_ "Quit Pd-extended, start Jack, then try again."] \
                       -icon question -type ok -parent .pdwindow]
        }
    } else {
        set a [tk_messageBox \
                   -title [_ "Jack not installed!"] \
                   -detail [_ "JackOSX is not fully installed, Pd-extended cannot use Jack without it."] \
                   -message [_ "Would you like to open JackOSX.com to download Jack?"] \
                   -icon question -type yesno -parent .pdwindow]
        if {$a eq "yes"} {
            yes {::pd_menucommands::menu_openfile "http://jackosx.com/"}
        }
    }
}

# some random things also pop up here and there, catch them

proc tclpd_stdout_override {args} {
    ::pdwindow::logpost {} 9 "tclpd_stdout_override: $args\n"
}

proc StartNotification {args} {
    tclpd_stdout_override "StartNotification $args"
}
