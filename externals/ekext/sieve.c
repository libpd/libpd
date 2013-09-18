/* takes a map like 0 1 3 4 7 and only returns the number if it is present */
/* in the map, or returns the closest, or the next up or down (wrapped)*/
#include "m_pd.h"
#include <math.h>
#include <string.h>
#define MAXENTRIES 2048
#define LASTENTRY 2047

static t_class *sieve_class;

/* mode = 0 : block when absent, 1: nearest when absent, 2: shunt when absent */
typedef struct _map
{
  t_atom map[MAXENTRIES];
  t_atom nomap[MAXENTRIES];
} t_map;

typedef struct _sieve
{
  t_object x_obj;
  t_map x_map;
  t_float input, mode, max, outmap;
  t_outlet *mapped, *value, *mapout, *inst;
} t_sieve;

void sieve_float(t_sieve *x, t_floatarg fin)
{
  int i, ip, in, arg, arga, argb, argaout, argbout, argxa, argxb, itest, itesta, itestb, iresult;
  itest = itesta = itestb = iresult = arga = argb = arg = 0;
  float test, testa, testb, fresult;
  test = testa = testb = fresult = 0;
  x->input = arg = fin;
  if (x->mode == 0) /* only let through floats when the corresponding 
                       index contains != 0 */
    {
      test = fin < 0 ? 0 : atom_getfloatarg(arg, MAXENTRIES, x->x_map.map);
      if(test!=0) 
	{
	  outlet_bang(x->inst);
	  outlet_float(x->value, test);
	  outlet_float(x->mapped, arg);
	}
    }
  else if (x->mode == 1) /* find the nearest float whose (int) index is
                            != 0 */
    {
      test =  fin < 0 ? 0 : atom_getfloatarg(arg, MAXENTRIES, x->x_map.map);
      if(test!=0)
	{
	  outlet_bang(x->inst);
	  outlet_float(x->value, test);
	  outlet_float(x->mapped, arg);
	}
      else
	{
	  arga = argb = arg;
	  while(itest == 0 && (arga > -1 || argb < MAXENTRIES))
	    {
	      arga--;
	      argb++;
	      argxa = arga >= 0 ? arga : 0;
	      argxb = argb <= LASTENTRY ? argb : LASTENTRY;
	      testa = atom_getfloatarg(argxa, MAXENTRIES, x->x_map.map);
	      testb = atom_getfloatarg(argxb, MAXENTRIES, x->x_map.map);
	      itesta = testa != 0 ? 1 : 0;
	      itestb = testb != 0 ? 1 : 0;
	      itest =  fin < 0 ? 0 : itesta + itestb;
	    }
	  switch(itest)
	    {
	    case 2: /* if we find two at equal distance, output the higher */
	      if (x->mode == 1)
		{
		  outlet_float(x->value, testb);
		  outlet_float(x->mapped, argb);
		}
	      else
		{
		  outlet_float(x->value, testa);
		  outlet_float(x->mapped, arga);
		}
	    case 1:
	      iresult = itesta == 1 ? arga : argb;
	      fresult = itesta == 1 ? testa : testb;
	      outlet_float(x->value, fresult);
	      outlet_float(x->mapped, iresult);
	    case 0:
	      break;
	    }
	}
    }
  else if (x->mode==2) /* if the index is 0, find the next highest */
    {
      itest = 0;
      test =  fin < 0 ? 0 : atom_getfloatarg(arg, MAXENTRIES, x->x_map.map);
      if(test!=0)
	{
	  outlet_bang(x->inst);
	  outlet_float(x->value, test);
	  outlet_float(x->mapped, arg);
	}
      else
	{
	  arga =  arg;
	  while(itest == 0 && (x->max > 0))
	    {
	      arga = (arga + 1) <= LASTENTRY ? (arga + 1) : 0;
	      testa = atom_getfloatarg(arga, MAXENTRIES, x->x_map.map);
	      itest = testa != 0 ? 1 : 0;
	    }
	  if(x->max > 0 && fin >= 0)
	    {
	      outlet_float(x->value, testa);
	      outlet_float(x->mapped, arga);
	    }
	}
    }
  else if (x->mode == 3) /* if the index is 0, find the next lowest */
    {
      itest = 0;
      test =  fin < 0 ? 0 : atom_getfloatarg(arg, MAXENTRIES, x->x_map.map);
      if(test!=0)
	{
	  outlet_bang(x->inst);
	  outlet_float(x->value, test);
	  outlet_float(x->mapped, arg);
	}
      else
	{
	  arga =  arg;
	  while(itest == 0 && (x->max > 0))
	    {
	      argb = arga - 1;
	      arga = argb >= 0 ? argb : LASTENTRY;
	      testa = atom_getfloatarg(arga, MAXENTRIES, x->x_map.map);
	      itest = testa != 0 ? 1 : 0;
	    }
	}
      outlet_float(x->value, testa);
      outlet_float(x->mapped, arga);
    }
}

void sieve_set(t_sieve *x, t_floatarg fmap, t_floatarg fval) /* set one value 
                                                                in the array */
{
  float fvaller;
  if(fmap < MAXENTRIES && fmap >= 0)
    {
      int imap = (int)fmap;
      fvaller = fval != 0 ? 0 : 1;
      SETFLOAT(&x->x_map.map[imap], fval);
      SETFLOAT(&x->x_map.nomap[imap], fvaller);
      x->max = fmap > x->max ? fmap : x->max;
    }
}

void sieve_delete(t_sieve *x, t_floatarg loc) /* remove a value */
{
  int addloc = (int)loc + 1;
  int maxentry = (int)x->max;
  int i;
  float buffer;
  if(loc<x->max && loc>=0)
    {
      for(i=addloc;i<=maxentry;i++)
	{
	  buffer = atom_getfloatarg(i,MAXENTRIES,x->x_map.map);
	  SETFLOAT(&x->x_map.map[i-1],buffer);
	  if(buffer!=0)
	    {
	      SETFLOAT(&x->x_map.nomap[i-1],0);
	    }
	  else
	    {
	      SETFLOAT(&x->x_map.nomap[i-1],1);
	    }
	}
      SETFLOAT(&x->x_map.map[maxentry],0);
      x->max--;
    }
  else if(loc==x->max)
    {
      x->max--;
      SETFLOAT(&x->x_map.map[maxentry],0);
    }
}

void sieve_shunt(t_sieve *x, t_floatarg loc) /* move down 
                                                and decrement subsequent */
{
  int addloc = (int)loc + 1;
  int maxentry = (int)x->max;
  int i;
  float buffer, shunt;
  if(loc<x->max && loc>=0)
    {
      for(i=addloc;i<=maxentry;i++)
	{
	  buffer = atom_getfloatarg(i,MAXENTRIES,x->x_map.map);
	  shunt = buffer - 1;
	  SETFLOAT(&x->x_map.map[i-1],shunt);
	  if(shunt!=0)
	    {
	      SETFLOAT(&x->x_map.nomap[i-1],0);
	    }
	  else
	    {
	      SETFLOAT(&x->x_map.nomap[i-1],1);
	    }
	}
      SETFLOAT(&x->x_map.map[maxentry],0);
      x->max--;
    }
  else if(loc==x->max)
    {
      x->max--;
      SETFLOAT(&x->x_map.map[maxentry],0);
    }
}

void sieve_shift(t_sieve *x, t_floatarg loc) /* move up and
                                                increment subsequent */
{
  int location = (int)loc;
  int addloc;
  int maxentry = (int)x->max+1;
  int i;
  float buffer, shift;
  if(location>=0 && maxentry < MAXENTRIES)
    {
      for(i=maxentry;i>=location;i--)
	{
	  buffer = atom_getfloatarg(i-1,MAXENTRIES,x->x_map.map);
	  shift = buffer + 1;
	  SETFLOAT(&x->x_map.map[i],shift);
	  if(shift!=0)
	    {
	      SETFLOAT(&x->x_map.nomap[i],0);
	    }
	  else
	    {
	      SETFLOAT(&x->x_map.nomap[i],1);
	    }
	}
      x->max++;
    }
}

void sieve_insert(t_sieve *x, t_floatarg loc, t_floatarg val)
/* insert a value at specific location, moving subsequent values up */
{
  int location = (int)loc;
  int maxentry = (int)x->max+1;
  int i;
  float buffer;
  if(loc>=0 && maxentry < MAXENTRIES)
    {
      for(i=maxentry;i>=location;i--)
	{
	  buffer = atom_getfloatarg(i-1,MAXENTRIES,x->x_map.map);
	  SETFLOAT(&x->x_map.map[i],buffer);
	  if(buffer!=0)
	    {
	      SETFLOAT(&x->x_map.nomap[i],0);
	    }
	  else
	    {
       	      SETFLOAT(&x->x_map.nomap[i],1);
	    }
	}
      x->max++;
      SETFLOAT(&x->x_map.map[location], val);
      if(val) 
	{
	  SETFLOAT(&x->x_map.nomap[location],0);
	}
      else
	{
	  SETFLOAT(&x->x_map.nomap[location],1);
	}
    }
}

void sieve_get(t_sieve *x, t_floatarg inv) /* outlet to map or inverse */
{
  if(inv!=0) 
    {
      outlet_list(x->mapout, gensym("list"), x->max+1, x->x_map.nomap);
    }
  else outlet_list(x->mapout, gensym("list"), x->max+1, x->x_map.map);
  x->outmap = inv;
}

void sieve_clear(t_sieve *x)
{
  int i;
  for(i=0;i<MAXENTRIES;i++) 
    {
      SETFLOAT(&x->x_map.map[i], 0);
      SETFLOAT(&x->x_map.nomap[i], 1);
    }
  x->max = 0;
}

void sieve_map(t_sieve *x, t_symbol *s, int argc, t_atom *argv) /* set the whole map */
{
  int i;
  for(i=0;i<MAXENTRIES;i++) 
    {
      SETFLOAT(x->x_map.map+i, 0);
      SETFLOAT(x->x_map.nomap+i, 1);
    }
  x->max = 0;
  float arg;
  for(i=0;i<argc;i++) 
    {
      arg = atom_getfloat(argv+i);
      if(arg != 0)
	{
	  SETFLOAT(&x->x_map.map[i], arg);
	  SETFLOAT(&x->x_map.nomap[i], 0);
	  x->max = i;
	}
    }
  if (x->max > 0 && x->outmap == 0)
    {
      outlet_list(x->mapout, gensym("list"), x->max+1, x->x_map.map);
    }
  else if (x->max > 0 && x->outmap == 1)
    {
      outlet_list(x->mapout, gensym("list"), x->max+1, x->x_map.nomap);
    }
}

void sieve_mode(t_sieve *x, t_floatarg fmode)
{
  x->mode = fmode < 0 ? 0 : fmode > 3 ? 3 : fmode;
}

void sieve_debug(t_sieve *x)
{
  float ele0, ele1, ele2, ele3, ele4, ele5, ele6, ele7, ele8, ele9;
  float nle0, nle1, nle2, nle3, nle4, nle5, nle6, nle7, nle8, nle9;
  ele0 = atom_getfloatarg(0, MAXENTRIES, x->x_map.map);
  ele1 = atom_getfloatarg(1, MAXENTRIES, x->x_map.map);
  ele2 = atom_getfloatarg(2, MAXENTRIES, x->x_map.map);
  ele3 = atom_getfloatarg(3, MAXENTRIES, x->x_map.map);
  ele4 = atom_getfloatarg(4, MAXENTRIES, x->x_map.map);
  ele5 = atom_getfloatarg(5, MAXENTRIES, x->x_map.map);
  ele6 = atom_getfloatarg(6, MAXENTRIES, x->x_map.map);
  ele7 = atom_getfloatarg(7, MAXENTRIES, x->x_map.map);
  ele8 = atom_getfloatarg(8, MAXENTRIES, x->x_map.map);
  ele9 = atom_getfloatarg(9, MAXENTRIES, x->x_map.map);
  nle0 = atom_getfloatarg(0, MAXENTRIES, x->x_map.nomap);
  nle1 = atom_getfloatarg(1, MAXENTRIES, x->x_map.nomap);
  nle2 = atom_getfloatarg(2, MAXENTRIES, x->x_map.nomap);
  nle3 = atom_getfloatarg(3, MAXENTRIES, x->x_map.nomap);
  nle4 = atom_getfloatarg(4, MAXENTRIES, x->x_map.nomap);
  nle5 = atom_getfloatarg(5, MAXENTRIES, x->x_map.nomap);
  nle6 = atom_getfloatarg(6, MAXENTRIES, x->x_map.nomap);
  nle7 = atom_getfloatarg(7, MAXENTRIES, x->x_map.nomap);
  nle8 = atom_getfloatarg(8, MAXENTRIES, x->x_map.nomap);
  nle9 = atom_getfloatarg(9, MAXENTRIES, x->x_map.nomap);
  post("mode = %f, max = %f", x->mode, x->max);
  post("first 10 elements = %f, %f, %f, %f, %f, %f, %f, %f, %f, %f", ele0, ele1, ele2, ele3, ele4, ele5, ele6, ele7, ele8, ele9);
  post("first 10 elements = %f, %f, %f, %f, %f, %f, %f, %f, %f, %f", nle0, nle1, nle2, nle3, nle4, nle5, nle6, nle7, nle8, nle9);
}  

void *sieve_new(t_floatarg f) 
{
  t_sieve *x = (t_sieve *)pd_new(sieve_class);
  x->mode = f;
  x->max = 0;
  x->outmap = 0;
  int i;
  for(i=0;i<MAXENTRIES;i++) 
    {
      SETFLOAT(x->x_map.map+i, 0);
      SETFLOAT(x->x_map.nomap+i, 1);
    }
  x->mapped = outlet_new(&x->x_obj, &s_float);
  x->value = outlet_new(&x->x_obj, &s_float);
  x->mapout = outlet_new(&x->x_obj, &s_list);
  x->inst = outlet_new(&x->x_obj, &s_bang);
  return (void *)x;
}

void sieve_setup(void) 
{
  sieve_class = class_new(gensym("sieve"),
  (t_newmethod)sieve_new,
  0, sizeof(t_sieve),
  0, A_DEFFLOAT, 0);
  post("|^^^^^^^^^^^^^sieve^^^^^^^^^^^^^|");
  post("|->^^^integer map to floats^^^<-|");
  post("|^^^^^^^Edward Kelly 2006^^^^^^^|");

  class_addfloat(sieve_class, sieve_float);
  class_addmethod(sieve_class, (t_method)sieve_set, gensym("set"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_map, gensym("map"), A_GIMME, 0);
  class_addmethod(sieve_class, (t_method)sieve_clear, gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_get, gensym("get"), A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_delete, gensym("delete"), A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_shunt, gensym("shunt"), A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_shift, gensym("shift"), A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_insert, gensym("insert"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_mode, gensym("mode"), A_DEFFLOAT, 0);
  class_addmethod(sieve_class, (t_method)sieve_debug, gensym("debug"), A_DEFFLOAT, 0);
}
