
package provide pd_menucommands 0.1

namespace eval ::pd_menucommands:: {
    variable untitled_number "1"

    namespace export menu_*
}

# ------------------------------------------------------------------------------
# functions called from File menu

proc ::pd_menucommands::menu_new {} {
    variable untitled_number
    if { ! [file isdirectory $::filenewdir]} {
        set ::filenewdir [file normalize $::env(HOME)]
    }
    # to localize "Untitled" there will need to be changes in g_canvas.c and
    # g_readwrite.c, where it tests for the string "Untitled"
    set untitled_name "Untitled"
    pdsend "pd filename $untitled_name-$untitled_number [enquote_path $::filenewdir]"
    pdsend "#N canvas"
    pdsend "#X pop 1"
    incr untitled_number
}

proc ::pd_menucommands::menu_open {} {
    if { ! [file isdirectory $::filenewdir]} {
        set ::filenewdir [file normalize $::env(HOME)]
    }
    set files [tk_getOpenFile -defaultextension .pd \
                       -multiple true \
                       -filetypes $::filetypes \
                       -initialdir $::fileopendir]
    if {$files ne ""} {
        foreach filename $files { 
            open_file $filename
        }
        set ::fileopendir [file dirname $filename]
    }
}

proc ::pd_menucommands::menu_print {mytoplevel} {
    set filename [tk_getSaveFile -initialfile pd.ps \
                      -defaultextension .ps \
                      -filetypes { {{postscript} {.ps}} }]
    if {$filename ne ""} {
        set tkcanvas [tkcanvas_name $mytoplevel]

        set pad 10 
        set bbox [$tkcanvas bbox all]
        set x [expr [lindex $bbox 0] - $pad]
        set y [expr [lindex $bbox 1] - $pad]
        set width  [expr [lindex $bbox 2] - $x + $pad]
        set height [expr [lindex $bbox 3] - $y + $pad]

        $tkcanvas postscript -x $x -y $y -width $width -height $height -file $filename 
    }
}

# ------------------------------------------------------------------------------
# functions called from Edit menu

proc ::pd_menucommands::menu_undo {} {
    if {$::focused_window eq $::undo_toplevel && $::undo_action ne "no"} {
        pdsend "$::focused_window undo"
    }
}

proc ::pd_menucommands::menu_redo {} {
    if {$::focused_window eq $::undo_toplevel && $::redo_action ne "no"} {
        pdsend "$::focused_window redo"
    }
}

proc ::pd_menucommands::menu_editmode {state} {
    if {[winfo class $::focused_window] ne "PatchWindow"} {return}
    set ::editmode_button $state
# this shouldn't be necessary because 'pd' will reply with pdtk_canvas_editmode
#    set ::editmode($::focused_window) $state
    pdsend "$::focused_window editmode $state"
}

proc ::pd_menucommands::menu_toggle_editmode {} {
    menu_editmode [expr {! $::editmode_button}]
}

proc ::pd_menucommands::menu_autopatch {state} {
    if {[winfo class $::focused_window] ne "PatchWindow"} {return}
    set ::autopatch_button $state
    set ::autopatch($::focused_window) $state
    # TODO does 'pd' need to track the autopatch state per patch?
    pdsend "pd autopatch $state"
}

proc ::pd_menucommands::menu_toggle_autopatch {} {
    menu_autopatch [expr {! $::autopatch_button}]
}

proc ::pd_menucommands::menu_magicglass {state} {
    if {[winfo class $::focused_window] ne "PatchWindow"} {return}
    set ::magicglass_button $state
    set ::magicglass($::focused_window) $state
    pdsend "$::focused_window magicglass $state"
}

proc ::pd_menucommands::menu_toggle_magicglass {} {
    menu_magicglass [expr {! $::magicglass_button}]
}

proc ::pd_menucommands::menu_perfmode {state} {
    set ::perfmode_button $state
    pdsend "pd perf $state"
}

proc ::pd_menucommands::menu_toggle_perfmode {} {
    menu_perfmode [expr {! $::perfmode_button}]
}

proc ::pd_menucommands::menu_autotips {state} {
    set ::autotips_button $state
}

proc ::pd_menucommands::menu_toggle_autotips {} {
    menu_autotips [expr {! $::autotips_button}]
}

proc ::pd_menucommands::menu_reselect {} {
    if {[winfo class $::focused_window] eq "PatchWindow"} {
        pdsend "$::focused_window reselect"
    }
}

# ------------------------------------------------------------------------------
# generic procs for sending menu events

# send a message to a pd canvas receiver
proc ::pd_menucommands::menu_send {window message} {
    set mytoplevel [winfo toplevel $window]
    if {[winfo class $mytoplevel] eq "PatchWindow"} {
        pdsend "$mytoplevel $message"
    } elseif {$mytoplevel eq ".pdwindow"} {
        if {$message eq "copy"} {
            tk_textCopy .pdwindow.text
        } elseif {$message eq "selectall"} {
            .pdwindow.text tag add sel 1.0 end
        } elseif {$message eq "menusaveas"} {
            ::pdwindow::save_logbuffer_to_file
        }
    }
}

# send a message to a pd canvas receiver with a float arg
proc ::pd_menucommands::menu_send_float {window message float} {
    set mytoplevel [winfo toplevel $window]
    if {[winfo class $mytoplevel] eq "PatchWindow"} {
        pdsend "$mytoplevel $message $float"
    }
}

# ------------------------------------------------------------------------------
# open the dialog panels

proc ::pd_menucommands::menu_message_dialog {} {
    ::dialog_message::open_message_dialog $::focused_window
}

proc ::pd_menucommands::menu_find_dialog {} {
    ::dialog_find::open_find_dialog $::focused_window
}

proc ::pd_menucommands::menu_font_dialog {} {
    if {[winfo exists .font]} {
        raise .font
    } elseif {$::focused_window eq ".pdwindow"} {
        pdtk_canvas_dofont .pdwindow [lindex [.pdwindow.text cget -font] 1]
    } else {
        pdsend "$::focused_window menufont"
    }
}

proc ::pd_menucommands::menu_path_dialog {} {
    if {[winfo exists .path]} {
        raise .path
    } else {
        pdsend "pd start-path-dialog"
    }
}

proc ::pd_menucommands::menu_startup_dialog {} {
    if {[winfo exists .startup]} {
        raise .startup
    } else {
        pdsend "pd start-startup-dialog"
    }
}

proc ::pd_menucommands::menu_helpbrowser {} {
    ::helpbrowser::open_helpbrowser
}

proc ::pd_menucommands::menu_texteditor {} {
    ::pdwindow::error "the text editor is not implemented"
}

# ------------------------------------------------------------------------------
# window management functions

proc ::pd_menucommands::menu_minimize {window} {
    wm iconify [winfo toplevel $window]
}

proc ::pd_menucommands::menu_maximize {window} {
    wm state [winfo toplevel $window] zoomed
}

proc ::pd_menucommands::menu_raise_pdwindow {} {
    if {$::focused_window eq ".pdwindow" && [winfo viewable .pdwindow]} {
        lower .pdwindow
    } else {
        wm deiconify .pdwindow
        raise .pdwindow
    }
}

# used for cycling thru windows of an app
proc ::pd_menucommands::menu_raisepreviouswindow {} {
    lower [lindex [wm stackorder .] end] [lindex [wm stackorder .] 0]
    focus [lindex [wm stackorder .] end]
}

# used for cycling thru windows of an app the other direction
proc ::pd_menucommands::menu_raisenextwindow {} {
    set mytoplevel [lindex [wm stackorder .] 0]
    raise $mytoplevel
    focus $mytoplevel
}

# ------------------------------------------------------------------------------
# Pd window functions
proc menu_clear_console {} {
    ::pdwindow::clear_console
}

# ------------------------------------------------------------------------------
# manage the saving of the directories for the new commands

# this gets the dir from the path of a window's title
proc ::pd_menucommands::set_filenewdir {mytoplevel} {
    if {$mytoplevel eq ".pdwindow"} {
        set ::filenewdir $::fileopendir
    } elseif {$::windowingsystem eq "aqua"} {
        set ::filenewdir [file dirname [wm attributes $mytoplevel -titlepath]]
    } else {
        regexp -- {[^/]+ - (.+)} [wm title $mytoplevel] ignored ::filenewdir
    }
}

# parse the textfile for the About Pd page
proc ::pd_menucommands::menu_aboutpd {} {
    ::pd_menucommands::menu_doc_open doc/5.reference about.pd
}

# ------------------------------------------------------------------------------
# opening docs as menu items (like the Test Audio and MIDI patch and the manual)
proc ::pd_menucommands::menu_doc_open {dir basename} {
    if {[file pathtype $dir] eq "relative"} {
        set dirname "$::sys_libdir/$dir"
    } else {
        set dirname $dir
    }
    set textextension "[string tolower [file extension $basename]]"
    if {[lsearch -exact [lindex $::filetypes 0 1]  $textextension] > -1} {
        set fullpath [file normalize [file join $dirname $basename]]
        set dirname [file dirname $fullpath]
        set basename [file tail $fullpath]
        pdsend "pd open [enquote_path $basename] [enquote_path $dirname]"
    } else {
        ::pd_menucommands::menu_openfile "$dirname/$basename"
    }
}

# open HTML docs from the menu using the OS-default HTML viewer
proc ::pd_menucommands::menu_openfile {filename} {
    if {$::tcl_platform(os) eq "Darwin"} {
        exec sh -c [format "open '%s'" $filename]
    } elseif {$::tcl_platform(platform) eq "windows"} {
        exec rundll32 url.dll,FileProtocolHandler [format "%s" $filename] &
    } else {
        foreach candidate { gnome-open xdg-open sensible-browser iceweasel firefox \
                                mozilla galeon konqueror netscape lynx } {
            set browser [lindex [auto_execok $candidate] 0]
            if {[string length $browser] != 0} {
                exec -- sh -c [format "%s '%s'" $browser $filename] &
                break
            }
        }
    }
}

# ------------------------------------------------------------------------------
# Mac OS X specific functions

proc ::pd_menucommands::menu_bringalltofront {} {
    # use [winfo children .] here to include windows that are minimized
    foreach item [winfo children .] {
        # get all toplevel windows, exclude menubar windows
        if { [string equal [winfo toplevel $item] $item] && \
                 [catch {$item cget -tearoff}]} {
            wm deiconify $item
        }
    }
    wm deiconify .
}

proc ::pd_menucommands::menu_makeapp {isdir} {
    set top_window [lindex [wm stackorder .] end]
    set patch [::makeapp::getpatchname $top_window]
    if {$patch == ""} {
        pdtk_post \
            [_ "No patch found! Select an open parent patch with the mouse, then try again."]\n
        return
    }
    # TODO set -parent to patch being turned into app
    pdtk_post [_ "Select name for app to build...\n"]
    set appdir [tk_getSaveFile -filetypes { {{Mac OS X Application} {.app}} } \
                    -parent $top_window -defaultextension .app \
                    -title [_ "Save application to..."]]
    if {$appdir != ""} {
        if {![string match "*.app" $appdir]} {
            set appdir "$appdir.app"
            #			pdtk_post "Adding .app extension: $appdir\n"
        }
        if {[::makeapp::promptreplace $appdir]} {
            ::makeapp::busypanel $appdir
            ::makeapp::createapp $appdir
            .makeapp.label configure -text [_ "Configuring Info.plist..."]
            ::makeapp::makeinfoplist $appdir
            .makeapp.label configure -text [_ "Setting patch name..."]
            regexp {.*/(.*?)\.pd} $patch -> patchname
            .makeapp.label configure -text [_ "Copying current patch..."]
            ::makeapp::copycurrentpatch $appdir $patch $patchname $isdir
            .makeapp.label configure -text [_ "Setting embedded preferences..."]
            ::makeapp::embedprefs $appdir "app-auto-load/$patchname.pd"
            pdtk_post [format [_ "%s is complete!"]\n $appdir]
            destroy .makeapp
        }
    }
}
