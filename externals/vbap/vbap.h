#ifndef VBAP_H
#define VBAP_H

#include <math.h>
#include "m_pd.h"  				
#include "max2pd.h"

#ifndef M_PI
	// don't know where it is in win32, so add it here
	#define M_PI        3.14159265358979323846264338327950288   /* pi */
#endif

#define MAX_LS_SETS 100			// maximum number of loudspeaker sets (triplets or pairs) allowed
#define MAX_LS_AMOUNT 55    // maximum amount of loudspeakers, can be increased
#define MIN_VOL_P_SIDE_LGTH 0.01  

#define VBAP_VERSION "vbap - v1.0.3.2 - 20 Nov 2010 - (c) Ville Pulkki 1999-2006 (Pd port by HCS)"
#define DFLS_VERSION "define_loudspeakers - v1.0.3.2 - 20 Nov 2010 - (c) Ville Pulkki 1999-2006"

static t_float rad2ang = 360.0 / ( 2.0f * M_PI );
static t_float atorad = (2.0f * M_PI) / 360.0f ;

#ifdef VBAP_OBJECT
	// We are inside vbap object, so sending matrix from define_loudspeakers is a simple call to the vbap receiver...
	#define sendLoudspeakerMatrices(x,list_length, at) \
					vbap_matrix(x, gensym("loudspeaker-matrices"),list_length, at); \
					vbap_bang(x)
#else
	// We are inside define_loudspeaker object, send matrix to outlet
	#define sendLoudspeakerMatrices(x,list_length, at) \
					outlet_anything(x->x_outlet0, gensym("loudspeaker-matrices"), list_length, at);
#endif


/* A struct for a loudspeaker instance */
typedef struct 
{  					// distance value is 1.0 == unit vectors
  t_float x;  // cartesian coordinates
  t_float y;
  t_float z;
  t_float azi;  // polar coordinates
  t_float ele;
  int channel_nbr;  // which speaker channel number 
} t_ls;

/* A struct for all loudspeaker sets */
typedef struct t_ls_set 
{
  int ls_nos[3];  // channel numbers
  t_float inv_mx[9]; // inverse 3x3 or 2x2 matrix
  struct t_ls_set *next;  // next set (triplet or pair)
} t_ls_set;

#ifdef VBAP_OBJECT
	typedef struct vbap				/* This defines the object as an entity made up of other things */
	{
		t_object x_obj;				
		t_float x_azi; 	// panning direction azimuth
		t_float x_ele;		// panning direction elevation			
		void *x_outlet0;				/* outlet creation - inlets are automatic */
		void *x_outlet1;				
		void *x_outlet2;				
		void *x_outlet3;				
		void *x_outlet4;				
		t_float x_set_inv_matx[MAX_LS_SETS][9];  // inverse matrice for each loudspeaker set
		t_float x_set_matx[MAX_LS_SETS][9];      // matrice for each loudspeaker set
		long x_lsset[MAX_LS_SETS][3];          // channel numbers of loudspeakers in each LS set 
		long x_lsset_available;                // have loudspeaker sets been defined with define_loudspeakers
		long x_lsset_amount;								   // amount of loudspeaker sets
		long x_ls_amount;                      // amount of loudspeakers
		long x_dimension;                      // 2 or 3
# ifdef PD
		t_float x_spread;                      // speading amount of virtual source (0-100)
# else /* Max */
		long x_spread;                         // speading amount of virtual source (0-100)
		double x_gain;                         // general gain control (0-2)
# endif /* PD */
		t_float x_spread_base[3];                // used to create uniform spreading

		// define_loudspeaker data
		long x_ls_read;	 						// 1 if loudspeaker directions have been read
		long x_triplets_specified;  // 1 if loudspeaker triplets have been chosen
		t_ls x_ls[MAX_LS_AMOUNT];   // loudspeakers
		t_ls_set *x_ls_set;					// loudspeaker sets
		long x_def_ls_amount;				// number of loudspeakers
		long x_def_ls_dimension;		// 2 (horizontal arrays) or 3 (3d setups)
	} t_vbap;

	// define loudspeaker data type...
	typedef t_vbap t_def_ls;
#else
	/* define_loudspeakers maxmsp object */
	typedef struct 				
	{
		t_object x_obj;				/* gotta say this... it creates a reference to your object */
		long x_ls_read;	 			// 1 if loudspeaker directions have been read
		long x_triplets_specified;  // 1 if loudspeaker triplets have been chosen
		t_ls x_ls[MAX_LS_AMOUNT];   // loudspeakers
		t_ls_set *x_ls_set;			// loudspeaker sets
		void *x_outlet0;			/* outlet creation - inlets are automatic */
		long x_def_ls_amount;			// number of loudspeakers
		long x_def_ls_dimension;		    // 2 (horizontal arrays) or 3 (3d setups)
	} t_def_ls;
#endif /* VBAP_OBJECT */

# ifndef PD
/** Enable/Disable traces */
static bool _enable_trace = false;
void traces(t_def_ls *x, long n) { _enable_trace = n ? true : false;}
#endif /* ! PD */

#endif
