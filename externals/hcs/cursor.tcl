
namespace eval ::hcs::cursor:: {
    variable continue_pollmotion 0
    variable last_x 0
    variable last_y 0
    variable receive_symbol
}

# idea from #tcl for a Tcl unbind
proc ::hcs::cursor::unbind {tag event script} {
    set bind {}
    foreach x [split [bind $tag $event] \"\n\"] {
        if {$x != $script} {
            lappend bind $x
        }
    }
    bind $tag $event {}
    foreach x $bind {bind $tag $event $x}
}

proc ::hcs::cursor::button {button state} {
    variable receive_symbol
    pdsend "$receive_symbol button $button $state"
}

proc ::hcs::cursor::mousewheel {delta} {
    variable receive_symbol
    pdsend "$receive_symbol mousewheel $delta"
} 

proc ::hcs::cursor::motion {x y} {
    variable last_x
    variable last_y
    variable receive_symbol
    if { $x != $last_x || $y != $last_y} {
        pdsend "$receive_symbol motion $x $y"
        set last_x $x
        set last_y $y
    }
}

proc ::hcs::cursor::pollmotion {} {
    variable continue_pollmotion
    motion [winfo pointerx .] [winfo pointery .]
    if {$continue_pollmotion != 0} { 
        after 10 ::hcs::cursor::pollmotion
    }
}

proc ::hcs::cursor::startpolling {} {
    variable continue_pollmotion 1
    pollmotion 
    bind all <ButtonPress> {+::hcs::cursor::button %b 1}
    bind all <ButtonRelease> {+::hcs::cursor::button %b 0}
    bind all <MouseWheel> {+::hcs::cursor::mousewheel %D}
}

proc ::hcs::cursor::stoppolling {} {
    variable continue_pollmotion 0 
    unbind all <ButtonPress> {::hcs::cursor::button %b 1}
    unbind all <ButtonRelease> {::hcs::cursor::button %b 0}
    unbind all <MouseWheel> {::hcs::cursor::mousewheel %D}
}

# in Pd 0.43, the internal proc changed from 'pd' to 'pdsend'
proc ::hcs::cursor::setup {symbol} {
    variable receive_symbol $symbol
    # check if we are Pd < 0.43, which has no 'pdsend', but a 'pd' coded in C
    if {[llength [info procs "::pdsend"]] == 0} {
        pdtk_post "creating 0.43+ 'pdsend' using legacy 'pd' proc"
        proc ::pdsend {args} {pd "[join $args { }] ;"}
    }
}
