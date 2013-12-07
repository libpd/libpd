/*
 *   Copyright 2003 Kjetil S. Matheussen.
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */


#define MAX_FUNCS 1000
#define MAX_FUNCLENGTH 50

static char k_cext_intfuncnames[MAX_FUNCLENGTH*MAX_FUNCS]={0};
static k_cext_f_int_callback k_cext_intfuncpointers[MAX_FUNCS]={0};
static t_k_cext *k_cext_intxs[MAX_FUNCS];

static char k_cext_floatfuncnames[MAX_FUNCLENGTH*MAX_FUNCS]={0};
static k_cext_f_float_callback k_cext_floatfuncpointers[MAX_FUNCS]={0};
static t_k_cext *k_cext_floatxs[MAX_FUNCS];



static int k_cext_intdummy(t_k_cext *x,...){
  post("Error. An integer k_func function has dissapeared. Returning 0 instead.\n");
  return 0;
}

static float k_cext_floatdummy(t_k_cext *x,...){
  post("Error. A float k_func function has dissapeared. Returning 0.0f instead.\n");
  return 0.0f;
}

void k_cext_setdummy(void *func){
  int lokke;
  for(lokke=0;lokke<MAX_FUNCS;lokke++){
    if(k_cext_intfuncpointers[lokke]==func){
      k_cext_intfuncpointers[lokke]=k_cext_intdummy;
      return;
    }
  }
  for(lokke=0;lokke<MAX_FUNCS;lokke++){
    if(k_cext_floatfuncpointers[lokke]==func){
      k_cext_floatfuncpointers[lokke]=k_cext_floatdummy;
      return;
    }
  }
}


static void k_cext_addintfunc(char *name,t_k_cext *x){
  int lokke;

  for(lokke=0;lokke<MAX_FUNCS;lokke++){
    if(k_cext_intfuncpointers[lokke]==NULL || !strcmp(&k_cext_intfuncnames[lokke*MAX_FUNCLENGTH],name)){
      post("---Adding ---%s--- at pos %d",name,lokke);
      sprintf(&k_cext_intfuncnames[lokke*MAX_FUNCLENGTH],"%s",name);
      k_cext_intxs[lokke]=x;
      k_cext_intfuncpointers[lokke]=(k_cext_f_int_callback)x->k_cext_process;
      return;
    }
  }
}

static void k_cext_addfloatfunc(char *name,t_k_cext *x){
  int lokke;
  for(lokke=0;lokke<MAX_FUNCS;lokke++){
    if(k_cext_floatfuncpointers[lokke]==NULL  || !strcmp(&k_cext_floatfuncnames[lokke*MAX_FUNCLENGTH],name)){
      post("---Adding ---%s--- at pos %d",name,lokke);
      sprintf(&k_cext_floatfuncnames[lokke*MAX_FUNCLENGTH],"%s",name);
      k_cext_floatxs[lokke]=x;
      k_cext_floatfuncpointers[lokke]=(k_cext_f_float_callback)x->k_cext_process;
      return;
    }
  }
}

static int k_cext_findintfromname(char *name){
  int lokke;
  for(lokke=0;lokke<MAX_FUNCS;lokke++){
    if(!strcmp(&k_cext_intfuncnames[lokke*MAX_FUNCLENGTH],name)){
      return lokke;
    }
  }
  return -1;
}
static int k_cext_findfloatfromname(char *name){
  int lokke;
  for(lokke=0;lokke<MAX_FUNCS;lokke++){
    if(!strcmp(&k_cext_floatfuncnames[lokke*MAX_FUNCLENGTH],name)){
      return lokke;
    }
  }
  return -1;
}



bool k_cext_get_int_funcs(k_cext_f_int_callback **funcs,t_k_cext **xs[],int length,...){
    int lokke;
    bool ret=true;
    va_list ap;
    va_start(ap,length);

    for(lokke=0;lokke<length;lokke++){
      char *name=va_arg(ap,char *);
      int num=k_cext_findintfromname(name);
      if(num==-1){
	post("Error, the k_func function with the name \"%s\" was not found.\n",name);
	ret=false;
	goto exit;
      }
      funcs[lokke]=&k_cext_intfuncpointers[num];
      xs[lokke]=&k_cext_intxs[lokke];
    }

 exit:
    va_end(ap);

    return ret;
}

bool k_cext_get_float_funcs(k_cext_f_float_callback **funcs,t_k_cext **xs[],int length,...){
    int lokke;
    bool ret=true;
    va_list ap;
    va_start(ap,length);

    for(lokke=0;lokke<length;lokke++){
      char *name=va_arg(ap,char *);
      int num=k_cext_findfloatfromname(name);
      if(num==-1){
	post("Error, the k_func function with the name \"%s\" was not found.\n",name);
	ret=false;
	goto exit;
      }
      funcs[lokke]=&k_cext_floatfuncpointers[num];
      xs[lokke]=&k_cext_floatxs[num];
      //post("xs[%d]=%x for %s",lokke,(unsigned int)xs[lokke],name);
    }

 exit:
    va_end(ap);
    return ret;
}


