# pddpserver.tcl

# Synopsis
#   not to be run by itself (see pddpboot.tcl)

# based on:

# Simple Sample httpd/1.[01] server
# Stephen Uhler (c) 1996-1997 Sun Microsystems

# http://cvs.sourceforge.net/viewcvs.py/tclhttpd/tclhttpd/bin/mini/mini1.1.tcl

# modified by krzYszcz (2005):
#   putting per-server data and all commands in a namespace "::pddp"
#   supporting sourcing from within Pd, through the "pddpboot.tcl" wrapper
#   inserting the .pd handler
#   lots of other changes, too many to list here (run "diff" if curious...)

if {![namespace exists ::pddp]} {
    puts stderr "Error: invalid invocation of pddpserver (boot pddp first)"
    puts stderr "exiting..."
    exit 1
}

if {$::pddp::testrun} {  ;# true if sourced from standalone "pddpboot.tcl"
    puts stderr "Loading pddpserver, test run..."
    proc bgerror {msg} {
	global errorInfo
	puts stderr "bgerror: $msg\n$errorInfo"
    }
} else {
    puts stderr "Loading pddpserver"
#    catch {console show}
}

namespace eval ::pddp {
    variable thePort 0
    variable theState
    variable theMimeTypes
    variable theErrors
    variable theErrorFormat

    # "theState" contains the server state:
    #  root:      the root of the document directory
    #  default:   default document name
    #  listen:    the main listening socket id
    #  naccepts:  a count of accepted connections so far
    #  maxtime:   the max time (msec) allowed to complete an http request
    #  maxused:   the max # of requests for a socket
    array set theState {
	root       ""
	default    index.html
	listen     ""
	naccepts   0
	nrequests  0
	nerrors    0
	maxtime	   600000
	maxused	   25
	bufsize	   32768
    }

    set theState(root)  $env(HOME)

    array set theMimeTypes {
	{}      text/plain
	.txt    text/plain
	.html   text/html
	.gif    image/gif
	.jpg    image/jpeg
	.pd     text/html
    }

    # HTTP/1.[01] error codes (the ones we use)
    array set theErrors {
	204 {No Content}
	400 {Bad Request}
	404 {Not Found}
	405 {Method Not Allowed}
	408 {Request Timeout}
	411 {Length Required}
	419 {Expectation Failed}
	500 {Internal Server Error}
	503 {Service Unavailable}
	504 {Service Temporarily Unavailable}
	505 {HTTP Version Not Supported}
    }

    # Generic error response
    set theErrorFormat {
	<title>Error: %1$s</title>
	Got the error: <b>%2$s</b><br>
	while trying to obtain <b>%3$s</b>
    }
}

proc ::pddp::srvUse {{root {}} {port 0}} {
    variable theState
    if {[string length $theState(listen)]} {
	if {[string length $root] && ![string equal $root $theState(root)]} {
	    srvLog $theState(listen) Warning "Redirection attempt for $root"
	}
    } else {
	srvStart $root $port
    }
}

# Start the server by listening for connections on the desired port.

proc ::pddp::srvStart {{root {}} {port 0}} {
    variable thePort
    variable theState

    puts stderr "Starting pddp server on [info hostname]"
    if {[string length $root]} {
	set theState(root) $root
    }
    # we do not handle multiple pddpservers, LATER rethink
    srvStop
    array set theState [list naccepts 0 nrequests 0 nerrors 0]

    for { set thePort $port } {$thePort < 65535 } {incr thePort } {
	if {[catch {set theState(listen) \
			[socket -server ::pddp::srvAccept $thePort]} res]} {
	    if {$thePort == 0} {
		# FIXME this is a critical error
		set thePort 32768
	    }
        } else { break }
    }
    if {$thePort == 65535} {
	srvLog none Error "Could not find port available for listening"
    } else {
	if {$thePort == 0} {
	    set thePort [lindex [fconfigure $theState(listen) -sockname] 2]
	}
	srvLog $theState(listen) Port $thePort
	srvLog $theState(listen) Root directory \"$root\"
    }
    after 120 update  ;# FIXME might be needed on windows they say, test there
    return $thePort
}

proc ::pddp::srvStop {} {
    variable thePort
    variable theState
    if {[string length $theState(listen)]} {
	if {[catch {close $theState(listen)} res]} {
	    srvLog $theState(listen) Warning [list $res while closing socket]
	} else {
	    srvLog $theState(listen) Closed.
	}
	set theState(listen) ""
	update
    }
}

# Accept a new connection from the server and set up a handler
# to read the request from the client.

proc ::pddp::srvAccept {sock ipaddr port} {
    variable theState
    variable theSockData$sock
    # reject remote requests, LATER revisit
    if {[string equal $ipaddr "127.0.0.1"]} {
	incr theState(naccepts)
	srvReset $sock $theState(maxused)
	srvLog $sock Connect $ipaddr $port
    } else {
	srvLog $sock Warning "rejecting remote connection request from $ipaddr"
	srvSockDone $sock 1
    }
}

# Initialize or reset the socket state

proc ::pddp::srvReset {sock nlft} {
    variable theState
    upvar 0 ::pddp::theSockData$sock sockData
    array set sockData [list state start linemode 1 version 0 nleft $nlft]
    set sockData(cancel) \
	[after $theState(maxtime) [list srvTimeout $sock]]
    fconfigure $sock -blocking 0 -buffersize $theState(bufsize) \
	-translation {auto crlf}
    fileevent $sock readable [list ::pddp::srvRead $sock]
}

# Read data from a client request
# 1) read the request line
# 2) read the mime headers
# 3) read the additional data (if post && content-length not satisfied)

proc ::pddp::srvRead {sock} {
    variable theState
    upvar 0 ::pddp::theSockData$sock sockData

    # Use line mode to read the request and the mime headers

    if {$sockData(linemode)} {
	set readCount [gets $sock line]
	set state [string compare $readCount 0],$sockData(state)
	switch -glob -- $state {
	    1,start {
		if {[regexp {(HEAD|POST|GET) ([^?]+)\??([^ ]*) HTTP/1.([01])} \
			 $line x sockData(proto) sockData(url) \
			 sockData(query) sockData(version)]} {
		    set sockData(state) mime
		    incr theState(nrequests)
		    srvLog $sock Request $sockData(nleft) $line
		} else {
		    srvError $sock 400 $line
		}
	    }
	    0,start {
		srvLog $sock Warning "Initial blank line fetching request"
	    }
	    1,mime {
		if {[regexp {([^:]+):[ 	]*(.*)}  $line {} key value]} {
		    set key [string tolower $key]
		    set sockData(key) $key
		    if {[info exists sockData(mime,$key)]} {
			append sockData(mime,$key) ", $value"
		    } else {
			set sockData(mime,$key) $value
		    }
		} elseif {[regexp {^[ 	]+(.+)} $line {} value] && \
			      [info exists sockData(key)]} {
		    append sockData(mime,$sockData($key)) " " $value
		} else {
		    srvError $sock 400 $line
		}
	    }
	    0,mime {
	        if {$sockData(proto) == "POST" && \
	        	[info exists sockData(mime,content-length)]} {
		    set sockData(linemode) 0
	            set sockData(count) $sockData(mime,content-length)
	            if {$sockData(version) && \
			    [info exists sockData(mime,expect)]} {
			if {$sockData(mime,expect) == "100-continue"} {
			    puts $sock "100 Continue HTTP/1.1\n"
			    flush $sock
			} else {
			    srvError $sock 419 $sockData(mime,expect)
			}
		    }
		    fconfigure $sock -translation {binary crlf}
	        } elseif {$sockData(proto) != "POST"}  {
		    srvRespond $sock
	        } else {
		    srvError $sock 411 "Confusing mime headers"
	        }
	    }
	    -1,* {
	    	if {[eof $sock]} {
		    srvLog $sock Error "Broken connection fetching request"
		    srvSockDone $sock 1
	    	} else {
	    	    puts stderr "Partial read, retrying"
	    	}
	    }
	    default {
		srvError $sock 404 "Invalid http state: $state,[eof $sock]"
	    }
	}

    # Use counted mode to get the post data

    } elseif {![eof $sock]} {
        append sockData(postdata) [read $sock $sockData(count)]
        set sockData(count) [expr {$sockData(mime,content-length) - \
				       [string length $sockData(postdata)]}]
        if {$sockData(count) == 0} {
	    srvRespond $sock
	}
    } else {
	srvLog $sock Error "Broken connection reading POST data"
	srvSockDone $sock 1
    }
}

# Done with the socket, either close it, or set up for next fetch
#  sock:     The socket I'm done with
#  doclose:  If true, close the socket, otherwise set up for reuse

proc ::pddp::srvSockDone {sock doclose} {
    variable theState
    upvar 0 ::pddp::theSockData$sock sockData

    after cancel $sockData(cancel)
    set nleft [incr sockData(nleft) -1]
    unset sockData
    if {$doclose} {
	close $sock
    } else {
	srvReset $sock $nleft
    }
    return ""
}

# A timeout happened

proc ::pddp::srvTimeout {sock} {
    srvError $sock 408
}

proc ::pddp::srvPdOpen {path} {
    global menu_windowlist
    set name [file tail $path]
    set dir [file dirname $path]
    # FIXME white space in $name and $dir
    # FIXME this is a fragile hack, there should be an "openx" message to pd...
    foreach en $menu_windowlist {
	set wd [lindex $en 1]
	set nm [lindex $en 0]
	set dr [lindex [wm title $wd] end]
	if {[string equal $name $nm] && [string equal $dir $dr]} {
	    # FIXME test on windows
	    raise $wd
	    focus -force $wd
	    return
	}
    }
    pd [concat pd open $name $dir \;]
    # FIXME raise and focus on windows?
}

proc ::pddp::srvPdHandler {sock path} {
    if {[catch {::pddp::srvPdOpen $path}]} {
	srvError $sock 504
    } else {
	srvError $sock 204
    }
}

# Handle file system queries.  This is a place holder for a more
# generic dispatch mechanism.

proc ::pddp::srvRespond {sock} {
    variable theState
    variable theUrlCache
    upvar 0 ::pddp::theSockData$sock sockData

    regsub {(^http://[^/]+)?} $sockData(url) {} url
    if {[info exists theUrlCache($url)]} {
    	set mypath $theUrlCache($url)
    } else {
	set mypath [srvUrl2File $theState(root) $url]
	if {[file isdirectory $mypath]} {
	    append mypath / $theState(default)
	}
	set theUrlCache($url) $mypath
    }
    if {[string length $mypath] == 0} {
	srvError $sock 400
    } elseif {![file readable $mypath]} {
	if {[string equal [file tail $mypath] "favicon.ico"]} {
	    srvError $sock 204  ;# FIXME design something
	} else {
	    srvError $sock 404 $mypath
	}
    } else {
	set ext [file extension $mypath]

	if {[string equal $ext ".pd"]} {
	    srvPdHandler $sock $mypath
	    return
	}

	puts $sock "HTTP/1.$sockData(version) 200 Data follows"
	puts $sock "Date: [srvGetDate [clock seconds]]"
	puts $sock "Last-Modified: [srvGetDate [file mtime $mypath]]"
	puts $sock "Content-Type: [srvContentType $ext]"
	puts $sock "Content-Length: [file size $mypath]"

	## Should also close socket if recvd connection close header
	set doclose [expr {$sockData(nleft) == 0}]

	if {$doclose} {
	    puts $sock "Connection close:"
	} elseif {$sockData(version) == 0 && \
		      [info exists sockData(mime,connection)]} {
	    if {$sockData(mime,connection) == "Keep-Alive"} {
	        set doclose 0
	        puts $sock "Connection: Keep-Alive"
	    }
	}
	puts $sock ""
	flush $sock

	if {$sockData(proto) != "HEAD"} {
	    set in [open $mypath]
	    fconfigure $sock -translation binary
	    fconfigure $in -translation binary
	    fcopy $in $sock -command \
		[list ::pddp::srvCopyDone $in $sock $doclose]
	} else {
	    srvSockDone $sock $doclose
	}
    }
}

# Callback when file is done being output to client
#  in:       The fd for the file being copied
#  sock:     The client socket
#  doclose:  close the socket if true
#  bytes:    The # of bytes copied
#  error:    The error message (if any)

proc ::pddp::srvCopyDone {in sock doclose bytes {error {}}} {
    close $in
    srvLog $sock Done $bytes bytes
    srvSockDone $sock $doclose
}

# Convert the file suffix into a mime type.

proc ::pddp::srvContentType {ext} {
    variable theMimeTypes
    set type text/plain
    catch {set type $theMimeTypes($ext)}
    return $type
}

# Respond with an error reply
# sock:  The socket handle to the client
# code:  The httpd error code
# args:  Additional information for error logging

proc ::pddp::srvError {sock code args} {
    variable theState
    variable theErrors
    variable theErrorFormat
    upvar 0 ::pddp::theSockData$sock sockData

    append sockData(url) ""
    incr theState(nerrors)
    set message [format $theErrorFormat $code $theErrors($code) $sockData(url)]
    append head "HTTP/1.$sockData(version) $code $theErrors($code)"  \n
    append head "Date: [srvGetDate [clock seconds]]"  \n
    append head "Connection: close"  \n
    append head "Content-Length: [string length $message]"  \n

    # Because there is an error condition, the socket may be "dead"

    catch {
	fconfigure $sock  -translation crlf
	puts -nonewline $sock $head\n$message
	flush $sock
    } reason
    srvSockDone $sock 1
    if {$code < 300} {set status Status} else {set status Error}
    srvLog $sock $status $code $theErrors($code) $args $reason
}

# Generate a date string in HTTP format.

proc ::pddp::srvGetDate {seconds} {
    return [clock format $seconds -format {%a, %d %b %Y %T %Z}]
}

# Log an Httpd transaction.
# This should be replaced as needed.

proc ::pddp::srvLog {sock args} {
    puts stderr "pddp log ($sock): $args"
}

# Convert a url into a pathname. (UNIX version only)
# This is probably not right, and belongs somewhere else.
# - Remove leading http://... if any
# - Collapse all /./ and /../ constructs
# - expand %xx sequences -> disallow "/"'s  and "."'s due to expansions

proc ::pddp::srvUrl2File {root url} {
    regsub -all {//+} $url / url		;# collapse multiple /'s
    while {[regsub -all {/\./} $url / url]} {}	;# collapse /./
    while {[regsub -all {/\.\.(/|$)} $url /\x81\\1 url]} {} ;# mark /../
    while {[regsub "/\[^/\x81]+/\x81/" $url / url]} {} ;# collapse /../
    if {![regexp "\x81|%2\[eEfF]" $url]} {	;# invalid /../, / or . ?
	return $root[srvCgiMap $url]
    } else {
	return ""
    }
}

# Decode url-encoded strings.

proc ::pddp::srvCgiMap {data} {
    regsub -all {([][$\\])} $data {\\\1} data
    regsub -all {%([0-9a-fA-F][0-9a-fA-F])} $data  {[format %c 0x\1]} data
    return [subst $data]
}

if {$::pddp::testrun} {  ;# true if tested as a standalone script
    if {$argc > 1} {
	set root [lindex $argv 1]
	set port [lindex $argv 2]
	if {![string is integer -strict $port]} {
	    set port 32768
	}
    } else {
	set root $env(HOME)
	set port 32768
    }
    ::pddp::srvStart $root $port
    vwait forever
}
