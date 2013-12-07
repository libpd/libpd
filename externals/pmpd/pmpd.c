/*
--------------------------  pmpd  ---------------------------------------- 
     
	  
 pmpd = physical modeling for pure data                                     
 Written by Cyrille Henry (cyrille.henry@la-kitchen.fr)
 
 Get sources at http://drpichon.free.fr/pure-data/physical-modeling/

 This program is free software; you can redistribute it and/or                
 modify it under the terms of the GNU General Public License                  
 as published by the Free Software Foundation; either version 2               
 of the License, or (at your option) any later version.                       
                                                                             
 This program is distributed in the hope that it will be useful,              
 but WITHOUT ANY WARRANTY; without even the implied warranty of               
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                
 GNU General Public License for more details.                           
                                                                              
 You should have received a copy of the GNU General Public License           
 along with this program; if not, write to the Free Software                  
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  
                                                                              
 Based on PureData by Miller Puckette and others.                             
                                                                             

----------------------------------------------------------------------------
*/

#ifndef VERSION
#define VERSION "0.07"
#endif

#include "m_pd.h"
#include "stdio.h"

#ifndef __DATE__ 
#define __DATE__ ""
#endif

#define nb_max_link   2000
#define nb_max_mass   2000
#define nb_max_out    200
#define nb_max_in     200
#define nb_max_outlet 20
#define nb_max_inlet  20 // hard-coded on the methods definition

#define max(a,b) ( ((a) > (b)) ? (a) : (b) ) 
#define min(a,b) ( ((a) < (b)) ? (a) : (b) ) 

/*
#include "masse.c"
#include "lia.c"
#include "masse2D.c"
#include "lia2D.c"
#include "masse3D.c"
#include "lia3D.c"

#include "iAmbient2D.c"
#include "iLine2D.c"
#include "iSeg2D.c"
#include "iCircle2D.c"

#include "tSquare2D.c"
#include "tLine2D.c"
#include "tSeg2D.c"
#include "tCircle2D.c"
#include "tLia2D.c"

#include "iAmbient3D.c"
#include "iSphere3D.c"
#include "iPlane3D.c"
#include "iCircle3D.c"
#include "iCylinder3D.c"


#include "tCube3D.c"
#include "tSphere3D.c"
#include "tPlane3D.c"
#include "tCircle3D.c"
#include "tCylinder3D.c"
#include "tLia3D.c"

#include "pmpd~.c"
*/

static t_class *pmpd_class;

typedef struct _mass {
	t_int Id;
	t_float invM;
	t_float speedX;
	t_float posX;
	t_float forceX;
} foo;

typedef struct _link {
	t_int Id;
	struct _mass *mass1;
	struct _mass *mass2;
	t_float Ke, K1, D1, K2, D2;
} foo1 ;

typedef struct _out {
	// TODO ajouter un type pour diferencier les outlets en forces et celles en position
	t_int Id;
	t_int nbr_outlet;
	struct _mass *mass1;
	t_float influence;
} foo2;

typedef struct _in {
	// TODO ajouter un type pour diferencier les inlets en forces et celles en position
	t_int Id;
	t_int nbr_inlet;
	struct _mass *mass1;
	t_float influence;
} foo3;

typedef struct _pmpd 
{
 	t_object  x_obj;
	struct _link link[nb_max_link];
	struct _mass mass[nb_max_mass];
	struct _out out[nb_max_out];
	struct _in in[nb_max_in];
	t_float outlet[nb_max_outlet];
	t_outlet *taboutlet[nb_max_outlet];
	t_float inlet[nb_max_inlet];
	int nb_link, nb_mass, nb_outlet, nb_inlet, nb_in, nb_out;
} t_pmpd;

void pmpd_bang(t_pmpd *x)
///////////////////////////////////////////////////////////////////////////////////
// this part is doing all the job!
{
	t_float F;
	t_int i;
	struct _mass mass_1, mass_2;

	for (i=0; i<x->nb_in; i++)
	// compute input
	{
		x->in[i].mass1->forceX += x->in[i].influence * x->inlet[x->in[i].nbr_inlet];
	}

	for (i=0; i<x->nb_inlet; i++)
	// clear inlet[i]
	{
		x->inlet[i]=0;
	}

	for (i=0; i<x->nb_link; i++)
	// comput link forces
	{
		F  = x->link[i].K1 * ( x->link[i].mass1->posX   - x->link[i].mass2->posX  ) ;
		F += x->link[i].D1 * ( x->link[i].mass1->speedX - x->link[i].mass2->speedX) ;
		x->link[i].mass1->forceX -= F;
		x->link[i].mass2->forceX += F;
	}

	for (i=1; i<x->nb_mass; i++)
	// compute new masses position
		if (x->mass[i].Id >0) // only if Id >0
			{
			x->mass[i].speedX += x->mass[i].forceX * x->mass[i].invM;
			x->mass[i].forceX = 0;
			x->mass[i].posX += x->mass[i].speedX ;
			}

	for (i=0; i<x->nb_out; i++)
	// compute output point
	{ 
		x->outlet[x->out[i].nbr_outlet] += x->out[i].mass1->posX * x->out[i].influence ;
	}

//	for (i=0; i<x->nb_outlet; i++)
	for (i=x->nb_outlet-1; i>=0; i--)
	// output everything on the corresponding outlet
	{
		outlet_float(x->taboutlet[i], x->outlet[i]);
		x->outlet[i] = 0;
	}
}

void pmpd_forceX(t_pmpd *x, t_float nbr_mass, t_float force)
{
// add a force to a specific mass
	nbr_mass = max(0, min( x->nb_mass, (int)nbr_mass));
	x->mass[(int)nbr_mass].forceX += force;
}

void pmpd_posX(t_pmpd *x, t_float nbr_mass, t_float posX)
{
// displace a mass to a certain position
	nbr_mass = max(0, min( x->nb_mass, (int)nbr_mass));
	x->mass[(int)nbr_mass].posX = posX;
}

void pmpd_mass(t_pmpd *x, t_float Id, t_float M, t_float posX)
// add a mass
// Id, invM speedX posX forceX
{
	if (M==0) M=1;
	x->mass[x->nb_mass].Id = (int)Id;
	x->mass[x->nb_mass].invM = 1/M;
	x->mass[x->nb_mass].speedX = 0;
	x->mass[x->nb_mass].posX = posX;
	x->mass[x->nb_mass].forceX = 0;

	x->nb_mass++ ;
	x->nb_mass = min ( nb_max_mass -1, x->nb_mass );
}

void pmpd_link(t_pmpd *x, t_float Id, t_float mass_1, t_float mass_2, t_float K1, t_float D1)
// add a link
// Id, *mass1, *mass2, Ke, K1, D1, K2, D2;
{

	x->link[x->nb_link].Id = (int)Id;
	x->link[x->nb_link].mass1 = &x->mass[max(0, min ( x->nb_mass, (int)mass_1))];
	x->link[x->nb_link].mass2 = &x->mass[max(0, min ( x->nb_mass, (int)mass_2))];
	x->link[x->nb_link].K1 = K1;
	x->link[x->nb_link].D1 = D1;

	x->nb_link++ ;
	x->nb_link = min ( nb_max_link -1, x->nb_link );
}

void pmpd_out(t_pmpd *x, t_float Id, t_float nb_outlet, t_float mass_1, t_float influence)
// add an output point
// Id, nbr_outlet, *mass1, influence;
{
	x->out[x->nb_out].Id = (int)Id;
	x->out[x->nb_out].nbr_outlet = max(0, min( x->nb_outlet,(int)nb_outlet));
	x->out[x->nb_out].mass1 = &x->mass[max(0, min ( x->nb_mass, (int)mass_1))];
	x->out[x->nb_out].influence = influence;

	x->nb_out++ ;
	x->nb_out = min ( nb_max_out - 1, x->nb_out );
}

void pmpd_in(t_pmpd *x, t_float Id, t_float nb_inlet, t_float mass_1, t_float influence)
//add an input point
// Id, nbr_inlet, *mass1, influence;
{
	x->in[x->nb_in].Id = (int)Id;
	x->in[x->nb_in].nbr_inlet = max(0, min( x->nb_inlet,(int)nb_inlet));
	x->in[x->nb_in].mass1 = &x->mass[max(0, min ( x->nb_mass, (int)mass_1))];
	x->in[x->nb_in].influence = influence;

	x->nb_in++;
	x->nb_in = min ( nb_max_in - 1, x->nb_in );
}

void pmpd_forceX_1(t_pmpd *x,  t_float force)
	{ x->inlet[0] += force; }
void pmpd_forceX_2(t_pmpd *x,  t_float force)
	{ x->inlet[1] += force; }
void pmpd_forceX_3(t_pmpd *x,  t_float force)
	{ x->inlet[2] += force; }
void pmpd_forceX_4(t_pmpd *x,  t_float force)
	{ x->inlet[3] += force; }
void pmpd_forceX_5(t_pmpd *x,  t_float force)
	{ x->inlet[4] += force; }
void pmpd_forceX_6(t_pmpd *x,  t_float force)
	{ x->inlet[5] += force; }
void pmpd_forceX_7(t_pmpd *x,  t_float force)
	{ x->inlet[6] += force; }
void pmpd_forceX_8(t_pmpd *x,  t_float force)
	{ x->inlet[7] += force; }
void pmpd_forceX_9(t_pmpd *x,  t_float force)
	{ x->inlet[8] += force; }
void pmpd_forceX_10(t_pmpd *x, t_float force)
	{ x->inlet[9] += force; }
void pmpd_forceX_11(t_pmpd *x, t_float force)
	{ x->inlet[10]+= force; }
void pmpd_forceX_12(t_pmpd *x, t_float force)
	{ x->inlet[11]+= force; }
void pmpd_forceX_13(t_pmpd *x, t_float force)
	{ x->inlet[12]+= force; }
void pmpd_forceX_14(t_pmpd *x, t_float force)
	{ x->inlet[13]+= force; }
void pmpd_forceX_15(t_pmpd *x, t_float force)
	{ x->inlet[14]+= force; }
void pmpd_forceX_16(t_pmpd *x, t_float force)
	{ x->inlet[15]+= force; }
void pmpd_forceX_17(t_pmpd *x, t_float force)
	{ x->inlet[16]+= force; }
void pmpd_forceX_18(t_pmpd *x, t_float force)
	{ x->inlet[17]+= force; }
void pmpd_forceX_19(t_pmpd *x, t_float force)
	{ x->inlet[18]+= force; }
void pmpd_forceX_20(t_pmpd *x, t_float force)
	{ x->inlet[19]+= force; }

void pmpd_reset(t_pmpd *x)
{
	x->nb_link = 0;
	x->nb_mass = 1;
	x->nb_out= 0;
	x->nb_in= 0;
}

void *pmpd_new(t_symbol *s, int argc, t_atom *argv)
{
	int i;
	char buffer[10];

	t_pmpd *x = (t_pmpd *)pd_new(pmpd_class);

	pmpd_reset(x);
	
	x->nb_outlet= (int)atom_getfloatarg(1, argc, argv);
	x->nb_outlet= max(0, min(nb_max_outlet, x->nb_outlet) );
	for(i=0; i<x->nb_outlet; i++)
		x->taboutlet[i]=outlet_new(&x->x_obj, 0);

	x->nb_inlet = (int)atom_getfloatarg(0, argc, argv);
	x->nb_inlet= max(0, min(nb_max_inlet, x->nb_inlet) );
	for(i=0; i<x->nb_inlet; i++)
	{
		sprintf (buffer, "forceX_%i", i+1);
		inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("forceX"), gensym(buffer));
	}
	return (void *)x;
}

void pmpd_setup(void) 
{
 pmpd_class = class_new(gensym("pmpd"),
        (t_newmethod)pmpd_new,
        0, sizeof(t_pmpd),CLASS_DEFAULT, A_GIMME, 0);

	class_addbang(pmpd_class, pmpd_bang);
	class_addmethod(pmpd_class, (t_method)pmpd_mass, gensym("mass"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_link, gensym("link"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_out,  gensym("out"),  A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_in,   gensym("in"),   A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_posX,       gensym("posX"),      A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX,     gensym("forceX"),    A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_1,   gensym("forceX_1"),  A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_2,   gensym("forceX_2"),  A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_3,   gensym("forceX_3"),  A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_4,   gensym("forceX_4"),  A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_5,   gensym("forceX_5"),  A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_6,   gensym("forceX_6"),  A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_7,   gensym("forceX_7"),  A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_8,   gensym("forceX_8"),  A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_9,   gensym("forceX_9"),  A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_10,  gensym("forceX_10"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_11,  gensym("forceX_11"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_12,  gensym("forceX_12"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_13,  gensym("forceX_13"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_14,  gensym("forceX_14"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_15,  gensym("forceX_15"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_16,  gensym("forceX_16"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_17,  gensym("forceX_17"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_18,  gensym("forceX_18"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_19,  gensym("forceX_19"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_forceX_20,  gensym("forceX_20"), A_DEFFLOAT, 0);
	class_addmethod(pmpd_class, (t_method)pmpd_reset,  gensym("reset"), 0);

/*
 post("");
 post("     pmpd = Physical Modeling for Pure Data");
 post("     version "VERSION);
 post("     compiled "__DATE__);
 post("     Contact : cyrille.henry@la-kitchen.fr");
 post("");

masse_setup() ;
lia_setup() ;
masse2D_setup() ;
lia2D_setup() ;
masse3D_setup() ;
lia3D_setup() ;

iAmbient2D_setup();
iLine2D_setup();
iSeg2D_setup();
iCircle2D_setup();

tSquare2D_setup();
tCircle2D_setup();
tLine2D_setup();
tSeg2D_setup();
tLia2D_setup();

iAmbient3D_setup();
iSphere3D_setup();
iPlane3D_setup();
iCircle3D_setup();
iCylinder3D_setup();

tLia3D_setup();
tCube3D_setup();
tPlane3D_setup();
tSphere3D_setup();
tCylinder3D_setup();
tCircle3D_setup();

pmpd_tilde_setup();

*/
}

