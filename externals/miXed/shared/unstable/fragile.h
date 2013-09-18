/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __FRAGILE_H__
#define __FRAGILE_H__

t_symbol *fragile_class_getexterndir(t_class *c);
int fragile_class_count(void);
int fragile_class_getnames(t_atom *av, int maxnames);
void fragile_class_raise(t_symbol *cname, t_newmethod thiscall);
t_pd *fragile_class_mutate(t_symbol *cname, t_newmethod thiscall,
			   int ac, t_atom *av);
t_newmethod fragile_class_getalien(t_symbol *cname, t_newmethod thiscall,
				   t_atomtype **argtypesp);
t_pd *fragile_class_createobject(t_symbol *cname, t_newmethod callthis,
				 t_atomtype *argtypes, int ac, t_atom *av);
void fragile_class_printnames(char *msg, int firstndx, int lastndx);
t_glist *fragile_garray_glist(void *arr);
t_outconnect *fragile_outlet_connections(t_outlet *o);
t_outconnect *fragile_outlet_nextconnection(t_outconnect *last,
					    t_object **destp, int *innop);
t_object *fragile_outlet_destination(t_outlet *op,
				     int ntypes, t_symbol **types,
				     t_pd *caller, char *errand);
t_sample *fragile_inlet_signalscalar(t_inlet *i);

#endif
