/*
Copyright (C) 2003 Antoine Rousseau

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/* a shared symbol array, ala "value" .*/

#include "m_pd.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static t_class *dispatch_class, *dispsnd_class;
static t_symbol *s__;

typedef struct _dispatch t_dispatch;

typedef struct dispsnd
{
    t_pd d_pd;
    t_symbol *d_eachsnd;
    t_symbol *d_allsnd;
    t_int d_num;
} t_dispsnd;

struct _dispatch
{
    t_object x_obj;
    t_symbol *x_sym;
    int x_from;
    int x_to;

    t_dispsnd **x_snds;

    t_symbol *x_allrcv;
    t_symbol **x_eachrcvs;
};

/*--------------------- dispsnd ------------------------------------*/

static void dispsnd_ff(t_dispsnd *x)
{
    pd_unbind((t_pd *)x, x->d_eachsnd);
}

static void *dispsnd_new(t_symbol *eachsnd,t_symbol *allsnd,int num)
{
    t_dispsnd *x = (t_dispsnd *)pd_new(dispsnd_class);

    //post("new dispsnd: num=%d rcv=%s snd=%s",num,eachsnd->s_name,allsnd->s_name);
    x->d_eachsnd=eachsnd;
    x->d_allsnd=allsnd;
    x->d_num=num;

    pd_bind((t_pd *)x, x->d_eachsnd);

    return (x);
}

static void dispsnd_float(t_dispsnd *x, t_float f)
{
    t_atom out[2];

    if (x->d_allsnd->s_thing)
    {
        SETFLOAT(&out[0],x->d_num);
        SETFLOAT(&out[1],f);

        typedmess(x->d_allsnd->s_thing, &s_list, 2, out);
    }
}

static void dispsnd_anything(t_dispsnd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *out;

    if (x->d_allsnd->s_thing)
    {
        out = (t_atom *)getbytes(sizeof(t_atom)*(argc+2));
        memcpy(&out[2], argv, argc*sizeof(t_atom));
        SETFLOAT(&out[0],x->d_num);
        SETSYMBOL(&out[1],s);

        typedmess(x->d_allsnd->s_thing, &s_list, argc+2, out);

        freebytes(out, sizeof(t_atom)*(argc+2));
    }
}



/*--------------------- dispatch ------------------------------------*/

static void *dispatch_new(t_symbol *s,t_float from,t_float to)
{
    int i,len;
    t_dispatch *x = (t_dispatch *)pd_new(dispatch_class);
    char str[512];
    t_symbol *allsnd,*eachsnd;

    x->x_snds=0;
    x->x_sym = s;
    x->x_from = from;
    x->x_to = to;
    len=x->x_to-x->x_from+1;

    if(len>0)
    {
        sprintf(str,"%s-snd",x->x_sym->s_name);
        allsnd=gensym(str);

        sprintf(str,"%s-rcv",x->x_sym->s_name);
        x->x_allrcv=gensym(str);
        pd_bind((t_pd *)x, x->x_allrcv);

        x->x_snds=getbytes(len*sizeof(t_dispsnd *));
        x->x_eachrcvs=getbytes(len*sizeof(t_symbol *));

        for(i=0; i<len; i++)
        {
            sprintf(str,"%s%d-snd",x->x_sym->s_name,i+x->x_from);
            eachsnd=gensym(str);
            x->x_snds[i]=dispsnd_new(eachsnd,allsnd,i+x->x_from);

            sprintf(str,"%s%d-rcv",x->x_sym->s_name,i+x->x_from);
            x->x_eachrcvs[i]=gensym(str);
        }
    }
    return (x);
}

static void dispatch_ff(t_dispatch *x)
{
    int i,len=x->x_to-x->x_from+1;

    if(len<=0) return;

    pd_unbind((t_pd *)x, x->x_allrcv);

    for(i=0; i<len; i++) pd_free((t_pd *)x->x_snds[i]);

    freebytes(x->x_snds,len*sizeof(t_dispsnd *));
    freebytes(x->x_eachrcvs,len*sizeof(t_symbol *));
}


static void dispatch_list(t_dispatch *x, t_symbol *s, int argc, t_atom *argv)
{
    int num;

    if((!argc)|(argv[0].a_type!=A_FLOAT))
    {
        error("dispatch: bad list format");
        return;
    }

    num=atom_getint(&argv[0]);

    if((num<x->x_from)|(num>x->x_to))
    {
        //error("dispatch: bad num");
        return;
    }

    if (x->x_eachrcvs[num-x->x_from]->s_thing)
        pd_forwardmess(x->x_eachrcvs[num-x->x_from]->s_thing, argc-1, argv+1);
}




/*--------------------------------------------------------------*/

void dispatch_setup(void)
{
    dispatch_class = class_new(gensym("dispatch"), (t_newmethod)dispatch_new,
                               (t_method)dispatch_ff,
                               sizeof(t_dispatch), 0, A_SYMBOL, A_FLOAT, A_FLOAT,0);

    class_addlist(dispatch_class, dispatch_list);
    dispsnd_class = class_new(gensym("dispatch"), 0, (t_method)dispsnd_ff,
                              sizeof(t_dispsnd), CLASS_PD, 0);
    class_addanything(dispsnd_class, dispsnd_anything);
    class_addfloat(dispsnd_class, dispsnd_float);
}

