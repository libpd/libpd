
package provide pdtk_canvas 0.1

#TODO: make a separate tcl file for tooltips and put
#afterid in its scope    
variable afterid 0
variable duplicate_tags -1
variable current_window 0

package require pd_bindings

namespace eval ::pdtk_canvas:: {
    namespace export pdtk_canvas_popup
    namespace export pdtk_canvas_editmode
    namespace export pdtk_canvas_autopatch
    namespace export pdtk_canvas_magicglass
    namespace export pdtk_canvas_getscroll
    namespace export pdtk_canvas_setparents
    namespace export pdtk_canvas_reflecttitle
    namespace export pdtk_canvas_menuclose
}

# One thing that is tricky to understand is the difference between a Tk
# 'canvas' and a 'canvas' in terms of Pd's implementation.  They are similar,
# but not the same thing.  In Pd code, a 'canvas' is basically a patch, while
# the Tk 'canvas' is the backdrop for drawing everything that is in a patch.
# The Tk 'canvas' is contained in a 'toplevel' window. That window has a Tk
# class of 'PatchWindow'.

# TODO figure out weird frameless window when you open a graph


#TODO: http://wiki.tcl.tk/11502
# MS Windows
#wm geometry . returns contentswidthxcontentsheight+decorationTop+decorationLeftEdge.
#and
#winfo rooty . returns contentsTop
#winfo rootx . returns contentsLeftEdge

# this proc is split out on its own to make it easy to override. This makes it
# easy for people to customize these calculations based on their Window
# Manager, desires, etc.
proc pdtk_canvas_place_window {width height geometry} {
    set screenwidth [lindex [wm maxsize .] 0]
    set screenheight [lindex [wm maxsize .] 1]

    # read back the current geometry +posx+posy into variables
    scan $geometry {%[+]%d%[+]%d} - x - y
    # fit the geometry onto screen
    set x [ expr $x % $screenwidth - $::windowframex]
    set y [ expr $y % $screenheight - $::windowframey]
    if {$width > $screenwidth} {
        set width $screenwidth
        set x 0
    }
    if {$height > $screenheight} {
        set height [expr $screenheight - $::menubarsize - 30] ;# 30 for window framing
        set y $::menubarsize
    }
    return [list $width $height ${width}x$height+$x+$y]
}


#------------------------------------------------------------------------------#
# canvas new/saveas

proc pdtk_canvas_new {mytoplevel width height geometry editable} {
    set l [pdtk_canvas_place_window $width $height $geometry]
    set width [lindex $l 0]
    set height [lindex $l 1]
    set geometry [lindex $l 2]

    # release the window grab here so that the new window will
    # properly get the Map and FocusIn events when its created
    ::pdwindow::busyrelease
    # set the loaded array for this new window so things can track state
    set ::loaded($mytoplevel) 0
    toplevel $mytoplevel -width $width -height $height -class PatchWindow
    wm group $mytoplevel .
    $mytoplevel configure -menu $::patch_menubar

    # we have to wait until $mytoplevel exists before we can generate
    # a <<Loading>> event for it, that's why this is here and not in the
    # started_loading_file proc.  Perhaps this doesn't make sense tho
    event generate $mytoplevel <<Loading>>

    wm geometry $mytoplevel $geometry
    wm minsize $mytoplevel $::canvas_minwidth $::canvas_minheight

    set tkcanvas [tkcanvas_name $mytoplevel]
    canvas $tkcanvas -width $width -height $height \
        -highlightthickness 0 -scrollregion [list 0 0 $width $height] \
        -xscrollcommand "$mytoplevel.xscroll set" \
        -yscrollcommand "$mytoplevel.yscroll set" \
        -background "$::canvas_fill"
    scrollbar $mytoplevel.xscroll -orient horizontal -command "$tkcanvas xview"
    scrollbar $mytoplevel.yscroll -orient vertical -command "$tkcanvas yview"
    pack $tkcanvas -side left -expand 1 -fill both

    # for some crazy reason, win32 mousewheel scrolling is in units of
    # 120, and this forces Tk to interpret 120 to mean 1 scroll unit
    if {$::windowingsystem eq "win32"} {
        $tkcanvas configure -xscrollincrement 1 -yscrollincrement 1
    }

    ::pd_bindings::patch_bindings $mytoplevel

    # give focus to the canvas so it gets the events rather than the window 	 
    focus $tkcanvas

    # let the scrollbar logic determine if it should make things scrollable
    set ::xscrollable($tkcanvas) 0
    set ::yscrollable($tkcanvas) 0

    # init patch properties arrays
    set ::editingtext($mytoplevel) 0
    set ::childwindows($mytoplevel) {}
    set ::autopatch($mytoplevel) 0
    set ::magicglass($mytoplevel) 0

    # this should be at the end so that the window and canvas are all ready
    # before this variable changes.
    set ::editmode($mytoplevel) $editable
}

# if the patch canvas window already exists, then make it come to the front
proc pdtk_canvas_raise {mytoplevel} {
    wm deiconify $mytoplevel
    raise $mytoplevel
    set mycanvas $mytoplevel.c
    focus $mycanvas
}

proc pdtk_canvas_saveas {name initialfile initialdir} {
    if { ! [file isdirectory $initialdir]} {set initialdir $::env(HOME)}
    set filename [tk_getSaveFile -initialfile $initialfile -initialdir $initialdir \
                      -defaultextension .pd -filetypes $::filetypes]
    if {$filename eq ""} return; # they clicked cancel

    set extension [file extension $filename]
    set oldfilename $filename
    set filename [regsub -- "$extension$" $filename [string tolower $extension]]
    if { ! [regexp -- "\.(pd|pat|mxt)$" $filename]} {
        # we need the file extention even on Mac OS X
        set filename $filename.pd
    }
    # test again after downcasing and maybe adding a ".pd" on the end
    if {$filename ne $oldfilename && [file exists $filename]} {
        set answer [tk_messageBox -type okcancel -icon question -default cancel\
                        -message [_ "\"$filename\" already exists. Do you want to replace it?"]]
        if {$answer eq "cancel"} return; # they clicked cancel
    }
    set dirname [file dirname $filename]
    set basename [file tail $filename]
    pdsend "$name savetofile [enquote_path $basename] [enquote_path $dirname]"
    set ::filenewdir $dirname
    # add to recentfiles
    ::pd_guiprefs::update_recentfiles $filename
}

##### ask user Save? Discard? Cancel?, and if so, send a message on to Pd ######
proc ::pdtk_canvas::pdtk_canvas_menuclose {mytoplevel reply_to_pd} {
    raise $mytoplevel
    set filename [wm title $mytoplevel]
    set message [format [_ "Do you want to save the changes you made in '%s'?"] $filename]
    set answer [tk_messageBox -message $message -type yesnocancel -default "yes" \
                    -parent $mytoplevel -icon question]
    switch -- $answer {
        yes { 
            pdsend "$mytoplevel menusave"
            if {[regexp {Untitled-[0-9]+} $filename]} {
                # wait until pdtk_canvas_saveas finishes and writes to
                # this var, otherwise the close command will be sent
                # immediately and the file won't get saved
                vwait ::filenewdir
            }
            pdsend $reply_to_pd
        }
        no {pdsend $reply_to_pd}
        cancel {}
    }
}

#------------------------------------------------------------------------------#
# mouse usage

# TODO put these procs into the pdtk_canvas namespace
proc pdtk_canvas_motion {tkcanvas x y mods} {
    set mytoplevel [winfo toplevel $tkcanvas]
    pdsend "$mytoplevel motion [$tkcanvas canvasx $x] [$tkcanvas canvasy $y] $mods"
}

proc pdtk_canvas_mouse {tkcanvas x y b f} {
    set mytoplevel [winfo toplevel $tkcanvas]
    pdsend "$mytoplevel mouse [$tkcanvas canvasx $x] [$tkcanvas canvasy $y] $b $f"
}

proc pdtk_canvas_mouseup {tkcanvas x y b} {
    set mytoplevel [winfo toplevel $tkcanvas]
    pdsend "$mytoplevel mouseup [$tkcanvas canvasx $x] [$tkcanvas canvasy $y] $b"
}

proc pdtk_canvas_rightclick {tkcanvas x y b} {
    set mytoplevel [winfo toplevel $tkcanvas]
    pdsend "$mytoplevel mouse [$tkcanvas canvasx $x] [$tkcanvas canvasy $y] $b 8"
}

proc pdtk_canvas_enteritem_gettags {tkcanvas x y item} {
    set mytoplevel [winfo toplevel $tkcanvas]
    set id [$tkcanvas find withtag current]
    set tags [$tkcanvas gettags $id]
    set xletno -1 
    foreach tag $tags {
	if [regexp "\.x.*\.t.*\[io\](\[0-9\]+)$" $tag -- xletno] {
	    break
	} else {
	    # iemgui tag
	    regexp ".*OUT(\[0-9\]+)$" $tag -- xletno
            regexp ".*IN(\[0-9\]+)$" $tag -- xletno
	}
    }
    pdsend "$mytoplevel enter $item \
        [$tkcanvas canvasx $x] [$tkcanvas canvasy $y] $xletno"
}

proc pdtk_canvas_enteritem {tkcanvas x y item enterid} {
    variable afterid
    variable duplicate_tags
    variable current_window
    if {$::autotips_button == 0} {return}
    if {$enterid != $duplicate_tags} {
	if {$::editmode([winfo toplevel $tkcanvas]) == 1} {
	    set duplicate_tags $enterid
	    if {$item eq "inlet" ||
	        $item eq "outlet"} {
                $tkcanvas itemconfigure $item -activewidth 5 
	    }
            if {$current_window eq $tkcanvas} {
		after cancel $afterid
	    }
	    set current_window $tkcanvas
            set afterid [after 600 pdtk_canvas_enteritem_gettags \
		$tkcanvas $x $y $item]
        } else {
	    set afterid 0
        }
    }
}

# move activewidth to toggle on editmode?
proc pdtk_canvas_leaveitem {w item} {
    variable afterid
    variable current_window
    if {$::autotips_button == 0} {return}
    after cancel $afterid
    if {[lsearch -exact [$w gettags $w.tipwindow] "sticky"] == -1} {
        if {$item eq "inlet" ||
	    $item eq "outlet"} {
            $w itemconfigure $item -activewidth 0
        }
	if {[winfo exists $w.tiplabel]} {
            set afterid [after 1500 "pdtk_tip $w 0 0"]
            set current_window $w
	}	
    }
}

proc pdtk_tip {w fromc show args} {
    set exists [winfo exists $w.tiplabel]
    if {$show == 0} {
        catch {destroy $w.tiplabel}
        catch {$w delete $w.tipwindow}
    } else {
	if {$exists} {
	    $w.tiplabel configure -text [join $args]
	    $w dtag $w.tipwindow "sticky"
	    if {$fromc == 1} {
		$w addtag "sticky" withtag $w.tipwindow
	    }  
	} else {
            label $w.tiplabel -text [join $args] -bd 1 \
	        -wraplength [winfo width $w] -bg "#c4dcdc" -bd 1 \
	        -padx 2 -pady 2 -relief raised
	} 
        set yreal [expr [$w canvasy 0] * -1 + \
	    [winfo pointery $w]-[winfo rooty $w]]
        set yoffset 0
        if {$yreal < [expr [winfo height $w] - \
	    [winfo reqheight $w.tiplabel]] - 5} {
	    set yoffset [winfo height $w]
            set anchor "sw"
        } else {
	    set anchor "nw"
        }
        set x [$w canvasx 0]
        set y [expr [$w canvasy 0] + $yoffset]
	set tags $w.tipwindow
	if {$fromc == 1} {
	    lappend tags "sticky"
	} 
	if {$exists} {
	    $w coords $w.tipwindow $x $y
	    $w itemconfigure $w.tipwindow -anchor $anchor
	} else {
            $w create window $x $y -window $w.tiplabel -anchor $anchor \
	        -tags $tags
	    $w bind $w.tipwindow <Enter> "pdtk_tip_mouseover $w"
	}
    }
}

# move the tip if the user happens to mouse over it
proc pdtk_tip_mouseover {w} {
    set msg [$w.tiplabel cget -text]
    set sticky [expr [lsearch -exact [$w gettags $w.tipwindow] \
	"sticky"] != -1]
    pdtk_tip $w $sticky 1 $msg 
}

proc pdtk_gettip { w item xletno name helpname dir } {
    if {$dir eq {}} {
	set dir $::sys_libdir/doc/5.reference
    }
    # trim off trailing ".pd" for abstractions
    regexp {^(.*)(?:\.pd)} $name -- name
    # use $varxlet to see if an object has a 
    # variable xlet (marked in the docs as
    # "INLET_N" or "OUTLET_N") 
    set varxlet {}
    set metatag description
    if {$item eq "inlet" ||
	$item eq "outlet"} {
	set varxlet [join [list $item "n"] "_"]
        set metatag [join [list $item $xletno] "_"]
	set msg "[string toupper [string map {_ " "} $metatag] 0 0] of $name"
    } elseif {$item eq "text"} {
	set metatag "description"
        set msg $name
    }
    set filefound 0
    if {![catch {set fp [open [file join $dir \
	    "$name-help.pd"]]}] ||
        ![catch {set fp [open [file join $dir \
	"$helpname-help.pd"]]}] } {
        set filefound 1
    }
    if {$filefound} {
	set filecontents [read $fp]
	close $fp
	# leave in pd's linebreaks-- serendipitously it
	# makes the tipwindow more compact/easier to read
	regsub -all {[{}]} $filecontents "" filecontents
	# turn escaped semicolons into linebreaks
	regsub -all {[\n\s]\\;[\n\s]} $filecontents "\n" filecontents
	set match {}
	# if $varxlet ne {} then the $item is an inlet or outlet
	if { $varxlet ne {}  &&
	    [regexp -nocase \
	    "#X text \[0-9\]+ \[0-9\]+ $varxlet (\[^;\]+)" \
	    $filecontents] } {
	    set match "(variable inlet)"
	}
        regexp -nocase \
	    "#X text \[0-9\]+ \[0-9\]+ $metatag (\[^;\]+)" \
	    $filecontents -- match
	if { $match ne {} } {
	    set msg [string trim "$msg: $match"]
	}
    }
    # make Pd's comma atoms look pretty
    regsub -all { \\,} $msg {,} msg
    regsub -all {\n\\,} $msg ",\n" msg
    pdtk_tip $w 0 1 $msg
}



# on X11, button 2 pastes from X11 clipboard, so simulate normal paste actions
proc pdtk_canvas_clickpaste {tkcanvas x y b} {
    pdtk_canvas_mouse $tkcanvas $x $y $b 0
    pdtk_canvas_mouseup $tkcanvas $x $y $b
    if { [catch {set pdtk_pastebuffer [selection get]}] } {
        # no selection... do nothing
    } else {
        for {set i 0} {$i < [string length $pdtk_pastebuffer]} {incr i 1} {
            set cha [string index $pdtk_pastebuffer $i]
            scan $cha %c keynum
            pdsend "pd key 1 $keynum 0"
        }
    }
}

#------------------------------------------------------------------------------#
# canvas popup menu

# since there is one popup that is used for all canvas windows, the menu
# -commands use {} quotes so that $::focused_window is interpreted when the
# menu item is called, not when the command is mapped to the menu item.  This
# is the same as the menubar in pd_menus.tcl but the opposite of the 'bind'
# commands in pd_bindings.tcl
proc ::pdtk_canvas::create_popup {} {
    if { ! [winfo exists .popup]} {
        # the popup menu for the canvas
        menu .popup -tearoff false
        .popup add command -label [_ "Properties"] \
            -command {::pdtk_canvas::done_popup $::focused_window 0}
        .popup add command -label [_ "Open"]       \
            -command {::pdtk_canvas::done_popup $::focused_window 1}
        .popup add command -label [_ "Help"]       \
            -command {::pdtk_canvas::done_popup $::focused_window 2}
    }
}

proc ::pdtk_canvas::done_popup {mytoplevel action} {
    pdsend "$mytoplevel done-popup $action $::popup_xcanvas $::popup_ycanvas"
}

proc ::pdtk_canvas::pdtk_canvas_popup {mytoplevel xcanvas ycanvas hasproperties hasopen} {
    set ::popup_xcanvas $xcanvas
    set ::popup_ycanvas $ycanvas
    if {$hasproperties} {
        .popup entryconfigure [_ "Properties"] -state normal
    } else {
        .popup entryconfigure [_ "Properties"] -state disabled
    }
    if {$hasopen} {
        .popup entryconfigure [_ "Open"] -state normal
    } else {
        .popup entryconfigure [_ "Open"] -state disabled
    }
    set tkcanvas [tkcanvas_name $mytoplevel]
    set scrollregion [$tkcanvas cget -scrollregion]
    # get the canvas location that is currently the top left corner in the window
    set left_xview_pix [expr [lindex [$tkcanvas xview] 0] * [lindex $scrollregion 2]]
    set top_yview_pix [expr [lindex [$tkcanvas yview] 0] * [lindex $scrollregion 3]]
    # take the mouse clicks in canvas coords, add the root of the canvas
    # window, and subtract the area that is obscured by scrolling
    set xpopup [expr int($xcanvas + [winfo rootx $tkcanvas] - $left_xview_pix)]
    set ypopup [expr int($ycanvas + [winfo rooty $tkcanvas] - $top_yview_pix)]
    tk_popup .popup $xpopup $ypopup 0
}


#------------------------------------------------------------------------------#
# procs for when file loading starts/finishes

proc ::pdtk_canvas::started_loading_file {patchname} {
    ::pdwindow::busygrab
}

# things to run when a patch is finished loading.  This is called when
# the OS sends the "Map" event for this window.
proc ::pdtk_canvas::finished_loading_file {mytoplevel} {
    # ::pdwindow::busyrelease is in pdtk_canvas_new so that the grab
    # is released before the new toplevel window gets created.
    # Otherwise the grab blocks the new window from getting the
    # FocusIn event on creation.

    # set editmode to make sure the menu item is in the right state
    pdtk_canvas_editmode $mytoplevel $::editmode($mytoplevel)
    set ::loaded($mytoplevel) 1
    # send the virtual events now that everything is loaded
    event generate $mytoplevel <<Loaded>>
}

#------------------------------------------------------------------------------#
# procs for canvas events

# check or uncheck the "edit" menu item
proc ::pdtk_canvas::pdtk_canvas_editmode {mytoplevel state} {
    set ::editmode_button $state
    set ::editmode($mytoplevel) $state
    event generate $mytoplevel <<EditMode>>
}

# check or uncheck the "Autopatch" menu item
proc ::pdtk_canvas::pdtk_canvas_autopatch {mytoplevel state} {
    set ::autopatch_button $state
    set ::autopatch($mytoplevel) $state
    event generate $mytoplevel <<Autopatch>>
    # 'pd' doesn't know about autopatch per-canvas, so we tell it here
    pdsend "pd autopatch $state"
}

# check or uncheck the "Magic Glass" menu item
proc ::pdtk_canvas::pdtk_canvas_magicglass {mytoplevel state} {
    set ::magicglass_button $state
    set ::magicglass($mytoplevel) $state
    event generate $mytoplevel <<MagicGlass>>
}

# message from Pd to update the currently available undo/redo action
proc pdtk_undomenu {mytoplevel undoaction redoaction} {
    set ::undo_toplevel $mytoplevel
    set ::undo_action $undoaction
    set ::redo_action $redoaction
    if {$mytoplevel ne "nobody"} {
        ::pd_menus::update_undo_on_menu $mytoplevel
    }
}

# This proc configures the scrollbars whenever anything relevant has
# been updated.  It should always receive a tkcanvas, which is then
# used to generate the mytoplevel, needed to address the scrollbars.
proc ::pdtk_canvas::pdtk_canvas_getscroll {tkcanvas} {
    # wait for Tk to finish drawing before resetting the scrollbars
    after idle ::pdtk_canvas::pdtk_canvas_do_getscroll $tkcanvas
}

proc ::pdtk_canvas::pdtk_canvas_do_getscroll {tkcanvas} {
    # check canvas exists, windows close before [after idle] returns
    if {![winfo exists $tkcanvas]} {return}
    set mytoplevel [winfo toplevel $tkcanvas]
    set bbox [$tkcanvas bbox all]
    if {$bbox eq "" || [llength $bbox] != 4} {return}
    set xupperleft [lindex $bbox 0]
    set yupperleft [lindex $bbox 1]
    if {$xupperleft > 0} {set xupperleft 0}
    if {$yupperleft > 0} {set yupperleft 0}
    set scrollregion [concat $xupperleft $yupperleft [lindex $bbox 2] [lindex $bbox 3]]
    $tkcanvas configure -scrollregion $scrollregion
    # X scrollbar
    if {[lindex [$tkcanvas xview] 0] == 0.0 && [lindex [$tkcanvas xview] 1] == 1.0} {
        set ::xscrollable($tkcanvas) 0
        pack forget $mytoplevel.xscroll
    } else {
        set ::xscrollable($tkcanvas) 1
        pack $mytoplevel.xscroll -side bottom -fill x -before $tkcanvas
    }
    # Y scrollbar, it gets touchy at the limit, so say > 0.995
    if {[lindex [$tkcanvas yview] 0] == 0.0 && [lindex [$tkcanvas yview] 1] > 0.995} {
        set ::yscrollable($tkcanvas) 0
        pack forget $mytoplevel.yscroll
    } else {
        set ::yscrollable($tkcanvas) 1
        pack $mytoplevel.yscroll -side right -fill y -before $tkcanvas
    }
}

proc ::pdtk_canvas::scroll {tkcanvas axis amount} {
    if {$axis eq "x" && $::xscrollable($tkcanvas) == 1} {
        $tkcanvas xview scroll [expr {- ($amount)}] units
    }
    if {$axis eq "y" && $::yscrollable($tkcanvas) == 1} {
        $tkcanvas yview scroll [expr {- ($amount)}] units
    }
}

#------------------------------------------------------------------------------#
# get patch window child/parent relationships

# add a child window ID to the list of children, if it isn't already there
proc ::pdtk_canvas::addchild {mytoplevel child} {
    # if either ::childwindows($mytoplevel) does not exist, or $child does not
    # exist inside of the ::childwindows($mytoplevel list
    if { [lsearch -exact [array names ::childwindows $mytoplevel]] == -1 \
             || [lsearch -exact $::childwindows($mytoplevel) $child] == -1} {
        set ::childwindows($mytoplevel) [lappend ::childwindows($mytoplevel) $child]
    }
}

# receive a list of all my parent windows from 'pd'
proc ::pdtk_canvas::pdtk_canvas_setparents {mytoplevel args} {
    set ::parentwindows($mytoplevel) $args
    foreach parent $args {
        addchild $parent $mytoplevel
    }
}

# receive information for setting the info the the title bar of the window
proc ::pdtk_canvas::pdtk_canvas_reflecttitle {mytoplevel \
                                              path name arguments dirty} {
    set ::windowname($mytoplevel) $name ;# TODO add path to this
    if {$::windowingsystem eq "aqua"} {
        wm attributes $mytoplevel -modified $dirty
        if {[file exists "$path/$name"]} {
            # for some reason -titlepath can still fail so just catch it 
            if [catch {wm attributes $mytoplevel -titlepath "$path/$name"}] {
                wm title $mytoplevel "$path/$name"
            }
        }
        wm title $mytoplevel "$name$arguments"
    } else {
        if {$dirty} {set dirtychar "*"} else {set dirtychar " "}
        wm title $mytoplevel "$name$dirtychar$arguments - $path" 
    }
}
