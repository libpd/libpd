package require Tclpd 0.3.0
package require TclpdLib 0.20

proc+ dynreceive::constructor {self args} {
    set @sym {}
    if {[pd::args] > 0} {
        set @sym [pd::arg 0 symbol]
        pd_bind $self $@sym
    }
    pd::add_outlet $self
}

proc+ dynreceive::destructor {self} {
    # don't forget to call pd_unbind, or sending things to a symbol
    # bound to dead object will crash pd!
    if {$@sym ne {}} {
        pd_unbind $self $@sym
    }
}

proc+ dynreceive::0_set {self args} {
    # send [set empty( to clear the receive symbol
    set s [pd::arg 0 symbol]
    if {$@sym eq {}} {
        pd_unbind $self $@sym
    }
    if {$s eq {empty}} {
        set @sym {}
    } else {
        set @sym $s
        pd_bind $self $@sym
    }
}

proc+ dynreceive::0_bang {self} {
    pd::outlet $self 0 bang
}

proc+ dynreceive::0_float {self args} {
    pd::outlet $self 0 float [pd::arg 0 float]
}

proc+ dynreceive::0_symbol {self args} {
    pd::outlet $self 0 symbol [pd::arg 0 symbol]
}

proc+ dynreceive::0_anything {self args} {
    set sel [pd::arg 0 symbol]
    set argz [lrange $args 1 end]
    pd::outlet $self 0 $sel $argz
}

pd::class dynreceive
