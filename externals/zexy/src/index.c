/* 
 * index: associative dictionary
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


/* 
   (c) 2005:forum::für::umläute:2000

   "index" simulates an associative index :: that is : convert a symbol to an index

   CAVEATS: starts to count at "1"

   TODO: use "symbol" instead of "char*" : FIXED
   TODO: "dump" the contents (so we can share between [index]es, ...) FIXED
   TODO: "compact": move all entries to the beginning of the array FIXED
   TODO: "sort" FIXED
   TODO: "add" at a specific position (like "add 10 hallo" of "add hallo 10") (??) FIXED
   TODO: "delete" from a specific position (like "delete 4" deletes the 4th element) FIXED
   TODO: get the number of stored entries ("bang") FIXED
   
   TODO: resize the array if it gets to small

*/

#include "zexy.h"
#include <string.h>

/* ----------------------- index --------------------- */

static t_class *index_class;

typedef struct _index
{
  t_object x_obj;

  int entries, maxentries;
  int auto_mode; /* 1--add if key doesn't already exist; 0--do not add; */
  int auto_resize; /* 1--resize the array if we are running out of slots; 0--don't */

  t_symbol **names;
} t_index;

/************************************
 * index_helpers
 */


/* find the last non-NULL entry in the array *
 * LATER: shouldn't this return "-1" on failure ?
 */
static int find_last(t_symbol **names, int maxentries)
{  /* returns the index of the last entry (0..(maxentries-1)) */
  while (maxentries--) if (names[maxentries]) return maxentries;
  return 0;
}

/* search the array for "key" 
 * if it is not there, return "-1" 
 */
static int find_item(const t_symbol *key, t_symbol **names, int maxentries)
{  /* returns index (0..[maxentries-1?]) on success; -1 if the item could not be found */
  int i=-1;
  int max = find_last(names, maxentries);
  
  while (++i<=max)
    if (names[i] && key==names[i]) return i;
  
  return -1;
}

/* find the first NULL entry in the array
 * return "-1" if none can be found 
 */
static int find_free(t_symbol **names, int maxentries)
{
  int i=0;

  while (i<maxentries) {
    if (!names[i]) return i;
    i++;
  }
  return -1;
}

/************************************
 * methods
 */
static void index_add(t_index *x, t_symbol *s, t_float f);

/* look up a symbol in the map */
static void index_symbol(t_index *x, t_symbol *s)
{
  int element;
  if ( (element = find_item(s, x->names, x->maxentries)+1) )
    outlet_float(x->x_obj.ob_outlet, (t_float)element);
  else if (x->auto_mode) /* not yet stored: add automatically */
    index_add(x, s, 0);
  else outlet_float(x->x_obj.ob_outlet, 0.f); /* not yet stored but do not add */
}

/* output the entry at a given index */
static void index_float(t_index *x, t_float findex)
{
  int iindex = (int)findex;
  if ((iindex > 0) && (iindex <= x->maxentries) && (x->names[iindex-1])) 
  {
      /* TB: output symbol to outlet */
      outlet_symbol (x->x_obj.ob_outlet,x->names[iindex-1]);
  }
}


/* add a symbol to the map (if possible) */
static void index_add(t_index *x, t_symbol *s, t_float f)
{
  int newentry=(int)f;

  if (! (find_item(s, x->names, x->maxentries)+1) ) {
    if (x->auto_resize && (x->entries==x->maxentries || newentry>=x->maxentries)){
      /* do some resizing */
      int maxentries=(newentry>x->maxentries)?newentry:(x->maxentries*2);
      int i;
      t_symbol**buf=(t_symbol **)getbytes(sizeof(t_symbol *) * maxentries);
      if(buf!=0){
        memcpy(buf, x->names, sizeof(t_symbol *) * x->maxentries);
        for(i=x->maxentries; i<maxentries; i++)buf[i]=0;

        freebytes(x->names, sizeof(t_symbol *) * x->maxentries);

        x->names=buf;
        x->maxentries=maxentries;
      }
    }

    if ( x->entries < x->maxentries ) {
      if(newentry>0){
        newentry--;
        if(x->names[newentry]){ /* it is already taken! */
          z_verbose(1, "index :: couldn't add element '%s' at position %d (already taken)", s->s_name, newentry+1);
          outlet_float(x->x_obj.ob_outlet, -1.f);
          return;
        }
      } else {
        newentry=find_free(x->names, x->maxentries);
      }
      if (newentry + 1) {
	x->entries++;
	x->names[newentry]=s;
	outlet_float(x->x_obj.ob_outlet, (t_float)newentry+1);
        return;

      } else error("index :: couldn't find any place for new entry");
    } else error("index :: max number of elements (%d) reached !", x->maxentries);
  } else  z_verbose(1, "index :: element '%s' already exists", s->s_name);
  /* couldn't add the symbol to our index table */
  outlet_float(x->x_obj.ob_outlet, -1.f);
}
/* delete a symbol from the map (if it is in there) */
static void index_delete(t_index *x, t_symbol *s, int argc, t_atom*argv)
{
  int idx=-1;
  ZEXY_USEVAR(s);
  if(argc!=1){
    error("index :: delete what ?");
    return;
  } else {
    if(argv->a_type==A_FLOAT){
      idx=atom_getint(argv)-1;
    } else if (argv->a_type==A_SYMBOL){
      idx=find_item(atom_getsymbol(argv),x->names, x->maxentries);
    } else {
      error("index :: delete what ?");
      return;    
    }
  }

  if ( idx >= 0 && idx < x->maxentries) {
    x->names[idx]=0;
    x->entries--;
    outlet_float(x->x_obj.ob_outlet, 0.0);
  } else {
    z_verbose(1, "index :: couldn't find element");
    outlet_float(x->x_obj.ob_outlet, -1.0);
  }
}

/* delete all symbols from the map */
static void index_reset(t_index *x)
{
  int i=x->maxentries;

  while (i--)
    if (x->names[i]) {
      x->names[i]=0;
    }

  x->entries=0;

  outlet_float(x->x_obj.ob_outlet, 0.f);
}

/* output the number of entries stored in the array */
static void index_bang(t_index *x)
{
  outlet_float(x->x_obj.ob_outlet, (t_float)x->entries);
}
/* dump each entry in the format: "list <symbol> <index>" */
static void index_dump(t_index *x)
{
  t_atom ap[2];
  int i=0;
  for(i=0; i<x->maxentries; i++){
    if(x->names[i]){
      SETSYMBOL(ap, x->names[i]);
      SETFLOAT(ap+1, i+1);
      outlet_list(x->x_obj.ob_outlet, 0, 2, ap);
    }
  }
}

/* compact all entries, removing all holes in the map */
static void index_compact(t_index *x){
  int i,j;
  for(i=0; i<x->entries; i++){
    if(!x->names[i]){
      for(j=i+1; j<x->maxentries; j++){
        if(x->names[j]){
          x->names[i]=x->names[j];
          x->names[j]=0;
          break;
        }
      }
    }
  }
}
/* sort the map alphabetically */
static void index_sort(t_index *x){
  int entries=x->entries;
  int step=entries;
  int loops=1, n;
  t_symbol**buf=x->names;
  index_compact(x); /* couldn't we do it more "in-place", e.g. don't touch empty slots ? */

  while(step>1){
    int i = loops;
    step+=step%2;
    step>>=1;
    loops+=2;

    while(i--) { /* there might be some optimization in here */
      for (n=0; n<(x->entries-step); n++) {
        int comp=strcmp(buf[n]->s_name,buf[n+step]->s_name);
        if (comp>0) { /* compare STRINGS not SYMBOLS */
          t_symbol*s_tmp = buf[n];
          buf[n]        = buf[n+step];
          buf[n+step]   = s_tmp;
        }
      }
    }
  }
}

/* turn on/off auto-adding of elements that are not yet in the map */
static void index_auto(t_index *x, t_float automod)
{
  x->auto_mode = !(!automod);
}
/* turn on/off auto-resizing of the map if it gets to small */
static void index_resize(t_index *x, t_float automod)
{
  x->auto_resize = !(!automod);
}



static void *index_new(t_symbol *s, int argc, t_atom *argv)
{
  t_index *x = (t_index *)pd_new(index_class);
  t_symbol** buf;

  int maxentries = 0, automod=0;

  ZEXY_USEVAR(s);

  if (argc--) {
    maxentries = (int)atom_getfloat(argv++);
    if (argc) automod = (int)atom_getfloat(argv++);
  }

  if (maxentries<1) maxentries=128;

  buf = (t_symbol **)getbytes(sizeof(t_symbol *) * maxentries);


  x->entries = 0;
  x->maxentries = maxentries;
  x->names = buf;
  x->auto_mode = !(!automod);
  x->auto_resize = 1;

  while (maxentries--) buf[maxentries]=0;

  outlet_new(&x->x_obj, gensym("float"));

  return (x);
}

static void index_free(t_index *x)
{
  freebytes(x->names, sizeof(t_symbol *) * x->maxentries);
}


static void index_helper(t_index *x)
{
  endpost();
  post("%c index :: index symbols to indices", HEARTSYMBOL);
  post("<symbol>             : look up the <symbol> in the index and return it's index");
  post("<int>                : look up the element at index <int> in the index");
  post("'add <symbol>'       : add a new symbol to the index-map");
  post("'add <symbol> <int>' : add a new symbol at the index <int>");
  post("'delete <symbol>'    : delete a symbol from the index-map");
  post("'delete <int>'       : delete the entry at index <int> from the index-map");
  post("'reset'              : delete the whole index-map");
  post("'bang'               : return the number of entries in the index-map");
  post("'dump'               : dump each entry in the format \"list <symbol> <index>\"");
  post("'compact'            : remove holes in the index-map");
  endpost();
  post("'sort'               : alphabetically sort the entries");
  post("'auto <1/0>          : if auto is 1 and a yet unknown symbol is looked up it is\n\t\t\t automatically added to the index-map");
  post("'resize <1/0>        : if resize is 1 (default), the index-map is resized\n\t\t\t automatically if needed");
  post("'help'               : view this");
  post("outlet : <n>         : index of the <symbol>");
  post("         <symbol>    : entry at <index>");
  endpost();
  post("creation:\"index [<maxelements> [<auto>]]\": creates a <maxelements> sized index");
}

void index_setup(void)
{
  index_class = class_new(gensym("index"),
			  (t_newmethod)index_new, (t_method)index_free,
			  sizeof(t_index), 0, A_GIMME, 0);

  class_addsymbol(index_class, index_symbol);

  class_addmethod(index_class, (t_method)index_reset,  gensym("reset"), 0);
  class_addmethod(index_class, (t_method)index_delete, gensym("delete"), A_GIMME, 0);
  /*  class_addmethod(index_class, (t_method)index_add,	 gensym("add"), A_SYMBOL, 0); */
  class_addmethod(index_class, (t_method)index_add,	 gensym("add"), A_SYMBOL, A_DEFFLOAT, 0);

  class_addmethod(index_class, (t_method)index_auto,	 gensym("auto"), A_FLOAT, 0);
  class_addmethod(index_class, (t_method)index_resize,	 gensym("resize"), A_FLOAT, 0);

  class_addfloat(index_class,  (t_method)index_float);
  class_addbang(index_class,   (t_method)index_bang);
  class_addmethod(index_class, (t_method)index_sort,  gensym("sort"), 0);
  class_addmethod(index_class, (t_method)index_compact,  gensym("compact"), 0);
  class_addmethod(index_class, (t_method)index_dump,  gensym("dump"), 0);

  class_addmethod(index_class, (t_method)index_helper, gensym("help"), 0);
  zexy_register("index");
}
