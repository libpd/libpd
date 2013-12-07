############ exciter procedures  -- ydegoyon@free.fr        #########

proc exciter_apply {id} {
# strip "." from the TK id to make a variable name suffix 
     set vid [string trimleft $id .]
# for each variable, make a local variable to hold its name...
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_nbevents [concat graph_nbevents_$vid]
     global $var_graph_nbevents
     set var_graph_timegrain [concat graph_timegrain_$vid]
     global $var_graph_timegrain
     set var_graph_loop [concat graph_loop_$vid]
     global $var_graph_loop
     set var_graph_save [concat graph_save_$vid]
     global $var_graph_save
 
     set cmd [concat $id dialog \
         [eval concat $$var_graph_width] \
         [eval concat $$var_graph_height] \
         [eval concat $$var_graph_nbevents] \
         [eval concat $$var_graph_timegrain] \
         [eval concat $$var_graph_loop] \
         [eval concat $$var_graph_save] \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc exciter_cancel {id} {
     set cmd [concat $id cancel \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc exciter_ok {id} {
     exciter_apply $id
     exciter_cancel $id
}
 
proc pdtk_exciter_dialog {id width height nbevents timegrain loop save } {
     set vid [string trimleft $id .]
 
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_nbevents [concat graph_nbevents_$vid]
     global $var_graph_nbevents
     set var_graph_timegrain [concat graph_timegrain_$vid]
     global $var_graph_timegrain
     set var_graph_loop [concat graph_loop_$vid]
     global $var_graph_loop
     set var_graph_save [concat graph_save_$vid]
     global $var_graph_save

     set $var_graph_width $width
     set $var_graph_height $height
     set $var_graph_nbevents $nbevents
     set $var_graph_timegrain $timegrain
     set $var_graph_loop $loop
     set $var_graph_save $save
 
     toplevel $id
     wm title $id {exciter}
     wm protocol $id WM_DELETE_WINDOW [concat exciter_cancel $id]
 
     label $id.label -text {EXCITER PROPERTIES}
     pack $id.label -side top
 
     frame $id.buttonframe
     pack $id.buttonframe -side bottom -fill x -pady 2m
     button $id.buttonframe.cancel -text {Cancel}\
     	-command "exciter_cancel $id"
     button $id.buttonframe.apply -text {Apply}\
     	-command "exciter_apply $id"
     button $id.buttonframe.ok -text {OK}\
     	-command "exciter_ok $id"
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
     label $id.3rangef.lnbevents -text "Nb Events :"
     entry $id.3rangef.nbevents -textvariable $var_graph_nbevents -width 7
     pack $id.3rangef.lnbevents $id.3rangef.nbevents -side left
 
     frame $id.4rangef
     pack $id.4rangef -side top
     label $id.4rangef.ltimegrain -text "Time Grain (seconds) :"
     entry $id.4rangef.timegrain -textvariable $var_graph_timegrain -width 7
     pack $id.4rangef.ltimegrain $id.4rangef.timegrain -side left
 
     checkbutton $id.loop -text {Loop} -variable $var_graph_loop \
        -anchor w
     pack $id.loop -side top  
 
     checkbutton $id.save -text {Save contents} -variable $var_graph_save \
        -anchor w
     pack $id.save -side top  
 
     bind $id.1rangef.width <KeyPress-Return> [concat exciter_ok $id]
     bind $id.2rangef.height <KeyPress-Return> [concat exciter_ok $id]
     bind $id.3rangef.nbevents <KeyPress-Return> [concat exciter_ok $id]
     bind $id.4rangef.timegrain <KeyPress-Return> [concat exciter_ok $id]
     focus $id.1rangef.width
}

############ exciter procedures END -- ydegoyon@free.fr     #########
