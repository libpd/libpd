package require Tclpd 0.3.0
package require TclpdLib 0.20

set ::script_path [file dirname [info script]]

pd::guiproc slider2_draw_new {self c x y config state} {
    # import variables from dicts:
    foreach v {headsz width height fgcolor bgcolor orient} \
        {set $v [dict get $config -$v]}
    set x2 [expr {$x+$width+1}]
    set y2 [expr {$y+$height+1}]
    $c create rectangle $x $y $x2 $y2 \
        -outline $fgcolor -fill $bgcolor -tags [list $self border$self]
    switch $orient {
        horizontal {set y1 $y; set x3 [expr {$x+$headsz}]}
        vertical {set y1 [expr {$y2-$headsz}]; set x3 $x2}
    }
    $c create rectangle $x $y1 $x3 $y2 -outline {} -fill $fgcolor \
        -tags [list $self head$self]
    slider2_update $self $c $x $y $config $state
}

pd::guiproc slider2_update {self c x y config state} {
    # import variables from dicts:
    foreach v {initvalue headsz width height label labelpos lblcolor orient} \
        {set $v [dict get $config -$v]}
    foreach v {min max rev} {set $v [dict get $state _$v]}
    set realvalue [expr {1.0*($initvalue-$min)/($max-$min)}]
    if {$realvalue < 0.0} {set realvalue 0}
    if {$realvalue > 1.0} {set realvalue 1}
    if {$rev} {set realvalue [expr {1.0-$realvalue}]}
    if {$orient eq {vertical}} {set realvalue [expr {1.0-$realvalue}]}
    switch $orient {
        horizontal {
            set hr [expr {$width-$headsz}]
            $c coords head$self [expr {$x+$hr*$realvalue}] $y \
                [expr {$x+$hr*$realvalue+$headsz}] [expr {$y+$height+1}]
        }
        vertical {
            set vr [expr {$height-$headsz}]
            $c coords head$self $x [expr {$y+$vr*$realvalue}] \
                [expr {$x+$width+1}] [expr {$y+$vr*$realvalue+$headsz}]
        }
    }
    $c delete label$self
    if {$label ne {}} {
        switch $labelpos {
            top
            {set lx [expr {$x+$width/2}]; set ly [expr {$y}]; set a "s"}
            bottom
            {set lx [expr {$x+$width/2}]; set ly [expr {$y+$height+2}]; set a "n"}
            left
            {set lx [expr {$x}]; set ly [expr {$y+$height/2}]; set a "e"}
            right
            {set lx [expr {$x+$width+2}]; set ly [expr {$y+$height/2}]; set a "w"}
        }
        $c create text $lx $ly -anchor $a -text $label -fill $lblcolor \
             -tags [list $self label$self]
    }
}

proc+ slider2::constructor {self args} {
    set @canvas [canvas_getcurrent]
    pd::add_outlet $self float
    sys_gui "source {[file join $::script_path properties.tcl]}\n"
    # set defaults:
    set @config {
        -width 15 -height 130 -headsz 3 -rangebottom 0 -rangetop 127
        -init 0 -initvalue 0 -jumponclick 0 -label {} -labelpos {top}
        -orient {vertical} -sendsymbol {} -receivesymbol {}
        -fgcolor {#000000} -bgcolor {#ffffff} -lblcolor {#000000}
    }
    set @state {_min 0 _max 127 _rev 0}
    # expanded ($n) send/recv symbols:
    set @send {}
    set @recv {}
    0_config $self {*}$args
}

proc+ slider2::destructor {self} {
    if {[dict get $@config -receivesymbol] ne {}} {
        pd_unbind $self $@recv
    }
}

proc+ slider2::0_loadbang {self} {
    if {[dict get $@config -init]} {0_bang $self}
}

proc+ slider2::0_printconfig {self args} {
    if {[llength $args] == 0} {
        pd::post $@config
        return
    }
}

proc+ slider2::0_config2 {self args} {
    uplevel "0_config $self [string map {$ @} $args]"
}

proc+ slider2::0_config {self args} {
    pd::post [info level 0]
    set newconf [list]
    set optlist [pd::strip_selectors $args]
    set optlist [pd::strip_empty $optlist]
    set int_opts {-width -height -cellsize}
    set bool_opts {-init -jumponclick}
    set ui_opts {-fgcolor -bgcolor -lblcolor -orient -width -height}
    set upd_opts {-rangebottom -rangetop -label -labelpos}
    set conn_opts {-sendsymbol -receivesymbol}
    set ui 0
    set upd 0
    foreach {k v} $optlist {
        if {![dict exists $@config $k]} {
            return -code error "unknown option '$k'"
        }
        if {[dict get $@config $k] eq $v} {continue}
        if {[lsearch -exact $int_opts $k] != -1} {set v [expr {int($v)}]}
        if {[lsearch -exact $bool_opts $k] != -1} {set v [expr {int($v) != 0}]}
        if {[lsearch -exact $ui_opts $k] != -1} {set ui 1}
        if {[lsearch -exact $upd_opts $k] != -1} {set upd 1}
        dict set newconf $k $v
    }
    # process -{send,receive}symbol
    if {[dict exists $newconf -receivesymbol]} {
        set new_recv [dict get $newconf -receivesymbol]
        if {[dict get $@config -receivesymbol] ne {}} {
            pd_unbind $self $@recv
        }
        if {$new_recv ne {}} {
            set @recv [canvas_realizedollar $@canvas $new_recv]
            pd_bind $self $@recv
        } else {set @recv {}}
    }
    if {[dict exists $newconf -sendsymbol]} {
        set new_send [dict get $newconf -sendsymbol]
        if {$new_send ne {}} {
            set @send [canvas_realizedollar $@canvas $new_send]
        } else {set @send {}}
    }
    # changing orient -> swap sizes
    if {[dict exists $newconf -orient] && ![dict exists $newconf -width]
        && ![dict exists $newconf -height]} {
        dict set newconf -width [dict get $@config -height]
        dict set newconf -height [dict get $@config -width]
    }
    # no errors up to this point. we can safely merge options
    set @config [dict merge $@config $newconf]
    # adjust reverse range
    set a [dict get $@config -rangebottom]
    set b [dict get $@config -rangetop]
    dict set @state _min [expr {$a>$b?$b:$a}]
    dict set @state _max [expr {$a>$b?$a:$b}]
    dict set @state _rev [expr {$a>$b}]
    set orient [dict get $@config -orient]
    switch $orient {
        horizontal {set dim [dict get $@config -width];  set mul  1}
        vertical   {set dim [dict get $@config -height]; set mul -1}
        default {return -code error "invalid value '$orient' for -orient"}
    }
    # recompute pix2units conversion
    set @pix2units [expr {(2.0 * [dict get $@state _rev] - 1.0) *
        ( [dict get $@state _max] - [dict get $@state _min] ) *
        $mul / ( $dim - [dict get $@config -headsz])}]
    # if ui changed, update it
    if {$ui && [info exists @c]} {
        sys_gui [list $@c delete $self]\n
        sys_gui [list slider2_draw_new $self $@c $@x $@y $@config $@state]\n
    } elseif {$upd && [info exists @c]} {
        sys_gui [list slider2_update $self $@c $@x $@y $@config $@state]\n
    }
    if {[dict exists $newconf -width] || [dict exists $newconf -height]} {
        canvas_fixlinesfor $@canvas $self
    }
}

proc+ slider2::0_set {self args} {
    foreach v {min max} {set $v [dict get $@state _$v]}
    set f [pd::arg 0 float]
    if {$f < $min} {set f $min}
    if {$f > $max} {set f $max}
    dict set @config -initvalue $f
    if {[info exists @c]} {
        # update ui:
        sys_gui [list slider2_update $self $@c $@x $@y $@config $@state]\n
    }
}

proc+ slider2::0_bang {self} {
    foreach v {initvalue} {set $v [dict get $@config -$v]}
    pd::outlet $self 0 float $initvalue
    if {$@send ne {}} {
        set s_thing [$@send cget -s_thing]
        if {$s_thing ne {NULL}} {pd_float $s_thing $initvalue}
    }
}

proc+ slider2::0_float {self args} {
    0_set $self {*}$args
    0_bang $self
}

proc+ slider2::save {self} {
    set c $@config

    # use -sendsymbol and -receivesymbol from original binbuf, because of '$'
    set c2 [pd::strip_selectors [lrange [pd::get_binbuf $self] 1 end]]
    foreach opt {-sendsymbol -receivesymbol} {
        dict set c $opt [dict get $c2 $opt]
    }

    set l [list #X obj $@x $@y slider2 {*}[pd::add_empty $c] \;]
    return $l
}

proc+ slider2::properties {self} {
    set c $@config

    # use -sendsymbol and -receivesymbol from original binbuf, because of '$'
    set c2 [pd::strip_selectors [lrange [pd::get_binbuf $self] 1 end]]
    foreach opt {-sendsymbol -receivesymbol} {
        dict set c $opt [dict get $c2 $opt]
    }

    set c [string map {$ @} $c]
    gfxstub_new $self $self \
        [list propertieswindow %s $c "\[slider2\] properties"]\n
}

proc+ slider2::widgetbehavior_getrect {self args} {
    lassign $args x1 y1
    set x2 [expr {1+$x1+[dict get $@config -width]}]
    set y2 [expr {1+$y1+[dict get $@config -height]}]
    return [list $x1 $y1 $x2 $y2]
}

proc+ slider2::widgetbehavior_displace {self args} {
    lassign $args dx dy
    if {$dx != 0 || $dy != 0} {
        incr @x $dx; incr @y $dy
        sys_gui [list $@c move $self $dx $dy]\n
    }
    return [list $@x $@y]
}

proc+ slider2::widgetbehavior_select {self args} {
    lassign $args sel
    sys_gui [list $@c itemconfigure $self&&!label$self -outline [lindex \
        [list [dict get $@config -fgcolor] {blue}] $sel]]\n
}

proc+ slider2::widgetbehavior_vis {self args} {
    lassign $args @c @x @y vis
    if {$vis} {
        sys_gui [list slider2_draw_new $self $@c $@x $@y $@config $@state]\n
    } else {
        sys_gui [list $@c delete $self]\n
    }
}

proc+ slider2::widgetbehavior_click {self args} {
    lassign $args x y shift alt dbl doit
    set h [dict get $@config -height]
    set ypix [expr {[lindex $args 1]-$@y-1}]
    if {$ypix < 0 || $ypix >= $h} {return}
    if {$doit} {
        switch [dict get $@config -orient] {
            horizontal {
                set @motion_start_x $x
                set @motion_curr_x $x
            }
            vertical {
                set @motion_start_y $y
                set @motion_curr_y $y
            }
        }
        set @motion_start_v [dict get $@config -initvalue]
        tclpd_guiclass_grab $self $@canvas $x $y
    }
}

proc+ slider2::widgetbehavior_motion {self args} {
    lassign $args dx dy
    switch [dict get $@config -orient] {
        horizontal {
            set @motion_curr_x [expr {$dx+$@motion_curr_x}]
            set pixdelta [expr {-1*($@motion_curr_x-$@motion_start_x)}]
        }
        vertical {
            set @motion_curr_y [expr {$dy+$@motion_curr_y}]
            set pixdelta [expr {-1*($@motion_curr_y-$@motion_start_y)}]
        }
    }
    set f [expr {$@motion_start_v+$pixdelta*$@pix2units}]
    0_float $self {*}[pd::add_selectors [list $f]]
}

pd::guiclass slider2
