############ scrolllist procedures  -- ydegoyon@free.fr        #########

proc scrolllist_apply {id} {
# strip "." from the TK id to make a variable name suffix 
     set vid [string trimleft $id .]
# for each variable, make a local variable to hold its name...
     set var_graph_capacity [concat graph_capacity_$vid]
     global $var_graph_capacity
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height$vid]
     global $var_graph_height
     set var_graph_font [concat graph_font$vid]
     global $var_graph_font
     set var_graph_bgcolor [concat graph_bgcolor$vid]
     global $var_graph_bgcolor
     set var_graph_fgcolor [concat graph_fgcolor$vid]
     global $var_graph_fgcolor
     set var_graph_secolor [concat graph_secolor$vid]
     global $var_graph_secolor
 
     set cmd [concat $id dialog \
     	[eval concat $$var_graph_capacity] \
     	[eval concat $$var_graph_width] \
     	[eval concat $$var_graph_height] \
     	[eval concat $$var_graph_font] \
     	[eval concat $$var_graph_bgcolor] \
     	[eval concat $$var_graph_fgcolor] \
     	[eval concat $$var_graph_secolor] \
 	\;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc scrolllist_cancel {id} {
     set cmd [concat $id cancel \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc scrolllist_ok {id} {
     scrolllist_apply $id
     scrolllist_cancel $id
}
 
proc pdtk_scrolllist_dialog {id capacity width height font bgcolor fgcolor secolor} {
     set vid [string trimleft $id .]
     set var_graph_capacity [concat graph_capacity_$vid]
     global $var_graph_capacity
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height$vid]
     global $var_graph_height
     set var_graph_font [concat graph_font$vid]
     global $var_graph_font
     set var_graph_bgcolor [concat graph_bgcolor$vid]
     global $var_graph_bgcolor
     set var_graph_fgcolor [concat graph_fgcolor$vid]
     global $var_graph_fgcolor
     set var_graph_secolor [concat graph_secolor$vid]
     global $var_graph_secolor
 
     set $var_graph_capacity $capacity
     set $var_graph_width   $width
     set $var_graph_height  $height
     set $var_graph_font    $font
     set $var_graph_bgcolor $bgcolor
     set $var_graph_fgcolor $fgcolor
     set $var_graph_secolor $secolor
 
     toplevel $id
     wm title $id {scrolllist}
     wm protocol $id WM_DELETE_WINDOW [concat scrolllist_cancel $id]
 
     label $id.label -text {SCROLLLIST PROPERTIES}
     pack $id.label -side top
 
     frame $id.buttonframe
     pack $id.buttonframe -side bottom -fill x -pady 2m
     button $id.buttonframe.cancel -text {Cancel}\
     	-command "scrolllist_cancel $id"
     button $id.buttonframe.apply -text {Apply}\
     	-command "scrolllist_apply $id"
     button $id.buttonframe.ok -text {OK}\
     	-command "scrolllist_ok $id"
     pack $id.buttonframe.cancel -side left -expand 1
     pack $id.buttonframe.apply -side left -expand 1
     pack $id.buttonframe.ok -side left -expand 1
     
     frame $id.1rangef
     pack $id.1rangef -side top
     label $id.1rangef.lcapacity -text "Capacity :"
     entry $id.1rangef.capacity -textvariable $var_graph_capacity -width 7
     pack $id.1rangef.lcapacity $id.1rangef.capacity -side left
 
     frame $id.2rangef
     pack $id.2rangef -side top
     label $id.2rangef.lwidth -text "Width :"
     entry $id.2rangef.width -textvariable $var_graph_width -width 7
     pack $id.2rangef.lwidth $id.2rangef.width -side left
 
     frame $id.3rangef
     pack $id.3rangef -side top
     label $id.3rangef.lheight -text "Height :"
     entry $id.3rangef.height -textvariable $var_graph_height -width 7
     pack $id.3rangef.lheight $id.3rangef.height -side left
 
     frame $id.3_5rangef
     pack $id.3_5rangef -side top
     label $id.3_5rangef.lfont -text "Font :"
     entry $id.3_5rangef.font -textvariable $var_graph_font -width 30
     pack $id.3_5rangef.lfont $id.3_5rangef.font -side left
 
     frame $id.4rangef
     pack $id.4rangef -side top
     label $id.4rangef.lbgcolor -text "Background Color :"
     entry $id.4rangef.bgcolor -textvariable $var_graph_bgcolor -width 7
     pack $id.4rangef.lbgcolor $id.4rangef.bgcolor -side left
 
     frame $id.5rangef
     pack $id.5rangef -side top
     label $id.5rangef.lfgcolor -text "Foreground Color :"
     entry $id.5rangef.fgcolor -textvariable $var_graph_fgcolor -width 7
     pack $id.5rangef.lfgcolor $id.5rangef.fgcolor -side left
 
     frame $id.6rangef
     pack $id.6rangef -side top
     label $id.6rangef.lsecolor -text "Selection Color :"
     entry $id.6rangef.secolor -textvariable $var_graph_secolor -width 7
     pack $id.6rangef.lsecolor $id.6rangef.secolor -side left
 
     bind $id.1rangef.capacity <KeyPress-Return> [concat scrolllist_ok $id]
     bind $id.2rangef.width   <KeyPress-Return> [concat scrolllist_ok $id]
     bind $id.3rangef.height  <KeyPress-Return> [concat scrolllist_ok $id]
     bind $id.3_5rangef.font  <KeyPress-Return> [concat scrolllist_ok $id]
     bind $id.4rangef.bgcolor <KeyPress-Return> [concat scrolllist_ok $id]
     bind $id.5rangef.fgcolor <KeyPress-Return> [concat scrolllist_ok $id]
     bind $id.6rangef.secolor <KeyPress-Return> [concat scrolllist_ok $id]
}

############ scrolllist procedures END -- ydegoyon@free.fr     #########
