package require Tclpd 0.3.0
package require TclpdLib 0.20

package require base64

pd::guiproc ::tclpd_console_exec {cmd} {
    if {$cmd eq {}} {return}

    global tclpd_console_hist
    global tclpd_console_histp

    if {$cmd ne [lindex $tclpd_console_hist end]} {
        lappend tclpd_console_hist $cmd
        set tclpd_console_histp [expr {[llength $tclpd_console_hist]-1}]
    }
    .pdwindow.tcl.tclpd.entry delete 0 end

    # encode message in base64 to prevent escaping and other FUDI annoyances
    set cmd [::base64::encode $cmd]

    set max_line_length 1024
    while {$cmd ne {}} {
        set line [string range $cmd 0 [expr $max_line_length - 1]]
        set cmd [string range $cmd $max_line_length end]
        ::pdsend "$::tclpd_console base64data $line"
    }
    ::pdsend "$::tclpd_console base64data -"
}

pd::guiproc ::tclpd_console_history {dir} {
    global tclpd_console_hist
    global tclpd_console_histp

    incr tclpd_console_histp $dir
    set l [llength $tclpd_console_hist]
    if {$tclpd_console_histp < 0} {set tclpd_console_histp 0}
    if {$tclpd_console_histp >= $l} {set tclpd_console_histp [expr {$l-1}]}

    .pdwindow.tcl.tclpd.entry delete 0 end
    .pdwindow.tcl.tclpd.entry insert 0 \
        [lindex $tclpd_console_hist $tclpd_console_histp]
}

proc tclpd-console::constructor {self} {
    if {[info exist ::tclpd-console::loaded]} {
        return -code error "only one instance of tclpd-console allowed"
    }

    set ::tclpd-console::loaded 1
    set ::${self}_loaded 1

    # beware: typemap magic (1st arg get cast to a t_pd, second to a t_symbol)
    pd_bind $self $self

    sys_gui "set ::tclpd_console $self"
    sys_gui {
        set ::tclpd_console_hist {}
        set ::tclpd_console_histp {}
        package require base64
        set w .pdwindow.tcl.tclpd
        frame $w -borderwidth 0
        pack $w -side bottom -fill x
        label $w.label -text [_ "tclpd: "] -anchor e
        pack $w.label -side left
        entry $w.entry -width 200 \
            -exportselection 1 -insertwidth 2 -insertbackground blue \
            -textvariable ::tclpd_cmd -font {$::font_family 12}
        pack $w.entry -side left -fill x
        bind $w.entry <$::modifier-Key-a> "%W selection range 0 end; break"
        bind $w.entry <Return> {::tclpd_console_exec $::tclpd_cmd}
        set bgrule {[lindex {#FFF0F0 #FFFFFF} [info complete $::tclpd_cmd]]}
        bind $w.entry <KeyRelease> "$w.entry configure -background $bgrule"
        bind $w.entry <Up> "::tclpd_console_history -1"
        bind $w.entry <Down> "::tclpd_console_history 1"
        bind .pdwindow.text <Key-Tab> "focus $w.entry; break"
        after idle .pdwindow.text.internal yview end
    }

    # make puts print into pdwindow
    if {[info procs puts_tclpd_console] eq {}} {
        rename puts puts_tclpd_console
        proc ::puts {args} {
            if {[llength $args] == 1} {
                uplevel "pd::post $args"
            } else {
                uplevel "puts_tclpd_console $args"
            }
        }
    }
}

proc tclpd-console::destructor {self} {
    if {[set ::${self}_loaded]} {
        sys_gui { destroy .pdwindow.tcl.tclpd ; unset ::tclpd_console }

        pd_unbind $self $self

        # restore original puts
        if {[info procs puts_tclpd_console] ne {}} {
            rename puts_tclpd_console puts
        }
    }

    unset ::tclpd-console::loaded
    unset ::${self}_loaded
}

proc tclpd-console::0_base64data {self data} {
    if {[llength $data] != 2 || [lindex $data 0] ne {symbol}} {
        return -code error "malformed arguments: $data"
    }

    global tclpd_console_buf

    set data [lindex $data 1]
    set op [string index $data 0]

    if {$op eq "-"} {
        set cmd [::base64::decode $tclpd_console_buf]
        set tclpd_console_buf {}
        pd::post [concat % $cmd]
        set result [uplevel #0 $cmd]
        if {$result ne {}} {pd::post $result}
        #sys_gui "tk_messageBox -message {Result:\n$result}"
    } else {
        append tclpd_console_buf $data
    }
}

pd::class tclpd-console -noinlet 1
