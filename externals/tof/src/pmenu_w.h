
#define UPDATE 0
#define CREATE 1
#define DESTROY 2

#define COLUMNBREAK 30

static void pmenu_w_activate(t_pmenu* x){
	//sys_vgui("$%xw activate %i\n", x,x->current_selection);
	//if ( x->indicator) {
		sys_vgui("set %xradio %i\n",x,x->current_selection); 
	//} else {
		
	 //  sys_vgui("set %xradio %i\n",x,-1); 
	//}
}


static void pmenu_w_clear(t_pmenu* x){
	sys_vgui("$%xw delete 0 end \n", x);
	//sys_vgui("set %xradio %i\n",x,-1); 
	x->current_selection = -1;
	pmenu_w_activate(x);
}

static void pmenu_w_additem(t_pmenu* x, t_symbol *label, int i) {
	if ( x->indicator) {
	sys_vgui("$%xw add radiobutton -label \"%s\" -command {select%x \"%d\"} -variable %xradio -value %d \n",
                   x, label->s_name, x, i,x,i);
                   
			   } else {
	sys_vgui("$%xw add command -label \"%s\" -command {select%x \"%d\"} \n",
                   x, label->s_name, x, i,x,i);
			   }
	if ( i == COLUMNBREAK ) sys_vgui("$%xw entryconfigure %i -columnbreak  true \n",x,i);
			   
}

static void pmenu_w_apply_colors(t_pmenu* x) {
	
	sys_vgui("$%xw configure -background \"%s\" -foreground \"%s\" -activeforeground \"%s\" -activebackground \"%s\" -selectcolor \"%s\"\n", x,
	x->bg_color->s_name,x->fg_color->s_name,x->fg_color->s_name,x->hi_color->s_name,x->fg_color->s_name);
	
	
}


static void pmenu_w_menu(t_pmenu *x, int draw)
{
  DEBUG(post("menu start");)
  
  if ( draw == CREATE ) {
	  //x->created = 1;
	   // Create menu 
	   //sys_vgui("set %xw .x%x.c.s%x ; menubutton $%xw -justify left 
	   // Create a variable to store a pointer to the menu, create the menu, create a variable to store the selected item
      sys_vgui("set %xw .%x ; menu $%xw -relief solid -tearoff 0; set %xradio -1 \n",x,x,x);
      
  } else if ( draw == DESTROY) {
	  //x->created = 0;
	
	 
	  sys_vgui("destroy $%xw \n",x);
	  
  }

  //DEBUG(post("id: .x%x.c.s%x", glist, x);)
  DEBUG(post("menu end");)
}


static void pmenu_w_pop(t_pmenu *x) {
	if (x->options_count > 0) {
			 //if ( x->created == 0 )  pmenu_w_menu(x, glist, CREATE);
			 if ( x->current_selection != -1 && x->focusing) {
				 sys_vgui("tk_popup $%xw [winfo pointerx .] [winfo pointery .] %i\n",x,x->current_selection);
			 } else {
				sys_vgui("tk_popup $%xw [winfo pointerx .] [winfo pointery .] \n",x);
		   	}
		 }
	
}



