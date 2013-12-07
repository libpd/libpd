package require Tclpd 0.3.0
package require TclpdLib 0.20

set ::script_path [file dirname [info script]]

pd::guiproc bitmap_draw_new {self c x y config data} {
    set w [dict get $config -uwidth]
    set h [dict get $config -uheight]
    set sz [dict get $config -cellsize]
    set fgcolor [dict get $config -fgcolor]
    set bgcolor [dict get $config -bgcolor]
    set colors [list $bgcolor $fgcolor]
    set z 0
    for {set i 0} {$i < $h} {incr i} {
        for {set j 0} {$j < $w} {incr j} {
            $c create rectangle \
                [expr {0+$x+$j*$sz}] [expr {0+$y+$i*$sz}] \
                [expr {1+$x+($j+1)*$sz}] [expr {1+$y+($i+1)*$sz}] \
                -outline $fgcolor -fill [lindex $colors [lindex $data $z]] \
                -tags [list $self cell_${j}_${i}_$self]
            incr z
        }
    }
    set x2 [expr {$x+$w*$sz+1}]
    set y2 [expr {$y+$h*$sz+1}]
    $c create rectangle $x $y $x2 $y2 \
        -outline $fgcolor -tags [list $self border$self]
}

proc+ bitmap::constructor {self args} {
    set @canvas [canvas_getcurrent]

    set s [file join $::script_path properties.tcl]
    sys_gui "source {$s}\n"

    pd::add_outlet $self float

    # set defaults:
    set @config [list]
    lappend @config -uwidth 8
    lappend @config -uheight 8
    lappend @config -cellsize 16
    lappend @config -label {}
    lappend @config -labelpos {top}
    lappend @config -sendsymbol {}
    lappend @config -receivesymbol {}
    lappend @config -fgcolor {#000000}
    lappend @config -bgcolor {#ffffff}
    lappend @config -lblcolor {#000000}
    set @data {
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    }
    # expanded ($n) send/recv symbols:
    set @send {}
    set @recv {}

    0_config $self {*}$args

    set @rcvLoadData {#bitmap}

    set x [pd_findbyclass $@rcvLoadData bitmap]
    if {$x ne "NULL"} {
        # prevent crash due to stale bound receivers:
        pd_unbind $x $@rcvLoadData
    }

    pd_bind $self $@rcvLoadData
}

proc+ bitmap::destructor {self} {
    set x [pd_findbyclass $@rcvLoadData bitmap]
    if {$x ne "NULL"} {
        pd_unbind $x $@rcvLoadData
    }
    if {[dict get $@config -receivesymbol] ne {}} {
        pd_unbind $self $@recv
    }
}

proc+ bitmap::0_config {self args} {
    if {$args eq {}} {
        return $@config
    } else {
        set newconf [list]
        set optlist [pd::strip_selectors $args]
        set optlist [pd::strip_empty $optlist]
        for {set i 0} {$i < [llength $optlist]} {} {
            set k [lindex $optlist $i]
            if {![dict exists $@config $k]} {
                return -code error "unknown option '$k'"
            }
            incr i
            set v [lindex $optlist $i]
            if {[lsearch -exact {-uwidth -uheight -cellsize} $k] != -1} {
                set v [expr {int($v)}]
            }
            dict set newconf $k $v
            incr i
        }
        if {[dict get $@config -uwidth] != [dict get $newconf -uwidth] ||
            [dict get $@config -uheight] != [dict get $newconf -uheight]} {
            0_resize $self {*}[pd::add_selectors [list \
                [dict get $newconf -uwidth] \
                [dict get $newconf -uheight] \
                ]]
        }
        set ui 0
        foreach opt {label labelpos cellsize fgcolor bgcolor lblcolor} {
            set old [dict get $@config -$opt]
            if {[dict exists $newconf -$opt]} {
                set new [dict get $newconf -$opt]
                if {$old ne $new} {
                    dict set @config -$opt $new
                    set ui 1
                }
            }
        }
        foreach opt {sendsymbol receivesymbol} {
            set old [dict get $@config -$opt]
            if {[dict exists $newconf -$opt]} {
                set new [dict get $newconf -$opt]
                if {$old ne $new} {
                    if {$opt eq {receivesymbol}} {
                        if {$old ne {}} {
                            pd_unbind $self $@recv
                        }
                        if {$new ne {}} {
                            set @recv [canvas_realizedollar $@canvas $new]
                            pd_bind $self $@recv
                        } else {
                            set @recv {}
                        }
                    }
                    dict set @config -$opt $new
                }
            }
        }
        if {$ui && [info exists @c]} {
            sys_gui [list $@c delete $self]\n
            sys_gui [list bitmap_draw_new $self \
                $@c $@x $@y $@config $@data]\n
        }
    }
}

proc+ bitmap::0_resize {self args} {
    set w [pd::arg 0 int]
    set h [pd::arg 1 int]
    set oldw [dict get $@config -uwidth]
    set oldh [dict get $@config -uheight]
    set newd {}
    for {set y 0} {$y < $h} {incr y} {
        for {set x 0} {$x < $w} {incr x} {
            if {$x < $oldw && $y < $oldh} {
                lappend newd [lindex $@data [expr {$y*$oldw+$x}]]
            } else {
                lappend newd 0
            }
        }
    }
    dict set @config -uwidth $w
    dict set @config -uheight $h
    set @data $newd
}

proc+ bitmap::0_getrow {self args} {
    set r [list]
    set n [pd::arg 0 int]
    set w [dict get $@config -uwidth]
    for {set i [expr {$n*$w}]} {$i < [expr {($n+1)*$w}]} {incr i} {
        lappend r [list float [lindex $@data $i]]
    }
    pd::outlet $self 0 list $r
}

proc+ bitmap::0_getcol {self args} {
    set r [list]
    set n [pd::arg 0 int]
    set w [dict get $@config -uwidth]
    set h [dict get $@config -uheight]
    for {set i [expr {$n}]} {$i < [expr {$w*$h}]} {incr i $w} {
        lappend r [list float [lindex $@data $i]]
    }
    pd::outlet $self 0 list $r
}

proc+ bitmap::0_getcell {self args} {
    set r [pd::arg 0 int]
    set c [pd::arg 1 int]
    set w [dict get $@config -uwidth]
    pd::outlet $self 0 float [lindex $@data [expr {$r*$w+$c}]]
}

proc+ bitmap::0_setrow {self args} {
    set row [pd::arg 0 int]
    set z 1
    set col 0
    set w [dict get $@config -uwidth]
    set fgcolor [dict get $@config -fgcolor]
    set bgcolor [dict get $@config -bgcolor]
    set colors [list $bgcolor $fgcolor]
    for {set idx [expr {$row*$w}]} {$idx < [expr {($row+1)*$w}]} {incr idx} {
        set d [expr {0 != [pd::arg $z int]}]
        lset @data $idx $d
        sys_gui [list $@c itemconfigure cell_${col}_${row}_$self \
            -fill [lindex $colors $d]]\n
        incr z
        incr col
    }
}

proc+ bitmap::0_setcol {self args} {
    set col [pd::arg 0 int]
    set z 1
    set row 0
    set w [dict get $@config -uwidth]
    set h [dict get $@config -uheight]
    set fgcolor [dict get $@config -fgcolor]
    set bgcolor [dict get $@config -bgcolor]
    set colors [list $bgcolor $fgcolor]
    for {set idx [expr {$col}]} {$idx < [expr {$w*$h}]} {incr idx $w} {
        set d [expr {0 != [pd::arg $z int]}]
        lset @data $idx $d
        sys_gui [list $@c itemconfigure cell_${col}_${row}_$self \
            -fill [lindex $colors $d]]\n
        incr z
        incr row
    }
}

proc+ bitmap::0_setcell {self args} {
    set r [pd::arg 0 int]
    set c [pd::arg 1 int]
    set d [expr {0 != [pd::arg 2 int]}]
    set w [dict get $@config -uwidth]
    set fgcolor [dict get $@config -fgcolor]
    set bgcolor [dict get $@config -bgcolor]
    set colors [list $bgcolor $fgcolor]
    set idx [expr {$r*$w+$c}]
    lset @data $idx $d
    sys_gui [list $@c itemconfigure cell_${r}_${c}_$self \
        -fill [lindex $colors $d]]\n
}

proc+ bitmap::0_setdata {self args} {
    set d [pd::strip_selectors $args]
    set l [llength $d]
    set w [dict get $@config -uwidth]
    set h [dict get $@config -uheight]
    if {$l != $w*$h} {
        return -code error "bad data size"
    }
    set @data [list]
    foreach i $d {lappend @data [expr {int($i)}]}

    set x [pd_findbyclass $@rcvLoadData bitmap]
    if {$x ne "NULL"} {
        pd_unbind $self $@rcvLoadData
    }
}

proc+ bitmap::save {self args} {
    return [list #X obj $@x $@y bitmap {*}[pd::add_empty $@config] \; \
        \#bitmap setdata {*}$@data \; ]
}

proc+ bitmap::properties {self args} {
    set title "\[bitmap\] properties"
    set buf [list propertieswindow %s $@config $title]\n
    gfxstub_new $self $self $buf
}

proc+ bitmap::widgetbehavior_getrect {self args} {
    lassign $args x1 y1
    set w [dict get $@config -uwidth]
    set h [dict get $@config -uheight]
    set sz [dict get $@config -cellsize]
    set x2 [expr {1+$x1+$w*$sz}]
    set y2 [expr {1+$y1+$h*$sz}]
    return [list $x1 $y1 $x2 $y2]
}

proc+ bitmap::widgetbehavior_displace {self args} {
    set dx [lindex $args 0]
    set dy [lindex $args 1]
    if {$dx != 0 || $dy != 0} {
        incr @x $dx
        incr @y $dy
        sys_gui [list $@c move $self $dx $dy]\n
    }
    return [list $@x $@y]
}

proc+ bitmap::widgetbehavior_select {self args} {
    set sel [lindex $args 0]
    set fgcolor [dict get $@config -fgcolor]
    set bgcolor [dict get $@config -bgcolor]
    set selcolor {blue}
    set colors [list $selcolor $fgcolor]
    sys_gui [list $@c itemconfigure $self \
        -outline [lindex $colors $sel]]\n
}

proc+ bitmap::widgetbehavior_activate {self args} {
}

proc+ bitmap::widgetbehavior_vis {self args} {
    set @c [lindex $args 0]
    set @x [lindex $args 1]
    set @y [lindex $args 2]
    set vis [lindex $args 3]
    set w [dict get $@config -uwidth]
    set h [dict get $@config -uheight]
    set sz [dict get $@config -cellsize]
    if {$vis} {
        sys_gui [list bitmap_draw_new $self $@c $@x $@y $@config $@data]\n
    } else {
        sys_gui [list $@c delete $self]\n
    }
}

proc+ bitmap::widgetbehavior_click {self args} {
    set w [dict get $@config -uwidth]
    set h [dict get $@config -uheight]
    set sz [dict get $@config -cellsize]
    set fgcolor [dict get $@config -fgcolor]
    set bgcolor [dict get $@config -bgcolor]
    set colors [list $bgcolor $fgcolor]
    set xpix [expr {[lindex $args 0]-$@x-1}]
    set ypix [expr {[lindex $args 1]-$@y-1}]
    if {$xpix < 0 || $xpix >= $w*$sz} {return}
    if {$ypix < 0 || $ypix >= $h*$sz} {return}
    set shift [lindex $args 2]
    set alt [lindex $args 3]
    set dbl [lindex $args 4]
    set doit [lindex $args 5]
    if {$doit} {
        set j [expr {$xpix/$sz}]
        set i [expr {$ypix/$sz}]
        set idx [expr {$w*${i}+${j}}]
        set d [expr {[lindex $@data $idx] == 0}]
        lset @data $idx $d
        sys_gui [list $@c itemconfigure cell_${j}_${i}_$self \
            -fill [lindex $colors $d]]\n
    }
}

pd::guiclass bitmap
