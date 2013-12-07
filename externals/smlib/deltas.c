#include "defines.h"

/*--------------- deltas ---------------*/

static t_class *deltas_class;

typedef struct _deltas
{
    t_object x_obj;
	t_float m_lo;
	t_float m_hi;
	int m_buffer_size;
	int m_buffer_index;
	t_float *m_buffer;  // circular buffer
} t_deltas;

static void deltas_set(t_deltas *x, t_float lo, t_float hi, t_float size);

static void deltas_perform_float(t_deltas *x, t_float f)
{
	int index;
	index=x->m_buffer_index+1;
	index=(index==x->m_buffer_size)?0:index;
	x->m_buffer_index=index;
	x->m_buffer[index]=f;
}

static void deltas_bang(t_deltas *x)
{
	int lo,hi,n,index,size;
	t_atom *ap,*app;
	t_float last;
	t_float *buffer, *bp;

    deltas_set(x, x->m_lo, x->m_hi, x->m_buffer_size);
	lo=(int)x->m_lo;
	hi=(int)x->m_hi;

	n=hi-lo;
 	size=x->m_buffer_size;
	index=x->m_buffer_index;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
	app=ap;
	buffer=x->m_buffer;
	last=buffer[index];
	bp=buffer+index-lo;
	bp=(bp>=buffer)?bp:bp+size; // wrap
	
	if (bp-buffer>=n)
	{ // no wrap-around needed
		index=n;
		while(index--){
		  SETFLOAT(app, last-*bp--);
		  app++;
		}
//		post("not wrapped, app-ap=%i",app-ap);
	}
	else // need to wrap
	{
		int ps, nn;
		ps = bp-buffer;
		nn=n;
//		post("        nn=%i",nn);
		for(;ps>=0;ps--)  // don't we miss one sample in signal??? 
		{
//			post("ps=%i",ps);
 			SETFLOAT(app, last-buffer[ps]);
			app++;
			nn--;
		}
		ps=size-1;
//		post("       nn=%i",nn);
		for(;nn>0;nn--)
		{
//			post("ps=%i",ps);
 			SETFLOAT(app, last-buffer[ps--]);
			app++;
		}

/*
		int i2;
		index=bp-buffer;
		i2=index;
		post("first part %i",index);
		while(index--){
		  SETFLOAT(app, last-*bp--);
		  app++;
		}
		index=n-i2;
		post("2nd part %i",index);
		bp=buffer+size-1;
		while(index--){
		  SETFLOAT(app, last-*bp--);
		  app++;
		}
*/
//		post("wrapped, app-ap=%i",app-ap);
	}

	outlet_list(x->x_obj.ob_outlet,gensym("list"),n,ap);
	freebytes(ap, sizeof(t_atom)*n);
}

static void deltas_clear(t_deltas *x)
{
	int i,s;
	t_float *f;
	f=x->m_buffer;
	s=x->m_buffer_size;
	for (i=0;i<s;i++)
		*f++=0.0;
}

static void deltas_set(t_deltas *x, t_float lo, t_float hi, t_float size)
{
	if (size<1)
	{
		size=1;
		logpost(x, 2, "[deltas]: minimum size is 1");
	}
	if (hi>size)
	{
		logpost(x, 2, "[deltas]: higher bound (%g) cannot be greater than the buffer size (%g)",
                hi, size);	
		hi=size;
	}
	if (lo<0)
	{
		logpost(x, 2, "[deltas]: lower bound cannot be negative");
		lo=0;
	}
	if (hi<1)
	{
		logpost(x, 2, "[deltas]: higher bound cannot be smaller than one");
		hi=1;
	}
	if (hi<=lo)
	{
		logpost(x, 2, "[deltas]: higher bound (%g) must be greater than lower bound (%g)", hi, lo);	
		lo=hi-1.0;
	}

	x->m_hi=(t_float)((int)hi);
	x->m_lo=(t_float)((int)lo);

    if (x->m_buffer_size != size)
    {
        freebytes(x->m_buffer, x->m_buffer_size);
        x->m_buffer_size = (int)size;
        x->m_buffer = (t_float*)getbytes(sizeof(t_float)*x->m_buffer_size);
        deltas_clear(x);
        x->m_buffer_index=0;
    }
}

static void *deltas_new(t_float lo, t_float hi, t_float size)
{
	t_deltas *x=(t_deltas *)pd_new(deltas_class);
	outlet_new(&x->x_obj, gensym("list"));
	x->m_buffer_size=0;
	x->m_buffer=0;
	deltas_set(x, lo, hi, size);

    floatinlet_new(&x->x_obj, &x->m_lo);
    floatinlet_new(&x->x_obj, &x->m_hi);

	return (void *)x;
}

static void deltas_free(t_deltas *x)
{
	freebytes(x->m_buffer, x->m_buffer_size);
}

void deltas_setup(void)
{
    deltas_class = class_new(gensym("deltas"),
    	(t_newmethod)deltas_new, (t_method)deltas_free,
		sizeof(t_deltas), 
		CLASS_DEFAULT,
	    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT,0);

	class_addmethod(deltas_class, (t_method)deltas_clear, gensym("clear"),0);
    class_addfloat(deltas_class, (t_method)deltas_perform_float);
    class_addbang(deltas_class, (t_method)deltas_bang);
}

