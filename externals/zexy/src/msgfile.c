/* 
 * msgfile: an improved version of [textfile]
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
    this is heavily based on code from [textfile],
    which is part of pd and written by Miller S. Puckette
    pd (and thus [textfile]) come with their own license
*/

#include "zexy.h"

#ifdef __WIN32__
# include <io.h>
#else
# include <unistd.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <string.h>



/* ****************************************************************************** */
/* msgfile : save and load messages... */

#define PD_MODE 0
#define CR_MODE 1
#define CSV_MODE 2
/* modi
   PD : separate items by ' '; seperate lines by ";\n"
   looks like a PD-file
   CR : separate items by ' '; seperate lines by " \n"
   how you would expect a file to look like
   CSV: separate items by ','; seperate lines by " \n"
   spreadsheet: each argument gets its own column
*/


typedef struct _msglist {
  int n;
  t_atom *thislist;

  void *next;
  void *previous;  
} t_msglist;

typedef struct _msgfile
{
  t_object x_obj;              /* everything */
  t_outlet *x_secondout;        /* "done" */

  int mode;

  t_msglist *start;

  t_msglist *current;         /* pointer to our list */
  t_msglist *previous; /* just in case we lost "current" */

  t_symbol *x_dir;
  t_canvas *x_canvas;

  char eol, separator;

} t_msgfile;

static t_class *msgfile_class;





/* ************************************************************************ */
/* forward declarations                                                     */

static void msgfile_end(t_msgfile *x);
static void msgfile_goto(t_msgfile *x, t_float f);

/* ************************************************************************ */
/* help functions                                                           */

static int node_wherearewe(t_msgfile *x)
{
  int counter = 0;
  t_msglist *cur = x->start;

  while (cur && cur->next && cur!=x->current) {
    counter++;
    cur = cur->next;
  }

  if(cur&&cur->thislist)
    return counter;
  return -1;
}

static void write_currentnode(t_msgfile *x, int ac, t_atom *av)
{
  /* append list to the current node list */
  
  t_msglist *cur=x->current;
  t_atom *ap=NULL;
  int newsize = 0; 

  if(!cur || (ac && av && A_SYMBOL==av->a_type && gensym("")==atom_getsymbol(av))){
    /* ignoring empty symbols! */
    return;
  }

  newsize = cur->n + ac; 

  ap = (t_atom *)getbytes(newsize * sizeof(t_atom));
  memcpy(ap, cur->thislist, cur->n * sizeof(t_atom));
  cur->thislist = ap;
  memcpy(cur->thislist + cur->n, av, ac * sizeof(t_atom));

  cur->n = newsize;
}

static void delete_currentnode(t_msgfile *x)
{
  if (x&&x->current){
    t_msglist *dummy = x->current;
    t_msglist *nxt=0;
    t_msglist *prv=0;

    if(dummy){
      nxt=dummy->next;
      prv=dummy->previous;

      if(dummy==x->start) {
        x->start=nxt;
      }

      freebytes(dummy->thislist, sizeof(dummy->thislist));
      dummy->thislist = 0;
      dummy->n = 0;
      dummy->next=0;
      dummy->previous=0;

      freebytes(dummy, sizeof(t_msglist));

      dummy=0;
    }

    if (nxt) nxt->previous = prv;
    if (prv) prv->next     = nxt;
    
    x->current = (nxt)?nxt:prv;
    if(x->current)
      x->previous=x->current->previous;
    else
      x->previous=prv;

  }
}

static void delete_emptynodes(t_msgfile *x)
{
  x->current=x->start;
  x->previous=0;
  if (!x->current) return;

  while (x->current && x->current->next) {
    if (!x->current->thislist) delete_currentnode(x);
    else {
      x->previous=x->current;
      x->current = x->current->next;
    }
  }
}

static void add_currentnode(t_msgfile *x)
{ 
  /* add (after the current node) a node at the current position (do not write the listbuf !!!) */
  t_msglist *newnode = (t_msglist *)getbytes(sizeof(t_msglist));
  t_msglist  *prv, *nxt, *cur=x->current;

  newnode->n = 0;
  newnode->thislist = 0;

  prv = cur;
  nxt = (cur)?cur->next:0;

  newnode->next = nxt;
  newnode->previous = prv;

  if (prv) prv->next = newnode;
  if (nxt) nxt->previous = newnode;

  x->current = newnode;
  x->previous=prv;

  if(!x->start) /* it's the first line in the buffer */
    x->start=x->current;
}
static void insert_currentnode(t_msgfile *x)
{  /* insert (add before the current node) a node (do not write a the listbuf !!!) */
  t_msglist *newnode;
  t_msglist  *prv, *nxt, *cur = x->current;

  if (!(cur && cur->thislist)) {
    add_currentnode(x);
  } else {
    newnode = (t_msglist *)getbytes(sizeof(t_msglist));

    newnode->n = 0;
    newnode->thislist = 0;

    nxt = cur;
    prv = (cur)?cur->previous:0;

    newnode->next = nxt;
    newnode->previous = prv;

    if (prv) prv->next = newnode;
    if (nxt) nxt->previous = newnode;

    x->previous=prv;
    x->current = newnode;
    if(0==prv) {
      /* oh, we have a new start! */
      x->start = newnode;
    }
  }
}

/* delete from line "start" to line "stop"
 * if "stop" is negative, delete from "start" to the end
 */
static void delete_region(t_msgfile *x, int start, int stop)
{
  int n;
  int newwhere, oldwhere = node_wherearewe(x);

  /* get the number of lists in the buffer */
  t_msglist *dummy = x->start;
  int counter = 0;

  /* go to the end of the buffer */
  while (dummy && dummy->next) {
    counter++;
    dummy = dummy->next;
  }

  if ((stop > counter) || (stop == -1)) stop = counter;
  if ((stop+1) && (start > stop)) return;
  if (stop == 0) return;

  newwhere = (oldwhere < start)?oldwhere:( (oldwhere < stop)?start:start+(oldwhere-stop));
  n = stop - start;

  msgfile_goto(x, start);

  while (n--) delete_currentnode(x);

  if (newwhere+1) msgfile_goto(x, newwhere);
  else msgfile_end(x);
}


static int atomcmp(t_atom *this, t_atom *that)
{
  if (this->a_type != that->a_type) return 1;

  switch (this->a_type) {
  case A_FLOAT:
    return !(atom_getfloat(this) == atom_getfloat(that));
    break;
  case A_SYMBOL:
    return strcmp(atom_getsymbol(this)->s_name, atom_getsymbol(that)->s_name);
    break;
  case A_POINTER:
    return !(this->a_w.w_gpointer == that->a_w.w_gpointer);
    break;
  default:
    return 0;
  }

  return 1;
}


static void msgfile_binbuf2listbuf(t_msgfile *x, t_binbuf *bbuf)
{
  int ac = binbuf_getnatom(bbuf);
  t_atom *ap = binbuf_getvec(bbuf);

  while (ac--) {
    if (ap->a_type == A_SEMI) {
      add_currentnode(x);
    } else {
      write_currentnode(x, 1, ap);
    }
    ap++;
  }

  delete_emptynodes(x);
}





/* ************************************************************************ */
/* object methods                                                           */

static void msgfile_rewind(t_msgfile *x)
{
  //  while (x->current && x->current->previous) x->current = x->current->previous;    
  x->current = x->start;
  x->previous=0;
}
static void msgfile_end(t_msgfile *x)
{
  if (!x->current) return;
  while (x->current->next) {
    x->previous= x->current;
    x->current = x->current->next;
  }
  
}
static void msgfile_goto(t_msgfile *x, t_float f)
{
  int i = f;

  if (i<0) return;
  if (!x->current) return;
  msgfile_rewind(x);

  while (i-- && x->current->next) {
    x->previous= x->current;
    x->current = x->current->next;
  }
}
static void msgfile_skip(t_msgfile *x, t_float f)
{
  int i;
  int counter = 0;

  t_msglist *dummy = x->start;

  if (!f) return;
  if (!x->current) return;

  while (dummy->next && dummy!=x->current) {
    counter++;
    dummy=dummy->next;
  }

  i = counter + f;
  if (i<0) i=0;

  msgfile_goto(x, i);
}

static void msgfile_clear(t_msgfile *x)
{
  /* find the beginning */
  msgfile_rewind(x);

  while (x->current) {
    delete_currentnode(x);
  }
}

static void msgfile_delete(t_msgfile *x, t_symbol *s, int ac, t_atom *av)
{
  if (ac==1) {
    int pos = atom_getfloat(av);
    int oldwhere = node_wherearewe(x);

    if (pos<0) return;
    if (oldwhere > pos) oldwhere--;
    msgfile_goto(x, pos);
    delete_currentnode(x);
    msgfile_goto(x, oldwhere);
  } else if (ac==2) {
    int pos1 = atom_getfloat(av++);
    int pos2 = atom_getfloat(av);

    if ((pos1 < pos2) || (pos2 == -1)) {
      if (pos2+1) delete_region(x, pos1, pos2+1);
      else delete_region(x, pos1, -1);
    } else {
      delete_region(x, pos1+1, -1);
      delete_region(x, 0, pos2);
    }
  } else delete_currentnode(x);
}

static void msgfile_add(t_msgfile *x, t_symbol *s, int ac, t_atom *av)
{
  msgfile_end(x);
  add_currentnode(x);
  write_currentnode(x, ac, av);
}
static void msgfile_add2(t_msgfile *x, t_symbol *s, int ac, t_atom *av)
{
  msgfile_end(x);
  if (x->current) {
    if(x->current->previous) x->current = x->current->previous;
  } else {
    add_currentnode(x);
  }
  write_currentnode(x, ac, av);
  if (x->current && x->current->next) {
    x->previous= x->current;
    x->current = x->current->next;
  }
}
static void msgfile_append(t_msgfile *x, t_symbol *s, int ac, t_atom *av)
{
  add_currentnode(x);
  write_currentnode(x, ac, av);
}
static void msgfile_append2(t_msgfile *x, t_symbol *s, int ac, t_atom *av)
{
  if(!x->current)
    add_currentnode(x);

  if (x->current->thislist) write_currentnode(x, ac, av);
  else msgfile_append(x, s, ac, av);
}
static void msgfile_insert(t_msgfile *x, t_symbol *s, int ac, t_atom *av)
{
  t_msglist *cur = x->current;
  insert_currentnode(x);
  write_currentnode(x, ac, av);
  x->current = cur;
}
static void msgfile_insert2(t_msgfile *x, t_symbol *s, int ac, t_atom *av)
{
  t_msglist *cur = x->current;
  if ((x->current) && (x->current->previous)) x->current = x->current->previous;
  write_currentnode(x, ac, av);
  x->current = cur;
}

static void msgfile_set(t_msgfile *x, t_symbol *s, int ac, t_atom *av)
{
  msgfile_clear(x);
  msgfile_add(x, s, ac, av);
}

static void msgfile_replace(t_msgfile *x, t_symbol *s, int ac, t_atom *av)
{
  if(x->current) {
    if(x->current->thislist) {
      freebytes(x->current->thislist, sizeof(x->current->thislist));
    }
    x->current->thislist = 0;
    x->current->n = 0;
  } else {
    add_currentnode(x);
  }
  write_currentnode(x, ac, av);
}

static void msgfile_flush(t_msgfile *x)
{
  t_msglist *cur = x->start;
  while (cur && cur->thislist) {
    outlet_list(x->x_obj.ob_outlet, gensym("list"), cur->n, cur->thislist);
    cur = cur->next;
  }
}
static void msgfile_this(t_msgfile *x)
{
  if ((x->current) && (x->current->thislist)) {
    outlet_list(x->x_obj.ob_outlet, gensym("list"), x->current->n, x->current->thislist);
  } else {
    outlet_bang(x->x_secondout);
  }
}
static void msgfile_next(t_msgfile *x)
{
  if ((x->current) && (x->current->next)) {
    t_msglist *next = x->current->next;
    if (next->thislist)
      outlet_list(x->x_obj.ob_outlet, gensym("list"), next->n, next->thislist);
    else outlet_bang(x->x_secondout);
  } else outlet_bang(x->x_secondout);
}
static void msgfile_prev(t_msgfile *x)
{
  t_msglist*prev=0;

  if ((x->current) && (x->current->previous)) {
    prev = x->current->previous;
  } else if (x->previous) {
    prev = x->previous;
  }
  if(prev) {
    if (prev->thislist)
      outlet_list(x->x_obj.ob_outlet, gensym("list"), prev->n, prev->thislist);
    else outlet_bang(x->x_secondout);

  } else outlet_bang(x->x_secondout);
}

static void msgfile_bang(t_msgfile *x)
{ 
  if ((x->current) && (x->current->thislist)) {
    t_msglist*cur=x->current;
    x->current=cur->next;
    x->previous=cur;
    outlet_list(x->x_obj.ob_outlet, gensym("list"), cur->n, cur->thislist);
  } else {
    outlet_bang(x->x_secondout);
  }
}

static void msgfile_find(t_msgfile *x, t_symbol *s, int ac, t_atom *av)
{
  t_msglist *found = 0;
  t_msglist *cur=x->current;

  while (cur) {
    int n = cur->n;
    int equal = 1;
    t_atom *that = av;
    t_atom *this = cur->thislist;

    if(0==this){
      cur=cur->next;
      continue;
    }
    
    if (ac < n) n = ac;

    while (n-->0) {
      if ( (strcmp("*", atom_getsymbol(that)->s_name) && atomcmp(that, this)) ) {
        equal = 0;
      }

      that++;
      this++;
    }

    if (equal) {
      found = cur;
      break;
    }

    cur=cur->next;
  }

  if(found){
    x->current = found;
    x->previous= found->previous;
    outlet_float(x->x_secondout, node_wherearewe(x));
    if(found->n && found->thislist)
      outlet_list(x->x_obj.ob_outlet, gensym("list"), found->n, found->thislist);
  } else {
    outlet_bang(x->x_secondout);
  }
}

static void msgfile_where(t_msgfile *x)
{
  if (x->current && x->current->thislist) outlet_float(x->x_secondout, node_wherearewe(x));
  else outlet_bang(x->x_secondout);
}


static void msgfile_sort(t_msgfile *x, t_symbol *s0, t_symbol*s1, t_symbol*r)
{
  post("sorting not implemented yet: '%s', '%s' -> '%s'", s0->s_name, s1->s_name, r->s_name);


#if 0
  int step = argc, n;
  t_atom *atombuf = (t_atom *)getbytes(sizeof(t_atom) * argc);
  t_float *buf;
  t_int   *idx;

  int i, loops = 1;

  sort_buffer(x, argc, argv);
  buf = x->buffer;
  idx = x->indices;

  while (step > 1) {
    step = (step % 2)?(step+1)/2:step/2;

    i = loops;
    loops += 2;

    while(i--) { /* there might be some optimization in here */
      for (n=0; n<(argc-step); n++) {
        if (buf[n] > buf[n+step]) {
          t_int   i_tmp = idx[n];
          t_float f_tmp = buf[n];
          buf[n]        = buf[n+step];
          buf[n+step]   = f_tmp;
          idx[n]        = idx[n+step];
          idx[n+step]   = i_tmp;
        }
      }
    }
  }
#endif

}


/* ********************************** */
/* file I/O                           */

static void msgfile_read2(t_msgfile *x, t_symbol *filename, t_symbol *format)
{
  int rmode = 0;

  int fd=0;
  FILE*fil=NULL;
  long readlength, length, pos;
  char filnam[MAXPDSTRING];
  char buf[MAXPDSTRING], *bufptr, *readbuf;
  char *charbinbuf=NULL, *cbb;
  int charbinbuflength=0;
  char*dirname=canvas_getdir(x->x_canvas)->s_name;

  int mode = x->mode;
  char separator, eol;

  t_binbuf *bbuf = binbuf_new();

#ifdef __WIN32__
  rmode |= O_BINARY;
#endif

  if ((fd = open_via_path(dirname,
                          filename->s_name, "", buf, &bufptr, MAXPDSTRING, 0)) < 0) {

    if((fd=sys_open(filename->s_name, rmode)) < 0) {
      pd_error(x, "can't open in %s/%s",  dirname, filename->s_name);
      return;
    } else {
      sprintf(filnam, "%s", filename->s_name);
    }
  } else {
    close(fd);
    sprintf(filnam, "%s/%s", buf, bufptr);
  }

  fil=sys_fopen(filnam, "rb");
  if(fil==NULL) {
    pd_error(x, "could not open '%s'", filnam);
    return;
  }
  fseek(fil, 0, SEEK_END);
  length=ftell(fil);
  fseek(fil, 0, SEEK_SET);

  if (!(readbuf = t_getbytes(length))) {
    pd_error(x, "msgfile_read: could not reserve %ld bytes to read into", length);
    fclose(fil);
    close(fd);
    return;
  }

  if (gensym("cr")==format) {
    mode = CR_MODE;
  } else if (gensym("csv")==format) {
    mode = CSV_MODE;
  } else if (gensym("pd")==format) {
    mode = PD_MODE;
  } else if (*format->s_name)
    pd_error(x, "msgfile_read: unknown flag: %s", format->s_name);

  switch (mode) {
  case CR_MODE:
    separator = ' ';
    eol = '\n';
    break;
  case CSV_MODE:
    separator = ',';
    eol = ' ';
    break;
  default:
    separator = '\n';
    eol = ';';
    break;
  }

  /* read */
  if ((readlength = fread(readbuf, sizeof(char), length, fil)) < length) {
    pd_error(x, "msgfile_read: unable to read %s: %ld of %ld", filnam, readlength, length);
    fclose(fil);
    t_freebytes(readbuf, length);
    return;
  }

  /* close */
  fclose(fil);

  /* convert separators and eols to what pd expects in a binbuf*/
  bufptr=readbuf;

# define MSGFILE_HEADROOM 1024
  charbinbuflength=2*length+MSGFILE_HEADROOM;

  charbinbuf=(char*)getbytes(charbinbuflength);
  
  cbb=charbinbuf;
  for(pos=0; pos<charbinbuflength; pos++)charbinbuf[pos]=0;

  *cbb++=';';
  pos=1;
  while (readlength--) {
    if(pos>=charbinbuflength){
      pd_error(x, "msgfile: read error (headroom %d too small!)", MSGFILE_HEADROOM);
      goto read_error;
      break;
    }
    if (*bufptr == separator) {
      *cbb = ' ';
    } else if (*bufptr==eol) {
      *cbb++=';';pos++;
      *cbb='\n';
    } else {
      *cbb=*bufptr;
    }

    bufptr++;
    cbb++;
    pos++;
  }

  /* convert to binbuf */
  binbuf_text(bbuf, charbinbuf, charbinbuflength);
  msgfile_binbuf2listbuf(x, bbuf);

 read_error:
  binbuf_free(bbuf);
  t_freebytes(readbuf, length);
  t_freebytes(charbinbuf, charbinbuflength);
}
static void msgfile_read(t_msgfile *x, t_symbol *filename, t_symbol *format)
{
  msgfile_clear(x);
  msgfile_read2(x, filename, format);
}

static void msgfile_write(t_msgfile *x, t_symbol *filename, t_symbol *format)
{
  char buf[MAXPDSTRING];
  t_binbuf *bbuf = binbuf_new();
  t_msglist *cur = x->start;

  char *mytext = 0, *dumtext;
  int textlen = 0, i;

  char separator, eol;
  int mode = x->mode;

  FILE *f=0;

  while(cur) {
    binbuf_add(bbuf, cur->n, cur->thislist);
    binbuf_addsemi(bbuf);
    cur = cur->next;
  }
    
  canvas_makefilename(x->x_canvas, filename->s_name,
                      buf, MAXPDSTRING);

  if(format&&gensym("")!=format) {
    if(gensym("cr")==format) {
      mode = CR_MODE;
    } else if(gensym("csv")==format) {
      mode = CSV_MODE;
    } else if(gensym("pd")==format) {
      mode = PD_MODE;
    } else if(format&&format->s_name) {
      pd_error(x, "msgfile_write: ignoring unknown flag: %s", format->s_name);
    }
  }
 
  switch (mode) {
  case CR_MODE:
    separator = ' ';
    eol = ' ';
    break;
  case CSV_MODE:
    separator = ',';
    eol = ' ';
    break;
  default:
    separator = ' ';
    eol = ';';
    break;
  }
  
  binbuf_gettext(bbuf, &mytext, &textlen);
  dumtext = mytext;
  i = textlen;

  while(i--) {
    if (*dumtext==' ')
      *dumtext=separator;
    else if ((*dumtext==';') && (dumtext[1]=='\n'))
      *dumtext = eol;
    dumtext++;
  }
  
  /* open */
  if (!(f = sys_fopen(filename->s_name, "w"))) {
    pd_error(x, "msgfile : failed to open %s", filename->s_name);
  } else {
    /* write */
    if (fwrite(mytext, textlen*sizeof(char), 1, f) < 1) {
      pd_error(x, "msgfile : failed to write %s", filename->s_name);
    }
  }
  /* close */
  if (f) fclose(f);

  binbuf_free(bbuf);
}

/* ********************************** */
/* misc                               */

static void msgfile_print(t_msgfile *x)
{
  t_msglist *cur = x->start;
  int j=0;
  post("--------- msgfile contents: -----------");

  while (cur) {
    t_msglist *dum=cur;
    int i;
    j++;
    startpost("line %d:", j);
    for (i = 0; i < dum->n; i++) {
      t_atom *a = dum->thislist + i;
      postatom(1, a);
    }
    endpost();
    cur = cur->next;
  }
}

static void msgfile_help(t_msgfile *x)
{
  post("\n%c msgfile\t:: handle and store files of lists", HEARTSYMBOL);
  post("goto <n>\t: goto line <n>"
       "\nrewind\t\t: goto the beginning of the file"
       "\nend\t\t: goto the end of the file"
       "\nskip <n>\t: move relatively to current position"
       "\nbang\t\t: output current line and move forward"
       "\nprev\t\t: output previous line"
       "\nthis\t\t: output this line"
       "\nnext\t\t: output next line"
       "\nflush\t\t: output all lines");
  post("set <list>\t: clear the buffer and add <list>"
       "\nadd <list>\t: add <list> at the end of the file"
       "\nadd2 <list>\t: append <list> to the last line of the file"
       "\nappend <list>\t: append <list> at the current position"
       "\nappend2 <list>\t: append <list> to the current line"
       "\ninsert <list>\t: insert <list> at the current position"
       "\ninsert2 <list>\t: append <list> to position [current-1]"
       "\nreplace <list>\t: replace current line by <list>"
       "\ndelete [<pos> [<pos2>]]\t: delete lines or regions"
       "\nclear\t\t: delete the whole buffer");
  post("where\t\t: output current position"
       "\nfind <list>\t: search for <list>"
       "\nread <file> [<format>]\t: read <file> as <format>"
       "\nwrite <file> [<format>]\t: write <file> as <format>"
       "\n\t\t: valid <formats> are\t: PD, CR, CSV"
       "\n\nprint\t\t: show buffer (for debugging)"
       "\nhelp\t\t: show this help");
  post("creation: \"msgfile [<format>]\": <format> defines fileaccess-mode(default is PD)");
}
static void msgfile_free(t_msgfile *x)
{
  msgfile_clear(x);
  freebytes(x->current, sizeof(t_msglist));
}

static void *msgfile_new(t_symbol *s, int argc, t_atom *argv)
{
  t_msgfile *x = (t_msgfile *)pd_new(msgfile_class);

  /* an empty node indicates the end of our listbuffer */
  x->current = 0;
  x->start   = 0;
  x->previous= 0;

  x->mode=PD_MODE; /* that's the default */

  if ((argc==1) && (argv->a_type == A_SYMBOL)) {
    t_symbol*mode=atom_getsymbol(argv);
    if      (gensym("cr") == mode) x->mode = CR_MODE;
    else if (gensym("csv")== mode) x->mode = CSV_MODE;
    else if (gensym("pd") == mode) x->mode = PD_MODE;
    else {
      pd_error(x, "msgfile: unknown argument %s", argv->a_w.w_symbol->s_name);
    }
  }

  outlet_new(&x->x_obj, gensym("list"));
  x->x_secondout = outlet_new(&x->x_obj, gensym("float"));
  x->x_canvas = canvas_getcurrent();

  x->eol=' ';
  x->separator=',';

  return (x);
}

void msgfile_setup(void)
{
  msgfile_class = class_new(gensym("msgfile"), (t_newmethod)msgfile_new,
                            (t_method)msgfile_free, sizeof(t_msgfile), 0, A_GIMME, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_goto, gensym("goto"), A_DEFFLOAT, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_rewind, gensym("rewind"), 0);
  class_addmethod(msgfile_class, (t_method)msgfile_rewind, gensym("begin"), 0);
  class_addmethod(msgfile_class, (t_method)msgfile_end, gensym("end"), 0);

  class_addmethod(msgfile_class, (t_method)msgfile_next, gensym("next"), A_DEFFLOAT, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_prev, gensym("prev"), A_DEFFLOAT, 0);

  class_addmethod(msgfile_class, (t_method)msgfile_skip, gensym("skip"), A_DEFFLOAT, 0);

  class_addmethod(msgfile_class, (t_method)msgfile_set, gensym("set"), A_GIMME, 0);
  
  class_addmethod(msgfile_class, (t_method)msgfile_clear, gensym("clear"), 0);
  class_addmethod(msgfile_class, (t_method)msgfile_delete, gensym("delete"), A_GIMME, 0);
  
  class_addmethod(msgfile_class, (t_method)msgfile_add, gensym("add"), A_GIMME, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_add2, gensym("add2"), A_GIMME, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_append, gensym("append"), A_GIMME, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_append2, gensym("append2"), A_GIMME, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_insert, gensym("insert"), A_GIMME, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_insert2, gensym("insert2"), A_GIMME, 0);

  class_addmethod(msgfile_class, (t_method)msgfile_replace, gensym("replace"), A_GIMME, 0);

  class_addmethod(msgfile_class, (t_method)msgfile_find, gensym("find"), A_GIMME, 0);

  class_addmethod(msgfile_class, (t_method)msgfile_read, gensym("read"), A_SYMBOL, A_DEFSYM, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_read2, gensym("read2"), A_SYMBOL, A_DEFSYM, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_write, gensym("write"), A_SYMBOL, A_DEFSYM, 0);
  class_addmethod(msgfile_class, (t_method)msgfile_print, gensym("print"), 0);
  class_addmethod(msgfile_class, (t_method)msgfile_flush, gensym("flush"), 0);

  class_addbang(msgfile_class, msgfile_bang);
  class_addmethod(msgfile_class, (t_method)msgfile_this, gensym("this"), 0);
  class_addmethod(msgfile_class, (t_method)msgfile_where, gensym("where"), 0);


  class_addmethod(msgfile_class, (t_method)msgfile_sort, gensym("sort"), A_SYMBOL, A_SYMBOL, A_SYMBOL, 0);

  class_addmethod(msgfile_class, (t_method)msgfile_help, gensym("help"), 0);

  zexy_register("msgfile");
}
