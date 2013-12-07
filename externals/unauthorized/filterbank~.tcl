############ filterbank procedures  -- ydegoyon@free.fr        #########

proc filterbank_apply {id} {
# strip "." from the TK id to make a variable name suffix 
     set vid [string trimleft $id .]
# for each variable, make a local variable to hold its name...
     set var_graph_lowfreq [concat graph_lowfreq_$vid]
     global $var_graph_lowfreq
     set var_graph_highfreq [concat graph_highfreq_$vid]
     global $var_graph_highfreq
 
     set cmd [concat $id dialog \
     	[eval concat $$var_graph_lowfreq] \
     	[eval concat $$var_graph_highfreq] \
 	\;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc filterbank_cancel {id} {
     set cmd [concat $id cancel \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc filterbank_ok {id} {
     filterbank_apply $id
     filterbank_cancel $id
}
 
proc pdtk_filterbank_dialog {id lowfreq highfreq} {
     set vid [string trimleft $id .]
     set var_graph_lowfreq [concat graph_lowfreq_$vid]
     global $var_graph_lowfreq
     set var_graph_highfreq [concat graph_highfreq_$vid]
     global $var_graph_highfreq
 
     set $var_graph_lowfreq $lowfreq
     set $var_graph_highfreq $highfreq
 
     toplevel $id
     wm title $id {filterbank}
     wm protocol $id WM_DELETE_WINDOW [concat filterbank_cancel $id]
 
     label $id.label -text {FILTERBANK PROPERTIES}
     pack $id.label -side top
 
     frame $id.buttonframe
     pack $id.buttonframe -side bottom -fill x -pady 2m
     button $id.buttonframe.cancel -text {Cancel}\
     	-command "filterbank_cancel $id"
     button $id.buttonframe.apply -text {Apply}\
     	-command "filterbank_apply $id"
     button $id.buttonframe.ok -text {OK}\
     	-command "filterbank_ok $id"
     pack $id.buttonframe.cancel -side left -expand 1
     pack $id.buttonframe.apply -side left -expand 1
     pack $id.buttonframe.ok -side left -expand 1
     
     frame $id.1rangef
     pack $id.1rangef -side top
     label $id.1rangef.llowfreq -text "Lower Frequency :"
     entry $id.1rangef.lowfreq -textvariable $var_graph_lowfreq -width 7
     pack $id.1rangef.llowfreq $id.1rangef.lowfreq -side left
 
     frame $id.2rangef
     pack $id.2rangef -side top
     label $id.2rangef.lhighfreq -text "Higher Frequency :"
     entry $id.2rangef.highfreq -textvariable $var_graph_highfreq -width 7
     pack $id.2rangef.lhighfreq $id.2rangef.highfreq -side left
 
     bind $id.1rangef.lowfreq <KeyPress-Return> [concat filterbank_ok $id]
     bind $id.2rangef.highfreq <KeyPress-Return> [concat filterbank_ok $id]
     focus $id.1rangef.lowfreq
}

############ filterbank procedures END -- ydegoyon@free.fr     #########
