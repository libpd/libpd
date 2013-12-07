/* some test stuff */

#if 0

    {
	t_pdp_list *l = pdp_list_from_cstring("(een 2 3. (1 1 1 1 1))", 0);
	t_pdp_list *s = pdp_list_from_cstring("(symbol int float (int int ...))", 0);
	//PDP_ASSERT(0);
	pdp_list_print(l);
	pdp_list_print(s);`
	post("%d", pdp_tree_check_syntax(l, s));
	exit(1);
    }
#endif

#if 0

    {
	char *c = "(test 1 2 (23 4)) ( 1 [asdflkj; las;dlfkj;a sdf]) (een (zes (ze)ven ())) [";
	while (*c){
	    t_pdp_list *l = pdp_list_from_cstring(c, &c);
	    if (l) pdp_list_print(l); 
	    else{
		post("parse error: remaining input: %s", c); break;
	    }
	}
	exit(1);
    }
#endif
