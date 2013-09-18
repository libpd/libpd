#include "m_pd.h"
#include <math.h>
#include <string.h>
#define MAXPOLY 32
#define POLYMAP 1024

/* The polymap is a two-dimensional array, but here instead of this we have a one dimensional array of length MAXPOLY^2 */

static t_class *polymap_class;

typedef struct _map
{
  t_atom polybins[MAXPOLY];
  t_atom polymap[POLYMAP];
  t_atom note[3];
} t_map;

typedef struct _polymap
{
  t_object x_obj;
  t_map x_map;
  t_float maxpoly, polynow;
  t_outlet *note, *new, *poly, *overflow;
} t_polymap;


/* perhaps polymap_note could be called from the sieve function, thus only setting up a matrix when a relevant sequencer is mentioned */
void polymap_note(t_polymap *x, t_floatarg whosent, t_floatarg whoam, t_floatarg ono)
{
  int voicenow, mapindex, voicepoly, whosentme;
  SETFLOAT(&x->x_map.note[0], whosent);
  SETFLOAT(&x->x_map.note[1], whoam);
  SETFLOAT(&x->x_map.note[2], ono);
  whosentme = (int)whosent;
  mapindex = (int)((whosent-1)+(whoam-1)*32);
  if(ono==0)
    {
      voicenow = atom_getfloatarg(mapindex, POLYMAP, x->x_map.polymap);
      if(voicenow>0&&mapindex>=0)
	{
	  voicepoly = atom_getfloatarg(whosentme, MAXPOLY, x->x_map.polybins);
	  voicepoly--;
	  x->polynow--;
	  SETFLOAT(&x->x_map.polybins[whosentme], voicepoly);
	  SETFLOAT(&x->x_map.polymap[mapindex], 0);
	  outlet_float(x->poly, x->polynow);
	  if(x->polynow<x->maxpoly)
	    {
	      outlet_float(x->note, whosentme);
	    }
	}
      else outlet_list(x->overflow, &s_list, 3, x->x_map.note);
    }
  else
    {
      voicenow = atom_getfloatarg(mapindex, POLYMAP, x->x_map.polymap);
      if(voicenow==0&&x->polynow<x->maxpoly&&mapindex>=0)
	{
	  voicepoly = atom_getfloatarg(whosentme, MAXPOLY, x->x_map.polybins);
	  voicepoly++;
	  x->polynow++;
	  SETFLOAT(&x->x_map.polybins[whosentme], voicepoly);
	  SETFLOAT(&x->x_map.polymap[mapindex], 1);
	  outlet_float(x->poly, x->polynow);
	  outlet_float(x->new, whoam);
	}
      else if(voicenow==0) outlet_list(x->overflow, &s_list, 3, x->x_map.note);
    }
}

void polymap_print(t_polymap *x)
{
  int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, indx;
  post("polymap - first 16x16");
  post("   1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16");
  for(indx=0;indx<16;indx++)
    {
      a = atom_getfloatarg((indx*32), POLYMAP, x->x_map.polymap);
      b = atom_getfloatarg((indx*32)+1, POLYMAP, x->x_map.polymap);
      c = atom_getfloatarg((indx*32)+2, POLYMAP, x->x_map.polymap);
      d = atom_getfloatarg((indx*32)+3, POLYMAP, x->x_map.polymap);
      e = atom_getfloatarg((indx*32)+4, POLYMAP, x->x_map.polymap);
      f = atom_getfloatarg((indx*32)+5, POLYMAP, x->x_map.polymap);
      g = atom_getfloatarg((indx*32)+6, POLYMAP, x->x_map.polymap);
      h = atom_getfloatarg((indx*32)+7, POLYMAP, x->x_map.polymap);
      i = atom_getfloatarg((indx*32)+8, POLYMAP, x->x_map.polymap);
      j = atom_getfloatarg((indx*32)+9, POLYMAP, x->x_map.polymap);
      k = atom_getfloatarg((indx*32)+10, POLYMAP, x->x_map.polymap);
      l = atom_getfloatarg((indx*32)+11, POLYMAP, x->x_map.polymap);
      m = atom_getfloatarg((indx*32)+12, POLYMAP, x->x_map.polymap);
      n = atom_getfloatarg((indx*32)+13, POLYMAP, x->x_map.polymap);
      o = atom_getfloatarg((indx*32)+14, POLYMAP, x->x_map.polymap);
      p = atom_getfloatarg((indx*32)+15, POLYMAP, x->x_map.polymap);
      if(indx<9)
	post("%d :%d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d", indx+1, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p);
      else if(indx>=9)
	post("%d:%d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d  %d", indx+1, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p);
    }
}

void polymap_print32(t_polymap *x)
{
  int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, aa, bb, cc, dd, ee, ff, gg, hh, ii, jj, kk, ll, mm, nn, oo, pp, indx;
  post("polymap - all 32x32");
  post("     2   4   6   8   10  12  14  16  18  20  22  24  26  28  30  32");
  post("   1   3   5   7   9   11  13  15  17  19  21  23  25  27  29  31  ");
  for(indx=0;indx<32;indx++)
    {
      a = atom_getfloatarg((indx*32), POLYMAP, x->x_map.polymap);
      b = atom_getfloatarg((indx*32)+1, POLYMAP, x->x_map.polymap);
      c = atom_getfloatarg((indx*32)+2, POLYMAP, x->x_map.polymap);
      d = atom_getfloatarg((indx*32)+3, POLYMAP, x->x_map.polymap);
      e = atom_getfloatarg((indx*32)+4, POLYMAP, x->x_map.polymap);
      f = atom_getfloatarg((indx*32)+5, POLYMAP, x->x_map.polymap);
      g = atom_getfloatarg((indx*32)+6, POLYMAP, x->x_map.polymap);
      h = atom_getfloatarg((indx*32)+7, POLYMAP, x->x_map.polymap);
      i = atom_getfloatarg((indx*32)+8, POLYMAP, x->x_map.polymap);
      j = atom_getfloatarg((indx*32)+9, POLYMAP, x->x_map.polymap);
      k = atom_getfloatarg((indx*32)+10, POLYMAP, x->x_map.polymap);
      l = atom_getfloatarg((indx*32)+11, POLYMAP, x->x_map.polymap);
      m = atom_getfloatarg((indx*32)+12, POLYMAP, x->x_map.polymap);
      n = atom_getfloatarg((indx*32)+13, POLYMAP, x->x_map.polymap);
      o = atom_getfloatarg((indx*32)+14, POLYMAP, x->x_map.polymap);
      p = atom_getfloatarg((indx*32)+15, POLYMAP, x->x_map.polymap);
      aa = atom_getfloatarg((indx*32)+16, POLYMAP, x->x_map.polymap);
      bb = atom_getfloatarg((indx*32)+17, POLYMAP, x->x_map.polymap);
      cc = atom_getfloatarg((indx*32)+18, POLYMAP, x->x_map.polymap);
      dd = atom_getfloatarg((indx*32)+19, POLYMAP, x->x_map.polymap);
      ee = atom_getfloatarg((indx*32)+20, POLYMAP, x->x_map.polymap);
      ff = atom_getfloatarg((indx*32)+21, POLYMAP, x->x_map.polymap);
      gg = atom_getfloatarg((indx*32)+22, POLYMAP, x->x_map.polymap);
      hh = atom_getfloatarg((indx*32)+23, POLYMAP, x->x_map.polymap);
      ii = atom_getfloatarg((indx*32)+24, POLYMAP, x->x_map.polymap);
      jj = atom_getfloatarg((indx*32)+25, POLYMAP, x->x_map.polymap);
      kk = atom_getfloatarg((indx*32)+26, POLYMAP, x->x_map.polymap);
      ll = atom_getfloatarg((indx*32)+27, POLYMAP, x->x_map.polymap);
      mm = atom_getfloatarg((indx*32)+28, POLYMAP, x->x_map.polymap);
      nn = atom_getfloatarg((indx*32)+29, POLYMAP, x->x_map.polymap);
      oo = atom_getfloatarg((indx*32)+30, POLYMAP, x->x_map.polymap);
      pp = atom_getfloatarg((indx*32)+31, POLYMAP, x->x_map.polymap);
      if(indx<9)
	post("%d :%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", indx+1, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll,mm,nn,oo,pp);
      else if(indx>=9)
	post("%d:%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", indx+1, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll,mm,nn,oo,pp);
    }
}

void polymap_clear(t_polymap *x)
{
  int i;
  for(i=0;i<POLYMAP;i++)
    {
      SETFLOAT(&x->x_map.polymap[i], 0);
    }
  for(i=0;i<MAXPOLY;i++)
    {
      SETFLOAT(&x->x_map.polybins[i], 0);
    }
  for(i=0;i<2;i++)
    {
      SETFLOAT(&x->x_map.note[i], 0);
    }
  x->polynow=0;
  outlet_float(x->poly, x->polynow);
}

void *polymap_new(t_floatarg poly)
{
  t_polymap *x = (t_polymap *)pd_new(polymap_class);
  int i;
  for(i=0;i<POLYMAP;i++)
    {
      SETFLOAT(&x->x_map.polymap[i], 0);
    }
  for(i=0;i<MAXPOLY;i++)
    {
      SETFLOAT(&x->x_map.polybins[i], 0);
    }
  for(i=0;i<2;i++)
    {
      SETFLOAT(&x->x_map.note[i], 0);
    }
  x->maxpoly = poly > 0 ? poly : 1;
  floatinlet_new(&x->x_obj, &x->maxpoly);
  x->note = outlet_new(&x->x_obj, &s_float);
  x->new = outlet_new(&x->x_obj, &s_float);
  x->poly = outlet_new(&x->x_obj, &s_float);
  x->overflow = outlet_new(&x->x_obj, &s_list);
  return (void *)x;
}

void polymap_setup(void)
{
  polymap_class = class_new(gensym("polymap"),
  (t_newmethod)polymap_new,
  0, sizeof(t_polymap),
  0, A_DEFFLOAT, 0);
  post("|. . . . . . . . .polymap. . . . . . . . .|");
  post("|_- polyphonic chain reaction regulator -_|");
  post("| . . . . . .Edward Kelly 2006. . . . . . |");

  class_addmethod(polymap_class, (t_method)polymap_note, gensym("note"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(polymap_class, (t_method)polymap_clear, gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(polymap_class, (t_method)polymap_print, gensym("print"), A_DEFFLOAT, 0);
  class_addmethod(polymap_class, (t_method)polymap_print32, gensym("print32"), A_DEFFLOAT, 0);
}
