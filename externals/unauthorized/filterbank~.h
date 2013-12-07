/* Copyright (c) 2002 Yves Degoyon.                                             */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "LICENSE.txt," in this distribution.               */
/*                                                                              */
/* a header for filterbank~ which outputs frequency response                    */
/* for a range of filters                                                       */              
/*                                                                              */
/* The filter code is taken from Speech Filing System                           */
/* from Mark Huckvale, University College London                                */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* See file LICENSE for further informations on licensing terms.                */
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
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* Made while listening to :                                                    */
/*                                                                              */
/* Twine -- Instrumentals                                                       */
/* Andy T -- Poetry                                                             */
/* ---------------------------------------------------------------------------- */

#ifndef __FILTERBANK_H
#define __FILTERBANK_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"


/* this code is borrowed from pd's object : bp~ */

typedef struct bpctl
{
    float c_x1;
    float c_x2;
    float c_coef1;
    float c_coef2;
    float c_gain;
} t_bpctl;

typedef struct _filterbank_tilde
{
    t_object x_obj;
    t_glist *x_glist;
    t_int   x_samplerate;    /* system sample rate                        */
    t_int   x_lowfreq;       /* lower frequency of all filters            */
    t_int   x_highfreq;      /* higher frequency of all filters           */
    t_int   x_nbfilters;     /* number of filters                         */
    t_outlet **x_outputs;    /* outputs for frequency responses           */
    t_int   x_nselected;     /* index of item selected                    */
    t_int   x_width;         /* graphical x size ( not setable )          */
    t_int   x_height;        /* graphical y size ( not setable )          */
    t_int   x_selected;      /* selection flag                            */
    t_int   x_allocate;      /* allocation flag                           */
    t_int   *x_outmapping;   /* output mapping array                      */

    // bp~ section : borrowed from pd's d_filter.c
    float x_sr;
    float *x_freq;
    float *x_q;
    t_bpctl *x_cspace;
    t_bpctl **x_ctl;

    t_float x_f;             /* classical float for signal input          */
} t_filterbank_tilde;

#endif
