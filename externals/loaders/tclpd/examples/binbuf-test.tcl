package require Tclpd 0.3.0
package require TclpdLib 0.20

proc binbuf-test::constructor {self args} {
    pd::add_outlet $self list
}

proc binbuf-test::0_bang {self} {
    pd::outlet $self 0 list [pd::get_binbuf $self]
}

pd::class binbuf-test
