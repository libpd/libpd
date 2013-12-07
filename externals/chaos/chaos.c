///////////////////////////////////////////////////////////////////////////////////
/* Chaos Math PD Externals                                                       */
/* Copyright Ben Bogart 2002                                                     */
/* This program is distributed under the terms of the GNU General Public License */
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
/* This file is part of Chaos PD Externals.                                      */
/*                                                                               */
/* Chaos PD Externals are free software; you can redistribute them and/or modify */
/* them under the terms of the GNU General Public License as published by        */
/* the Free Software Foundation; either version 2 of the License, or             */
/* (at your option) any later version.                                           */
/*                                                                               */
/* Chaos PD Externals are distributed in the hope that they will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 */
/* GNU General Public License for more details.                                  */
/*                                                                               */
/* You should have received a copy of the GNU General Public License             */
/* along with the Chaos PD Externals; if not, write to the Free Software         */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     */
///////////////////////////////////////////////////////////////////////////////////


#include "m_pd.h"


#ifndef __DATE__
#define __DATE__ "without using a gnu compiler"
#endif

typedef struct _chaos
{
	t_object x_obj;
} t_chaos;

static t_class* chaos_class;

/* objects */
extern void attract1_setup();
extern void base_setup();
extern void base3_setup();
extern void dejong_setup();
extern void gingerbreadman_setup();
extern void henon_setup();
extern void hopalong_setup();
extern void ikeda_setup();
extern void lotkavolterra_setup();
extern void latoocarfian_setup();
extern void latoomutalpha_setup();
extern void latoomutbeta_setup();
extern void latoomutgamma_setup();
extern void logistic_setup();
extern void lorenz_setup();
extern void martin_setup();
extern void mlogistic_setup();
extern void pickover_setup();
extern void popcorn_setup();
extern void quadruptwo_setup();
extern void rossler_setup();
extern void standardmap_setup();
extern void strange1_setup();
extern void tent_setup();
extern void three_d_setup();
extern void threeply_setup();
extern void tinkerbell_setup();
extern void unity_setup();

static void* chaos_new(t_symbol* s)
{
    t_chaos *x = (t_chaos *)pd_new(chaos_class);
    return (x);
}

void chaos_setup(void)
{
	chaos_class = class_new(gensym("chaos"), (t_newmethod)chaos_new, 0, sizeof(t_chaos), 0,0);

	post("-------------------------");              /* Copyright info */
	post("Chaos PD Externals");
	post("Copyright Ben Bogart 2002, Copyright Ben Bogart and Michael McGonagle 2003");
	post("Win32 compilation by joge 2002");

	attract1_setup();
	base_setup();
	base3_setup();
	dejong_setup();
	gingerbreadman_setup();
	henon_setup();
	hopalong_setup();
	ikeda_setup();
	lotkavolterra_setup();
	latoocarfian_setup();
	latoomutalpha_setup();
	latoomutbeta_setup();
	latoomutgamma_setup();
	logistic_setup();
	lorenz_setup();
	martin_setup();
	mlogistic_setup();
	pickover_setup();
	popcorn_setup();
	quadruptwo_setup();
	rossler_setup();
	standardmap_setup();
	strange1_setup();
	tent_setup();
	three_d_setup();
	threeply_setup();
	tinkerbell_setup();
	unity_setup();

	post("-------------------------");
}
