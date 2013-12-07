
pddp
----

This library was started to provide useful objects for documentation, and its
name comes from the Pure Data Documentation Project.  The pddplink object is
derived from the pddplink object written by Kzrysztof Czaja in 
externals/miXed/pddp/pddplink.c.  That version also includes a web server.


[pddplink] ChangeLog
-----------------

alpha3
  * .pd back-link tries not to open patches that are already open:
    if a patch window already exists, it is raised and given focus instead
  * new option "-gop" (visible in a gop)
  * bug fix: unclickable in gop, unless visible

alpha2
  * loading tcl scripts through "package require pddp":
    . storing them in an immediate subdirectory of the path of pddplink's binary
    . storing a hand-crafted pkgIndex.tcl there
    . pddplink's setup appends the path of its binary to tcl's "auto_path"
  * pddpserver asks for auto-assigning its port first, then starts
    incrementing from 32768
  * pddplink's appearance controlled with creation options (an option switch
    is any symbol starting from '-' followed by a letter)
  * options currently recognized:
    . "-box" (standard object box)
    . "-text" followed by any number of non-option atoms (body text)
  * nonboxed version (default) has a custom widgetbehavior, which is a thin
    layer on top of the standard text widgetbehavior (using all rtext routines),
    so that merging into core Pd, as a new object type, T_LINK, would be easy.

alpha1
  * prototype versions of the external "pddplink" and two pd-gui extensions
    "pddpserver.tcl", "pddpclient.tcl"
