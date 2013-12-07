/* 
 * blockshuffle~: shuffle a signal block
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "zexy.h"

/* ------------------------ blockshuffle~ ----------------------------- */

/* mirrors a signalblock around it's center:
   {x[0], x[1], ... x[n-1]} --> {x[n-1], x[n-2], ... x[0]}
*/

static t_class *blockshuffle_class;

typedef struct _blockshuffle
{  t_object x_obj;

  t_sample*blockbuf;
  t_int*   indices;
  int      size;

  t_float *shuffle;
  int shufflesize;
} t_blockshuffle;

static void blockshuffle_buildindex(t_blockshuffle *x, int blocksize){
  int i=0;
  if(blocksize!=x->size){
    if(x->indices)freebytes(x->indices, x->size);
    if(x->blockbuf)freebytes(x->blockbuf, x->size);
    x->indices=getbytes (sizeof(t_int)*blocksize);
    x->blockbuf=getbytes(sizeof(t_sample)*blocksize);
    x->size=blocksize;
  }
  for(i=0;i<x->shufflesize&&i<blocksize; i++){
    int idx=x->shuffle[i];
    if(idx>=blocksize)idx=blocksize-1;
    if(idx<0)idx=0;
    x->indices[i]=idx;
  }
  for(;i<blocksize; i++){
    x->indices[i]=i;
  }
}

static void blockshuffle_list(t_blockshuffle *x, t_symbol*s, int argc, t_atom*argv)
{
  int i;
  if(x->shuffle){
    freebytes(x->shuffle, x->shufflesize);
    x->shuffle=0;
  }
  x->shufflesize=argc;
  x->shuffle=getbytes(sizeof(*x->shuffle)*argc);

  for(i=0; i<argc; i++){
    x->shuffle[i]=atom_getfloat(argv++);
  }
  blockshuffle_buildindex(x, x->size);
}

static t_int *blockshuffle_perform(t_int *w)
{
  t_blockshuffle*x = (t_blockshuffle *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]);
  int i=0;
  t_sample *temp=x->blockbuf;
  t_int    *idx =x->indices;

  if(idx){
    for(i=0; i<n; i++){
      temp[i]=in[idx[i]];
    }
    temp=x->blockbuf;
    for(i=0; i<n; i++){
      *out++=*temp++;
    }
  } else
    while(n--)*out++=*in++;

  return (w+5);
}

static void blockshuffle_dsp(t_blockshuffle *x, t_signal **sp)
{
  blockshuffle_buildindex(x, sp[0]->s_n);

  dsp_add(blockshuffle_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void blockshuffle_helper(void)
{
  post("\n%c blockshuffle~-object for shuffling the samples within a signal-block", HEARTSYMBOL);
  post("'help' : view this\n"
       "signal~");
  post("outlet : signal~");
}
static void blockshuffle_free(t_blockshuffle *x){
  if(x->indices) freebytes(x->indices,  sizeof(*x->indices) *x->size);
  if(x->blockbuf)freebytes(x->blockbuf, sizeof(*x->blockbuf)*x->size);
  if(x->shuffle) freebytes(x->shuffle,  sizeof(*x->shuffle) *x->shufflesize);
}

static void *blockshuffle_new(void)
{
  t_blockshuffle *x = (t_blockshuffle *)pd_new(blockshuffle_class);
  outlet_new(&x->x_obj, gensym("signal"));
  x->size=0;
  x->blockbuf=0;
  x->indices=0;
  x->shuffle=0;
  x->shufflesize=0;
  return (x);
}

void blockshuffle_tilde_setup(void)
{
  blockshuffle_class = class_new(gensym("blockshuffle~"), (t_newmethod)blockshuffle_new, 
                                 (t_method)blockshuffle_free,
                                 sizeof(t_blockshuffle), 0, A_NULL);
  class_addmethod(blockshuffle_class, nullfn, gensym("signal"), 0);
  class_addmethod(blockshuffle_class, (t_method)blockshuffle_dsp, gensym("dsp"), 0);
  
  class_addlist(blockshuffle_class, blockshuffle_list);
  
  class_addmethod(blockshuffle_class, (t_method)blockshuffle_helper, gensym("help"), 0);
  zexy_register("blockshuffle~");
}
