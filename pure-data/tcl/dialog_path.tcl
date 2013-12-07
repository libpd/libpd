
package provide dialog_path 0.1

namespace eval ::dialog_path:: {
    variable use_standard_extensions_button 1
    variable verbose_button 0
    variable realtime_button 0
    variable startup_flags ""

    namespace export pdtk_path_dialog
}

############ pdtk_path_dialog -- run a path dialog #########

# set up the panel with the info from pd
proc ::dialog_path::pdtk_path_dialog {mytoplevel extrapath verbose} {
    variable use_standard_extensions_button $extrapath
    variable verbose_button $verbose

    if {[winfo exists $mytoplevel]} {
        wm deiconify $mytoplevel
        raise $mytoplevel
    } else {
        create_dialog $mytoplevel
    }
}

proc ::dialog_path::create_dialog {mytoplevel} {
    # filter out internal paths
    set searchpath {}
    foreach path $::sys_searchpath {
        set normalized [file normalize [file dirname [file dirname $path]]]
        if {$normalized ne $::sys_libdir} {
            lappend searchpath $path
        }
    }
    set ::sys_searchpath $searchpath
    scrollboxwindow::make $mytoplevel $::sys_searchpath \
        dialog_path::add dialog_path::edit dialog_path::commit \
        [_ "Pd search path for objects, help, fonts, and other files"] \
        500 400
    
    frame $mytoplevel.extraframe
    pack $mytoplevel.extraframe -side top
    frame $mytoplevel.extraframe.l
    frame $mytoplevel.extraframe.r
    pack $mytoplevel.extraframe.l $mytoplevel.extraframe.r -side left -padx 2m -pady 2m
    checkbutton $mytoplevel.extraframe.l.extra -text [_ "Use standard extensions"] \
        -variable ::dialog_path::use_standard_extensions_button -anchor w
    checkbutton $mytoplevel.extraframe.l.verbose -text [_ "Verbose"] \
        -variable ::dialog_path::verbose_button -anchor w
    pack $mytoplevel.extraframe.l.extra $mytoplevel.extraframe.l.verbose \
        -side top -expand 1 -fill x
    if {$::windowingsystem ne "win32"} {
        checkbutton $mytoplevel.extraframe.r.realtime -anchor w \
            -text [_ "Real-time scheduling"] \
            -variable ::dialog_path::realtime_button
        pack $mytoplevel.extraframe.r.realtime -side right
    }

    frame $mytoplevel.flagsframe
    pack $mytoplevel.flagsframe -side top -pady 2m
    label $mytoplevel.flagsframe.entryname -text [_ "Startup flags:"]
    entry $mytoplevel.flagsframe.entry -textvariable ::startup_flags -width 60
    pack $mytoplevel.flagsframe.entryname $mytoplevel.flagsframe.entry -side left
}


############ pdtk_path_dialog -- dialog window for search path #########
proc ::dialog_path::choosePath { currentpath title } {
    if {$currentpath == ""} {
        set currentpath "~"
    }
    return [tk_chooseDirectory -initialdir $currentpath -title $title]
}

proc ::dialog_path::add {} {
    return [::dialog_path::choosePath "" {Add a new path}]
}

proc ::dialog_path::edit { currentpath } {
    return [::dialog_path::choosePath $currentpath "Edit existing path \[$currentpath\]"]
}

proc ::dialog_path::commit { new_path } {
    variable use_standard_extensions_button
    variable verbose_button
    variable realtime_button

    set ::sys_searchpath $new_path
    pdsend "pd realtime $realtime_button"
    pdsend "pd startup-flags [pdtk_encodedialog $::startup_flags]"
    pdsend "pd path-dialog $use_standard_extensions_button $verbose_button $::sys_searchpath"
}
