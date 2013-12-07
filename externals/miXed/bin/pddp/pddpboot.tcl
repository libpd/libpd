#!/bin/sh
# \
exec tclsh "$0" -- "$@"

# Synopsis
#   test run:
#     ./pddpboot.tcl [root [port [path]]]
#   from Pd:
#     source pddpboot.tcl
#     ::pddp::srvUse root (or ::pddp::srvStart root [port])
#     ::pddp::cliOpen path
#     ... (more "::pddp::cliOpen" calls) ...
#     ::pddp::srvStop

if {[namespace exists ::pddp]} {  ;# created by pddplink's setup
    puts stderr "Booting pddp"
    set ::pddp::testrun 0
} else {
    puts stderr "Booting pddp, test run..."
    namespace eval ::pddp { variable testrun 1 }
}

if {[info exists ::pddp::theDir]} {
    source [file join $::pddp::theDir pddpclient.tcl]
    source [file join $::pddp::theDir pddpserver.tcl]
    if {[info exists ::pddp::theVersion]} {
	package provide pddp $::pddp::theVersion
    }
} else {
    source pddpclient.tcl]
    source pddpserver.tcl]
}
