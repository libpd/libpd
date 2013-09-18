package require Tclpd 0.3.0
package require TclpdLib 0.20

proc+ list_change::constructor {self args} {
    # add second inlet (first created by default)
    pd::add_inlet $self list

    # add outlet
    pd::add_outlet $self list

    set @curlist {}
}

proc+ list_change::0_list {self args} {
    # HOT inlet
    if {$args ne $@curlist} {
        set @curlist $args
        pd::outlet $self 0 list $@curlist
    }
}

proc+ list_change::0_bang {self} {
    if {$@curlist eq {}} return
    pd::outlet $self 0 list $@curlist
}

proc+ list_change::1_list {self args} {
    # COLD inlet
    set @curlist $args
}

pd::class list_change
