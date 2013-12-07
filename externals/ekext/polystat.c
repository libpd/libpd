#include "m_pd.h"
#include <math.h>
#include <string.h>
#define MAXENTRIES 512
#define LASTENTRY 511

static t_class *polystat_class;

typedef struct _map
{
  t_atom map[MAXENTRIES];
  t_atom maxlist[MAXENTRIES];
  t_atom outlist[MAXENTRIES];
} t_map;

typedef struct _polystat
{
  t_object x_obj;
  t_map x_map;
  t_int voices, max, maxval, maxindex, maxflag, maxcount, highest, mode;
  t_outlet *voice, *value, *vars, *maxvox, *mapped;
} t_polystat;

void polystat_float(t_polystat *x, t_floatarg fin)
{
  int voice = (int)fin;
  int inst;
  int maxindex = (int)x->maxindex + 1;
  if(voice >= 0 && voice < MAXENTRIES)
    {
      int instances = atom_getfloatarg(voice, MAXENTRIES, x->x_map.map);
      if(instances == 0)x->voices++;
      inst = instances + 1;
      SETFLOAT(&x->x_map.map[voice], inst);
      if(x->mode == 0)
	{
	  if(inst >= x->max && voice != x->maxval)
	    {
	      x->maxflag = x->maxcount >= MAXENTRIES ? 1 : x->maxflag == 1 ? 1 : 0;
	      x->maxcount += 1;
	      x->maxcount = x->maxcount > MAXENTRIES ? MAXENTRIES : x->maxcount; 
	      x->maxindex = maxindex % MAXENTRIES;
	      SETFLOAT(&x->x_map.maxlist[x->maxindex], voice);
	      x->maxval = voice;
	      x->max = inst;
	      outlet_float(x->maxvox, voice);
	    }
	  else if(inst >= x->max && voice == x->maxval)
	    {
	      x->max = inst;
	      outlet_float(x->maxvox, voice);
	    }
	}
      else if(x->mode != 0)
	{
	  if(inst > x->max && voice != x->maxval)
	    {
	      x->maxflag = x->maxcount >= MAXENTRIES ? 1 : x->maxflag == 1 ? 1 : 0;
	      x->maxcount += 1;
	      x->maxcount = x->maxcount > MAXENTRIES ? MAXENTRIES : x->maxcount; 
	      x->maxindex = (float)(maxindex % MAXENTRIES);
	      SETFLOAT(&x->x_map.maxlist[x->maxindex], voice);
	      x->maxval = voice;
	      x->max = inst;
	      outlet_float(x->maxvox, voice);
	    }
	  else if(inst > x->max && voice == x->maxval)
	    {
	      x->max = inst;
	      outlet_float(x->maxvox, voice);
	    }
	}
      x->highest = voice > x->highest ? voice : x->highest;
      outlet_float(x->vars, x->voices);
      outlet_float(x->value, inst);
      outlet_float(x->voice, voice);
    }
}

void polystat_clear(t_polystat *x, t_floatarg fin)
{
  int inst = (int)fin;
  int indexed = (int)(x->maxindex+(MAXENTRIES*x->maxflag));
  int modindex = 0;
  int counted = 0;
  float current = atom_getfloatarg(inst, MAXENTRIES, x->x_map.map);
  float voice;
  voice = 0;
  if(x->voices > 0 && current > 0)
    {
      x->voices--;
      SETFLOAT(&x->x_map.map[inst], 0);
      current = 0;
      if(current == 0 && x->maxcount > 0)
	{
	  indexed--;
	  x->maxcount--;
	  modindex = (indexed+MAXENTRIES) % MAXENTRIES;
	  voice = atom_getfloatarg(modindex, MAXENTRIES, x->x_map.maxlist);
	  current = atom_getfloatarg(voice, MAXENTRIES, x->x_map.map);
	}
      if(x->maxcount == 0 || current == 0)
	{
	  x->max = 0;
	  x->maxval = 0;
	  x->maxindex = 0;
	}
      else if(x->maxcount > 0 || current > 0)
	{
	  x->max = current;
	  x->maxval = voice;
	  x->maxindex = modindex;
	}
    }
  outlet_float(x->maxvox, x->maxval);
  outlet_float(x->vars, x->voices);
  outlet_float(x->value, x->max);
}

void polystat_print(t_polystat *x, t_symbol *s)
{
  post("most_instances = %d at voice %d, voices = %d, highest_voice = %d", x->max, x->maxval, x->voices, x->highest);
}

void polystat_get(t_polystat *x, t_floatarg fin)
{
  int inst = (int)fin;
  float voice, instances;
  voice = instances = 0;
  if(x->voices>0 && inst>=0 && inst<MAXENTRIES)
    {
      instances = atom_getfloatarg(inst, MAXENTRIES, x->x_map.map);
      voice = inst;
    }
  if(instances>0)
    {
      outlet_float(x->value, instances);
      outlet_float(x->voice, voice);
    }
}

void polystat_clearall(t_polystat *x)
{
  int i;
  for(i=0;i<MAXENTRIES;i++)
    {
      SETFLOAT(&x->x_map.map[i],0);
      SETFLOAT(&x->x_map.maxlist[i],0);
    }
  x->voices = x->max = x->maxval = x->maxindex = x->highest = x->maxcount = 0;
  outlet_float(x->vars, x->voices);
}

void polystat_getmap(t_polystat *x, t_symbol *s) //output map. terminate map at highest voice.
{
  if(x->voices>0)
    {
      outlet_list(x->mapped, gensym("list"), x->highest+1, x->x_map.map);
    }
}

void polystat_setmap(t_polystat *x, t_symbol *s, int argc, t_atom *argv)
{
  int i;
  float arg, max, maxval, high;
  max = maxval = high = 0;
  for(i=0;i<argc;i++) 
    {
      arg = atom_getfloat(argv+i);
      if(arg != 0)
	{
	  if(x->mode==0)
	    {
	      if(arg>=maxval)
		{
		  maxval = arg;
		  max = i;
		}
	    }
	  else if(x->mode!=0)
	    {
	      if(arg>maxval)
		{
		  maxval = arg;
		  max = i;
		}
	    }
	  x->highest = i;
	  SETFLOAT(&x->x_map.map[i], arg);
	  SETFLOAT(&x->x_map.maxlist[i], 0);
	}
    }
  x->max = max;
  x->maxval = maxval;
  SETFLOAT(&x->x_map.maxlist[0], max);
}

void polystat_bang(t_polystat *x, t_symbol *s)
{
  outlet_float(x->vars, x->voices);
  outlet_float(x->value, x->maxval);
  outlet_float(x->voice, x->max);
}

void polystat_voices(t_polystat *x, t_symbol *s) //list all voices 
{
  int i, mindex, current;
  mindex=0;
  for(i=0;i<(x->highest+1);i++)
    {
      current = atom_getfloatarg(i, MAXENTRIES, x->x_map.map);
      if(current>0)
	{
	  SETFLOAT(&x->x_map.outlist[mindex],(float)i);
	  mindex++;
	}
    }
  outlet_list(x->mapped, gensym("list"), x->voices, x->x_map.outlist);
}

void polystat_mode(t_polystat *x, t_floatarg fmode)
{
  x->mode = fmode;
}

void *polystat_new(t_floatarg f) 
{
  t_polystat *x = (t_polystat *)pd_new(polystat_class);
  x->mode = f;
  x->max = x->maxval = x->voices = x->maxindex = x->maxcount = x->highest = 0;
  //memset(x->x_map.map, 0, MAXENTRIES);
  //memset(x->x_map.nomap, 1, MAXENTRIES);
  int i;
  for(i=0;i<MAXENTRIES;i++) 
    {
      SETFLOAT(x->x_map.map+i, 0);
      SETFLOAT(x->x_map.maxlist+i, 0);
    }
  x->voice = outlet_new(&x->x_obj, &s_float);
  x->value = outlet_new(&x->x_obj, &s_float);
  x->vars = outlet_new(&x->x_obj, &s_float);
  x->maxvox = outlet_new(&x->x_obj, &s_float);
  x->mapped = outlet_new(&x->x_obj, &s_list);
  return (void *)x;
}

void polystat_setup(void) 
{

  polystat_class = class_new(gensym("polystat"),
  (t_newmethod)polystat_new,
  0, sizeof(t_polystat),
  0, A_DEFFLOAT, 0);
  post("|^^^^^^^^^^^^polystat^^^^^^^^^^^^|");
  post("|->^^^^polyphony statistics^^^^<-|");
  post("|^^^^^^^^Edward Kelly 2006^^^^^^^|");

  class_addfloat(polystat_class, polystat_float);
  class_addmethod(polystat_class, (t_method)polystat_clear, gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(polystat_class, (t_method)polystat_get, gensym("get"), A_DEFFLOAT, 0);
  class_addmethod(polystat_class, (t_method)polystat_clearall, gensym("clearall"), A_DEFFLOAT, 0);
  class_addmethod(polystat_class, (t_method)polystat_getmap, gensym("getmap"), A_DEFFLOAT, 0);
  class_addmethod(polystat_class, (t_method)polystat_setmap, gensym("setmap"), A_GIMME, 0);
  class_addmethod(polystat_class, (t_method)polystat_voices, gensym("voices"), A_DEFFLOAT, 0);
  class_addmethod(polystat_class, (t_method)polystat_mode, gensym("mode"), A_DEFFLOAT, 0);
  class_addmethod(polystat_class, (t_method)polystat_print, gensym("print"), A_DEFFLOAT, 0);
  class_addbang(polystat_class, (t_method)polystat_bang);
}
