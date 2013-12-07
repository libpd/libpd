/* --------------------------- k_cext  ----------------------------------- */
/*                                                                              */
/* Program c directly within a pd object. */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


#include "m_pd.h"
#include "k_cext.h"

#include <ctype.h>

static char *version = 
"k_cext v0.3.1, written by Kjetil S. Matheussen, k.s.matheussen@notam02.no\n"
"Windows-port by Olaf Matthes. Contributors: Thomas Grill and Tim Blechmann.";

int instancenumber=0; // Can not be static because of the win-port.


static t_class *k_cext_class;
static t_class *k_cfunc_class;

int k_cext_intcompare(const void *p1, const void *p2)
{
  int i = *((int *)p1);
  int j = *((int *)p2);
  
  if (i > j)
    return (1);
  if (i < j)
    return (-1);
  return (0);
}

int k_cext_floatcompare(const void *p1, const void *p2)
{
  float i = *((int *)p1);
  float j = *((int *)p2);
  
  if (i > j)
    return (1);
  if (i < j)
    return (-1);
  return (0);
}


static void k_cext_print(t_k_cext *x){
  FILE *printfile;
  char name[500];
  int lokke;
  char temp[500];

  sprintf(name,"%s.c",x->filename);

  printfile=fopen(name,"r");
  post("------------------------------------------");
  for(lokke=1;;lokke++){
    char temp2[500];
    int c;
    if((c=fgetc(printfile))==EOF) break;
    ungetc(c,printfile);
    fgets(temp,400,printfile);
    sprintf(temp2,"%3d %s",lokke,temp);
    temp2[strlen(temp2)-1]=0;
    post(temp2);
  }
  fclose(printfile);
  post("------------------------------------------");

}

static void k_cext_bang(t_k_cext *x)
{
  /* Fixme, for some reason, k_cfunc's float method points to k_cext_float
     instead of k_cfunc_float.
     As a workaround, there is this x->iscext==true test below. */
  if(x->k_cext_process!=NULL && x->iscext==true)
    (*x->k_cext_process)(x);
}


static void k_cext_float(t_k_cext *x, t_floatarg f)
{
  x->values[0]=f;

  k_cext_bang(x);
}

static void k_cfunc_float(t_k_cext *x, t_floatarg f)
{
  post("k_cfunc_float");
  x->values[0]=f;
}


#include "k_cext_funchandler.c"


struct k_cext_init{
  int indentation;
  int set_indentation[500]; // Very unlikely that anyone wants to indent more than 500 levels.
  int thisisanelifline;
  FILE *file;
  char name[500];
  char funcname[500];

  int num_intfuncs;
  int num_floatfuncs;

  char intfuncnames[50000]; // Max functionname length=50, max number of functions=1000.
  char floatfuncnames[50000];

  int doinitpos1;
  int doinitpos2;

  /* The rest is used by k_cfunc objects. */
  bool cfuncnamefound;
  char cfuncname[200];
  int cfuncrettype; //0=int, 1=float
  int numargs;
  char cfuncargtypes[50000];
  char cfuncargnames[50000];
};




/******************************/
/* Set up inlets and outlets. */
/******************************/
static void k_cext_makeinletsandoutlets(t_k_cext *x,t_int argc,t_atom* argv){
  int lokke;

  if(argc<2 || argv[1].a_type!=A_FLOAT){
    x->num_outs=0;
  }else{
    x->num_outs=atom_getfloatarg(1,argc,argv);    
    x->outlets=calloc(sizeof(t_outlet*),x->num_outs);
  }

  if(argc<1  || argv[1].a_type!=A_FLOAT){
    x->num_ins=1;
  }else{
    x->num_ins=atom_getfloatarg(0,argc,argv);    
  }

  x->inlets=calloc(sizeof(t_inlet*),x->num_ins);
  x->values=calloc(sizeof(t_float),x->num_ins);

  for(lokke=1;lokke<x->num_ins;lokke++){
    x->inlets[lokke-1] = floatinlet_new(&x->x_ob, &x->values[lokke]);
  }

  for(lokke=0;lokke<x->num_outs;lokke++){
    x->outlets[lokke] = outlet_new(&x->x_ob, gensym("float"));
  }

}


/******************************/
/* Set default values for the inlets. */
/******************************/
static int k_cext_setdefaultvalues(t_k_cext *x,t_int argc, t_atom* argv){
  int i;
  for(i=2;i<argc;i++){
    char string[500];
    switch(argv[i].a_type){
    case A_FLOAT:
      x->values[i-2]=atom_getfloatarg(i,argc,argv);
      break;
    case A_SYMBOL:
      return i;
    }
  }
  return i;
}



#include "k_cext_generatecode.c"



static void *k_cextandfunc_new(t_symbol *s, t_int argc, t_atom* argv,bool iscext)
{
  char temp[500];
  int i;

  struct k_cext_init k;

  t_k_cext *x = (t_k_cext *)pd_new(k_cext_class);
  x->iscext=iscext;

  memset(&k,0,sizeof(struct k_cext_init));

  k_cext_makeinletsandoutlets(x,argc,argv);

  if(argv[2].a_type==A_FLOAT){
    i=k_cext_setdefaultvalues(x,argc,argv);
  }else{
    if(argv[1].a_type==A_FLOAT){
      i=2;
    }else{
      if(argv[0].a_type==A_FLOAT){
	i=1;
      }else{
	i=0;
      }
    }
  }

  k_sys_mktempfilename(x->filename);
  sprintf(k.name,"%s",x->filename);
  k.name[strlen(k.name)+2]=0;
  k.name[strlen(k.name)+1]='c';
  k.name[strlen(k.name)]='.';

  k.file=fopen(k.name,"w");
  //  post("name: %s\n",name)

  k_sys_writeincludes(k.file);



  k_cext_generatecode(x,argc,argv,i,&k);



  /*************************************/
  /* Compile and link                  */
  /*************************************/

  k_sys_makecompilestring(temp,k.name,k.funcname);
  post("Compiling %s",k.name);
  system(temp);

  sprintf(k.name,"%s.o",k.name);

  if(!k_sys_getprocessfunction(x,k.funcname,k.name)){
    FILE *printfile;
    post("Error in loader!");
    x->k_cext_process=NULL;
    k_cext_print(x);
    return NULL;
  }


  if(x->iscext==false){
    if(k.cfuncrettype==0){
      k_cext_addintfunc(k.cfuncname,x);
    }else{
      k_cext_addfloatfunc(k.cfuncname,x);
    }
  }

  return (void *)x;
}

static void *k_cext_new(t_symbol *s, t_int argc, t_atom* argv){
  return k_cextandfunc_new(s,argc,argv,true);
}

static void *k_cfunc_new(t_symbol *s, t_int argc, t_atom* argv){
  return k_cextandfunc_new(s,argc,argv,false);
}



static void k_cext_free(t_k_cext *x)
{
  char temp[500];

  if(x->iscext==false){
    k_cext_setdummy(x->k_cext_process);
  }

  if(x->handle!=NULL){
    k_sys_freehandle(x);
  }

  sprintf(temp,"%s.c",x->filename);
  k_sys_deletefile(temp);

  free(x->inlets);
  free(x->outlets);
  free(x->values);
}




void k_cext_setup(void)
{

  k_sys_init();

  /* k_cext */
  k_cext_class = class_new(gensym("k_cext"), (t_newmethod)k_cext_new,
			   (t_method)k_cext_free, sizeof(t_k_cext), 0, A_GIMME, 0);
  class_addfloat(k_cext_class, k_cext_float);
  class_addbang(k_cext_class, (t_method)k_cext_bang);
  class_addmethod(k_cext_class, (t_method)k_cext_print, gensym("print"), 0);
  class_sethelpsymbol(k_cext_class, gensym("help-k_cext.pd"));

  /* k_cfunc */
  k_cfunc_class = class_new(gensym("k_cfunc"), (t_newmethod)k_cfunc_new,
			   (t_method)k_cext_free, sizeof(t_k_cext), 0, A_GIMME, 0);

  // This does not work! Why? (Have to make workaround-code in k_cext_bang)
  class_addfloat(k_cfunc_class, k_cfunc_float);
  class_addmethod(k_cfunc_class, (t_method)k_cext_print, gensym("print"), 0);
  class_sethelpsymbol(k_cfunc_class, gensym("help-k_cfunc.pd")); 

 
  post(version);
}

void k_cfunc_setup(void){
  k_cext_setup();
}

/* TB: for accessing $0 values */
t_float k_cext_getvalue(char c[])
{
    while ( isspace(c[0]) )
    {
	c++;
    }
    return (*(value_get(gensym(c))));
}

int k_cext_setvalue(char c[],float f)
{
    while ( isspace(c[0]) )
    {
	c++;
    }
    return value_setfloat(gensym(c),f);
}
