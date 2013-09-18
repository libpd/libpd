/*  listmoses - separate a list according to its contents' values
 *  Copyright (C) 2005 Edward Kelly <morph_2016@yahoo.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "m_pd.h"

static t_class *listmoses_class;

typedef struct _listmoses
{
  t_object x_obj;
  t_atom lowlist[1024];
  t_atom midlist[1024];
  t_atom highlist[1024];
  t_atom lowamps[1024];
  t_atom midamps[1024];
  t_atom highamps[1024];
  t_atom ampslist[1024];
  t_float lowsplit, highsplit;
  t_int low_argc, mid_argc, high_argc;
  t_outlet *low_list, *mid_list, *high_list, *low_buddy, *mid_buddy, *high_buddy;
} t_listmoses;

void listmoses_list(t_listmoses *x, t_symbol *s, int argc, t_atom *argv)
{
  float temp;
  if (x->highsplit < x->lowsplit)
    {
	temp = x->highsplit;
	x->highsplit = x->lowsplit;
	x->lowsplit = temp;
	}
  float current, curamps;
  int i;
  x->low_argc = 0;
  x->mid_argc = 0;
  x->high_argc = 0;
  for (i=0; i<argc; i++)
    {
      current = atom_getfloat(argv+i);
      curamps = atom_getfloatarg(i,argc,x->ampslist);
      if (current < x->lowsplit)
	{
	  SETFLOAT(&x->lowlist[x->low_argc], current);
	  SETFLOAT(&x->lowamps[x->low_argc], curamps);
	  x->low_argc++;
	}
      else if (current >= x->lowsplit && current < x->highsplit)
	{
	  SETFLOAT(&x->midlist[x->mid_argc], current);
	  SETFLOAT(&x->midamps[x->mid_argc], curamps);
	  x->mid_argc++;
	}
      else
	{
	  SETFLOAT(&x->highlist[x->high_argc], current);
	  SETFLOAT(&x->highamps[x->high_argc], curamps);
	  x->high_argc++;
	}
    }
  outlet_list(x->high_buddy, gensym("list"), x->high_argc, x->highamps);
  outlet_list(x->mid_buddy, gensym("list"), x->mid_argc, x->midamps);
  outlet_list(x->low_buddy, gensym("list"), x->low_argc, x->lowamps);
  outlet_list(x->high_list, gensym("list"), x->high_argc, x->highlist);
  outlet_list(x->mid_list, gensym("list"), x->mid_argc, x->midlist);
  outlet_list(x->low_list, gensym("list"), x->low_argc, x->lowlist);
}

void listmoses_amps(t_listmoses *x, t_symbol *s, int argc, t_atom *argv)
{
  float curamp;
  int i;
  for (i=0; i<argc; i++)
    {
      curamp = atom_getfloat(argv+i);
      SETFLOAT (&x->ampslist[i], curamp);
    }
}

void listmoses_bang(t_listmoses *x)
{
  outlet_list(x->high_buddy, gensym("list"), x->high_argc, x->highamps);
  outlet_list(x->mid_buddy, gensym("list"), x->mid_argc, x->midamps);
  outlet_list(x->low_buddy, gensym("list"), x->low_argc, x->lowamps);
  outlet_list(x->high_list, gensym("list"), x->high_argc, x->highlist);
  outlet_list(x->mid_list, gensym("list"), x->mid_argc, x->midlist);
  outlet_list(x->low_list, gensym("list"), x->low_argc, x->lowlist);
}

void *listmoses_new(t_symbol *s, int argc, t_atom *argv)
{
  t_listmoses *x = (t_listmoses *)pd_new(listmoses_class);
  x->highsplit = 96;
  x->lowsplit = 36;
/*  switch(argc)
    {
      default:
      case 2:
      x->highsplit = atom_getfloat(argv+1);
      //x->lowsplit = atom_getfloat(argv);
      case 1:
      //x->highsplit = atom_getfloat(argv);
      x->lowsplit = atom_getfloat(argv);
      break;
      case 0:
    } */ // I don't know why it doesn't work with args! 
	
  x->low_argc = 0;
  x->mid_argc = 0;
  x->high_argc = 0;

  x->low_list = outlet_new(&x->x_obj, gensym("list"));
  x->mid_list = outlet_new(&x->x_obj, gensym("list"));
  x->high_list = outlet_new(&x->x_obj, gensym("list"));
  x->low_buddy = outlet_new(&x->x_obj, gensym("list"));
  x->mid_buddy = outlet_new(&x->x_obj, gensym("list"));
  x->high_buddy = outlet_new(&x->x_obj, gensym("list"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym("amps"));
  floatinlet_new(&x->x_obj, &x->lowsplit);
  floatinlet_new(&x->x_obj, &x->highsplit);
  return (void *)x;
}

void listmoses_setup(void) {
  listmoses_class = class_new(gensym("listmoses"),
  (t_newmethod)listmoses_new,
  0, sizeof(t_listmoses),
  0, A_DEFFLOAT, 0);
  post("|<<<<<<<<<<<<<<<<<<<<<listmoses>>>>>>>>>>>>>>>>>>>>>>|");
  post("|<<split two lists according to values of the first>>|");
  post("|<<<<<<<<<<<<edward-------kelly------2005>>>>>>>>>>>>|");

  class_addbang(listmoses_class, listmoses_bang);
  class_addlist(listmoses_class, listmoses_list);
  class_addmethod(listmoses_class, (t_method)listmoses_amps, gensym("amps"), A_GIMME, 0);
}
