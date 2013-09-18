############ pianoroll procedures  -- ydegoyon@free.fr        #########

proc pianoroll_apply {id} {
# strip "." from the TK id to make a variable name suffix 
     set vid [string trimleft $id .]
# for each variable, make a local variable to hold its name...
     set var_graph_name [concat graph_name_$vid]
     global $var_graph_name
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_pmin [concat graph_pmin_$vid]
     global $var_graph_pmin
     set var_graph_pmax [concat graph_pmax_$vid]
     global $var_graph_pmax
     set var_graph_nbgrades [concat graph_nbgrades_$vid]
     global $var_graph_nbgrades
     set var_graph_nbsteps [concat graph_nbsteps_$vid]
     global $var_graph_nbsteps
     set var_graph_defvalue [concat graph_defvalue_$vid]
     global $var_graph_defvalue
     set var_graph_save [concat graph_save_$vid]
     global $var_graph_save
 
     set cmd [concat $id dialog \
         [eval concat $$var_graph_name] \
         [eval concat $$var_graph_width] \
         [eval concat $$var_graph_height] \
         [eval concat $$var_graph_pmin] \
         [eval concat $$var_graph_pmax] \
         [eval concat $$var_graph_nbgrades] \
         [eval concat $$var_graph_nbsteps] \
         [eval concat $$var_graph_defvalue] \
         [eval concat $$var_graph_save] \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc pianoroll_cancel {id} {
     set cmd [concat $id cancel \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc pianoroll_ok {id} {
     pianoroll_apply $id
     pianoroll_cancel $id
}
 
proc pdtk_pianoroll_dialog {id name width height pmin pmax nbgrades nbsteps defvalue save } {
     set vid [string trimleft $id .]
     set var_graph_name [concat graph_name_$vid]
     global $var_graph_name
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_pmin [concat graph_pmin_$vid]
     global $var_graph_pmin
     set var_graph_pmax [concat graph_pmax_$vid]
     global $var_graph_pmax
     set var_graph_nbgrades [concat graph_nbgrades_$vid]
     global $var_graph_nbgrades
     set var_graph_nbsteps [concat graph_nbsteps_$vid]
     global $var_graph_nbsteps
     set var_graph_defvalue [concat graph_defvalue_$vid]
     global $var_graph_defvalue
     set var_graph_save [concat graph_save_$vid]
     global $var_graph_save
 
     set $var_graph_name $name
     set $var_graph_width $width
     set $var_graph_height $height
     set $var_graph_pmin $pmin
     set $var_graph_pmax $pmax
     set $var_graph_nbgrades $nbgrades
     set $var_graph_nbsteps $nbsteps
     set $var_graph_defvalue $defvalue
     set $var_graph_save $save
 
     toplevel $id
     wm title $id {pianoroll}
     wm protocol $id WM_DELETE_WINDOW [concat pianoroll_cancel $id]
 
     label $id.label -text {PIANOROLL PROPERTIES}
     pack $id.label -side top
 
     frame $id.buttonframe
     pack $id.buttonframe -side bottom -fill x -pady 2m
     button $id.buttonframe.cancel -text {Cancel}\
     	-command "pianoroll_cancel $id"
     button $id.buttonframe.apply -text {Apply}\
     	-command "pianoroll_apply $id"
     button $id.buttonframe.ok -text {OK}\
     	-command "pianoroll_ok $id"
     pack $id.buttonframe.cancel -side left -expand 1
     pack $id.buttonframe.apply -side left -expand 1
     pack $id.buttonframe.ok -side left -expand 1
     
     frame $id.1rangef
     pack $id.1rangef -side top
     label $id.1rangef.lname -text "Name :"
     entry $id.1rangef.name -textvariable $var_graph_name -width 15
     pack $id.1rangef.lname $id.1rangef.name -side left
 
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
 
     frame $id.4rangef
     pack $id.4rangef -side top
     label $id.4rangef.lpmin -text "Pitch low :"
     entry $id.4rangef.pmin -textvariable $var_graph_pmin -width 7
     pack $id.4rangef.lpmin $id.4rangef.pmin -side left
 
     frame $id.5rangef
     pack $id.5rangef -side top
     label $id.5rangef.lpmax -text "Pitch high :"
     entry $id.5rangef.pmax -textvariable $var_graph_pmax -width 7
     pack $id.5rangef.lpmax $id.5rangef.pmax -side left
 
     frame $id.6rangef
     pack $id.6rangef -side top
     label $id.6rangef.lnbgrades -text "Grades :"
     entry $id.6rangef.nbgrades -textvariable $var_graph_nbgrades -width 7
     pack $id.6rangef.lnbgrades $id.6rangef.nbgrades -side left
 
     frame $id.7rangef
     pack $id.7rangef -side top
     label $id.7rangef.lnbsteps -text "Steps :"
     entry $id.7rangef.nbsteps -textvariable $var_graph_nbsteps -width 7
     pack $id.7rangef.lnbsteps $id.7rangef.nbsteps -side left
 
     frame $id.8rangef
     pack $id.8rangef -side top
     label $id.8rangef.ldefvalue -text "Default Value :"
     entry $id.8rangef.defvalue -textvariable $var_graph_defvalue -width 7
     pack $id.8rangef.ldefvalue $id.8rangef.defvalue -side left

     checkbutton $id.save -text {Save contents} -variable $var_graph_save \
        -anchor w
     pack $id.save -side top  
 
     bind $id.1rangef.name <KeyPress-Return> [concat pianoroll_ok $id]
     bind $id.2rangef.width <KeyPress-Return> [concat pianoroll_ok $id]
     bind $id.3rangef.height <KeyPress-Return> [concat pianoroll_ok $id]
     bind $id.4rangef.pmin <KeyPress-Return> [concat pianoroll_ok $id]
     bind $id.5rangef.pmax <KeyPress-Return> [concat pianoroll_ok $id]
     bind $id.6rangef.nbgrades <KeyPress-Return> [concat pianoroll_ok $id]
     bind $id.7rangef.nbsteps <KeyPress-Return> [concat pianoroll_ok $id]
     bind $id.8rangef.defvalue <KeyPress-Return> [concat pianoroll_ok $id]
     focus $id.1rangef.name
}

############ pianoroll procedures END -- ydegoyon@free.fr     #########
