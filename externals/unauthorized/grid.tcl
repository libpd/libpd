############ grid procedures  -- ydegoyon@free.fr        #########

proc grid_apply {id} {
# strip "." from the TK id to make a variable name suffix 
     set vid [string trimleft $id .]
# for each variable, make a local variable to hold its name...
     set var_graph_name [concat graph_name_$vid]
     global $var_graph_name
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_xmin [concat graph_xmin_$vid]
     global $var_graph_xmin
     set var_graph_xmax [concat graph_xmax_$vid]
     global $var_graph_xmax
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_ymin [concat graph_ymin_$vid]
     global $var_graph_ymin
     set var_graph_ymax [concat graph_ymax_$vid]
     global $var_graph_ymax
     set var_graph_grid [concat graph_grid_$vid]
     global $var_graph_grid
     set var_graph_xstep [concat graph_xstep_$vid]
     global $var_graph_xstep
     set var_graph_ystep [concat graph_ystep_$vid]
     global $var_graph_ystep
     set var_graph_xlines [concat graph_xlines_$vid]
     global $var_graph_xlines
     set var_graph_ylines [concat graph_ylines_$vid]
     global $var_graph_ylines
 
     set cmd [concat $id dialog \
     	[eval concat $$var_graph_name] \
     	[eval concat $$var_graph_width] \
     	[eval concat $$var_graph_xmin] \
     	[eval concat $$var_graph_xmax] \
     	[eval concat $$var_graph_height] \
     	[eval concat $$var_graph_ymin] \
     	[eval concat $$var_graph_ymax] \
     	[eval concat $$var_graph_grid] \
     	[eval concat $$var_graph_xstep] \
     	[eval concat $$var_graph_ystep] \
     	[eval concat $$var_graph_xlines] \
     	[eval concat $$var_graph_ylines] \
 	\;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc grid_cancel {id} {
     set cmd [concat $id cancel \;]
     #puts stderr $cmd
     pdsend $cmd
}
 
proc grid_ok {id} {
     grid_apply $id
     grid_cancel $id
}
 
proc pdtk_grid_dialog {id name width xmin xmax height ymin ymax grid xstep ystep xlines ylines} {
     set vid [string trimleft $id .]
     set var_graph_name [concat graph_name_$vid]
     global $var_graph_name
     set var_graph_width [concat graph_width_$vid]
     global $var_graph_width
     set var_graph_xmin [concat graph_xmin_$vid]
     global $var_graph_xmin
     set var_graph_xmax [concat graph_xmax_$vid]
     global $var_graph_xmax
     set var_graph_height [concat graph_height_$vid]
     global $var_graph_height
     set var_graph_ymin [concat graph_ymin_$vid]
     global $var_graph_ymin
     set var_graph_ymax [concat graph_ymax_$vid]
     global $var_graph_ymax
     set var_graph_grid [concat graph_grid_$vid]
     global $var_graph_grid
     set var_graph_xstep [concat graph_xstep_$vid]
     global $var_graph_xstep
     set var_graph_ystep [concat graph_ystep_$vid]
     global $var_graph_ystep
     set var_graph_xlines [concat graph_xlines_$vid]
     global $var_graph_xlines
     set var_graph_ylines [concat graph_ylines_$vid]
     global $var_graph_ylines
 
     set $var_graph_name $name
     set $var_graph_width $width
     set $var_graph_xmin $xmin
     set $var_graph_xmax $xmax
     set $var_graph_height $height
     set $var_graph_ymin $ymin
     set $var_graph_ymax $ymax
     set $var_graph_grid $grid
     set $var_graph_xstep $xstep
     set $var_graph_ystep $ystep
     set $var_graph_xlines $xlines
     set $var_graph_ylines $ylines
 
     toplevel $id
     wm title $id {grid}
     wm protocol $id WM_DELETE_WINDOW [concat grid_cancel $id]
 
     label $id.label -text {GRID PROPERTIES}
     pack $id.label -side top
 
     frame $id.buttonframe
     pack $id.buttonframe -side bottom -fill x -pady 2m
     button $id.buttonframe.cancel -text {Cancel}\
     	-command "grid_cancel $id"
     button $id.buttonframe.apply -text {Apply}\
     	-command "grid_apply $id"
     button $id.buttonframe.ok -text {OK}\
     	-command "grid_ok $id"
     pack $id.buttonframe.cancel -side left -expand 1
     pack $id.buttonframe.apply -side left -expand 1
     pack $id.buttonframe.ok -side left -expand 1
     
     frame $id.1rangef
     pack $id.1rangef -side top
     label $id.1rangef.lname -text "Name :"
     entry $id.1rangef.name -textvariable $var_graph_name -width 7
     pack $id.1rangef.lname $id.1rangef.name -side left
 
     frame $id.2rangef
     pack $id.2rangef -side top
     label $id.2rangef.lwidth -text "Width :"
     entry $id.2rangef.width -textvariable $var_graph_width -width 7
     pack $id.2rangef.lwidth $id.2rangef.width -side left
 
     frame $id.3rangef
     pack $id.3rangef -side top
     label $id.3rangef.lxmin -text "X min :"
     entry $id.3rangef.xmin -textvariable $var_graph_xmin -width 7
     pack $id.3rangef.lxmin $id.3rangef.xmin -side left
 
     frame $id.4rangef
     pack $id.4rangef -side top
     label $id.4rangef.lxmax -text "X max :"
     entry $id.4rangef.xmax -textvariable $var_graph_xmax -width 7
     pack $id.4rangef.lxmax $id.4rangef.xmax -side left
 
     frame $id.41rangef
     pack $id.41rangef -side top
     label $id.41rangef.lxstep -text "X step :"
     entry $id.41rangef.xstep -textvariable $var_graph_xstep -width 7
     pack $id.41rangef.lxstep $id.41rangef.xstep -side left
 
     frame $id.42rangef
     pack $id.42rangef -side top
     label $id.42rangef.lxlines -text "X sections :"
     entry $id.42rangef.xlines -textvariable $var_graph_xlines -width 7
     pack $id.42rangef.lxlines $id.42rangef.xlines -side left
 
     frame $id.5rangef
     pack $id.5rangef -side top
     label $id.5rangef.lheight -text "Height :"
     entry $id.5rangef.height -textvariable $var_graph_height -width 7
     pack $id.5rangef.lheight $id.5rangef.height -side left
 
     frame $id.6rangef
     pack $id.6rangef -side top
     label $id.6rangef.lymin -text "Y min :"
     entry $id.6rangef.ymin -textvariable $var_graph_ymin -width 7
     pack $id.6rangef.lymin $id.6rangef.ymin -side left
 
     frame $id.7rangef
     pack $id.7rangef -side top
     label $id.7rangef.lymax -text "Y max :"
     entry $id.7rangef.ymax -textvariable $var_graph_ymax -width 7
     pack $id.7rangef.lymax $id.7rangef.ymax -side left
 
     frame $id.71rangef
     pack $id.71rangef -side top
     label $id.71rangef.lystep -text "Y step :"
     entry $id.71rangef.ystep -textvariable $var_graph_ystep -width 7
     pack $id.71rangef.lystep $id.71rangef.ystep -side left
 
     frame $id.72rangef
     pack $id.72rangef -side top
     label $id.72rangef.lylines -text "Y sections :"
     entry $id.72rangef.ylines -textvariable $var_graph_ylines -width 7
     pack $id.72rangef.lylines $id.72rangef.ylines -side left
 
     checkbutton $id.showgrid -text {Show Grid} -variable $var_graph_grid \
     	-anchor w
     pack $id.showgrid -side top
 
     bind $id.1rangef.name <KeyPress-Return> [concat grid_ok $id]
     bind $id.2rangef.width <KeyPress-Return> [concat grid_ok $id]
     bind $id.3rangef.xmin <KeyPress-Return> [concat grid_ok $id]
     bind $id.4rangef.xmax <KeyPress-Return> [concat grid_ok $id]
     bind $id.41rangef.xstep <KeyPress-Return> [concat grid_ok $id]
     bind $id.42rangef.xlines <KeyPress-Return> [concat grid_ok $id]
     bind $id.5rangef.height <KeyPress-Return> [concat grid_ok $id]
     bind $id.6rangef.ymin <KeyPress-Return> [concat grid_ok $id]
     bind $id.7rangef.ymax <KeyPress-Return> [concat grid_ok $id]
     bind $id.71rangef.ystep <KeyPress-Return> [concat grid_ok $id]
     bind $id.72rangef.ylines <KeyPress-Return> [concat grid_ok $id]
     focus $id.1rangef.name
}

############ grid procedures END -- ydegoyon@free.fr     #########
