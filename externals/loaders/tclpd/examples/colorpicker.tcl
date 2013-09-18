if {[info exists ::colorpicker::version]} {return}
namespace eval ::colorpicker {
    namespace export colorpicker
    # =========================================
    # colorpicker
    set version 0.1
    # (C) 2009 - Federico Ferri
    # mescalinum (at) gmail (dot) com
    #
    # Released under GPL-3 license:
    # http://www.gnu.org/licenses/gpl-3.0.html
    # =========================================
    package provide colorpicker $version

    variable presets {
        ffffff dfdfdf bbbbbb ffc7c6 ffe3c6
        feffc6 c6ffc7 c6feff c7c6ff e3c6ff
        9f9f9f 7c7c7c 606060 ff0400 ff8300
        faff00 00ff04 00faff 0400ff 9c00ff
        404040 202020 000000 551312 553512
        535512 0f4710 0e4345 131255 2f004d
    }

    proc colorpicker {w mode args} {
        variable {}
        set modes {switches hsv}
        if {[lsearch -exact $modes $mode] == -1} {
            error "bad mode: $mode. must be one of: $modes."
        }
        set ($w:mode) $mode
        set ($w:color) {#000000}
        set ($w:command) {}
        set ($w:textvar) {}
        frame $w
        init_$mode $w
        rename $w ::colorpicker::_$w
        interp alias {} $w {} ::colorpicker::dispatch $w
        if {$args ne {}} {uplevel 1 ::colorpicker::config $w $args}
        return $w
    }

    proc dispatch {w cmd args} {
        variable {}
        switch -glob -- $cmd {
            get {set ($w:color)}
            set {uplevel 1 [linsert $args 0 ::colorpicker::set_color_ext $w]}
            con* {uplevel 1 [linsert $args 0 ::colorpicker::config $w]}
            default {uplevel 1 [linsert $args 0 ::colorpicker::_$w $cmd]}
        }
    }

    proc config {w args} {
        variable {}
        set options {}
        set flag 0
        foreach {key value} $args {
            switch -glob -- $key {
                -com* {
                    set ($w:command) $value
                    set flag 1
                }
                -textvar* {
                    set ($w:textvar) $value
                    set flag 1
                }
                default { lappend options $key $value }
            }
        }
        if {!$flag || $options ne {}} {
            uplevel 1 [linsert $options 0 ::scrolledframe::_$w config]
        }
    }

    proc set_color_ext {w c} {
        # called by the widget public method
        variable {}
        set c [string tolower $c]
        if {![regexp {^#[0-9a-f]{6,6}$} $c]} {
            error "Invalid color: $c. Specify a color in the format #HHHHHH"
        }
        switch -exact -- $($w:mode) {
            switches {
                set_color $w $c
            }
            hsv {
                set r [expr 0x[string range $c 1 2]]
                set g [expr 0x[string range $c 3 4]]
                set b [expr 0x[string range $c 5 6]]
                set hsv [rgbToHsv $r $g $b]
                hsv_set $w h [lindex $hsv 0]
                hsv_set $w s [lindex $hsv 1]
                hsv_set $w v [lindex $hsv 2]
                set_color $w $c
            }
        }
    }

    proc set_color {w c} {
        # called internally in reaction to events
        variable {}
        set c [string tolower $c]
        set ($w:color) $c
        if {$($w:command) ne {}} {
            set cmd $($w:command)
            lappend cmd $c
            uplevel #0 $cmd
        }
        if {$($w:textvar) ne {}} {
            uplevel #0 [list set $($w:textvar) $c]
        }
        switch -exact -- $($w:mode) {
            switches {
                variable presets
                set q 0
                for {set row 0} {$row < 3} {incr row} {
                    for {set col 0} {$col < 10} {incr col} {
                        set b [expr {$c eq "#[lindex $presets $q]"}]
                        ${w}.r${row}c${col} configure \
                            -relief [lindex {raised sunken} $b]
                        incr q
                    }
                }
            }
            hsv {
            }
        }
    }

    proc mkColor {rgb} {
        set r [lindex $rgb 0]; set g [lindex $rgb 1]; set b [lindex $rgb 2]
        if {$r < 0} {set r 0} elseif {$r > 255} {set r 255}
        if {$g < 0} {set g 0} elseif {$g > 255} {set g 255}
        if {$b < 0} {set b 0} elseif {$b > 255} {set b 255}
        return #[format {%2.2x%2.2x%2.2x} $r $g $b]
    }

    proc rgbToHsv {r g b} {
        set sorted [lsort -real [list $r $g $b]]
        set temp [lindex $sorted 0]
        set v [lindex $sorted 2]
        set value $v
        set bottom [expr {$v-$temp}]
        if {$bottom == 0} {
            set hue 0
            set saturation 0
            set value $v
        } else {
            if {$v == $r} {
                set top [expr {$g-$b}]
                if {$g >= $b} {
                    set angle 0
                } else {
                    set angle 360
                }
            } elseif {$v == $g} {
                set top [expr {$b-$r}]
                set angle 120
            } elseif {$v == $b} {
                set top [expr {$r-$g}]
                set angle 240
            }
            set hue [expr {round(60*(double($top)/$bottom)+$angle)}]
        }
        if {$v == 0} {
            set saturation 0
        } else {
            set saturation [expr {round(255-255*(double($temp)/$v))}]
        }
        return [list $hue $saturation $value]
    }

    proc hsvToRgb {h s v} {
        set hi [expr {int(double($h)/60)%6}]
        set f [expr {double($h)/60-$hi}]
        set s [expr {double($s)/255}]
        set v [expr {double($v)/255}]
        set p [expr {double($v)*(1-$s)}]
        set q [expr {double($v)*(1-$f*$s)}]
        set t [expr {double($v)*(1-(1-$f)*$s)}]
        switch -- $hi {
            0 {set r $v; set g $t; set b $p}
            1 {set r $q; set g $v; set b $p}
            2 {set r $p; set g $v; set b $t}
            3 {set r $p; set g $q; set b $v}
            4 {set r $t; set g $p; set b $v}
            5 {set r $v; set g $p; set b $q}
            default {error "[lindex [info level 0] 0]: bad H value"}
        }
        set r [expr {round($r*255)}]
        set g [expr {round($g*255)}]
        set b [expr {round($b*255)}]
        return [list $r $g $b]
    }

    proc init_switches {w} {
        variable {}
        variable presets
        set q 0
        for {set row 0} {$row < 3} {incr row} {
            for {set col 0} {$col < 10} {incr col} {
                set c "#[lindex $presets $q]"
                set b [expr {$($w:color) eq $c}]
                grid [frame ${w}.r${row}c${col} -width 18 -height 16 \
                    -borderwidth 1 -relief [lindex {raised sunken} $b] \
                    -background $c -highlightthickness 0] \
                    -row $row -column $col
                bind ${w}.r${row}c${col} <ButtonPress-1> \
                    "[namespace current]::set_color $w $c"
                incr q
            }
        }
    }

    proc init_hsv {w} {
        variable colorhsv
        set colorhsv($w:h) 0
        set colorhsv($w:s) 255
        set colorhsv($w:v) 255
        grid [canvas ${w}.hue -width 130 -height 15 -borderwidth 1 \
            -relief sunken -highlightthickness 0] -column 0 -row 0
        grid [canvas ${w}.sat -width 130 -height 14 -borderwidth 1 \
            -relief sunken -highlightthickness 0] -column 0 -row 1
        grid [canvas ${w}.val -width 130 -height 14 -borderwidth 1 \
            -relief sunken -highlightthickness 0] -column 0 -row 2
        grid [canvas ${w}.test -width 46 -height 46 -borderwidth 1 \
            -relief sunken -highlightthickness 0 -background red] \
            -column 1 -row 0 -rowspan 3
        variable mh
        variable ms
        variable mv
        set mh($w) 0; set ms($w) 0; set mv($w) 0;
        set sh "[namespace current]::hsv_set $w h \[expr {%x*360.0/130.0}\]"
        set ss "[namespace current]::hsv_set $w s \[expr {%x*255.0/130.0}\]"
        set sv "[namespace current]::hsv_set $w v \[expr {%x*255.0/130.0}\]"
        bind ${w}.hue <ButtonPress-1> "set [namespace current]::mh($w) 1; $sh"
        bind ${w}.sat <ButtonPress-1> "set [namespace current]::ms($w) 1; $ss"
        bind ${w}.val <ButtonPress-1> "set [namespace current]::mv($w) 1; $sv"
        bind ${w}.hue <ButtonRelease-1> "set [namespace current]::mh($w) 0"
        bind ${w}.sat <ButtonRelease-1> "set [namespace current]::ms($w) 0"
        bind ${w}.val <ButtonRelease-1> "set [namespace current]::mv($w) 0"
        bind ${w}.hue <Motion> "if {\$[namespace current]::mh($w)} {$sh}"
        bind ${w}.sat <Motion> "if {\$[namespace current]::ms($w)} {$ss}"
        bind ${w}.val <Motion> "if {\$[namespace current]::mv($w)} {$sv}"
        for {set x 0} {$x < 130} {incr x 3} {
            set c [mkColor [hsvToRgb [expr {$x*360.0/130.0}] 255 255]]
            ${w}.hue create rectangle $x 0 [expr {4+$x}] 16 -fill $c -outline {}
        }
        hsv_regen $w $colorhsv($w:h)
    }

    proc hsv_regen {w hue} {
        ${w}.sat delete all
        ${w}.val delete all
        for {set x 0} {$x < 130} {incr x 3} {
            set x1 [expr {$x*255.0/130.0}]
            set c1 [mkColor [hsvToRgb $hue $x1 255]]
            set c2 [mkColor [hsvToRgb $hue 255 $x1]]
            ${w}.sat create rectangle $x 0 [expr {4+$x}] 16 \
            -fill $c1 -outline {}
            ${w}.val create rectangle $x 0 [expr {4+$x}] 16 \
            -fill $c2 -outline {}
        }
    }

    proc hsv_set {w what val} {
        variable colorhsv
        if {$what ne {h} && $what ne {s} && $what ne {v}} {return}
        set colorhsv($w:$what) $val
        if {$colorhsv($w:$what) < 0.0} {set colorhsv($w:$what) 0}
        if {$what eq {h}} {
            if {$colorhsv($w:$what) >= 360.0} {set colorhsv($w:$what) 0}
            hsv_regen $w $colorhsv($w:$what)
        } else {
            if {$colorhsv($w:$what) > 255.0} {set colorhsv($w:$what) 255}
        }
        set c [mkColor [hsvToRgb \
            $colorhsv($w:h) $colorhsv($w:s) $colorhsv($w:v)]]
        ${w}.test configure -background $c
        set_color $w $c
    }
}
