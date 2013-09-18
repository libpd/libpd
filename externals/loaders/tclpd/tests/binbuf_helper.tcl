package require Tclpd 0.3.0
package require TclpdLib 0.20

proc binbuf_helper::constructor {self args} {
    pd::add_outlet $self list
}

proc binbuf_helper::0_bang {self} {
    set binbuf [pd::get_binbuf $self]
    foreach atom $binbuf {
        foreach {atomtype atomvalue} $atom break
        pd::outlet $self 0 list [list [list symbol atomtype] [list symbol $atomtype]]
        pd::outlet $self 0 list [list [list symbol atomvalue] [list symbol $atomvalue]]
    }
}

pd::class binbuf_helper
