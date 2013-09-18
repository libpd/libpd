/*
vox - a musical real-time vocoder. version 1.0
Copyright (C) 2000  Simon MORLAT (simon.morlat@free.fr)

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
*/

/* variables globales */
extern int dsp_fd;
extern int bsize;
extern double voix[512+256];
extern double synth[512+256];
extern double sortie[512+12];
extern double *out;

void lev_durb(double *corr,double *lpc_coef);
void comp_lpc(double *buf_x,double *corr,double *lpc_coef,int n);
void lpc_filter(double *buf_ppf, double *lpc_coef, double *buf_sy,int n);
void lsp2lpc(double *lsp_coef,double *lpc_coef);
void lpc2lsp(double lpc_coef[],double *f1,double *f2,double lsp_coef[]);
double evalc(double cw,double *fonc);
