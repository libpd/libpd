############ scratcher procedures  -- ydegoyon@free.fr        #########

proc scratcher_apply {id} {
# strip "." from the TK id to make a variable name suffix 
     set vid [string trimleft $id .]
# for each variable, make a local variable to hold its name...
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
 
     set cmd [concat $id dialog \
     	[eval concat $$var_graph_width] \
     	[eval concat $$var_graph_height] \
 	\;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc scratcher_cancel {id} {
     set cmd [concat $id cancel \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc scratcher_ok {id} {
     scratcher_apply $id
     scratcher_cancel $id
}
 
proc pdtk_scratcher_dialog {id width height} {
     set vid [string trimleft $id .]
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
 
     set $var_graph_width $width
     set $var_graph_height $height
 
     toplevel $id
     wm title $id {scratcher}
     wm protocol $id WM_DELETE_WINDOW [concat scratcher_cancel $id]
 
     label $id.label -text {SCRATCHER PROPERTIES}
     pack $id.label -side top
 
     frame $id.buttonframe
     pack $id.buttonframe -side bottom -fill x -pady 2m
     button $id.buttonframe.cancel -text {Cancel}\
     	-command "scratcher_cancel $id"
     button $id.buttonframe.apply -text {Apply}\
     	-command "scratcher_apply $id"
     button $id.buttonframe.ok -text {OK}\
     	-command "scratcher_ok $id"
     pack $id.buttonframe.cancel -side left -expand 1
     pack $id.buttonframe.apply -side left -expand 1
     pack $id.buttonframe.ok -side left -expand 1
     
     frame $id.1rangef
     pack $id.1rangef -side top
     label $id.1rangef.lwidth -text "Width :"
     entry $id.1rangef.width -textvariable $var_graph_width -width 7
     pack $id.1rangef.lwidth $id.1rangef.width -side left
 
     frame $id.2rangef
     pack $id.2rangef -side top
     label $id.2rangef.lheight -text "Height :"
     entry $id.2rangef.height -textvariable $var_graph_height -width 7
     pack $id.2rangef.lheight $id.2rangef.height -side left
 
     bind $id.1rangef.name <KeyPress-Return> [concat scratcher_ok $id]
     bind $id.2rangef.height <KeyPress-Return> [concat scratcher_ok $id]
     focus $id.1rangef.name
}

############ scratcher procedures END -- ydegoyon@free.fr     #########
