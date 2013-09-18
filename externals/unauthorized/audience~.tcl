############ audience procedures  -- ydegoyon@free.fr        #########

proc audience_apply {id} {
# strip "." from the TK id to make a variable name suffix 
     set vid [string trimleft $id .]
# for each variable, make a local variable to hold its name...
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_nboutputs [concat graph_nboutputs_$vid]
     global $var_graph_nboutputs
 
     set cmd [concat $id dialog \
     	[eval concat $$var_graph_width] \
     	[eval concat $$var_graph_height] \
     	[eval concat $$var_graph_nboutputs] \
 	\;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc audience_cancel {id} {
     set cmd [concat $id cancel \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc audience_ok {id} {
     audience_apply $id
     audience_cancel $id
}
 
proc pdtk_audience_dialog {id width height nboutputs} {
     set vid [string trimleft $id .]
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_nboutputs [concat graph_nboutputs_$vid]
     global $var_graph_nboutputs
 
     set $var_graph_width $width
     set $var_graph_height $height
     set $var_graph_nboutputs $nboutputs
 
     toplevel $id
     wm title $id {audience}
     wm protocol $id WM_DELETE_WINDOW [concat audience_cancel $id]
 
     label $id.label -text {2$ SPACE PROPERTIES}
     pack $id.label -side top
 
     frame $id.buttonframe
     pack $id.buttonframe -side bottom -fill x -pady 2m
     button $id.buttonframe.cancel -text {Cancel}\
     	-command "audience_cancel $id"
     button $id.buttonframe.apply -text {Apply}\
     	-command "audience_apply $id"
     button $id.buttonframe.ok -text {OK}\
     	-command "audience_ok $id"
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
 
     frame $id.3rangef
     pack $id.3rangef -side top
     label $id.3rangef.lnboutputs -text "Nb Listeners :"
     entry $id.3rangef.nboutputs -textvariable $var_graph_nboutputs -width 7
     pack $id.3rangef.lnboutputs $id.3rangef.nboutputs -side left
 
     bind $id.1rangef.width <KeyPress-Return> [concat audience_ok $id]
     bind $id.2rangef.height <KeyPress-Return> [concat audience_ok $id]
     bind $id.3rangef.nboutputs <KeyPress-Return> [concat audience_ok $id]
     focus $id.1rangef.width
}

############ audience procedures END -- ydegoyon@free.fr     #########
