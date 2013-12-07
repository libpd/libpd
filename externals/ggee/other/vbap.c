/* (C) Guenter Geiger <geiger@epy.co.at> */


/*
(C) Guenter Geiger


based on code by:
(c) Ville Pulkki   2.2.1999   
Helsinki University of Technology
Laboratory of Acoustics and Audio Signal Processing
*/

#include <m_pd.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include <math.h>
#include <stdio.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <string.h>   /* strtok */

#define MAX_TRIPLET_AMOUNT 64

/* this is related to the number of ls .. 

   and shoud be 3**(MAX_LS_AMOUNT / 4)

*/

#define MAX_LS_AMOUNT 16
#define MAX_DIM_SQUARE 9


/* ------------------------ vbap ----------------------------- */

static t_class *vbap_class;


typedef struct _vbap
{
     t_object x_ob;
     t_outlet* x_out2;
     t_outlet* x_out3;
     t_float lsm[MAX_TRIPLET_AMOUNT][MAX_LS_AMOUNT+1]; /* loudspeaker triplet matrices */
     int lstripl[MAX_TRIPLET_AMOUNT][3]; /* loudspeaker triplet ldspeaker numbers */
     int lasttrip[3];
     int triplet_amount;
     int dimension;
     int opened;
} t_vbap;




t_float *angle_to_cart(t_float azi, t_float ele);
void vbap(t_vbap* x,t_float g[3], int ls[3], t_float azi, t_float ele) ;
int read_ls_conf(t_vbap* x,FILE *fp );


void vbap_list(t_vbap *x,t_symbol* s,t_int argc,t_atom* argv)
{
     t_float azi,ele;
     t_float g[3];
     int ls[3];
     t_atom  a[2];

     if (argc != 2)
	  post("vbap: list message <azimuth> <elevation> required %d",argc);
     else {
	  int i;
	  azi =  atom_getfloat(argv++);
	  ele =  atom_getfloat(argv++);
	  
	  vbap(x,g,ls,azi,ele);

	  for (i=0;i<x->dimension;i++) {
	       if (x->lasttrip[i] != ls[i]) {
		    SETFLOAT(a,(t_float)x->lasttrip[i]);
		    SETFLOAT(a+1,0.0f);
		    outlet_list(x->x_ob.ob_outlet, &s_list,2 ,(t_atom*)&a);
		    x->lasttrip[i] = ls[i];
	       }
	  }

	  for (i=0;i<x->dimension;i++) {
	       SETFLOAT(a,(t_float)ls[i]);
	       SETFLOAT(a+1,g[i]);
	       outlet_list(x->x_ob.ob_outlet, &s_list,2 ,(t_atom*)&a);
	       
	  }
     }     

}


void vbap_bang(t_vbap *x)
{
     post("vbap: bang");
}



static void *vbap_new(t_symbol* s)
{
     FILE *fp;
     t_vbap *x = (t_vbap *)pd_new(vbap_class);
     char fname[MAXPDSTRING];   
 
     if (s == &s_) {
	  post("vbap: Using default loudspeaker setup");
	  s = gensym("ls_setup");
     }

     /* opening the loudspeaker matrix file*/

     canvas_makefilename(canvas_getcurrent(),s->s_name,
                         fname,MAXPDSTRING);

     if((fp=sys_fopen(fname,"r"))==NULL){
	  post("vbap: Could not open loudspeaker data file %s\n",fname);
	  x->opened = 0;
     }
     else {
	  if (!read_ls_conf(x,fp))
	       return NULL;
	  x->opened = 1;
	  fclose(fp);
     }

     x->lasttrip[0] = 0;
     x->lasttrip[1] = 1;
     x->lasttrip[2] = 2;
     
     outlet_new(&x->x_ob, &s_list);

     return (x);
}

void vbap_setup(void)
{
    vbap_class = class_new(gensym("vbap"), (t_newmethod)vbap_new, 0,
				sizeof(t_vbap), 0,A_DEFSYM,0);
/*    class_addbang(vbap_class,vbap_bang);*/
    class_addlist(vbap_class,vbap_list);
}




t_float *angle_to_cart(t_float azi, t_float ele)
{
  t_float *res;
  t_float atorad = (2 * 3.1415927 / 360) ;
  res = (t_float *) malloc(3*sizeof(t_float));
  res[0] = (float) (cos((t_float) (azi * atorad)) * cos((t_float)  (ele * atorad)));
  res[1] = (float) (sin((t_float) (azi * atorad)) * cos((t_float) (ele * atorad)));
  res[2] = (float) (sin((t_float) (ele * atorad)));
  return res;
}


void vbap(t_vbap* x,t_float g[3], int ls[3], float azi, float ele) 
{
     /* calculates gain factors using loudspeaker setup and given direction */
     t_float *cartdir;
     t_float power;
     int i,j,k;
     t_float small_g;
     t_float big_sm_g, gtmp[3];
     int winner_triplet;
     
     cartdir=angle_to_cart(azi,ele);  
     big_sm_g = -100000.0;
     for(i=0;i<x->triplet_amount;i++){
	  small_g = 10000000.0;
	  for(j=0;j<x->dimension;j++){
	       gtmp[j]=0.0;
	       for(k=0;k<x->dimension;k++)
		    gtmp[j]+=cartdir[k]*x->lsm[i][k+j*x->dimension]; 
	       if(gtmp[j] < small_g)
		    small_g = gtmp[j];
	  }
	  if(small_g > big_sm_g){
	       big_sm_g = small_g;
	       winner_triplet=i;
	       g[0]=gtmp[0]; g[1]=gtmp[1]; 
	       ls[0]=x->lstripl[i][0]; ls[1]=x->lstripl[i][1]; 
	       if(x->dimension==3){
		    g[2]=gtmp[2];
		    ls[2]=x->lstripl[i][2];
	       } else {
		    g[2]=0.0;
		    ls[2]=0;
	       }
	  }
     }
     
     /* this should probably be optimized somehow */

     power=sqrt(g[0]*g[0] + g[1]*g[1] + g[2]*g[2]);
     /*  power=g[0]+g[1];*/
     
     g[0] /= power; 
     g[1] /= power;
     g[2] /= power;


     free(cartdir);
}

int read_ls_conf(t_vbap* x,FILE *fp ){
  /* reads from specified file the loudspeaker triplet setup */
     int amount,i,j,a,b,d=0;
     char *toke;
     char c[1000];
     t_float  mx[MAX_DIM_SQUARE];
     fgets(c,1000,fp);
     toke = (char *) strtok(c, " ");
     toke = (char *) strtok(NULL, " ");
     toke = (char *) strtok(NULL, " ");
     if((toke = (char *) strtok(NULL, " "))==NULL){
	  fprintf(stderr,"Wrong ls matrix file?\n");
	  return 0;
     }
     sscanf(toke, "%d",&amount);
     toke = (char *) strtok(NULL, " ");
     toke = (char *) strtok(NULL, " ");
     if((toke = (char *) strtok(NULL, " "))==NULL){
	  fprintf(stderr,"Wrong ls matrix file?\n");
	  return 0;
     }
     sscanf(toke, "%d",&x->dimension);

     x->triplet_amount = amount;

     for(i=0;i<amount;i++){
	  fgets(c,1000,fp);
	  toke = (char *) strtok(c, " "); 
	  if(strncmp(toke,"Trip",4)!=0 && x->dimension==3){
	       fprintf(stderr,"Something wrong in ls matrix file\n");
	       return 0;
	  }
	  if(strncmp(toke,"Pair",4)!=0 && x->dimension==2){
	       fprintf(stderr,"Something wrong in ls matrix file\n");
	       return 0;
	  }
	  toke = (char *) strtok(NULL, " "); 
	  toke = (char *) strtok(NULL, " "); toke = (char *) strtok(NULL, " ");
	  sscanf(toke, "%d",&a);
	  x->lstripl[i][0]=a; 
	  toke = (char *) strtok(NULL, " ");
	  sscanf(toke, "%d",&b);
	  x->lstripl[i][1]=b; 
	  if (x->dimension==3){
	       toke = (char *) strtok(NULL, " ");
	       sscanf(toke, "%d",&d);
	       x->lstripl[i][2]=d;
	  }


	  toke = (char *) strtok(NULL, " ");
	  for(j=0;j<(x->dimension*x->dimension);j++){
	       toke = (char *) strtok(NULL, " ");
	       sscanf(toke, "%f",&(mx[j]));
	       x->lsm[i][j]=mx[j];
	  }

     }
     return 1;
}
























