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



void lpc_filter(double *buf_ppf, double *lpc_coef, double *buf_sy, int n)
{
    int i,j;
    double acc;

    for(i=0; i<n; i++)
    {
        acc=buf_ppf[i]*lpc_coef[0];
        for(j=1; j<11; j++)
        {
            acc=acc-lpc_coef[j]*buf_sy[i-j];
        };
        buf_sy[i]=acc;
    };
}

void hp_filter(double *in,double cut,int n)
{
    int i;
    double prev_e,s_out;
    prev_e=s_out=0;
    for(i=0; i<n; i++)
    {
        s_out=in[i]-prev_e+cut*s_out;
        prev_e=in[i];
        in[i]=s_out;
    };
}
