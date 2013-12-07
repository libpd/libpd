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


static char *has_intfunc(char *string){
  char *s=strstr(string,"INT_");
  if(s==NULL) return NULL;
  if(s[4]=='(') return has_intfunc(s+4);
  return s;
}

static char *has_floatfunc(char *string){
  char *s=strstr(string,"FLOAT_");
  if(s==NULL) return NULL;
  if(s[6]=='(') return has_floatfunc(s+6);
  return s;
}


static void k_cext_fixfuncs(struct k_cext_init *k,char *string){
  char newstring[500];

  sprintf(newstring,"%s",string);

  if(has_intfunc(newstring)){
    char name[500]={0};
    char *f_intstart=has_intfunc(newstring);
    char *f_namestart=f_intstart+4;
    char *f_nameend=strstr(f_namestart+1,"(");
    int lastarg=false;

    //post("namestart: -%s-",f_namestart);
    //post("nameend: -%s-",f_nameend);

    if(f_nameend==NULL){
      post("Error. k_cext needs this syntax: \"%s(\", not \"%s (\"",f_intstart,f_intstart);
      return;
    }

    if(f_nameend[1]==')') lastarg=true;

    memcpy(&k->intfuncnames[k->num_intfuncs*50],f_namestart,f_nameend-f_namestart);
    memcpy(name,f_namestart,f_nameend-f_namestart);

    f_intstart[0]=0;
    sprintf(string,
	    "%s INT_(%s,%d)(I_(%d)%s %s",
	    newstring,
	    name,
	    k->num_intfuncs,
	    k->num_intfuncs,
	    lastarg==true?"":",",
	    f_nameend+1
	    );
    k->num_intfuncs++;
  }else{
    if(has_floatfunc(newstring)){
      char name[500]={0};
      char *f_floatstart=has_floatfunc(newstring);
      char *f_namestart=f_floatstart+6;
      char *f_nameend=strstr(f_namestart+1,"(");
      int lastarg=false;
      
      //post("namestart: -%s-",f_namestart);
      //post("nameend: -%s-",f_nameend);


      if(f_nameend==NULL){
	post("Error, k_cext needs this syntax: \"%s(\", not \"%s (\"",f_floatstart,f_floatstart);
	return;
      }
    
      if(f_nameend[1]==')') lastarg=true;
      
      memcpy(&k->floatfuncnames[k->num_floatfuncs*50],f_namestart,f_nameend-f_namestart);
      memcpy(name,f_namestart,f_nameend-f_namestart);

      f_floatstart[0]=0;
      sprintf(string,
	      "%s FLOAT_(%s,%d)(F_(%d)%s %s",
	      newstring,
	      name,
	      k->num_floatfuncs,
	      k->num_floatfuncs,
	      lastarg==true?"":",",
	      f_nameend+1
	    );

      k->num_floatfuncs++;
    }else{
      return;
    }
  }

  if(has_intfunc(string) || has_floatfunc(string)){
    k_cext_fixfuncs(k,string);
  }
}


static void k_cext_gen_funcname(struct k_cext_init *k){
  sprintf(k->funcname,"k_cext_process%d",instancenumber++);

  k->doinitpos1=ftell(k->file);
  fprintf(k->file,"                         \n");
  //               static bool doinit(void);

  fprintf(k->file,"void %s(t_k_cext *x){\n",k->funcname);
  k->doinitpos2=ftell(k->file);
  fprintf(k->file,"             \n");
  //               if(doinit()){

}


static int k_cext_gen_cfunc_funcname(t_k_cext *x,int argc, t_atom* argv,int i, struct k_cext_init *k){
  char string[500];
  int lokke;

  k->doinitpos1=ftell(k->file);
  fprintf(k->file,"                         \n");

  sprintf(string,"%s",atom_getsymbolarg(i,argc,argv)->s_name);
  if(!strncmp(string,"INT_",4)){
    k->cfuncrettype=0;
    sprintf(k->cfuncname,"%s",string+4);
  }else{
    if(!strncmp(string,"FLOAT_",6)){
      k->cfuncrettype=1;
      sprintf(k->cfuncname,"%s",string+6);
    }else{
      post("k_cfunc: Error. Function name must begin with either INT_ or FLOAT_.");
      return 1;
    }
  }
  i++;

  for(;i<argc;i+=2){  
    sprintf(string,"%s",atom_getsymbolarg(i,argc,argv)->s_name);
    if(!strcmp(string,";")) goto end;
    if(!strcmp(string,"float") || !strcmp(string,"t_float")){
      sprintf(string,"double");
    }
    sprintf(&k->cfuncargtypes[k->numargs*50],"%s",string);
    sprintf(&k->cfuncargnames[k->numargs*50],"%s",atom_getsymbolarg(i+1,argc,argv)->s_name);
    k->numargs++;
  }

 end:

  fprintf(k->file,"static %s %s(t_k_cext *x%s",k->cfuncrettype==0?"int":"float",k->cfuncname,k->numargs>0?",":"){\n");
  for(lokke=0;lokke<k->numargs;lokke++){
    fprintf(k->file,"%s %s%s",&k->cfuncargtypes[lokke*50],&k->cfuncargnames[lokke*50],lokke==k->numargs-1?"){\n":",");
  }

  k->doinitpos2=ftell(k->file);
  fprintf(k->file,"             \n");

  return i;
}


static void k_cext_gen_mainfunccode(t_k_cext *x,int argc, t_atom* argv,int i, struct k_cext_init *k){
  int lokke;
  int prevwasnewline=1;

  for(;i<argc;i++){
    char string[500];
    switch(argv[i].a_type){
    case A_FLOAT:
      if((float)atom_getintarg(i,argc,argv)==atom_getfloatarg(i,argc,argv)){
	fprintf(k->file,"%d ",(int)atom_getintarg(i,argc,argv));
      }else{
	fprintf(k->file,"%f ",atom_getfloatarg(i,argc,argv));
      }
      prevwasnewline=0;
      break;
    case A_SYMBOL:
      sprintf(string,"%s",atom_getsymbolarg(i,argc,argv)->s_name);

      if(strstr(string,"s<")!=NULL){
	char *pos=strstr(string,"s<");
	pos[0]=' ';
	pos[1]='\"';
      }

      if(strstr(string,">s")!=NULL){
	char *pos=strstr(string,">s");
	pos[0]='\"';
	pos[1]=' ';
      }

      if(strstr(string,"SEND(")!=NULL){
	char *pos=strstr(string,"SEND(")+3;
	int len;
	pos[0]='(';
	pos[1]='\"';

	len=strlen(string);
	string[len]='\"';
	string[len+1]=0;
      }
      
      if(!strcmp(".",string)){
	sprintf(string," ");
	k->indentation++;

      
      }else{if(!strcmp("DO",string)){
	k->set_indentation[k->indentation]=1;
	sprintf(string,"BEGIN");
	
      }else{if(!strcmp(";",string)){
	if(prevwasnewline==1) break;
	sprintf(string,";\n");
	prevwasnewline=1;
	k->indentation=0;
	k->thisisanelifline=0;
	
      }else{if(!strcmp("ELIF",string)){
	k->thisisanelifline=1;
	
      }else{
	
	if(has_intfunc(string) || has_floatfunc(string)){
	  k_cext_fixfuncs(k,string);
	}
	
	prevwasnewline=0;
	//post("%d: -%s-",k->indentation,string);
	
	if(
	   strcmp("ELSE",string)
	   && k->thisisanelifline==0
	   )
	  {
	    int hasindented=0;
	    char orgind[500];
	    for(lokke=0;lokke<k->indentation*2;lokke++){
	      orgind[lokke]=' ';
	    }
	    orgind[lokke]=0;
	    for(lokke=499;lokke>=k->indentation;lokke--){
	      if(k->set_indentation[lokke]==1){
		k->set_indentation[lokke]=0;
		fprintf(k->file,"END\n");
		hasindented++;
	      }		    
	    }
	    if(hasindented>0){
	      char temp2[500];
	      sprintf(temp2,"%s%s",orgind,string);
	      sprintf(string,temp2);
	    }
	  }
      }
      }
      /*
	    if(!strcmp("\n",string)){
	    sprintf(string," ");
	    }
      */
      }
      }

      fprintf(k->file,"%s",string);
      if(string[strlen(string)-1]!='\n') fprintf(k->file," ");
      break;
    default:
      post("k_cext.c: Unknown argv type: %d",argv[i].a_type);
      post("Please send this patch to k->s.matheussen@notam02.no .");
      break;
    }
  }
}

static void k_cext_gen_endbrackets(struct k_cext_init *k){
  int lokke;
  for(lokke=499;lokke>=0;lokke--){
    if(k->set_indentation[lokke]==1){
      int lokke2;
      for(lokke2=0;lokke2<lokke;lokke2++){
	fprintf(k->file,"  ");
      }
      fprintf(k->file,"END\n");
    }
  }
  fprintf(k->file,"}");
}


static void k_cext_gen_funcs_dasfunc(struct k_cext_init *k){
  sprintf(k->funcname,"k_cext_process%d",instancenumber++);
  
  fprintf(k->file,"\n%s %s(t_k_cext *x%s){\n",k->cfuncrettype==0?"int":"float",k->funcname,k->numargs>0?",...":"");
  
  if(k->numargs>0){
    int lokke;
    fprintf(k->file,"  va_list k_cext_a;\n");
    for(lokke=0;lokke<k->numargs;lokke++){
      fprintf(k->file,"  %s %s;\n",&k->cfuncargtypes[lokke*50],&k->cfuncargnames[lokke*50]);
    }
    fprintf(k->file,"  va_start(k_cext_a,x);\n");
    for(lokke=0;lokke<k->numargs;lokke++){
      fprintf(k->file,"  %s=va_arg(k_cext_a,%s);\n",&k->cfuncargnames[lokke*50],&k->cfuncargtypes[lokke*50]);
    }    
    fprintf(k->file,"  va_end(k_cext_a);\n");

    fprintf(k->file,"  return %s(x,",k->cfuncname);
    for(lokke=0;lokke<k->numargs;lokke++){
      fprintf(k->file,"%s%s",&k->cfuncargnames[lokke*50],lokke==k->numargs-1?");\n}\n":",");
    }

  }else{
    fprintf(k->file,"  return %s(x);\n}\n",k->cfuncname);
  }
}


static void k_cext_gen_doinit(t_k_cext *x,struct k_cext_init *k){
  int lokke;
  if(k->num_intfuncs>0 || k->num_floatfuncs>0){
    if(x->iscext==true){
      fprintf(k->file,"}\n");
    }

    fprintf(k->file,
	    "static bool doinit(void){\n"
	    "  static bool k_cext_inited=false;\n"
	    "  if(k_cext_inited==false){\n"
	    );
    if(k->num_intfuncs>0){
      fprintf(k->file,"    if(k_cext_get_int_funcs(&k_cext_int_funcs[0],&k_cext_int_x[0],%d,",k->num_intfuncs);
      for(lokke=0;lokke<k->num_intfuncs;lokke++){
	fprintf(k->file,"\"%s\"%s",k->intfuncnames+(lokke*50),lokke<k->num_intfuncs-1?",":"");
      }
      fprintf(k->file,")==false) return false;\n");
    }

    if(k->num_floatfuncs>0){
      fprintf(k->file,"    if(k_cext_get_float_funcs(&k_cext_float_funcs[0],&k_cext_float_x[0],%d,",k->num_floatfuncs);
      for(lokke=0;lokke<k->num_floatfuncs;lokke++){
	fprintf(k->file,"\"%s\"%s",k->floatfuncnames+(lokke*50),lokke<k->num_floatfuncs-1?",":"");
      }
      fprintf(k->file,")==false) return false;\n");
    }

    fprintf(k->file,
	    "    k_cext_inited=true;\n"
	    "  }\n"
	    "  return true;\n"
	    "}\n"
	    );

    if(k->num_intfuncs>0){
      fprintf(k->file,
#ifndef _MSC_VER
	      "static "
#endif
	      "k_cext_f_int_callback *k_cext_int_funcs[%d]={0};\n"
#ifndef _MSC_VER
	      "static "
#endif
	      "t_k_cext **k_cext_int_x[%d]={0};\n",
	      k->num_intfuncs,k->num_intfuncs
	      );
    }
    
    if(k->num_floatfuncs>0){
      fprintf(k->file,
#ifndef _MSC_VER
	      "static "
#endif
	      "k_cext_f_float_callback *k_cext_float_funcs[%d]={0};\n"
#ifndef _MSC_VER
	      "static "
#endif
	      "t_k_cext **k_cext_float_x[%d]={0};\n",
	      k->num_floatfuncs,k->num_floatfuncs
	      );
    }

    fseek(k->file,k->doinitpos1,SEEK_SET);
    fprintf(k->file,"static bool doinit(void);");

    fseek(k->file,k->doinitpos2,SEEK_SET);
    fprintf(k->file,"if(doinit()){");
  }else{
    fprintf(k->file,"\n");
  }

}

static void k_cext_generatecode(t_k_cext *x,int argc, t_atom* argv,int i, struct k_cext_init *k){

  if(x->iscext==true){
    k_cext_gen_funcname(k);
  }else{
    i=k_cext_gen_cfunc_funcname(x,argc,argv,i,k);
  }

  k_cext_gen_mainfunccode(x,argc,argv,i,k);
  k_cext_gen_endbrackets(k);

  if(x->iscext==false){
    if(k->num_intfuncs>0 || k->num_floatfuncs>0){    
      fprintf(k->file,"return 0;}\n");
    }
    k_cext_gen_funcs_dasfunc(k);
  }

  k_cext_gen_doinit(x,k);

  fclose(k->file);
}
