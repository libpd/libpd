############ probalizer procedures  -- ydegoyon@free.fr        #########

proc probalizer_apply {id} {
# strip "." from the TK id to make a variable name suffix 
     set vid [string trimleft $id .]
# for each variable, make a local variable to hold its name...
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_nvalues [concat graph_nvalues_$vid]
     global $var_graph_nvalues
     set var_graph_noccurrences [concat graph_noccurrences_$vid]
     global $var_graph_noccurrences
     set var_graph_save [concat graph_save_$vid]
     global $var_graph_save
 
     set cmd [concat $id dialog \
         [eval concat $$var_graph_width] \
         [eval concat $$var_graph_height] \
         [eval concat $$var_graph_nvalues] \
         [eval concat $$var_graph_noccurrences] \
         [eval concat $$var_graph_save] \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc probalizer_cancel {id} {
     set cmd [concat $id cancel \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc probalizer_ok {id} {
     probalizer_apply $id
     probalizer_cancel $id
}
 
proc pdtk_probalizer_dialog {id width height nvalues noccurrences save } {
     set vid [string trimleft $id .]
 
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_nvalues [concat graph_nvalues_$vid]
     global $var_graph_nvalues
     set var_graph_noccurrences [concat graph_noccurrences_$vid]
     global $var_graph_noccurrences
     set var_graph_save [concat graph_save_$vid]
     global $var_graph_save

     set $var_graph_width $width
     set $var_graph_height $height
     set $var_graph_nvalues $nvalues
     set $var_graph_noccurrences $noccurrences
     set $var_graph_save $save
 
     toplevel $id
     wm title $id {probalizer}
     wm protocol $id WM_DELETE_WINDOW [concat probalizer_cancel $id]
 
     label $id.label -text {PROBALIZER PROPERTIES}
     pack $id.label -side top
 
     frame $id.buttonframe
     pack $id.buttonframe -side bottom -fill x -pady 2m
     button $id.buttonframe.cancel -text {Cancel}\
     	-command "probalizer_cancel $id"
     button $id.buttonframe.apply -text {Apply}\
     	-command "probalizer_apply $id"
     button $id.buttonframe.ok -text {OK}\
     	-command "probalizer_ok $id"
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
     label $id.3rangef.lnvalues -text "Values :"
     entry $id.3rangef.nvalues -textvariable $var_graph_nvalues -width 7
     pack $id.3rangef.lnvalues $id.3rangef.nvalues -side left
 
     frame $id.4rangef
     pack $id.4rangef -side top
     label $id.4rangef.lnoccurrences -text "Max Occurrences :"
     entry $id.4rangef.noccurrences -textvariable $var_graph_noccurrences -width 7
     pack $id.4rangef.lnoccurrences $id.4rangef.noccurrences -side left
 
     checkbutton $id.save -text {Save contents} -variable $var_graph_save \
        -anchor w
     pack $id.save -side top  
 
     bind $id.1rangef.width <KeyPress-Return> [concat probalizer_ok $id]
     bind $id.2rangef.height <KeyPress-Return> [concat probalizer_ok $id]
     bind $id.3rangef.nvalues <KeyPress-Return> [concat probalizer_ok $id]
     bind $id.4rangef.noccurrences <KeyPress-Return> [concat probalizer_ok $id]
     focus $id.1rangef.width
}

############ probalizer procedures END -- ydegoyon@free.fr     #########
