/* vbap.c 

written by Ville Pulkki 1999-2003
Helsinki University of Technology 
and 
University of California at Berkeley 

See copyright in file with name LICENSE.txt  */

// Indicate that we are within VBAP object (specific to include define_loudspeakers content within vbap)
#define VBAP_OBJECT

#include "vbap.h"
#include "s_stuff.h"

// Function prototypes
static void new_spread_dir(t_vbap *x, t_float spreaddir[3], t_float vscartdir[3], t_float spread_base[3]);
static void new_spread_base(t_vbap *x, t_float spreaddir[3], t_float vscartdir[3]);
static void *vbap_class;				
static void vect_cross_prod(t_float v1[3], t_float v2[3],t_float v3[3]);
static void additive_vbap(t_float *final_gs, t_float cartdir[3], t_vbap *x);
static void vbap_bang(t_vbap *x);
static void vbap_matrix(t_vbap *x, Symbol *s, int ac, Atom *av);
#ifndef PD /* Max */
/* these are for getting data from a cold inlet on Max/MSP, in Pd you use t_floatinlet_new() in new() */
void vbap_ft1(t_vbap *x, double n);
void vbap_ft2(t_vbap *x, double n);
void vbap_in3(t_vbap *x, long n);
void vbap_ft4(t_vbap *x, double g);
#endif
static void spread_it(t_vbap *x, t_float *final_gs);
static void *vbap_new(t_float azi, t_float ele, t_float spread);
static void vbap(t_float g[3], long ls[3], t_vbap *x);
static void angle_to_cart(t_float azi, t_float ele, t_float res[3]);
static void cart_to_angle(t_float cvec[3], t_float avec[3]);

/*****************************************************
	 INCLUDE ALL define_loudspeakers functions directly into VBAP
******************************************************/
#include "define_loudspeakers.c"

/*****************************************************
	 Max Object Assist
******************************************************/
#ifndef PD /* Max */
void vbap_assist(t_vbap *x, void *b, long m, long a, char *s)
{
	char*mess = "unknown";
	if (m == ASSIST_INLET)
	{
		switch(a)
		{
			case 0 : mess = "bang to calc and output vbap gains. loudspeakers definition"; break;
			case 1 : mess = "panning angle azimuth"; break;
			case 2 : mess = "panning angle elevation"; break;
			case 3 : mess = "spread amount"; break;
			case 4 : mess = "gain control"; break;
		}
	}
	else
	{
		switch(a)
		{
			case 0 : mess = "vbap gains"; break;
			case 1 : mess = "panning angle azimuth"; break;
			case 2 : mess = "panning angle elevation"; break;
			case 3 : mess = "spread amount"; break;
			case 4 : mess = "gain control"; break;
		}
	}
	sprintf(s,mess);
}
#endif /* Max */

/* above are the prototypes for the methods/procedures/functions you will use */
/*--------------------------------------------------------------------------*/
#ifdef PD
void vbap_setup(void)
{
	vbap_class = class_new(gensym("vbap"), (t_newmethod)vbap_new, 0, (short)sizeof(t_vbap), 0, 
                           A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0); 

	class_addbang(vbap_class, (t_method)vbap_bang);	
/* these are for getting data from a cold inlet on Max/MSP, in Pd you use floatinlet_new() in new()
	addftx((t_method)vbap_ft1, 1);
	addftx((t_method)vbap_ft2, 2);
	addftx((t_method)vbap_in3, 3);
	addftx((t_method)vbap_ft4, 4);
*/
	class_addmethod(vbap_class, (t_method)vbap_matrix, gensym("loudspeaker-matrices"), A_GIMME, 0);

	// define_loudspeaker messages
    class_addmethod(vbap_class, (t_method)vbap_def_ls, gensym("define-loudspeakers"), A_GIMME, 0);
    class_addmethod(vbap_class, (t_method)vbap_def_ls, gensym("define_loudspeakers"), A_GIMME, 0);
    class_addmethod(vbap_class, (t_method)def_ls_read_directions, gensym("ls-directions"), A_GIMME, 0);	
    class_addmethod(vbap_class, (t_method)def_ls_read_triplets, gensym("ls-triplets"), A_GIMME, 0);

	logpost(NULL, 4, VBAP_VERSION);
}
#else /* MAX */
void main(void)
{
	setup((t_messlist **)&vbap_class, (method)vbap_new, 0L, (short)sizeof(t_vbap), 0L, 
          A_DEFLONG,A_DEFLONG,A_DEFLONG, 0); 

	addbang((method)vbap_bang);	
	addftx((method)vbap_ft1, 1);
	addftx((method)vbap_ft2, 2);
	addftx((method)vbap_in3, 3);
	addftx((method)vbap_ft4, 4);
	addmess((method)vbap_matrix, "loudspeaker-matrices", A_GIMME, 0);
	addmess((method)traces, "enabletrace", A_LONG, 0);

	// define_loudspeaker messages
	addmess((method)vbap_def_ls, "define-loudspeakers", A_GIMME, 0);
	addmess((method)vbap_def_ls, "define_loudspeakers", A_GIMME, 0);
	addmess((method)def_ls_read_directions, "ls-directions", A_GIMME, 0);	
	addmess((method)def_ls_read_triplets, "ls-triplets", A_GIMME, 0);

	addmess((method)vbap_assist,"assist",A_CANT,0);

	post(VBAP_VERSION);
}

/* these are for getting data from a cold inlet on Max/MSP, in Pd you use floatinlet_new() in new() */
/*--------------------------------------------------------------------------*/
// panning angle azimuth
void vbap_ft1(t_vbap *x, double n) { x->x_azi = (float) n; }
/*--------------------------------------------------------------------------*/
// panning angle elevation
void vbap_ft2(t_vbap *x, double n) { x->x_ele = (float) n; }
/*--------------------------------------------------------------------------*/
// spread amount
void vbap_in3(t_vbap *x, long n) { x->x_spread = (n<0) ? 0 : (n>100) ? 100 : n; }
/*--------------------------------------------------------------------------*/
// gain control
void vbap_ft4(t_vbap *x, double g) { x->x_gain = g; }
#endif /* MAX */

/*--------------------------------------------------------------------------*/
// create new instance of object... 
static void *vbap_new(t_float azi, t_float ele, t_float spread)
{
#ifdef PD
	t_vbap *x = (t_vbap *)newobject(vbap_class);

	floatinlet_new(&x->x_obj, &x->x_azi);
	floatinlet_new(&x->x_obj, &x->x_ele);
	floatinlet_new(&x->x_obj, &x->x_spread);

	x->x_outlet0 = outlet_new(&x->x_obj, &s_float);
	x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
	x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
	x->x_outlet3 = outlet_new(&x->x_obj, &s_float);
#else /* Max */
	t_vbap *x = (t_vbap *)newobject(vbap_class);

	floatin(x,4);	
	floatin(x,3);	
	floatin(x,2);					
	floatin(x,1);					

	x->x_outlet4 = floatout(x);
	x->x_outlet3 = floatout(x);
	x->x_outlet2 = floatout(x);
	x->x_outlet1 = floatout(x);
	x->x_outlet0 = listout(x);
#endif /* PD */
	
	x->x_spread_base[0] = 0.0;
	x->x_spread_base[1] = 1.0;
	x->x_spread_base[2] = 0.0;
	x->x_lsset_available =0;

	x->x_azi = azi;
	x->x_ele = ele;
	x->x_spread = spread;

	return(x);					/* return a reference to the object instance */
}


static void angle_to_cart(t_float azi, t_float ele, t_float res[3])
// converts angular coordinates to cartesian
{ 
  res[0] = cos(azi * atorad) * cos( ele * atorad);
  res[1] = sin( azi * atorad) * cos( ele * atorad);
  res[2] = sin( ele * atorad);
}

static void cart_to_angle(t_float cvec[3], t_float avec[3])
// converts cartesian coordinates to angular
{
  //float tmp, tmp2, tmp3, tmp4;
  //float power;
  t_float dist, atan_y_per_x, atan_x_pl_y_per_z;
  t_float azi, ele;
  
  if(cvec[0]==0.0)
  	atan_y_per_x = M_PI / 2;
  else
    atan_y_per_x = atan(cvec[1] / cvec[0]);
  azi = atan_y_per_x / atorad;
  if(cvec[0]<0.0)
    azi +=180.0;
  dist = sqrt(cvec[0]*cvec[0] + cvec[1]*cvec[1]);
  if(cvec[2]==0.0)
    atan_x_pl_y_per_z = 0.0;
  else
    atan_x_pl_y_per_z = atan(cvec[2] / dist);
  if(dist == 0.0)
	{
    if(cvec[2]<0.0)
      atan_x_pl_y_per_z = -M_PI/2.0;
    else
      atan_x_pl_y_per_z = M_PI/2.0;
	}
  ele = atan_x_pl_y_per_z / atorad;
  dist = sqrtf(cvec[0] * cvec[0] +cvec[1] * cvec[1] +cvec[2]*cvec[2]);
  avec[0]=azi;
  avec[1]=ele;
  avec[2]=dist;
}


static void vbap(t_float g[3], long ls[3], t_vbap *x)
{
  /* calculates gain factors using loudspeaker setup and given direction */
  t_float power;
  int i,j,k, gains_modified;
  t_float small_g;
  t_float big_sm_g, gtmp[3];
  long winner_set = 0;
  t_float cartdir[3];
  t_float new_cartdir[3];
  t_float new_angle_dir[3];
  long dim = x->x_dimension;
  long neg_g_am, best_neg_g_am;
  
  // transfering the azimuth angle to a decent value
  if(x->x_azi > 360.0 || x->x_azi < -360.0)
    x->x_azi = fmod(x->x_azi, 360.0);
  if(x->x_azi > 180.0)
    x->x_azi -= 360.0;
  if(x->x_azi < -179.0)
    x->x_azi += 360.0;

  	
  // transferring the elevation to a decent value
  if(dim == 3){
    if(x->x_ele > 360.0 || x->x_ele < -360.0)
      x->x_ele = fmod(x->x_ele, 360.0);
    if(x->x_ele > 180.0)
      x->x_ele -= 360.0;
    if(x->x_ele < -179.0)
  	  x->x_ele += 360.0;
  } else
  	x->x_ele = 0.0;
  
  
  // go through all defined loudspeaker sets and find the set which
  // has all positive values. If such is not found, set with largest
  // minimum value is chosen. If at least one of gain factors of one LS set is negative
  // it means that the virtual source does not lie in that LS set. 
  
  angle_to_cart(x->x_azi,x->x_ele,cartdir);
  big_sm_g = -100000.0;   // initial value for largest minimum gain value
  best_neg_g_am=3; 		  // how many negative values in this set
  
  for(i=0;i<x->x_lsset_amount;i++)
	{
    small_g = 10000000.0;
    neg_g_am = 3;
    for(j=0;j<dim;j++)
		{
      gtmp[j]=0.0;
      for(k=0;k<dim;k++)
        gtmp[j]+=cartdir[k]* x->x_set_inv_matx[i][k+j*dim];
      if(gtmp[j] < small_g)
        small_g = gtmp[j];
      if(gtmp[j]>= -0.01)
      	neg_g_am--;
    }
    if(small_g > big_sm_g && neg_g_am <= best_neg_g_am)
		{
      big_sm_g = small_g;
      best_neg_g_am = neg_g_am; 
      winner_set=i;
      g[0]=gtmp[0]; g[1]=gtmp[1];
      ls[0]= x->x_lsset[i][0]; ls[1]= x->x_lsset[i][1];
      if(dim==3)
			{
      	g[2]=gtmp[2];
        ls[2]= x->x_lsset[i][2];
      } 
			else 
			{
        g[2]=0.0;
        ls[2]=0;
      }
    }
  }
  
  // If chosen set produced a negative value, make it zero and
  // calculate direction that corresponds  to these new
  // gain values. This happens when the virtual source is outside of
  // all loudspeaker sets. 
  
  //
  	gains_modified=0;
  	for(i=0;i<dim;i++)
  		if(g[i]<-0.01){
  			g[i]=0.0001;
  			gains_modified=1;
  		}	
 	if(gains_modified==1){
 	 	new_cartdir[0] =  x->x_set_matx[winner_set][0] * g[0] 
 	 					+ x->x_set_matx[winner_set][1] * g[1]
 	 					+ x->x_set_matx[winner_set][2] * g[2];
 	 	new_cartdir[1] =  x->x_set_matx[winner_set][3] * g[0] 
 	 					+ x->x_set_matx[winner_set][4] * g[1] 
 	 					+ x->x_set_matx[winner_set][5] * g[2];
 	 	if(dim==3){
 	 		new_cartdir[2] =  x->x_set_matx[winner_set][6] * g[0] 
 	 						+ x->x_set_matx[winner_set][7] * g[1]
 	 						+ x->x_set_matx[winner_set][8] * g[2];
 	 	} else new_cartdir[2] = 0;
 	 	cart_to_angle(new_cartdir,new_angle_dir);
 	 	x->x_azi = (new_angle_dir[0] );
		post("[vbap] use azimuth %g",x->x_azi );
 	 	x->x_ele = (new_angle_dir[1]);
 	 }
  //}
  
  power=sqrt(g[0]*g[0] + g[1]*g[1] + g[2]*g[2]);
  g[0] /= power;
  g[1] /= power;
  g[2] /= power;
}


static void vect_cross_prod(t_float v1[3], t_float v2[3],
                t_float v3[3]) 
// vector cross product            
{
  t_float length;
  v3[0] = (v1[1] * v2[2] ) - (v1[2] * v2[1]);
  v3[1] = (v1[2] * v2[0] ) - (v1[0] * v2[2]);
  v3[2] = (v1[0] * v2[1] ) - (v1[1] * v2[0]);

  length= sqrt(v3[0]*v3[0] + v3[1]*v3[1] + v3[2]*v3[2]);
  v3[0] /= length;
  v3[1] /= length;
  v3[2] /= length;
}

static void additive_vbap(t_float *final_gs, t_float cartdir[3], t_vbap *x)
// calculates gains to be added to previous gains, used in
// multiple direction panning (source spreading)
{
	t_float power;
    int i,j,k, gains_modified;
  	t_float small_g;
  	t_float big_sm_g, gtmp[3];
  	long winner_set;
  	//float new_cartdir[3];
  	//float new_angle_dir[3];
  	long dim = x->x_dimension;
  	long neg_g_am, best_neg_g_am;
	t_float g[3] = {0,0,0};
	long ls[3] = {0,0,0};
	
  	big_sm_g = -100000.0;
  	best_neg_g_am=3;
  
  	for(i=0;i<x->x_lsset_amount;i++){
  	  small_g = 10000000.0;
  	  neg_g_am = 3;
  	  for(j=0;j<dim;j++){
  	    gtmp[j]=0.0;
  	    for(k=0;k<dim;k++)
  	      gtmp[j]+=cartdir[k]* x->x_set_inv_matx[i][k+j*dim];
  	    if(gtmp[j] < small_g)
  	      small_g = gtmp[j];
  	    if(gtmp[j]>= -0.01)
  	    	neg_g_am--;
    	}
    	if(small_g > big_sm_g && neg_g_am <= best_neg_g_am){
      	big_sm_g = small_g;
      	best_neg_g_am = neg_g_am; 
      	winner_set=i;
      	g[0]=gtmp[0]; g[1]=gtmp[1];
      	ls[0]= x->x_lsset[i][0]; ls[1]= x->x_lsset[i][1];
      	if(dim==3){
      		g[2]=gtmp[2];
      	  	ls[2]= x->x_lsset[i][2];
      	} else {
      	  	g[2]=0.0;
      	  	ls[2]=0;
      	}
    	}
  	}

  	gains_modified=0;
  	for(i=0;i<dim;i++)
  		if(g[i]<-0.01){
  			gains_modified=1;
  		}
  
  	if(gains_modified != 1){
  		power=sqrt(g[0]*g[0] + g[1]*g[1] + g[2]*g[2]);
  		g[0] /= power;
  		g[1] /= power;
  		g[2] /= power;
  		
  		final_gs[ls[0]-1] += g[0];
  		final_gs[ls[1]-1] += g[1];
  		if (dim==3)
  			final_gs[ls[2]-1] += g[2];
  	}
}


static void new_spread_dir(t_vbap *x, t_float spreaddir[3], t_float vscartdir[3], t_float spread_base[3])
// subroutine for spreading
{
	t_float beta,gamma;
	t_float a,b;
	t_float power;
	
	gamma = acos(vscartdir[0] * spread_base[0] +
					vscartdir[1] * spread_base[1] +
					vscartdir[2] * spread_base[2])/M_PI*180;
	if(fabs(gamma) < 1){
		angle_to_cart(x->x_azi+90.0, 0, spread_base);
		gamma = acos(vscartdir[0] * spread_base[0] +
					vscartdir[1] * spread_base[1] +
					vscartdir[2] * spread_base[2])/M_PI*180;
	}
	beta = 180 - gamma;
	b=sin(x->x_spread * M_PI / 180) / sin(beta * M_PI / 180);
	a=sin((180- x->x_spread - beta) * M_PI / 180) / sin (beta * M_PI / 180);
	spreaddir[0] = a * vscartdir[0] + b * spread_base[0];
	spreaddir[1] = a * vscartdir[1] + b * spread_base[1];
	spreaddir[2] = a * vscartdir[2] + b * spread_base[2];
	
	power=sqrt(spreaddir[0]*spreaddir[0] + spreaddir[1]*spreaddir[1] 
				+ spreaddir[2]*spreaddir[2]);
  	spreaddir[0] /= power;
  	spreaddir[1] /= power;
  	spreaddir[2] /= power;
}

static void new_spread_base(t_vbap *x, t_float spreaddir[3], t_float vscartdir[3])
// subroutine for spreading
{
	t_float d;
	t_float power;
	
	d = cos(x->x_spread/180*M_PI);
	x->x_spread_base[0] = spreaddir[0] - d * vscartdir[0];
	x->x_spread_base[1] = spreaddir[1] - d * vscartdir[1];
	x->x_spread_base[2] = spreaddir[2] - d * vscartdir[2];
	power=sqrt(x->x_spread_base[0]*x->x_spread_base[0] + x->x_spread_base[1]*x->x_spread_base[1] 
				+ x->x_spread_base[2]*x->x_spread_base[2]);
  	x->x_spread_base[0] /= power;
  	x->x_spread_base[1] /= power;
  	x->x_spread_base[2] /= power;
}

static void spread_it(t_vbap *x, t_float *final_gs)
// apply the sound signal to multiple panning directions
// that causes some spreading.
// See theory in paper V. Pulkki "Uniform spreading of amplitude panned
// virtual sources" in WASPAA 99

{
	t_float vscartdir[3];
	t_float spreaddir[16][3];
	t_float spreadbase[16][3];
	long i, spreaddirnum;
	t_float power;
	if(x->x_dimension == 3){
		spreaddirnum=16;
		angle_to_cart(x->x_azi,x->x_ele,vscartdir);
		new_spread_dir(x, spreaddir[0], vscartdir, x->x_spread_base);
		new_spread_base(x, spreaddir[0], vscartdir);
		vect_cross_prod(x->x_spread_base, vscartdir, spreadbase[1]); // four orthogonal dirs
		vect_cross_prod(spreadbase[1], vscartdir, spreadbase[2]);
		vect_cross_prod(spreadbase[2], vscartdir, spreadbase[3]);
	
		// four between them
		for(i=0;i<3;i++) spreadbase[4][i] =  (x->x_spread_base[i] + spreadbase[1][i]) / 2.0;
		for(i=0;i<3;i++) spreadbase[5][i] =  (spreadbase[1][i] + spreadbase[2][i]) / 2.0;
		for(i=0;i<3;i++) spreadbase[6][i] =  (spreadbase[2][i] + spreadbase[3][i]) / 2.0;
		for(i=0;i<3;i++) spreadbase[7][i] =  (spreadbase[3][i] + x->x_spread_base[i]) / 2.0;
		
		// four at half spreadangle
		for(i=0;i<3;i++) spreadbase[8][i] =  (vscartdir[i] + x->x_spread_base[i]) / 2.0;
		for(i=0;i<3;i++) spreadbase[9][i] =  (vscartdir[i] + spreadbase[1][i]) / 2.0;
		for(i=0;i<3;i++) spreadbase[10][i] =  (vscartdir[i] + spreadbase[2][i]) / 2.0;
		for(i=0;i<3;i++) spreadbase[11][i] =  (vscartdir[i] + spreadbase[3][i]) / 2.0;
		
		// four at quarter spreadangle
		for(i=0;i<3;i++) spreadbase[12][i] =  (vscartdir[i] + spreadbase[8][i]) / 2.0;
		for(i=0;i<3;i++) spreadbase[13][i] =  (vscartdir[i] + spreadbase[9][i]) / 2.0;
		for(i=0;i<3;i++) spreadbase[14][i] =  (vscartdir[i] + spreadbase[10][i]) / 2.0;
		for(i=0;i<3;i++) spreadbase[15][i] =  (vscartdir[i] + spreadbase[11][i]) / 2.0;
		
		additive_vbap(final_gs,spreaddir[0],x); 
		for(i=1;i<spreaddirnum;i++){
			new_spread_dir(x, spreaddir[i], vscartdir, spreadbase[i]);
			additive_vbap(final_gs,spreaddir[i],x); 
		}
	} else if (x->x_dimension == 2) {
		spreaddirnum=6;		
		
		angle_to_cart(x->x_azi - x->x_spread, 0, spreaddir[0]);
		angle_to_cart(x->x_azi - x->x_spread/2, 0, spreaddir[1]);
		angle_to_cart(x->x_azi - x->x_spread/4, 0, spreaddir[2]);
		angle_to_cart(x->x_azi + x->x_spread/4, 0, spreaddir[3]);
		angle_to_cart(x->x_azi + x->x_spread/2, 0, spreaddir[4]);
		angle_to_cart(x->x_azi + x->x_spread, 0, spreaddir[5]);
		
		for(i=0;i<spreaddirnum;i++)
			additive_vbap(final_gs,spreaddir[i],x); 
	} else
		return;
		
	if(x->x_spread > 70)
		for(i=0;i<x->x_ls_amount;i++){
			final_gs[i] += (x->x_spread - 70) / 30.0 * (x->x_spread - 70) / 30.0 * 10.0;
		}
	
	for(i=0,power=0.0;i<x->x_ls_amount;i++){
		power += final_gs[i] * final_gs[i];
	}
		
	power = sqrt(power);
	for(i=0;i<x->x_ls_amount;i++){
		final_gs[i] /= power;
	}
}	
	

static void vbap_bang(t_vbap *x)			
// top level, vbap gains are calculated and outputted	
{
	Atom at[MAX_LS_AMOUNT]; 
	t_float g[3];
	long ls[3];
	long i;
	t_float *final_gs = (t_float *) getbytes(x->x_ls_amount * sizeof(t_float));

	if(x->x_lsset_available ==1)
	{
		vbap(g,ls, x);
		for(i=0;i<x->x_ls_amount;i++)
			final_gs[i]=0.0; 			
		for(i=0;i<x->x_dimension;i++)
		{
			final_gs[ls[i]-1]=g[i];  
		}
		if(x->x_spread != 0)
		{
			spread_it(x,final_gs);
		}
		for(i=0;i<x->x_ls_amount;i++) 
		{
#ifdef PD
			SETFLOAT(&at[0], (t_float)i);	
			SETFLOAT(&at[1], (t_float)final_gs[i]);
			outlet_list(x->x_obj.ob_outlet, &s_list, 2, at);
#else /* Max */
			SETLONG(&at[0], i);	
			SETFLOAT(&at[1], final_gs[i]*x->x_gain); // freeverb gain is applied here
			outlet_list(x->x_outlet0, 0L, 2, at);
#endif /* PD */
		}
		outlet_float(x->x_outlet1, x->x_azi); 
		outlet_float(x->x_outlet2, x->x_ele); 
		outlet_int(x->x_outlet3, x->x_spread); 
		//outlet_int(x->x_outlet4, x->x_gain); 
	}
	else
		error("vbap: Configure loudspeakers first!");

	freebytes(final_gs, x->x_ls_amount * sizeof(t_float)); // bug fix added 9/00
}

/*--------------------------------------------------------------------------*/

static void vbap_matrix(t_vbap *x, Symbol *s, int ac, Atom *av)
// read in loudspeaker matrices
{
	int datapointer = 0; 
 	if(ac>0) 
	{
		int d = 0;
 		/*if(av[datapointer].a_type == A_LONG) d = av[datapointer++].a_w.w_long;
		else*/ if(av[datapointer].a_type == A_FLOAT) d = (long)av[datapointer++].a_w.w_float;
		else { error("vbap: Dimension NaN"); x->x_lsset_available=0; return; }

		if (d!=2 && d!=3) { error("vbap %s: Dimension can be only 2 or 3",s->s_name); x->x_lsset_available=0; return; }

		x->x_dimension = d;
		x->x_lsset_available=1;
	}
 	else { error("vbap %s: bad empty parameter list",s->s_name); x->x_lsset_available=0; return; }

	if(ac>1) 
	{
		long a = 0;
 		/*if(av[datapointer].a_type == A_LONG) a = av[datapointer++].a_w.w_long;
		else*/ if(av[datapointer].a_type == A_FLOAT) a = (long) av[datapointer++].a_w.w_float;
		else { error("vbap: ls_amount NaN");  x->x_lsset_available=0; return; }

		x->x_ls_amount = a;
	}
 	
	long counter = (ac - 2) / ((x->x_dimension * x->x_dimension*2) + x->x_dimension);
 	x->x_lsset_amount=counter;

 	if(counter==0) { error("vbap %s: not enough parameters",s->s_name); x->x_lsset_available=0; return; }
 	
	long setpointer=0;
	long i;
 
 	while(counter-- > 0)
	{
 		for(i=0; i < x->x_dimension; i++)
		{
# ifdef PD
 			if(av[datapointer].a_type == A_FLOAT)
			{
                x->x_lsset[setpointer][i]=(long)av[datapointer++].a_w.w_float;
 			}
 			else { error("vbap %s: param %d is not a float",s->s_name,datapointer); x->x_lsset_available=0; return; }
# else /* Max */
 			if(av[datapointer].a_type == A_LONG)
			{
 				 x->x_lsset[setpointer][i]=av[datapointer++].a_w.w_long;
 			}
 			else { error("vbap %s: param %d is not an in",s->s_name,datapointer); x->x_lsset_available=0; return; }
# endif /* PD */
 		}	
 		for(i=0; i < x->x_dimension*x->x_dimension; i++)
		{
 			if(av[datapointer].a_type == A_FLOAT)
			{
 				x->x_set_inv_matx[setpointer][i]=av[datapointer++].a_w.w_float;
 			}
 			else { error("vbap %s: param %d is not a float",s->s_name,datapointer); x->x_lsset_available=0; return; }
 		}
 		
 		for(i=0; i < x->x_dimension*x->x_dimension; i++)
		{
 			if(av[datapointer].a_type == A_FLOAT)
			{
 				x->x_set_matx[setpointer][i]=av[datapointer++].a_w.w_float;
 			}
 			else { error("vbap %s: param %d is not a float",s->s_name,datapointer); x->x_lsset_available=0; return; }
 			
 		}
 	
 		setpointer++;
	}
	if (_enable_trace) post("vbap: Loudspeaker setup configured!");
}
