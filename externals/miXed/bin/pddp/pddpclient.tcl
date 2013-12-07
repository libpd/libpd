# pddpclient.tcl

# Synopsis
#   not to be run by itself (see pddpboot.tcl)

if {![namespace exists ::pddp]} {
    puts stderr "Error: invalid invocation of pddpclient (boot pddp first)"
    puts stderr "exiting..."
    exit 1
}

if {$::pddp::testrun} {  ;# true if sourced from standalone "pddpboot.tcl"
    puts stderr "Loading pddpclient, test run..."
    if {$argc > 3} {
	set path [lindex $argv 3]
	if {[string length $path]} {
	    puts stderr "Scheduling \"$path\" for opening"
	    after idle ::pddp::cliOpen $path
	}
	unset path
    }
} else {
    puts stderr "Loading pddpclient"
}

namespace eval ::pddp {
    variable theBrowserCommand

    switch -- $::tcl_platform(platform) {
	unix {
	    switch -- $tcl_platform(os) {
		Darwin {
		    set theBrowserCommand "sh -c \"open %s\""
		}
		Linux {
		    foreach candidate \
			{gnome-open xdg-open sensible-browser firefox mozilla galeon konqueror netscape lynx} {
			set browser [lindex [auto_execok $candidate] 0]
			if {[string length $browser]} {
			    set theBrowserCommand "$browser %s &"
			    break
			}
		    }
		}
	    }
	}
	windows {
	    # should not this be just: [auto_execok start]?
	    set theBrowserCommand \
		"rundll32 url.dll,FileProtocolHandler file:%s &"
	}
    }
}

proc ::pddp::cliError {err} {
    puts stderr "Error in pddpclient: $err"
}

proc ::pddp::cliOpen {path} {
    if {[string first "://" $path] < 1} {
	if {[info exists ::pddp::thePort]} {
	    set path "http://localhost:$::pddp::thePort/$path"
	} else {
	    cliError "pddpserver not running"
	    return
	}
    }
    variable theBrowserCommand
    if {[string length $theBrowserCommand]} {
	set command [format $theBrowserCommand $path]
	puts stderr "pddpclient: exec $command"
	if {[catch {eval [list exec] $command} err]} {
	    if {[lindex $::errorCode 0] eq "CHILDSTATUS"} {
		cliError "$err (child status [lindex $::errorCode 2])"
	    } else {
		cliError $err
	    }
	}
    } else {
	cliError "browser unavailable"
    }
}
