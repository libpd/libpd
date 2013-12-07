package require Tclpd 0.2.3
package require TclpdLib 0.19

# utilities for getting informations about tclpd's interpreter

proc tclpd-interp-info::constructor {self} {
}

proc tclpd-interp-info::0_bang {self} {
    pd::post "-------- namespaces and procs: -------------"
    set nss [linsert [namespace children ::] 0 ::]
    foreach ns $nss {
        pd::post "<NS>  $ns"
        set procs [info procs ${ns}::*]
        foreach p $procs {
            pd::post "<PROC>    $p"
        }
    }
}

pd::class tclpd-interp-info
