/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* ------------------------- f2note ---------------------- */
/* ------ frequency to note plus cents converter --------- */

typedef struct _f2note
{
  t_object x_obj;
  void     *x_outlet_midi;
  void     *x_outlet_note;
  void     *x_outlet_cent;
  int      x_centomidi;
  t_float  x_refhz;
  t_float  x_refexp;
  t_float  x_reflog;
  t_symbol *x_set;
} t_f2note;

static t_class *f2note_class;

t_float f2note_mtof(t_f2note *x, t_float midi)
{
  return(x->x_refexp * exp(0.057762265047 * midi));
}

t_float f2note_ftom(t_f2note *x, t_float freq)
{
  return (freq > 0 ? 17.31234049 * log(x->x_reflog * freq) : -1500);
}

void f2note_calc_ref(t_f2note *x)
{
  t_float ln2=log(2.0);
  
  x->x_refexp = x->x_refhz*exp(-5.75*ln2);
  x->x_reflog = 1.0/x->x_refexp;
}

static void f2note_make_note(char *str, int midi)
{
  int j,k,l=0;
  
  j = midi / 12;
  k = midi % 12;
  if(k <= 5)
  {
    if(k <= 2)
    {
      if(k==0)
        str[l]='c';
      else if(k==1)
      {
        str[l++]='#';
        str[l]='c';
      }
      else
        str[l]='d';
    }
    else
    {
      if(k==3)
      {
        str[l++]='#';
        str[l]='d';
      }
      else if(k==4)
        str[l]='e';
      else
        str[l]='f';
    }
  }
  else
  {
    if(k <= 8)
    {
      if(k==6)
      {
        str[l++]='#';
        str[l]='f';
      }
      else if(k==7)
        str[l]='g';
      else
      {
        str[l++]='#';
        str[l]='g';
      }
    }
    else
    {
      if(k==9)
        str[l]='a';
      else if(k==10)
      {
        str[l++]='#';
        str[l]='a';
      }
      else
        str[l]='h';
    }
  }
  
  if(j < 4)
  {
    str[l] -= 'a';
    str[l] += 'A';
  }
  l++;
  if(j < 3)
  {
    str[l++] = '0' + (char)(3 - j);
  }
  else if(j > 4)
  {
    str[l++] = '0' + (char)(j - 4);
  }
  str[l] = 0;
}

static void f2note_bang(t_f2note *x)
{
  int i,j;
  t_atom at;
  char s[4];
  
  i = (x->x_centomidi + 50)/100;
  j = x->x_centomidi - 100*i;
  outlet_float(x->x_outlet_cent, (t_float)j);
  f2note_make_note(s, i);
  SETSYMBOL(&at, gensym(s));
  outlet_anything(x->x_outlet_note, x->x_set, 1, &at);
  outlet_float(x->x_outlet_midi, 0.01f*(t_float)(x->x_centomidi));
}

static void f2note_float(t_f2note *x, t_floatarg freq)
{
  x->x_centomidi = (int)(100.0f*f2note_ftom(x, freq) + 0.5f);
  f2note_bang(x);
}

void f2note_ref(t_f2note *x, t_floatarg ref)
{
  x->x_refhz = ref;
  f2note_calc_ref(x);
}

static void *f2note_new(t_floatarg ref)
{
  t_f2note *x = (t_f2note *)pd_new(f2note_class);
  
  if(ref == 0.0f)
    ref=440.0f;
  x->x_refhz = ref;
  x->x_centomidi = (int)(100.0f*ref + 0.499f);
  f2note_calc_ref(x);
  x->x_outlet_midi = outlet_new(&x->x_obj, &s_float);
  x->x_outlet_note = outlet_new(&x->x_obj, &s_list);
  x->x_outlet_cent = outlet_new(&x->x_obj, &s_float);
  x->x_set = gensym("set");
  return (x);
}

static void f2note_free(t_f2note *x)
{
}

void f2note_setup(void)
{
  f2note_class = class_new(gensym("f2note"), (t_newmethod)f2note_new, (t_method)f2note_free,
    sizeof(t_f2note), 0, A_DEFFLOAT, 0);
  class_addbang(f2note_class,f2note_bang);
  class_addfloat(f2note_class,f2note_float);
  class_addmethod(f2note_class, (t_method)f2note_ref, gensym("ref"), A_FLOAT, 0);
//  class_sethelpsymbol(f2note_class, gensym("iemhelp/help-f2note"));
}
