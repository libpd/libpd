/*************************************************************************** 
 * File: shuffle.c 
 * Auth: Iain Mott [iain.mott@bigpond.com] 
 * Maintainer: Iain Mott [iain.mott@bigpond.com] 
 * Version: Part of motex_1.1.2 
 * Date: January 2001
 * 
 * Description: Pd control external. a no-repeat random generator.
 * Outputs numbers within a set range
 * See supporting Pd patch: shuffle.pd
 * 
 * Copyright (C) 2001 by Iain Mott [iain.mott@bigpond.com] 
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2, or (at your option) 
 * any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License, which should be included with this 
 * program, for more details. 
 * 
 ****************************************************************************/ 

/* code for shuffle pd class */

#include "m_pd.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>


typedef struct shuffle
{
  t_object t_ob;
  float begin;
  float end;
  int size;   /*   size of pshuffle */
  int size_ptemp;
  float fraction;	/*  fractional size of ptemp */
  int *pshuffle;
  int *ptemp; 
  int index;
} t_shuffle;

int itemPresent (t_shuffle *x, int test)
{
  int flag = 0;
  int temp_flag = 0;
  int i;
  for (i = 0; i < x->size; i++)
    {
      if (test == x->pshuffle[i])
	temp_flag = 1;
      flag = flag || temp_flag;
    }
  return flag;
}


void fillWithMin (t_shuffle *x)
{
  int i;
  for (i = 0; i < x->size; i++)
    {
      x->pshuffle[i] = INT_MIN;
    }
}

void srubLastFraction (t_shuffle *x)
{
  int i;
  for (i = 0; i < x->size_ptemp; i++)
    x->pshuffle[x->size - 1 - i] = INT_MIN;
}

void shuffleDeck (t_shuffle *x)
{
  double scaled_rand;
  int answer;
  int i = 0;
  int done = 0;
  while (i < x->size)
    {
      if (i >= x->size_ptemp && !done)
	{
	  srubLastFraction(x);
	  done = 1;
	}
      scaled_rand = rand () / (float) RAND_MAX ;
      scaled_rand *= (x->end - x->begin);
      answer = (int) (scaled_rand + 0.5) + (int) x->begin;		
      if (!itemPresent (x, answer))
	{
	  x->pshuffle[i] = answer;
	  i++;
	}
    }
}

void shuffle_float(t_shuffle *x, t_floatarg f)
{ 
  x->begin = f;
  if (x->end < x->begin)
    {
      float tmp = x->begin;
      x->begin = x->end;
      x->end = tmp;
    }
  x->size = (int) x->end - (int) x->begin + 1;
  x->index = 0;
  free(x->pshuffle);
  free(x->ptemp);
  x->pshuffle = malloc ( x->size * sizeof(int));
  x->size_ptemp = (int) (x->fraction * x->size);
  x->ptemp = malloc( x->size_ptemp * sizeof(int));
  fillWithMin (x);
  shuffleDeck(x);
}

void shuffle_ft1(t_shuffle *x, t_floatarg g)
{
  x->end = g;
}

void shuffle_free(t_shuffle *x)
{
  free(x->pshuffle);
  free(x->ptemp);
}

t_class *shuffle_class;

void shuffle_ft2(t_shuffle *x, t_floatarg f)
{
  if(f > 0.5)
    {
      post("Shuffle: fraction too great - set to 0.5");
      f = 0.5;
    }
  x->fraction = f;
}



void shuffle_bang(t_shuffle *x)
{
  if (x->index == x->size) 
    {
      int i;
      for (i = 0; i < x->size_ptemp; i++)
	{
	  x->ptemp[i] = x->pshuffle[x->size - 1 - i];
	}
      fillWithMin (x);
      for (i = 0; i < x->size_ptemp; i++)
	x->pshuffle[x->size - 1 - i] = x->ptemp[i];
      shuffleDeck(x);
      x->index = 0;
      outlet_float(x->t_ob.ob_outlet, (float) x->pshuffle[x->index]);  
    }
  else
    outlet_float(x->t_ob.ob_outlet, (float) x->pshuffle[x->index]);
  x->index++;
}

void *shuffle_new(t_floatarg be, t_floatarg en, t_floatarg frac)
{
  t_shuffle *x = (t_shuffle *)pd_new(shuffle_class);
  inlet_new(&x->t_ob, &x->t_ob.ob_pd, gensym("float"), gensym("ft1"));
  inlet_new(&x->t_ob, &x->t_ob.ob_pd, gensym("float"), gensym("ft2"));
  outlet_new(&x->t_ob, gensym("bang"));
  if(frac > 0.5)
    {
      post("Shuffle: fraction too great - set to 0.5");
      frac = 0.5;
    }
  x->fraction = frac;
  if(be < en)
    {
      x->end = en;
      x->begin = be;
    }
  else 
    {
      x->end = be;
      x->begin = en;
    }
  x->index = 0;
  x->size = (int) x->end - (int) x->begin + 1;
  x->size_ptemp = (int) (x->fraction * x->size);
  x->pshuffle = malloc (x->size * sizeof(int));
  x->ptemp = malloc (x->size_ptemp * sizeof(int));
  fillWithMin (x);
  shuffleDeck(x);
  return (void *)x;
}



void shuffle_setup(void)
{
  shuffle_class = class_new(gensym("shuffle"), (t_newmethod)shuffle_new,
			    (t_method)shuffle_free, sizeof(t_shuffle), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(shuffle_class, (t_method)shuffle_bang, gensym("bang"), (t_atomtype) 0);
  class_addmethod(shuffle_class, (t_method)shuffle_ft1, gensym("ft1"), A_FLOAT, 0);  
  class_addmethod(shuffle_class, (t_method)shuffle_ft2, gensym("ft2"), A_FLOAT, 0);  
  class_addfloat(shuffle_class, shuffle_float);
  srand( (unsigned)time( NULL ) );  
}


