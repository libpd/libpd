/*__________________________________________________________________________

	strtok     á      strtok c-string function wrapper
						
    Copyright (C) 2003 jan schacher
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
    
    initial build 20030507
    
____________________________________________________________________________*/

#include "m_pd.h"
#include <string.h>


typedef struct strtok 		
{
	t_object		ob;		
	void 			*s_outlet;	
	char 			s_tempstring[4096];
} t_strtok;

void *strtok_class;
void *strtok_new(t_symbol *s, long argc, t_atom *argv);
void strtok_free(t_strtok *x);
//void strtok_assist(t_strtok *x, void *b, long msg, long arg, char *dst);
void strtok_anything(t_strtok *x, t_symbol *s, long argc, t_atom *argv);
void strtok_set(t_strtok *x, t_symbol *s, long argc, t_atom *argv);

void strtok_setup(void)
{
	strtok_class = class_new(gensym("strtok"), (t_newmethod)strtok_new, 
		(t_method)strtok_free, sizeof(t_strtok), 0, A_GIMME, 0);
	class_addsymbol(strtok_class, (t_method)strtok_anything);
	class_addmethod(strtok_class, (t_method)strtok_set,		gensym("set"),	A_GIMME, 0);
	post(".    strtok    .    jasch    .    "__DATE__" ",0);
	
	
}

void *strtok_new(t_symbol *s, long argc, t_atom *argv)
{
	t_strtok	*x;
	x = (t_strtok *)pd_new(strtok_class);
	x->s_outlet = outlet_new(&x->ob, gensym("list"));
	if((argc >= 1)&&(argv[0].a_type == A_SYMBOL)){
		strcpy(x->s_tempstring, argv[0].a_w.w_symbol->s_name);
		// post("argument is %s", argv[0].a_w.w_symbol->s_name);
	}else{
		strcpy(x->s_tempstring, "/");
		// post("defualt-argument is %s", x->s_tempstring);
	}
	return (x);									
}

void strtok_anything(t_strtok *x, t_symbol *s, long argc, t_atom *argv)
{
	char 	*ptr;
	char	local[4096];
	long	status = 0;
	short	i, j;
	t_atom	result[256];
	t_atom  head;
	i = 0;
	
	strcpy(local, s->s_name);
  	ptr = strtok(local,x->s_tempstring);
  	while (ptr != NULL){
  	  	result[i].a_type = A_SYMBOL;
  	  	result[i].a_w.w_symbol = gensym(ptr);
	    ptr = strtok (NULL, x->s_tempstring);
	    i++;
  	}
  	j = i;
  	head.a_w.w_symbol = result[0].a_w.w_symbol;
  	for(j=0;j<i;j++) result[j] = result[j+1];
  	outlet_anything(x->s_outlet,head.a_w.w_symbol,i-1,result);
	return;
}

void strtok_set(t_strtok *x, t_symbol *s, long argc, t_atom *argv)
{
	switch (argv[0].a_type) {
		case A_FLOAT: error("wrong argument type for set");break;
		// case A_LONG:error("wrong argument type for set"); break;
		case A_SYMBOL: strcpy(x->s_tempstring, argv[0].a_w.w_symbol->s_name); break;
		}
}

void strtok_free(t_strtok *x)
{
		// notify_free((t_object *)x);
}


