package require Tclpd 0.3.0
package require TclpdLib 0.20

proc basic_input_helper::constructor {self args} {
    pd::add_outlet $self list
}

proc basic_input_helper::0_float {self args} {
    pd::outlet $self 0 symbol float
}

proc basic_input_helper::0_symbol {self args} {
    pd::outlet $self 0 symbol symbol
}

proc basic_input_helper::0_list {self args} {
    pd::outlet $self 0 symbol list
}

proc basic_input_helper::0_anything {self args} {
    pd::outlet $self 0 symbol anything
}

pd::class basic_input_helper
