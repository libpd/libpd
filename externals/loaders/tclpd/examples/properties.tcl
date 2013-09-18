if {[catch {package require colorpicker}]} {
    source [file join [file dirname [info script]] colorpicker.tcl]
    package require colorpicker
}
namespace import ::colorpicker::colorpicker

proc propertieswindow {gfxstub_id {options {}} {title {}}} {
    set win $gfxstub_id
    set ::id($win.p) $gfxstub_id
    set ::optkeys($win.p) [list]
    set options [string map {@ $} $options]
    foreach {k v} $options {
        if {$v eq {empty}} {set v {}}
        #set v [string map {\\$ $} $v]
        set ::config($win.p:$k) $v
        lappend ::optkeys($win.p) $k
    }
    toplevel $win
    pack [propertiespanel $win.p]
    wm resizable $win 0 0
    wm title $win $title
    set win
}

proc has_key {w key} {
    expr {[lsearch -exact $::optkeys($w) $key] != -1}
}

proc propertiespanel {w} {
    set pad [propertiespanel_padding $w]
    incr pad $pad
    frame $w -borderwidth 0 -relief raised -padx $pad -pady $pad
    set subpanels {dimensions output behavior connective label colors}
    foreach subpanel $subpanels {
        set x [propertiespanel_$subpanel $w]
        if {$x ne {}} {grid $x -sticky ew -in $w}
    }
    set x [propertiespanel_buttons $w]
    grid $x -in $w
    grid columnconfigure . 0 -weight 1
    set w
}

proc propertiespanel_padding {w} {
    return 3
}

proc propertiespanel_dimensions {w} {
    set x ${w}.dimensions
    set pad [propertiespanel_padding $w]
    labelframe $x -text "Dimensions:" -borderwidth 1 -relief raised
    set count 0
    set row 0; set col 0
    if {[has_key $w -width]} {
        grid [label ${x}.wl -text "Width (px):" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.w -textvar ::config($w:-width) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    if {[has_key $w -height]} {
        grid [label ${x}.hl -text "Height (px):" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.h -textvar ::config($w:-height) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    incr row; set col 0
    if {[has_key $w -uwidth]} {
        grid [label ${x}.uwl -text "Width (cells):" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.uw -textvar ::config($w:-uwidth) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    if {[has_key $w -uheight]} {
        grid [label ${x}.uhl -text "Height (cells):" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.uh -textvar ::config($w:-uheight) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    incr row; set col 0
    if {[has_key $w -cellsize]} {
        grid [label ${x}.csl -text "Cell size (pixels):" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.cs -textvar ::config($w:-cellsize) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    incr row; set col 0
    if {[has_key $w -cellwidth]} {
        grid [label ${x}.uwl -text "Cell width:" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.uw -textvar ::config($w:-cellwidth) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    if {[has_key $w -cellheight]} {
        grid [label ${x}.uhl -text "Cell height:" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.uh -textvar ::config($w:-cellheight) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    incr row; set col 0
    if {$count == 0} {return {}}
    set x
}

proc propertiespanel_output {w} {
    set x ${w}.output
    set pad [propertiespanel_padding $w]
    labelframe $x -text "Output range:" -borderwidth 1 -relief raised
    set count 0
    set row 0; set col 0
    if {[has_key $w -rangebottom]} {
        grid [label ${x}.rbl -text "Bottom:" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.rb -textvar ::config($w:-rangebottom) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    if {[has_key $w -rangetop]} {
        grid [label ${x}.rtl -text "Top:" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.rt -textvar ::config($w:-rangetop) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    incr row; set col 0
    if {[has_key $w -rangeleft]} {
        grid [label ${x}.rll -text "Left:" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.rl -textvar ::config($w:-rangeleft) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    if {[has_key $w -rangeright]} {
        grid [label ${x}.rrl -text "Right:" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.rr -textvar ::config($w:-rangeright) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    incr row; set col 0
    if {[has_key $w -rangemin]} {
        grid [label ${x}.rml -text "Min:" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.rm -textvar ::config($w:-rangemin) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    if {[has_key $w -rangemax]} {
        grid [label ${x}.rMl -text "Max:" -anchor e] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        grid [entry ${x}.rM -textvar ::config($w:-rangemax) -width 5] \
            -row $row -column $col -sticky ew -padx $pad -pady $pad
        incr col
        incr count
    }
    incr row; set col 0
    if {[has_key $w -logarithmic]} {
        incr col
        grid [checkbutton ${x}.rL -variable ::config($w:-logarithmic) \
            -text "Logarithmic"] \
            -row $row -column $col -columnspan 3 -sticky w -padx $pad -pady $pad
        incr col
        incr count
    }
    incr row; set col 0
    if {$count == 0} {return {}}
    set x
}

proc propertiespanel_behavior {w} {
    set x ${w}.behavior
    set pad [propertiespanel_padding $w]
    labelframe $x -text "Widget behavior:" -borderwidth 1 -relief raised
    set count 0
    set row 0; set col 0
    if {[has_key $w -jumponclick]} {
        grid [checkbutton ${x}.joc -variable ::config($w:-jumponclick) \
            -text "Jump on click"] \
            -row $row -column $col -sticky w -padx $pad -pady $pad
        incr col
        incr count
    }
    if {[has_key $w -init]} {
        grid [checkbutton ${x}.init -variable ::config($w:-init) \
            -text "Output init value"] \
            -row $row -column $col -sticky w -padx $pad -pady $pad
        incr col
        incr count
    }
    incr row; set col 0
    if {$count == 0} {return {}}
    set x
}

proc propertiespanel_label {w} {
    set x ${w}.label
    set pad [propertiespanel_padding $w]
    labelframe $x -text "Label:" -borderwidth 1 -relief raised
    set count 0
    set row 0
    if {[has_key $w -label]} {
        grid [label ${x}.ll -text "Text:" -anchor e] \
            -row $row -column 0 -sticky ew -padx $pad -pady $pad
        grid [entry ${x}.l -textvar ::config($w:-label)] \
            -row $row -column 1 -sticky ew -padx $pad -pady $pad
        incr row
        incr count
    }
    if {[has_key $w -labelpos]} {
        grid [label ${x}.lpl -text "Position:" -anchor e] \
            -row $row -column 0 -sticky ew -padx $pad -pady $pad
        frame ${x}.f
        if {![info exists ::config($w:-labelpos)]} {
            set ::config($w:-labelpos) top
        }
        grid [radiobutton ${x}.f.lp1 -variable ::config($w:-labelpos) \
            -value top -text Top] \
            -row 1 -column 1 -sticky w -padx $pad -pady $pad -in ${x}.f
        grid [radiobutton ${x}.f.lp2 -variable ::config($w:-labelpos) \
            -value bottom -text Bottom] \
            -row 1 -column 2 -sticky w -padx $pad -pady $pad -in ${x}.f
        grid [radiobutton ${x}.f.lp3 -variable ::config($w:-labelpos) \
            -value left -text Left] \
            -row 2 -column 1 -sticky w -padx $pad -pady $pad -in ${x}.f
        grid [radiobutton ${x}.f.lp4 -variable ::config($w:-labelpos) \
            -value right -text Right] \
            -row 2 -column 2 -sticky w -padx $pad -pady $pad -in ${x}.f
        grid ${x}.f -sticky w -row $row -column 1
        incr row
        incr count
    }
    if {$count == 0} {return {}}
    set x
}

proc propertiespanel_connective {w} {
    set x ${w}.connective
    set pad [propertiespanel_padding $w]
    labelframe $x -text "Messages:" -borderwidth 1 -relief raised
    set count 0
    set row 0
    if {[has_key $w -sendsymbol]} {
        grid [label ${x}.ssl -text "Send symbol:" -anchor e] \
            -row $row -column 0 -sticky ew -padx $pad -pady $pad
        grid [entry ${x}.ss -textvar ::config($w:-sendsymbol) -width 15] \
            -row $row -column 1 -sticky ew -padx $pad -pady $pad
        incr row
        incr count
    }
    if {[has_key $w -receivesymbol]} {
        grid [label ${x}.rsl -text "Receive symbol:" -anchor e] \
            -row $row -column 0 -sticky ew -padx $pad -pady $pad
        grid [entry ${x}.rs -textvar ::config($w:-receivesymbol) -width 15] \
            -row $row -column 1 -sticky ew -padx $pad -pady $pad
        incr row
        incr count
    }
    if {$count == 0} {return {}}
    set x
}

proc propertiespanel_colors {w} {
    set colors {-bgcolor Background -fgcolor Foreground -lblcolor Label}
    set x ${w}.colors
    set pad [propertiespanel_padding $w]
    labelframe $x -text "Colors:" -borderwidth 1 -relief raised
    set count 0
    set row 0
    foreach {optkey color} $colors {
        if {![has_key $w $optkey]} {continue}
        grid [label ${x}.l$color -text "${color}:" -anchor e] \
            -row $row -column 0 -sticky ew -padx $pad -pady $pad
        grid [entry ${x}.t$color -textvar ::config($w:$optkey) -width 8] \
            -row $row -column 1 -sticky ew -padx $pad -pady $pad
        grid [frame ${x}.p$color -width 20 -height 20 \
            -borderwidth 1 -relief sunken] \
            -row $row -column 2 -sticky ew -padx $pad -pady $pad
        grid [button ${x}.b$color -text "Pick..." -overrelief {} \
            -command {} \
            ] -row $row -column 3 -sticky ew -padx $pad -pady $pad
        bind ${x}.b$color <Enter> {break}
        bind ${x}.b$color <Leave> {break}
        bind ${x}.b$color <ButtonPress-1> [list \
            propertiespanel_colors_pick \
            $w $x $colors ${x}.b$color ${x}.p$color ${x}.t$color]
        trace add variable ::config($w:$optkey) write [list \
            propertiespanel_colors_set_wrap $w $x ${x}.p$color $optkey]
        incr row
        incr count
    }
    if {![info exists ::cpt($w)]} {set ::cpt($w) switches}
    foreach {optkey color} $colors {
        if {![has_key $w $optkey]} {continue}
        # trigger the variable trace:
        if {[info exists ::config($w:$optkey)]} {
            set ::config($w:$optkey) $::config($w:$optkey)
        }
    }
    if {$count == 0} {return {}}
    frame ${x}.f
    grid [radiobutton ${x}.f.cpt1 -variable ::cpt($w) -justify right \
        -value switches -text Switches] \
        -row 0 -column 0 -sticky ew -padx $pad -pady $pad
    grid [radiobutton ${x}.f.cpt2 -variable ::cpt($w) -justify right \
        -value hsv -text HSV] \
        -row 1 -column 0 -sticky ew -padx $pad -pady $pad
    grid ${x}.f -row $row -column 0
    grid [colorpicker ${x}.cp2 hsv] \
        -row $row -column 1 -columnspan 3 -sticky ew -padx $pad -pady $pad
    grid [colorpicker ${x}.cp1 switches -command [list ${x}.cp2 set]] \
        -row $row -column 1 -columnspan 3 -sticky ew -padx $pad -pady $pad
    raise ${x}.cp1
    trace add variable ::cpt($w) write \
        [list propertiespanel_colors_switchpicker $w $x $row]
    set x
}

proc propertiespanel_colors_set_wrap {w x wp optkey config_ idx op} {
    propertiespanel_colors_set $w $x $wp {} -1 $::config($w:$optkey)
}

proc propertiespanel_colors_switchpicker {w x row cpt idx op} {
    raise ${x}.cp[expr {1+($::cpt($w) eq {hsv})}]
}

proc propertiespanel_colors_pick {w x colors wb wp wt} {
    foreach {k color} $colors {
        ${x}.b$color configure -relief raised -state normal
    }
    set r [$wb cget -relief]
    if {$r eq {sunken}} {
        $wb configure -relief raised
        ${x}.cp1 configure -command {}
        ${x}.cp2 configure -command {}
    } else {
        $wb configure -relief sunken
        ${x}.cp1 configure -command \
            [list propertiespanel_colors_set $w $x $wp $wt 1]
        ${x}.cp2 configure -command \
            [list propertiespanel_colors_set $w $x $wp $wt 2]
    }
}

proc propertiespanel_colors_set {w x wp wt from color} {
    if {$wt ne {}} {$wt delete 0 end ; $wt insert 0 $color}
    $wp configure -background $color
    if {$::cpt($w) eq {switches} && $from == 1} {
        ${x}.cp2 set $color
    }
}

proc propertiespanel_buttons {w} {
    set x ${w}.buttons
    set pad [propertiespanel_padding $w]
    frame $x -padx $pad -pady $pad
    set col 0
    foreach action {Cancel Apply Ok} {
        grid [button ${x}.btn$action \
            -command [list propertiespanel_buttons_action $w $action] \
            -text $action] \
            -row 0 -column $col -padx $pad -pady $pad
        incr col
    }
    set x
}

proc propertiespanel_buttons_action {w action} {
    switch -- $action {
        Cancel {
            propertiespanel_close $w
        }
        Apply {
            propertiespanel_apply $w
        }
        Ok {
            propertiespanel_apply $w
            propertiespanel_close $w
        }
    }
}

proc propertiespanel_apply {w} {
    set newconf [list]
    foreach key $::optkeys($w) {
        set v $::config($w:$key)
        if {$v eq {}} {set v {empty}}
        lappend newconf $key $v
    }
    #set newconf [string map {$ \\$} $newconf]
    set newconf [string map {$ @} $newconf]
    pdsend "$::id($w) config2 $newconf"
}

proc propertiespanel_close {w} {
    pdsend "$::id($w) cancel"
}
