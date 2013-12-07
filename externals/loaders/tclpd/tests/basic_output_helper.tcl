package require Tclpd 0.3.0
package require TclpdLib 0.20

proc basic_output_helper::constructor {self args} {
    pd::add_outlet $self list
}

proc basic_output_helper::0_symbol {self args} {
    switch -exact -- [pd::arg 0 symbol] {
        float {
            pd::outlet $self 0 float 123
        }
        symbol {
            pd::outlet $self 0 symbol baz
        }
        list {
            pd::outlet $self 0 list {{symbol foo} {float 123} {symbol bar}}
        }
    }
}

pd::class basic_output_helper
