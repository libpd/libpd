/* rvbap.c vers 1.1 

written by Ville Pulkki 1999-2003
Helsinki University of Technology 
and 
Unversity of California at Berkeley
and written by Olaf Matthes 2003, 2007
Pd port by Frank Barknecht

See copyright in file with name LICENSE.txt  */


#include <math.h>

#ifdef MAXMSP
#include "ext.h"                /* you must include this - it contains the external object's link to max */
#define t_float float
#endif

#ifdef PD 
#include "m_pd.h"               /* you must include this - it contains the external object's link to pure data */
#endif

#define MAX_LS_SETS 100         // maximum number of loudspeaker sets (triplets or pairs) allowed
#define MAX_LS_AMOUNT 55        // maximum amount of loudspeakers, can be increased

#ifdef _WINDOWS
#define sqrtf sqrt
#endif

#ifdef MAXMSP
typedef struct vbap             /* This defines the object as an entity made up of other things */
{
    t_object x_ob;                
    long x_azi;     // panning direction azimuth
    long x_ele;     // panning direction elevation      
    t_float x_dist; // sound source distance    (1.0-infinity)
    void *x_outlet0;                /* outlet creation - inlets are automatic */
    void *x_outlet1;                
    void *x_outlet2;                
    void *x_outlet3;                
    void *x_outlet4;                
    t_float x_set_inv_matx[MAX_LS_SETS][9];// inverse matrice for each loudspeaker set
    t_float x_set_matx[MAX_LS_SETS][9];    // matrice for each loudspeaker set
    long x_lsset[MAX_LS_SETS][3];          // channel numbers of loudspeakers in each LS set 
    long x_lsset_available;                // have loudspeaker sets been defined with define_loudspeakers
    long x_lsset_amount;                   // amount of loudspeaker sets
    long x_ls_amount;                      // amount of loudspeakers
    long x_dimension;                      // 2 or 3
    long x_spread;                         // speading amount of virtual source (0-100)
    t_float x_spread_base[3];              // used to create uniform spreading
    t_float x_reverb_gs[MAX_LS_SETS];      // correction value for each loudspeaker set to get equal volume
} t_rvbap;
#endif


#ifdef PD
typedef struct vbap             /* This defines the object as an entity made up of other things */
{
    t_object x_ob;              
    t_float x_azi;  // panning direction azimuth
    t_float x_ele;  // panning direction elevation      
    t_float x_dist; // sound source distance    (1.0-infinity)
    void *x_outlet0;                /* outlet creation - inlets are automatic */
    void *x_outlet1;                
    void *x_outlet2;                
    void *x_outlet3;                
    void *x_outlet4;                
    t_float x_set_inv_matx[MAX_LS_SETS][9];// inverse matrice for each loudspeaker set
    t_float x_set_matx[MAX_LS_SETS][9];    // matrice for each loudspeaker set
    long x_lsset[MAX_LS_SETS][3];          // channel numbers of loudspeakers in each LS set 
    long x_lsset_available;                // have loudspeaker sets been defined with define_loudspeakers
    long x_lsset_amount;                   // amount of loudspeaker sets
    long x_ls_amount;                      // amount of loudspeakers
    long x_dimension;                      // 2 or 3
    t_float x_spread;                      // speading amount of virtual source (0-100)
    t_float x_spread_base[3];              // used to create uniform spreading
    t_float x_reverb_gs[MAX_LS_SETS];      // correction value for each loudspeaker set to get equal volume
} t_rvbap;
#endif

// Globals

static void new_spread_dir(t_rvbap *x, t_float spreaddir[3], t_float vscartdir[3], t_float spread_base[3]);
static void new_spread_base(t_rvbap *x, t_float spreaddir[3], t_float vscartdir[3]);
#ifdef MAXMSP
static void *rvbap_class;
static void rvbap_assist(t_rvbap *x, void *b, long m, long a, char *s);
static void rvbap_in1(t_rvbap *x, long n);
static void rvbap_in2(t_rvbap *x, long n);
static void rvbap_in3(t_rvbap *x, long n);
static void rvbap_in4(t_rvbap *x, long n);
static void rvbap_ft1(t_rvbap *x, double n);
static void rvbap_ft2(t_rvbap *x, double n);
static void rvbap_ft3(t_rvbap *x, double n);
static void rvbap_ft4(t_rvbap *x, double n);
#endif
#ifdef PD
static t_class *rvbap_class;
#endif
static void cross_prod(t_float v1[3], t_float v2[3],
                t_float v3[3]);
static void additive_vbap(t_float *final_gs, t_float cartdir[3], t_rvbap *x);
static void rvbap_bang(t_rvbap *x);
static void rvbap_matrix(t_rvbap *x, t_symbol *s, int ac, t_atom *av);
static void spread_it(t_rvbap *x, t_float *final_gs);
static void *rvbap_new(t_symbol *s, int ac, t_atom *av); // using A_GIMME - typed message list
static void vbap(t_float g[3], long ls[3], t_rvbap *x);
static void angle_to_cart(long azi, long ele, t_float res[3]);
static void cart_to_angle(t_float cvec[3], t_float avec[3]);
static void equal_reverb(t_rvbap *x, t_float *final_gs);

/* above are the prototypes for the methods/procedures/functions you will use */

#ifdef PD
void rvbap_setup(void)
{
    rvbap_class = class_new(gensym("rvbap"), (t_newmethod)rvbap_new, 0, (short)sizeof(t_rvbap), 0, A_GIMME, 0); 
    /* rvbap_new = creation function, A_DEFLONG = its (optional) arguement is a long (32-bit) int */
    class_addbang(rvbap_class, rvbap_bang);
    class_addmethod(rvbap_class, (t_method)rvbap_matrix, gensym("loudspeaker-matrices"), A_GIMME, 0);
}
#endif

#ifdef MAXMSP
int main(void)
{
    setup((t_messlist **)&rvbap_class, (method)rvbap_new, 0L, (short)sizeof(t_rvbap), 0L, A_GIMME, 0); 
    /* rvbap_new = creation function, A_DEFLONG = its (optional) arguement is a long (32-bit) int */
    addmess((method)rvbap_assist, "assist", A_CANT, 0);
    addbang((method)rvbap_bang);         /* the procedure it uses when it gets a bang in the left inlet */
    addinx((method)rvbap_in1, 1);        /* the rocedure for an int in the right inlet (inlet 1) */
    addinx((method)rvbap_in2, 2);        /* the rocedure for an int in the right inlet (inlet 2) */
    addinx((method)rvbap_in3, 3);
    addinx((method)rvbap_in4, 4);
    addftx((method)rvbap_ft1, 1);        /* the rocedure for an int in the right inlet (inlet 1) */
    addftx((method)rvbap_ft2, 2);        /* the rocedure for an int in the right inlet (inlet 2) */
    addftx((method)rvbap_ft3, 3);
    addftx((method)rvbap_ft4, 4);
    addmess((method)rvbap_matrix, "loudspeaker-matrices", A_GIMME, 0);
    post("rvbap v1.1, © 2003-2007 by Olaf Matthes, based on vbap by Ville Pulkki");
	return 0;
}

static void rvbap_assist(t_rvbap *x, void *b, long m, long a, char *s)
{
    switch(m) {
        case 1: // inlet
            switch(a) {
                case 0:
                sprintf(s, "define_loudspeakers / Bang to output actual values.");
                break;
                case 1:
                sprintf(s, "(int) azimuth");
                break;
                case 2:
                sprintf(s, "(int) elevation");
                break;
                case 3:
                sprintf(s, "(int) spreading");
                break;
                case 4:
                sprintf(s, "(t_float) distance");
                break;
            }
        break;
        case 2: // outlet
            switch(a) {
                case 0:
                sprintf(s, "(list) matrix~ values");
                break;
                case 1:
                sprintf(s, "(int) actual azimuth");
                break;
                case 2:
                sprintf(s, "(int) actual elevation");
                break;
                case 3:
                sprintf(s, "(int) actual spreading");
                break;
                case 4:
                sprintf(s, "(t_float) actual distance");
                break;
            }
        break;
    }
}
#endif
/* end MAXMSP */

static void angle_to_cart(long azi, long ele, t_float res[3])
/* converts angular coordinates to cartesian */
{
  t_float atorad = (2.0 * 3.141592653589793 / 360.0) ;
  res[0] = cos((t_float) azi * atorad) * cos((t_float) ele * atorad);
  res[1] = sin((t_float) azi * atorad) * cos((t_float) ele * atorad);
  res[2] = sin((t_float) ele * atorad);
}

static void cart_to_angle(t_float cvec[3], t_float avec[3])
// converts cartesian coordinates to angular
{
  t_float tmp, tmp2, tmp3, tmp4;
  t_float atorad = (t_float)(2.0 * 3.141592653589793 / 360.0) ;
  t_float pi =  (t_float)3.141592653589793;
  t_float power;
  t_float dist, atan_y_per_x, atan_x_pl_y_per_z;
  t_float azi, ele;
  
  if(cvec[0]==0.0)
    atan_y_per_x = pi / 2;
  else
    atan_y_per_x = atan(cvec[1] / cvec[0]);
  azi = atan_y_per_x / atorad;
  if(cvec[0]<0.0)
    azi +=180;
  dist = sqrt(cvec[0]*cvec[0] + cvec[1]*cvec[1]);
  if(cvec[2]==0.0)
    atan_x_pl_y_per_z = 0.0;
  else
    atan_x_pl_y_per_z = atan(cvec[2] / dist);
  if(dist == 0.0)
    if(cvec[2]<0.0)
      atan_x_pl_y_per_z = -pi/2.0;
    else
      atan_x_pl_y_per_z = pi/2.0;
  ele = atan_x_pl_y_per_z / atorad;
  dist = sqrt(cvec[0] * cvec[0] +cvec[1] * cvec[1] +cvec[2]*cvec[2]);
  avec[0]=azi;
  avec[1]=ele;
  avec[2]=dist;
}


static void vbap(t_float g[3], long ls[3], t_rvbap *x)
{
  /* calculates gain factors using loudspeaker setup and given direction */
  t_float power;
  int i,j,k, gains_modified;
  t_float small_g;
  t_float big_sm_g, gtmp[3];
  long winner_set=0;
  t_float cartdir[3];
  t_float new_cartdir[3];
  t_float new_angle_dir[3];
  long dim = x->x_dimension;
  long neg_g_am, best_neg_g_am;
  
  // transfering the azimuth angle to a decent value
  while(x->x_azi > 180)
    x->x_azi -= 360;
  while(x->x_azi < -179)
    x->x_azi += 360;
    
  // transferring the elevation to a decent value
  if(dim == 3){
    while(x->x_ele > 180)
        x->x_ele -= 360;
    while(x->x_ele < -179)
        x->x_ele += 360;
  } else
    x->x_ele = 0;
  
  
  // go through all defined loudspeaker sets and find the set which
  // has all positive values. If such is not found, set with largest
  // minimum value is chosen. If at least one of gain factors of one LS set is negative
  // it means that the virtual source does not lie in that LS set. 
  
  angle_to_cart(x->x_azi,x->x_ele,cartdir);
  big_sm_g = -100000.0;   // initial value for largest minimum gain value
  best_neg_g_am=3;        // how many negative values in this set
  
  
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
  
  // If chosen set produced a negative value, make it zero and
  // calculate direction that corresponds  to these new
  // gain values. This happens when the virtual source is outside of
  // all loudspeaker sets. 
  
  if(dim==3){
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
        new_cartdir[2] =  x->x_set_matx[winner_set][6] * g[0] 
                        + x->x_set_matx[winner_set][7] * g[1]
                        + x->x_set_matx[winner_set][8] * g[2];
        cart_to_angle(new_cartdir,new_angle_dir);
        x->x_azi = (long) (new_angle_dir[0] + 0.5);
        x->x_ele = (long) (new_angle_dir[1] + 0.5);
     }
  }
  
  power=sqrt(g[0]*g[0] + g[1]*g[1] + g[2]*g[2]);
  g[0] /= power;
  g[1] /= power;
  g[2] /= power;
}


static void cross_prod(t_float v1[3], t_float v2[3],
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

static void additive_vbap(t_float *final_gs, t_float cartdir[3], t_rvbap *x)
// calculates gains to be added to previous gains, used in
// multiple direction panning (source spreading)
{
    t_float power;
    int i,j,k, gains_modified;
    t_float small_g;
    t_float big_sm_g, gtmp[3];
    long winner_set;
    t_float new_cartdir[3];
    t_float new_angle_dir[3];
    long dim = x->x_dimension;
    long neg_g_am, best_neg_g_am;
    t_float g[3];
    long ls[3] = { 0, 0, 0 };
    
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
        if(dim==3)
            power=sqrt(g[0]*g[0] + g[1]*g[1] + g[2]*g[2]);
        else
            power=sqrt(g[0]*g[0] + g[1]*g[1]);
        g[0] /= power;
        g[1] /= power;
        if(dim==3) 
            g[2] /= power;
        
        final_gs[ls[0]-1] += g[0];
        final_gs[ls[1]-1] += g[1];
        /* BUG FIX: this was causing negative indices with 2 dimensions so I
         * made it only try when using 3 dimensions.
         * 2006-08-13 <hans@at.or.at> */
        if(dim==3)
            final_gs[ls[2]-1] += g[2];
    }
}


static void new_spread_dir(t_rvbap *x, t_float spreaddir[3], t_float vscartdir[3], t_float spread_base[3])
// subroutine for spreading
{
    t_float beta,m_gamma;
    t_float a,b;
    t_float pi = 3.141592653589793;
    t_float power;
    
    m_gamma = acos(vscartdir[0] * spread_base[0] +
                    vscartdir[1] * spread_base[1] +
                    vscartdir[2] * spread_base[2])/pi*180;
    if(fabs(m_gamma) < 1){
        angle_to_cart(x->x_azi+90, 0, spread_base);
        m_gamma = acos(vscartdir[0] * spread_base[0] +
                    vscartdir[1] * spread_base[1] +
                    vscartdir[2] * spread_base[2])/pi*180;
    }
    beta = 180 - m_gamma;
    b=sin(x->x_spread * pi / 180) / sin(beta * pi / 180);
    a=sin((180- x->x_spread - beta) * pi / 180) / sin (beta * pi / 180);
    spreaddir[0] = a * vscartdir[0] + b * spread_base[0];
    spreaddir[1] = a * vscartdir[1] + b * spread_base[1];
    spreaddir[2] = a * vscartdir[2] + b * spread_base[2];
    
    power=sqrt(spreaddir[0]*spreaddir[0] + spreaddir[1]*spreaddir[1] 
                + spreaddir[2]*spreaddir[2]);
    spreaddir[0] /= power;
    spreaddir[1] /= power;
    spreaddir[2] /= power;
}

static void new_spread_base(t_rvbap *x, t_float spreaddir[3], t_float vscartdir[3])
// subroutine for spreading
{
    t_float d;
    t_float pi = 3.141592653589793;
    t_float power;
    
    d = cos(x->x_spread/180*pi);
    x->x_spread_base[0] = spreaddir[0] - d * vscartdir[0];
    x->x_spread_base[1] = spreaddir[1] - d * vscartdir[1];
    x->x_spread_base[2] = spreaddir[2] - d * vscartdir[2];
    power=sqrt(x->x_spread_base[0]*x->x_spread_base[0] + x->x_spread_base[1]*x->x_spread_base[1] 
                + x->x_spread_base[2]*x->x_spread_base[2]);
    x->x_spread_base[0] /= power;
    x->x_spread_base[1] /= power;
    x->x_spread_base[2] /= power;
}

static void spread_it(t_rvbap *x, t_float *final_gs)
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
        cross_prod(x->x_spread_base, vscartdir, spreadbase[1]); // four orthogonal dirs
        cross_prod(spreadbase[1], vscartdir, spreadbase[2]);
        cross_prod(spreadbase[2], vscartdir, spreadbase[3]);
    
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
    

static void equal_reverb(t_rvbap *x, t_float *final_gs)
// calculate constant reverb gains for equally distributed
// reverb levels
// this is achieved by calculating gains for a sound source 
// that is everywhere, i.e. present in all directions

{
    t_float vscartdir[3];
    t_float spreaddir[16][3];
    t_float spreadbase[16][3];
    long i, spreaddirnum;
    t_float power;
    if(x->x_dimension == 3){
        spreaddirnum=5;     
        
        // horizontal plane
        angle_to_cart(90, 0, spreaddir[0]);
        angle_to_cart(180, 0, spreaddir[1]);
        angle_to_cart(270, 0, spreaddir[2]);
        
        // above, below
        angle_to_cart(0, 90, spreaddir[3]);
        angle_to_cart(0, -90, spreaddir[4]);
        
        for(i=1;i<spreaddirnum;i++){
            additive_vbap(x->x_reverb_gs,spreaddir[i],x); 
        }
    } else if (x->x_dimension == 2) {
        // for 2-D we claculate virtual sources 
        // every 45 degrees in a horizontal plane
        spreaddirnum=7;     
        
        angle_to_cart(90, 0, spreaddir[0]);
        angle_to_cart(180, 0, spreaddir[1]);
        angle_to_cart(270, 0, spreaddir[2]);
        angle_to_cart(45, 0, spreaddir[3]);
        angle_to_cart(135, 0, spreaddir[4]);
        angle_to_cart(225, 0, spreaddir[5]);
        angle_to_cart(315, 0, spreaddir[6]);
        
        for(i=0;i<spreaddirnum;i++)
            additive_vbap(x->x_reverb_gs,spreaddir[i],x); 
    } else
        return;
        
    for(i=0,power=0.0;i<x->x_ls_amount;i++){
        power += x->x_reverb_gs[i] * x->x_reverb_gs[i];
    }
        
    power = sqrt(power);
    for(i=0;i<x->x_ls_amount;i++){
        final_gs[i] /= power;
    }
}   
    
static void rvbap_bang(t_rvbap *x)            
// top level, vbap gains are calculated and outputted   
{
    t_atom at[MAX_LS_AMOUNT]; 
    t_float g[3];
    long ls[3];
    long i;
    t_float *final_gs, overdist, oversqrtdist;
    final_gs = (t_float *) getbytes(x->x_ls_amount * sizeof(t_float));
    if(x->x_lsset_available ==1){
        vbap(g, ls, x);
        for(i=0;i<x->x_ls_amount;i++)
            final_gs[i]=0.0;  
        for(i=0;i<x->x_dimension;i++){
            final_gs[ls[i]-1]=g[i];  
        }
        if(x->x_spread != 0){
            spread_it(x,final_gs);
        }
        overdist = 1 / x->x_dist;
        oversqrtdist = 1 / sqrt(x->x_dist);
        // build output for every loudspeaker
        for(i=0;i<x->x_ls_amount;i++)
		{
            // first, we output the gains for the direct (unreverberated) signals
            // these just decrease as the distance increases
#ifdef MAXMSP
            SETLONG(&at[0], i); 
            SETFLOAT(&at[1], (final_gs[i] / x->x_dist));
            outlet_list(x->x_outlet0, NULL, 2, at);
#endif
#ifdef PD
            SETFLOAT(&at[0], i);    
            SETFLOAT(&at[1], (final_gs[i] / x->x_dist));
            outlet_list(x->x_outlet0, gensym("list"), 2, at);
#endif
            // second, we output the gains for the reverberated signals
            // these are made up of a global (all speakers) and a local part
#ifdef MAXMSP
            SETLONG(&at[0], i+x->x_ls_amount);  // direct signals come first in matrix~
            SETFLOAT(&at[1], (((oversqrtdist / x->x_dist) * x->x_reverb_gs[i]) + (oversqrtdist * (1 - overdist) * final_gs[i])));
            outlet_list(x->x_outlet0, NULL, 2, at);
#endif
#ifdef PD
            SETFLOAT(&at[0], (i+x->x_ls_amount));   // direct signals come first in matrix~
            SETFLOAT(&at[1], (((oversqrtdist / x->x_dist) * x->x_reverb_gs[i]) + (oversqrtdist * (1 - overdist) * final_gs[i])));
            outlet_list(x->x_outlet0, gensym("list"), 2, at);
#endif
        }
#ifdef MAXMSP
        outlet_int(x->x_outlet1, x->x_azi); 
        outlet_int(x->x_outlet2, x->x_ele); 
        outlet_int(x->x_outlet3, x->x_spread); 
        outlet_float(x->x_outlet4, (double)x->x_dist); 
#endif
#ifdef PD
        outlet_float(x->x_outlet1, x->x_azi); 
        outlet_float(x->x_outlet2, x->x_ele); 
        outlet_float(x->x_outlet3, x->x_spread); 
        outlet_float(x->x_outlet4, x->x_dist); 
#endif
    }
    else
        post("rvbap: Configure loudspeakers first!");
    freebytes(final_gs, x->x_ls_amount * sizeof(t_float)); // bug fix added 9/00
}

/*--------------------------------------------------------------------------*/

static void rvbap_matrix(t_rvbap *x, t_symbol *s, int ac, t_atom *av)
// read in loudspeaker matrices
// and calculate the gains for the equally distributed
// reverb signal part (i.e. global reverb)
{
    long counter=0;
    long datapointer=0;
    long setpointer=0;
    long i;
    long deb=0;
    long azi = x->x_azi, ele = x->x_ele;    // store original values
    t_float g[3];
    long ls[3];
 
    if(ac>0)
#ifdef MAXMSP
        if(av[datapointer].a_type == A_LONG){
            x->x_dimension = av[datapointer++].a_w.w_long;
            x->x_lsset_available=1;
        } else
#endif
        if(av[datapointer].a_type == A_FLOAT){
            x->x_dimension = (long) av[datapointer++].a_w.w_float;
            x->x_lsset_available=1;
        } else {
            post("Error in loudspeaker data!");
            x->x_lsset_available=0;
            return;
        }
    //post("%d",deb++);
    if(ac>1) 
#ifdef MAXMSP
        if(av[datapointer].a_type == A_LONG)
            x->x_ls_amount = av[datapointer++].a_w.w_long;
        else
#endif
        if(av[datapointer].a_type == A_FLOAT)
            x->x_ls_amount = (long) av[datapointer++].a_w.w_float;
        else {
            post("rvbap: Error in loudspeaker data!");
            x->x_lsset_available=0;
            return;
        }
    else
        x->x_lsset_available=0;
    
    if(x->x_dimension == 3)
        counter = (ac - 2) / ((x->x_dimension * x->x_dimension*2) + x->x_dimension);
    if(x->x_dimension == 2)
        counter = (ac - 2) / ((x->x_dimension * x->x_dimension) + x->x_dimension);
    x->x_lsset_amount=counter;

    if(counter<=0) {
        post("rvbap: Error in loudspeaker data!");
        x->x_lsset_available=0;
        return;
    }
    
 
    while(counter-- > 0){
        for(i=0; i < x->x_dimension; i++){
#ifdef MAXMSP
            if(av[datapointer].a_type == A_LONG)
#endif
#ifdef PD
            if(av[datapointer].a_type == A_FLOAT)
#endif
            {
                 x->x_lsset[setpointer][i]=(long)av[datapointer++].a_w.w_float;
            }
            else{
                post("rvbap: Error in loudspeaker data!");
                x->x_lsset_available=0;
                return;
            }
        }   
        for(i=0; i < x->x_dimension*x->x_dimension; i++){
            if(av[datapointer].a_type == A_FLOAT){
                x->x_set_inv_matx[setpointer][i]=av[datapointer++].a_w.w_float;
            }
            else {
                post("rvbap: Error in loudspeaker data!");
                x->x_lsset_available=0;
                return;
            }
        }
        if(x->x_dimension == 3){ 
            for(i=0; i < x->x_dimension*x->x_dimension; i++){
                if(av[datapointer].a_type == A_FLOAT){
                    x->x_set_matx[setpointer][i]=av[datapointer++].a_w.w_float;
                }
                else {
                    post("rvbap: Error in loudspeaker data!");
                    x->x_lsset_available=0;
                    return;
                }
            }
        }
    
        setpointer++;
    }
    // now configure static reverb correction values...
    x->x_azi = x->x_ele = 0;
    vbap(g,ls, x);
    for(i=0;i<x->x_ls_amount;i++){
        x->x_reverb_gs[i]=0.0;
    }       
    for(i=0;i<x->x_dimension;i++){
        x->x_reverb_gs[ls[i]-1]=g[i];
        // post("reverb gs #%d = %f", i, x->x_reverb_gs[i]);
    }
    equal_reverb(x,x->x_reverb_gs);
            
/*  for(i=0; i<x->x_ls_amount; i++) // do this for every speaker
    {
            post("reverb gs #%d = %f", i, x->x_reverb_gs[i]);
    }   */
    post("rvbap: Loudspeaker setup configured!");
    x->x_azi = azi;     // restore original panning directions
    x->x_ele = ele;
}

#ifdef MAXMSP
static void rvbap_in1(t_rvbap *x, long n)                /* x = the instance of the object, n = the int received in the right inlet */
// panning angle azimuth
{
    x->x_azi = n;                           /* store n in a global variable */
    
}

static void rvbap_in2(t_rvbap *x, long n)                /* x = the instance of the object, n = the int received in the right inlet */
// panning angle elevation
{
    x->x_ele = n;                           /* store n in a global variable */

}
/*--------------------------------------------------------------------------*/

static void rvbap_in3(t_rvbap *x, long n)                /* x = the instance of the object, n = the int received in the right inlet */
// spread amount
{
    if (n<0) n = 0;
    if (n>100) n = 100;
    x->x_spread = n;                            /* store n in a global variable */
    
}

/*--------------------------------------------------------------------------*/

static void rvbap_in4(t_rvbap *x, long n)                /* x = the instance of the object, n = the int received in the right inlet */
// distance
{
    if (n<1) n = 1;
    x->x_dist = (t_float)n;                           /* store n in a global variable */
    
}

static void rvbap_ft1(t_rvbap *x, double n)              /* x = the instance of the object, n = the int received in the right inlet */
// panning angle azimuth
{
    x->x_azi = (long) n;                            /* store n in a global variable */
    
}

static void rvbap_ft2(t_rvbap *x, double n)              /* x = the instance of the object, n = the int received in the right inlet */
// panning angle elevation
{
     x->x_ele = (long) n;                           /* store n in a global variable */

}
/*--------------------------------------------------------------------------*/

static void rvbap_ft3(t_rvbap *x, double n)              /* x = the instance of the object, n = the int received in the right inlet */
// spreading
{
    if (n<0.0) n = 0.0;
    if (n>100.0) n = 100.0;
    x->x_spread = (long) n;                         /* store n in a global variable */
    
}

/*--------------------------------------------------------------------------*/

static void rvbap_ft4(t_rvbap *x, double n)              /* x = the instance of the object, n = the int received in the right inlet */
// distance
{
    if (n<1.0) n = 1.0;
    x->x_dist = (t_float)n;                           /* store n in a global variable */
}


#endif

static void *rvbap_new(t_symbol *s, int ac, t_atom *av)  
/* create new instance of object... MUST send it an int even if you do nothing with this int!! */
{
    t_rvbap *x;
#ifdef MAXMSP
    x = (t_rvbap *)newobject(rvbap_class);
    
    t_floatin(x,4);       /* takes the distance */
    intin(x,3); 
    intin(x,2);                 /* create a second (int) inlet... remember right-to-left ordering in Max */
    intin(x,1);                 /* create a second (int) inlet... remember right-to-left ordering in Max */
    x->x_outlet4 = floatout(x); /* distance */
    x->x_outlet3 = intout(x);
    x->x_outlet2 = intout(x);   /* create an (int) outlet  - rightmost outlet first... */
    x->x_outlet1 = intout(x);   /* create an (int) outlet */
    x->x_outlet0 = listout(x);  /* create a (list) outlet */
#endif
#ifdef PD
    x = (t_rvbap *)pd_new(rvbap_class);
    floatinlet_new(&x->x_ob, &x->x_azi);
    floatinlet_new(&x->x_ob, &x->x_ele);
    floatinlet_new(&x->x_ob, &x->x_spread);
    floatinlet_new(&x->x_ob, &x->x_dist);

    x->x_outlet0 = outlet_new(&x->x_ob, gensym("list"));
    x->x_outlet1 = outlet_new(&x->x_ob, gensym("float"));   
    x->x_outlet2 = outlet_new(&x->x_ob, gensym("float"));   
    x->x_outlet3 = outlet_new(&x->x_ob, gensym("float"));   
    x->x_outlet4 = outlet_new(&x->x_ob, gensym("float"));   
#endif

    x->x_azi = 0;
    x->x_ele = 0;
    x->x_dist = 1.0;
    x->x_spread_base[0] = 0.0;
    x->x_spread_base[1] = 1.0;
    x->x_spread_base[2] = 0.0;
    x->x_spread = 0;
    x->x_lsset_available =0;
    if (ac>0) {
#ifdef MAXMSP
        if (av[0].a_type == A_LONG)
            x->x_azi = av[0].a_w.w_long;
        else 
#endif
        if (av[0].a_type == A_FLOAT)
            x->x_azi = av[0].a_w.w_float;       
    }
    if (ac>1) {
#ifdef MAXMSP
        if (av[1].a_type == A_LONG)
            x->x_ele = av[1].a_w.w_long;
        else
#endif
        if (av[1].a_type == A_FLOAT)
            x->x_ele = av[1].a_w.w_float;   
    }
    if (ac>2) {
#ifdef MAXMSP
        if (av[2].a_type == A_LONG)
            x->x_dist = (float)av[2].a_w.w_long;
        else
#endif
        if (av[2].a_type == A_FLOAT)
            x->x_dist = av[2].a_w.w_float;  
    }
    return(x);                  /* return a reference to the object instance */
}

