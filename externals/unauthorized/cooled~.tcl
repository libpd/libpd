############ cooled procedures  -- ydegoyon@free.fr        #########

proc cooled_apply {id} {
# strip "." from the TK id to make a variable name suffix 
     set vid [string trimleft $id .]
# for each variable, make a local variable to hold its name...
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_draw [concat graph_draw_$vid]
     global $var_graph_draw
 
     set cmd [concat $id dialog \
     	[eval concat $$var_graph_width] \
     	[eval concat $$var_graph_height] \
     	[eval concat $$var_graph_draw] \
 	\;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc cooled_cancel {id} {
     set cmd [concat $id cancel \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc cooled_ok {id} {
     cooled_apply $id
     cooled_cancel $id
}
 
proc pdtk_cooled_dialog {id width height draw} {
     set vid [string trimleft $id .]
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_draw [concat graph_draw_$vid]
     global $var_graph_draw
 
     set $var_graph_width $width
     set $var_graph_height $height
     set $var_graph_draw $draw
 
     toplevel $id
     wm title $id {cooled}
     wm protocol $id WM_DELETE_WINDOW [concat cooled_cancel $id]
 
     label $id.label -text {EDITOR PROPERTIES}
     pack $id.label -side top
 
     frame $id.buttonframe
     pack $id.buttonframe -side bottom -fill x -pady 2m
     button $id.buttonframe.cancel -text {Cancel}\
     	-command "cooled_cancel $id"
     button $id.buttonframe.apply -text {Apply}\
     	-command "cooled_apply $id"
     button $id.buttonframe.ok -text {OK}\
     	-command "cooled_ok $id"
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

     checkbutton $id.draw -text {Draw Sample} -variable $var_graph_draw \
        -anchor w
     pack $id.draw -side top
 
     bind $id.1rangef.width <KeyPress-Return> [concat cooled_ok $id]
     bind $id.2rangef.height <KeyPress-Return> [concat cooled_ok $id]
     focus $id.1rangef.width
}

############ cooled procedures END -- ydegoyon@free.fr     #########
