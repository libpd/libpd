/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* sampling */

#include "iem16_table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>      /* for read/write to files */

#if (defined(_MSC_VER) && (_MSC_VER > 600))
# define fdopen(fd,type) _fdopen(fd,type)
#endif


static int am_bigendian(void){
    /* actually this should be in m_pd.h */
    unsigned short s = 1;
    unsigned char c = *(char *)(&s);
    return (c==0);
}


static void table16_const(t_table16*x, t_float f);

static void *table16_new(t_symbol *s, t_float f){
  t_table16 *x = (t_table16*)pd_new(table16_class);
  int i=f;
  if(i<1)i=100;
  x->x_tablename=s;
  x->x_size=i;
  x->x_table=getbytes(x->x_size*sizeof(t_iem16_16bit));
  x->x_usedindsp=0;
  pd_bind(&x->x_obj.ob_pd, x->x_tablename);
  x->x_canvas = canvas_getcurrent();

  table16_const(x, 0);
  return(x);
}

static void table16_free(t_table16 *x){
  if(x->x_table)freebytes(x->x_table, x->x_size*sizeof(t_iem16_16bit));
  pd_unbind(&x->x_obj.ob_pd, x->x_tablename);
}

int table16_getarray16(t_table16*x, int*size,t_iem16_16bit**vec){
  *size=x->x_size;
  *vec =x->x_table;
  return 1;
}
void table16_usedindsp(t_table16*x){
  x->x_usedindsp=1;
}
static void table16_resize(t_table16*x, t_float f){
  int i=f;
  int was=x->x_size;
  if (i<1){
    error("can only resize to sizes >0");
    return;
  }
  x->x_table=resizebytes(x->x_table, was*sizeof(t_iem16_16bit), i*sizeof(t_iem16_16bit));
  if(i>was)memset(x->x_table+was, 0, (i-was)*sizeof(t_iem16_16bit));
  x->x_size  =i;
  if (x->x_usedindsp) canvas_update_dsp();
}

static void table16_const(t_table16*x, t_float f){
  t_iem16_16bit s = (t_iem16_16bit)f;
  int i = x->x_size;
  t_iem16_16bit*buf=x->x_table;
  while(i--)*buf++=s;
}


static void table16_from(t_table16*x, t_symbol*s, int argc, t_atom*argv){
  float scale=IEM16_SCALE_UP;
  int resize=0;
  int startfrom=0, startto=0, endfrom=0, endto=x->x_size;
  t_garray *a=0;
  int npoints;
  t_float *vec=(0), *src=(0);
  t_iem16_16bit   *dest;

  int i,length=0;

  if(argc<1 || argv->a_type!=A_SYMBOL){
    error("you have to specify the from-table !");
    return;
  }
  s=atom_getsymbol(argv);  argc--;argv++;
#if defined __WIN32__ || defined __WIN32
  // hmm, how do i import garray_class on w32??
  error("ack, windows hit the wall...tell me how to see garray_class");
  return;
#else
  if (!(a = (t_garray *)pd_findbyclass(s, garray_class))){
    error("%s: no such array", s->s_name);
    return;
  } else 
#endif
  if (!garray_getfloatarray(a, &npoints, &vec)){
    error("%s: bad template for tabread4", s->s_name);
    return;
  }

  if(argc>0 && atom_getsymbol(argv+argc-1)==gensym("resize")){
    resize=1;
    argc--;
  }
  endfrom=npoints;

  switch(argc){
  case 0:break;
  case 4:
    endto    =atom_getfloat(argv+3);
  case 3:
    startto  =atom_getfloat(argv+2);
  case 2:
    endfrom  =atom_getfloat(argv+1);
  case 1:
    startfrom=atom_getfloat(argv);
    break;
  default:
    error("table16: from <tablename> [<startfrom> [<endfrom> [<startto> [<endto>]]]] [resize]");
    return;
  }
  if(startfrom<0)startfrom=0;
  if  (startto<0)startto=0;
  if(endfrom<=startfrom)return;
  if(endto  <=startto)  return;

  length=endfrom-startfrom;
  if(resize){
    if(x->x_size < (startto+length))table16_resize(x, startto+length);    
  } else{
    if(x->x_size < (startto+length))length=x->x_size-startto;
  }
  endfrom=startfrom+length;
  endto  =startto+length;

  dest=x->x_table+startto;
  src =vec+startfrom;
  i=length;
  while(i--)*dest++=(*src++)*scale;
  //post("from %s (%d, %d) --> (%d, %d)\tresize=%s", s->s_name, startfrom, endfrom, startto, endto, (resize)?"yes":"no");
}

#define BINREADMODE "rb"
#define BINWRITEMODE "wb"
static void table16_read16(t_table16 *x, t_symbol *filename,  t_symbol *endian, t_floatarg fskip)
{
    int skip = fskip, filedesc;
    int i, nelem;
    t_iem16_16bit *vec;
    FILE *fd;
    char buf[MAXPDSTRING], *bufptr;
    short s;
    int cpubig = am_bigendian(), swap = 0;
    char c = endian->s_name[0];
    if (c == 'b')
    {
        if (!cpubig) swap = 1;
    }
    else if (c == 'l')
    {
        if (cpubig) swap = 1;
    }
    else if (c)
    {
        error("array_read16: endianness is 'l' (low byte first ala INTEL)");
        post("... or 'b' (high byte first ala MIPS,DEC,PPC)");
    }
    if (!table16_getarray16(x, &nelem, &vec))
    {
        error("%s: not a 16bit array", x->x_tablename->s_name);
        return;
    }
    if ((filedesc = open_via_path(
        canvas_getdir(x->x_canvas)->s_name,
            filename->s_name, "", buf, &bufptr, MAXPDSTRING, 1)) < 0 
                || !(fd = fdopen(filedesc, BINREADMODE))
	)
    {
        error("%s: can't open", filename->s_name);
        return;
    }
    if (skip)
    {
        long pos = fseek(fd, (long)skip, SEEK_SET);
        if (pos < 0)
        {
            error("%s: can't seek to byte %d", buf, skip);
            fclose(fd);
            return;
        }
    }

    for (i = 0; i < nelem; i++)
    {
        if (fread(&s, sizeof(s), 1, fd) < 1)
        {
            post("%s: read %d elements into table of size %d",
                filename->s_name, i, nelem);
            break;
        }
        if (swap) s = ((s & 0xff) << 8) | ((s & 0xff00) >> 8);
        vec[i] = s;
    }
    while (i < nelem) vec[i++] = 0;
    fclose(fd);
}

 
void table16_setup(void){
  table16_class = class_new(gensym("table16"),
			    (t_newmethod)table16_new, (t_method)table16_free,
			    sizeof(t_table16), 0, A_DEFSYM, A_DEFFLOAT, 0);
  class_addmethod(table16_class, (t_method)table16_resize, gensym("resize"), A_DEFFLOAT, 0);
  class_addmethod(table16_class, (t_method)table16_const, gensym("const"), A_DEFFLOAT, 0);
  class_addmethod(table16_class, (t_method)table16_from, gensym("from"), A_GIMME, 0);
  class_addmethod(table16_class, (t_method)table16_read16, gensym("read16"),  A_SYMBOL, 
                  A_DEFFLOAT, A_DEFSYM, 0);
}


void iem16_table_setup(void)
{
    table16_setup();
}

