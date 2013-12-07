
package provide pdwindow 0.1

namespace eval ::pdwindow:: {
    variable logbuffer {}
    variable tclentry {}
    variable tclentry_history {"console show"}
    variable history_position 0
    variable linecolor 0 ;# is toggled to alternate text line colors
    variable logmenuitems
    variable maxloglevel 4

    variable lastlevel 0
    variable filtering 0 ;# flag to mark when the filtering is running
    variable currentcursor ;# store cursor to reset after filtering

    namespace export create_window
    namespace export pdtk_post
    namespace export pdtk_pd_dsp
    namespace export pdtk_pd_meters
    namespace export pdtk_pd_dio
}

# TODO make the Pd window save its size and location between running

proc ::pdwindow::set_layout {} {
    variable maxloglevel
    .pdwindow.text.internal tag configure log0 -foreground "#d00" -background "#ffe0e8"
    .pdwindow.text.internal tag configure log1 -foreground "#d00"
    # log2 messages are normal black on white
    .pdwindow.text.internal tag configure log3 -foreground "#484848"

    # 0-20(4-24) is a rough useful range of 'verbose' levels for impl debugging
    set start 4
    set end 25
    for {set i $start} {$i < $end} {incr i} {
        set B [expr int(($i - $start) * (40 / ($end - $start))) + 50]
        .pdwindow.text.internal tag configure log${i} -foreground grey${B}
    }
}


# grab focus on part of the Pd window when Pd is busy
proc ::pdwindow::busygrab {} {
    # set the mouse cursor to look busy and grab focus so it stays that way    
    .pdwindow.text configure -cursor watch
    grab set .pdwindow.text
}

# release focus on part of the Pd window when Pd is finished
proc ::pdwindow::busyrelease {} {
    .pdwindow.text configure -cursor xterm
    grab release .pdwindow.text
}

# ------------------------------------------------------------------------------
# pdtk functions for 'pd' to send data to the Pd window

proc ::pdwindow::buffer_message {object_id level message} {
    variable logbuffer
    lappend logbuffer $object_id $level $message
}

proc ::pdwindow::insert_log_line {object_id level message} {
    if {$object_id eq ""} {
        .pdwindow.text.internal insert end $message log$level
    } else {
        .pdwindow.text.internal insert end $message [list log$level obj$object_id]
        .pdwindow.text.internal tag bind obj$object_id <$::modifier-ButtonRelease-1> \
            "::pdwindow::select_by_id $object_id; break"
        .pdwindow.text.internal tag bind obj$object_id <Key-Return> \
            "::pdwindow::select_by_id $object_id; break"
        .pdwindow.text.internal tag bind obj$object_id <Key-KP_Enter> \
            "::pdwindow::select_by_id $object_id; break"
    }
}

# this is one "chunk" of the filtering, scheduled by "after idle"
proc ::pdwindow::filter_one_chunk {start_position} {
    variable logbuffer
    variable maxloglevel
    set end_position [expr $start_position + 3000]
    while {$start_position < $end_position && $start_position < [llength $logbuffer]} {
        set object_id [lindex $logbuffer $start_position]
        incr start_position
        set level [lindex $logbuffer $start_position]
        incr start_position
        set message [lindex $logbuffer $start_position]
        incr start_position
        if { $level <= $::loglevel || $maxloglevel == $::loglevel} {
            insert_log_line $object_id $level $message
        }
    }
    if {$start_position < [llength $logbuffer]} {
        after idle [list after 0 ::pdwindow::filter_one_chunk $start_position]
    } else {
        after idle [list after 10 ::pdwindow::filter_complete [expr $start_position / 3]]
    }
}

proc ::pdwindow::filter_complete {num} {
    variable filtering
    variable currentcursor
    .pdwindow.text.internal yview end
    .pdwindow.text.internal configure -cursor $currentcursor
    set filtering 0
    ::pdwindow::verbose 10 "The Pd window filtered $num lines\n"
}

# this has 'args' to satisfy trace, but its not used
proc ::pdwindow::filter_buffer_to_text {args} {
    variable maxloglevel
    variable filtering
    variable currentcursor
    set filtering 1
    # set the mouse cursor to a watch while busy
    set currentcursor [.pdwindow.text.internal cget -cursor]
    .pdwindow.text.internal configure -cursor watch
    .pdwindow.text.internal delete 0.0 end
    after idle [list after 0 ::pdwindow::filter_one_chunk 0]
}

proc ::pdwindow::select_by_id {args} {
    if [llength $args] { # Is $args empty?
        pdsend "pd findinstance $args"
    }
}

# logpost posts to Pd window with an object to trace back to and a
# 'log level'. The logpost and related procs are for generating
# messages that are useful for debugging patches.  They are messages
# that are meant for the Pd programmer to see so that they can get
# information about the patches they are building
proc ::pdwindow::logpost {object_id level message} {
    variable maxloglevel
    variable lastlevel $level

    buffer_message $object_id $level $message
    # if the level is low enough, and the Pd window is not busy
    # filtering thru the buffer, and the pdwindow has already been
    # created, then insert the message into the Pd window text widget
    if {$level <= $::loglevel &&
        ! $::pdwindow::filtering && 
        [llength [info commands .pdwindow.text.internal]] } {
        # cancel any pending move of the scrollbar, and schedule it
        # after writing a line. This way the scrollbar is only moved once
        # when the inserting has finished, greatly speeding things up
        after cancel .pdwindow.text.internal yview end
        insert_log_line $object_id $level $message
        after idle .pdwindow.text.internal yview end
    }
    # -stderr only sets $::stderr if 'pd-gui' is started before 'pd'
    if {$::stderr} {puts stderr $message}
}

# shortcuts for posting to the Pd window
proc ::pdwindow::fatal {message} {logpost {} 0 $message}
proc ::pdwindow::error {message} {logpost {} 1 $message}
proc ::pdwindow::post {message} {logpost {} 2 $message}
proc ::pdwindow::debug {message} {logpost {} 3 $message}
# for backwards compatibility
proc ::pdwindow::bug {message} {logpost {} 3 $message}
proc ::pdwindow::pdtk_post {message} {post $message}

proc ::pdwindow::endpost {} {
    variable linecolor
    variable lastlevel
    logpost {} $lastlevel "\n"
    set linecolor [expr ! $linecolor]
}

# this verbose proc has a separate numbering scheme since its for
# debugging implementations, and therefore falls outside of the 0-3
# numbering on the Pd window.  They should only be shown in ALL mode.
proc ::pdwindow::verbose {level message} {
    incr level 4
    logpost {} $level $message
}

# clear the log and the buffer
proc ::pdwindow::clear_console {} {
    variable logbuffer {}
    .pdwindow.text.internal delete 0.0 end
}

# save the contents of the pdwindow::logbuffer to a file
proc ::pdwindow::save_logbuffer_to_file {} {
    variable logbuffer
    set filename [tk_getSaveFile -initialfile "pdwindow.txt" -defaultextension .txt]
    if {$filename eq ""} return; # they clicked cancel
    set f [open $filename w]
    puts $f "Pd $::PD_MAJOR_VERSION.$::PD_MINOR_VERSION.$::PD_BUGFIX_VERSION.$::PD_TEST_VERSION on $::windowingsystem"
    puts $f "Tcl/Tk [info patchlevel]"
    puts $f "------------------------------------------------------------------------------"
    puts $f $logbuffer
    close $f
}


#--compute audio/DSP checkbutton-----------------------------------------------#

# set the checkbox on the "Compute Audio" menuitem and checkbox
proc ::pdwindow::pdtk_pd_dsp {value} {
    # TODO canvas_startdsp/stopdsp should really send 1 or 0, not "ON" or "OFF"
    if {$value eq "ON"} {
        set ::dsp 1
        .pdwindow.header.frame.dsp configure -background green
    } else {
        set ::dsp 0
        .pdwindow.header.frame.dsp configure -background lightgray
    }
}

proc ::pdwindow::pdtk_pd_dio {red} {
    if {$red == 1} {
        .pdwindow.header.dio configure -foreground red
    } else {
        .pdwindow.header.dio configure -foreground lightgray
    }
        
}

#--VU meter procedures---------------------------------------------------------#

proc ::pdwindow::calc_vu_color {position} {
    if {$position > 32} {
        return "#DF3A32"
    } elseif {$position > 30} {
        return "#EEAD54"
    } elseif {$position > 25} {
        return "#E9E448"
    } elseif {$position > 0} {
        return "#65DF37"
    } else {
        return "#6CEFF1"
    }
}

proc ::pdwindow::create_vu {tkcanvas name} {
    for {set i 0} {$i<40} {incr i} {
        set x [expr $i * 4]
        $tkcanvas create line $x 2 $x 18 -width 3 -tag "$name$i" \
            -fill [calc_vu_color $i] -state hidden
    }
}

proc ::pdwindow::set_vu {name db} {
    # TODO figure out the actual math here to properly map the incoming values
    set elements [expr ($db + 1) / 3]
    for {set i 0} {$i<$elements} {incr i} {
        .pdwindow.header.vu.$name itemconfigure "$name$i" -state normal
    }
    for {set i $elements} {$i<40} {incr i} {
        .pdwindow.header.vu.$name itemconfigure "$name$i" -state hidden
    }
}

proc ::pdwindow::pdtk_pd_meters {indb outdb inclip outclip} {
#    puts stderr [concat meters $indb $outdb $inclip $outclip]
    set_vu in $indb
    if {$inclip == 1} {
        .pdwindow.header.cliplabel.in configure -background red
    } else {
        .pdwindow.header.cliplabel.in configure -background lightgray
    }
    set_vu out $outdb
    if {$outclip == 1} {
        .pdwindow.header.cliplabel.out configure -background red
    } else {
        .pdwindow.header.cliplabel.out configure -background lightgray
    }
}

#--bindings specific to the Pd window------------------------------------------#

proc ::pdwindow::pdwindow_bindings {} {
    # these bindings are for the whole Pd window, minus the Tcl entry
    foreach window {.pdwindow.text .pdwindow.header} {
        bind $window <$::modifier-Key-x> "tk_textCut .pdwindow.text"
        bind $window <$::modifier-Key-c> "tk_textCopy .pdwindow.text"
        bind $window <$::modifier-Key-v> "tk_textPaste .pdwindow.text"
    }
    # Select All doesn't seem to work unless its applied to the whole window
    bind .pdwindow <$::modifier-Key-a> ".pdwindow.text tag add sel 1.0 end"
    # the "; break" part stops executing another binds, like from the Text class
    bind .pdwindow.text <Key-Tab> "focus .pdwindow.tcl.entry; break"

    # bindings for the Tcl entry widget
    bind .pdwindow.tcl.entry <$::modifier-Key-a> "%W selection range 0 end; break"
    bind .pdwindow.tcl.entry <Return> "::pdwindow::eval_tclentry"
    bind .pdwindow.tcl.entry <Up>     "::pdwindow::get_history 1"
    bind .pdwindow.tcl.entry <Down>   "::pdwindow::get_history -1"
    bind .pdwindow.tcl.entry <KeyRelease> +"::pdwindow::validate_tcl"

    # these don't do anything in the Pd window, so alert the user, then break
    # so no more bindings run
    bind .pdwindow <$::modifier-Key-s> "bell; break"
    bind .pdwindow <$::modifier-Key-p> "bell; break"

    # ways of hiding/closing the Pd window
    if {$::windowingsystem eq "aqua"} {
        # on Mac OS X, you can close the Pd window, since the menubar is there
        bind .pdwindow <$::modifier-Key-w>   "wm withdraw .pdwindow"
        wm protocol .pdwindow WM_DELETE_WINDOW "wm withdraw .pdwindow"
    } else {
        # TODO should it possible to close the Pd window and keep Pd open?
        bind .pdwindow <$::modifier-Key-w>   "wm iconify .pdwindow"
        wm protocol .pdwindow WM_DELETE_WINDOW "pdsend \"pd verifyquit\""
    }
}

#--Tcl entry procs-------------------------------------------------------------#

proc ::pdwindow::eval_tclentry {} {
    variable tclentry
    variable tclentry_history
    variable history_position 0
    if {$tclentry eq ""} {return} ;# no need to do anything if empty
    if {[catch {uplevel #0 $tclentry} errorname]} {
        global errorInfo
        switch -regexp -- $errorname { 
            "missing close-brace" {
                ::pdwindow::error [concat [_ "(Tcl) MISSING CLOSE-BRACE '\}': "] $errorInfo]\n
            } "missing close-bracket" {
                ::pdwindow::error [concat [_ "(Tcl) MISSING CLOSE-BRACKET '\]': "] $errorInfo]\n
            } "^invalid command name" {
                ::pdwindow::error [concat [_ "(Tcl) INVALID COMMAND NAME: "] $errorInfo]\n
            } default {
                ::pdwindow::error [concat [_ "(Tcl) UNHANDLED ERROR: "] $errorInfo]\n
            }
        }
    }
    lappend tclentry_history $tclentry
    set tclentry {}
}

proc ::pdwindow::get_history {direction} {
    variable tclentry_history
    variable history_position

    incr history_position $direction
    if {$history_position < 0} {set history_position 0}
    if {$history_position > [llength $tclentry_history]} {
        set history_position [llength $tclentry_history]
    }
    .pdwindow.tcl.entry delete 0 end
    .pdwindow.tcl.entry insert 0 \
        [lindex $tclentry_history end-[expr $history_position - 1]]
}

proc ::pdwindow::validate_tcl {} {
    variable tclentry
    if {[info complete $tclentry]} {
        .pdwindow.tcl.entry configure -background "white"
    } else {
        .pdwindow.tcl.entry configure -background "#FFF0F0"
    }
}

#--create tcl entry-----------------------------------------------------------#

proc ::pdwindow::create_tcl_entry {} {
# Tcl entry box frame
    label .pdwindow.tcl.label -text [_ "Tcl:"] -anchor e
    pack .pdwindow.tcl.label -side left
    entry .pdwindow.tcl.entry -width 200 \
       -exportselection 1 -insertwidth 2 -insertbackground blue \
       -textvariable ::pdwindow::tclentry -font {$::font_family 12}
    pack .pdwindow.tcl.entry -side left -fill x
# bindings for the Tcl entry widget
    bind .pdwindow.tcl.entry <$::modifier-Key-a> "%W selection range 0 end; break"
    bind .pdwindow.tcl.entry <Return> "::pdwindow::eval_tclentry"
    bind .pdwindow.tcl.entry <Up>     "::pdwindow::get_history 1"
    bind .pdwindow.tcl.entry <Down>   "::pdwindow::get_history -1"
    bind .pdwindow.tcl.entry <KeyRelease> +"::pdwindow::validate_tcl"

    bind .pdwindow.text <Key-Tab> "focus .pdwindow.tcl.entry; break"
}

proc ::pdwindow::set_findinstance_cursor {widget key state} {
    set triggerkeys [list Control_L Control_R Meta_L Meta_R]
    if {[lsearch -exact $triggerkeys $key] > -1} {
        if {$state == 0} {
            $widget configure -cursor xterm
        } else {
            $widget configure -cursor based_arrow_up
        }
    }
}

#--create the window-----------------------------------------------------------#

proc ::pdwindow::create_window {} {
    variable logmenuitems
    set ::loaded(.pdwindow) 0

    # colorize by class before creating anything
    option add *PdWindow*Entry.highlightBackground "lightgray" startupFile
    option add *PdWindow*Frame.background "lightgray" startupFile
    option add *PdWindow*Label.background "lightgray" startupFile
    option add *PdWindow*Checkbutton.background "lightgray" startupFile
    option add *PdWindow*Menubutton.background "lightgray" startupFile
    option add *PdWindow*Text.background "white" startupFile
    option add *PdWindow*Entry.background "white" startupFile

    toplevel .pdwindow -class PdWindow
    wm title .pdwindow [_ "Pd-extended"]
    set ::windowname(.pdwindow) [_ "Pd-extended"]
    if {$::windowingsystem eq "x11"} {
        wm minsize .pdwindow 400 75
    } else {
        wm minsize .pdwindow 400 51
    }
    wm geometry .pdwindow =500x400+20+50
    .pdwindow configure -menu .menubar

    frame .pdwindow.header -borderwidth 1 -relief flat
    pack .pdwindow.header -side top -fill x -ipady 5
    frame .pdwindow.header.spacer -borderwidth 0
    pack .pdwindow.header.spacer -side left -fill y -anchor w -ipadx 1
    checkbutton .pdwindow.header.meters -variable ::meters -takefocus 1 \
        -command {pdsend "pd meters $::meters"}
    pack .pdwindow.header.meters -side left -fill y -anchor w -padx 5 -pady 10
    frame .pdwindow.header.vu -borderwidth 0
    pack .pdwindow.header.vu -side left
    canvas .pdwindow.header.vu.in -width 150 -height 20 -background "#3F3F3F" \
        -highlightthickness 1 -highlightbackground lightgray
    create_vu .pdwindow.header.vu.in "in"
    canvas .pdwindow.header.vu.out -width 150 -height 20 -background "#3F3F3F" \
        -highlightthickness 1 -highlightbackground lightgray
    create_vu .pdwindow.header.vu.out "out"
    pack .pdwindow.header.vu.in -side top
    pack .pdwindow.header.vu.out -side top
    frame .pdwindow.header.cliplabel -borderwidth 0
    pack .pdwindow.header.cliplabel -side left
    label .pdwindow.header.cliplabel.in -text [_ "IN"] \
        -width [string length [_ "IN"]]
    label .pdwindow.header.cliplabel.out -text [_ "OUT"] \
        -width [string length [_ "OUT"]]
    pack .pdwindow.header.cliplabel.in .pdwindow.header.cliplabel.out -side top
    frame .pdwindow.header.frame -borderwidth 0 -padx 5
    pack .pdwindow.header.frame -side right
    checkbutton .pdwindow.header.frame.dsp -text [_ "DSP"] -variable ::dsp \
        -font {$::font_family 18 bold} -takefocus 1 -padx 5 \
        -borderwidth 0  -command {pdsend "pd dsp $::dsp"}
    pack .pdwindow.header.frame.dsp -side right -fill y -anchor e -padx 5 -pady 0
# DIO button
    label .pdwindow.header.dio -text [_ "audio I/O error"] -borderwidth 0 \
        -foreground lightgray -takefocus 0 -font {$::font_family 14}
    pack .pdwindow.header.dio -side right -fill y -padx 30 -pady 0

# Tcl entry box and log level frame
    frame .pdwindow.tcl -borderwidth 0
    pack .pdwindow.tcl -side bottom -fill x
    frame .pdwindow.tcl.pad
    pack .pdwindow.tcl.pad -side right -padx 12

    set loglevels {0 1 2 3 4}
    lappend logmenuitems "0 [_ fatal]"
    lappend logmenuitems "1 [_ error]"
    lappend logmenuitems "2 [_ normal]"
    lappend logmenuitems "3 [_ debug]"
    lappend logmenuitems "4 [_ all]"
    set logmenu \
        [eval tk_optionMenu .pdwindow.tcl.logmenu ::loglevel $loglevels]
    .pdwindow.tcl.logmenu configure
    foreach i $loglevels {
        $logmenu entryconfigure $i -label [lindex $logmenuitems $i]
    }
    trace add variable ::loglevel write ::pdwindow::filter_buffer_to_text

    # TODO figure out how to make the menu traversable with the keyboard
    #.pdwindow.tcl.logmenu configure -takefocus 1
    pack .pdwindow.tcl.logmenu -side right
    label .pdwindow.tcl.loglabel -text [_ "Log:"] -anchor e
    pack .pdwindow.tcl.loglabel -side right
    label .pdwindow.tcl.label -text [_ "Tcl:"] -anchor e
    pack .pdwindow.tcl.label -side left
    entry .pdwindow.tcl.entry -width 200 \
        -exportselection 1 -insertwidth 2 -insertbackground blue \
        -textvariable ::pdwindow::tclentry -font {$::font_family 12}
    pack .pdwindow.tcl.entry -side left -fill x
# TODO this should use the pd_font_$size created in pd-gui.tcl    
    text .pdwindow.text -relief raised -bd 2 -font {-size 10} \
        -highlightthickness 0 -borderwidth 1 -relief flat \
        -yscrollcommand ".pdwindow.scroll set" -width 60 \
        -undo false -autoseparators false -maxundo 1 -takefocus 0
    scrollbar .pdwindow.scroll -command ".pdwindow.text.internal yview"
    pack .pdwindow.scroll -side right -fill y
    pack .pdwindow.text -side right -fill both -expand 1
    raise .pdwindow
    focus .pdwindow.text
    # run bindings last so that .pdwindow.tcl.entry exists
    pdwindow_bindings
    # set cursor to show when clicking in 'findinstance' mode
    bind .pdwindow <KeyPress> "+::pdwindow::set_findinstance_cursor %W %K %s"
    bind .pdwindow <KeyRelease> "+::pdwindow::set_findinstance_cursor %W %K %s"

    # wait until .pdwindow.text is visible before opening files so that
    # the loading logic can grab it and put up the busy cursor
    tkwait visibility .pdwindow.text

    # hack to make a good read-only text widget from http://wiki.tcl.tk/1152
    rename ::.pdwindow.text ::.pdwindow.text.internal
    proc ::.pdwindow.text {args} {
        switch -exact -- [lindex $args 0] {
            "insert" {}
            "delete" {}
            "default" { return [eval ::.pdwindow.text.internal $args] }
        }
    }
    
    # print whatever is in the queue after the event loop finishes
    after idle [list after 0 ::pdwindow::filter_buffer_to_text]

    set ::loaded(.pdwindow) 1

    # set some layout variables
    ::pdwindow::set_layout
}
