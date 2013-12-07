/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include "m_pd.h"
#include "common/loud.h"
#include "hammer/file.h"

#define CAPTURE_DEFSIZE  512

typedef struct _capture
{
    t_object       x_ob;
    t_canvas      *x_canvas;
    char           x_intmode;  /* if nonzero ('x' or 'm') floats are ignored */
    float         *x_buffer;
    int            x_bufsize;
    int            x_count;
    int            x_head;
    t_hammerfile  *x_filehandle;
} t_capture;

static t_class *capture_class;

static void capture_float(t_capture *x, t_float f)
{
    if (x->x_intmode && f != (int)f)  /* CHECKME float */
	return;
    x->x_buffer[x->x_head++] = f;
    if (x->x_head >= x->x_bufsize)
	x->x_head = 0;
    if (x->x_count < x->x_bufsize)
	x->x_count++;
}

static void capture_list(t_capture *x, t_symbol *s, int ac, t_atom *av)
{
    while (ac--)
    {
        if (av->a_type == A_FLOAT)  /* CHECKME */
	    capture_float(x, av->a_w.w_float);
	av++;
    }
}

static void capture_clear(t_capture *x)
{
    x->x_count = 0;
    x->x_head = 0;
}

static void capture_count(t_capture *x)
{
    post("capture: %d items received",  /* CHECKED */
	 x->x_count);  /* CHECKED incompatible (4.07 seems buggy here) */
}

static void capture_dump(t_capture *x)
{
    int count = x->x_count;
    if (count < x->x_bufsize)
    {
	float *bp = x->x_buffer;
	while (count--)
	    outlet_float(((t_object *)x)->ob_outlet, *bp++);
    }
    else
    {
	float *bp = x->x_buffer + x->x_head;
	count = x->x_bufsize - x->x_head;
	while (count--)
	    outlet_float(((t_object *)x)->ob_outlet, *bp++);
	bp = x->x_buffer;
	count = x->x_head;
	while (count--)
	    outlet_float(((t_object *)x)->ob_outlet, *bp++);
    }
}

static int capture_formatint(int i, char *buf, int col,
			     int maxcol, char *fmt)
{
    char *bp = buf;
    int cnt = 0;
    if (col > 0)
	*bp++ = ' ', cnt++;
    cnt += sprintf(bp, fmt, i);
    if (col + cnt > maxcol)
	buf[0] = '\n', col = cnt - 1;  /* assuming col > 0 */
    else
	col += cnt;
    return (col);
}

static int capture_formatfloat(float f, char *buf, int col,
			       int maxcol, char *fmt)
{
    char *bp = buf;
    int cnt = 0;
    if (col > 0)
	*bp++ = ' ', cnt++;
    cnt += sprintf(bp, fmt, f);
    if (col + cnt > maxcol)
	buf[0] = '\n', col = cnt - 1;  /* assuming col > 0 */
    else
	col += cnt;
    return (col);
}

static int capture_formatnumber(t_capture *x, float f, char *buf,
				int col, int maxcol)
{
    char intmode = x->x_intmode;
    if (intmode == 'm')
	intmode = (f < 128 && f > -128 ? 'd' : 'x');  /* CHECKME */
    if (intmode == 'x')
	col = capture_formatint((int)f, buf, col, maxcol, "%x");
    else if (intmode)
	col = capture_formatint((int)f, buf, col, maxcol, "%d");
    else
	col = capture_formatfloat(f, buf, col, maxcol, "%g");
    return (col);
}

static int capture_writefloat(t_capture *x, float f, char *buf, int col,
			      FILE *fp)
{
    /* CHECKED no linebreaks (FIXME) */
    col = capture_formatnumber(x, f, buf, col, 80);
    return (fputs(buf, fp) < 0 ? -1 : col);
}

static void capture_dowrite(t_capture *x, t_symbol *fn)
{
    FILE *fp = 0;
    int count = x->x_count;
    char buf[MAXPDSTRING];
    canvas_makefilename(x->x_canvas, fn->s_name, buf, MAXPDSTRING);
    if (fp = sys_fopen(buf, "w"))  /* LATER ask if overwriting, CHECKED */
    {
	int col = 0;
	if (count < x->x_bufsize)
	{
	    float *bp = x->x_buffer;
	    while (count--)
		if ((col = capture_writefloat(x, *bp++, buf, col, fp)) < 0)
		    goto fail;
	}
	else
	{
	    float *bp = x->x_buffer + x->x_head;
	    count = x->x_bufsize - x->x_head;
	    while (count--)
		if ((col = capture_writefloat(x, *bp++, buf, col, fp)) < 0)
		    goto fail;
	    bp = x->x_buffer;
	    count = x->x_head;
	    while (count--)
		if ((col = capture_writefloat(x, *bp++, buf, col, fp)) < 0)
		    goto fail;
	}
	if (col) fputc('\n', fp);
	fclose(fp);
	return;
    }
fail:
    if (fp) fclose(fp);
    loud_syserror((t_pd *)x, 0);
}

static void capture_writehook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    capture_dowrite((t_capture *)z, fn);
}

static void capture_write(t_capture *x, t_symbol *s)
{
    if (s && s != &s_)
	capture_dowrite(x, s);
    else
	hammerpanel_save(x->x_filehandle, 0, 0);
}

static int capture_appendfloat(t_capture *x, float f, char *buf, int col)
{
    /* CHECKED 80 columns */
    col = capture_formatnumber(x, f, buf, col, 80);
    hammereditor_append(x->x_filehandle, buf);
    return (col);
}

static void capture_open(t_capture *x)
{
    int count = x->x_count;
    char buf[MAXPDSTRING];
    hammereditor_open(x->x_filehandle, "Capture", "");  /* CHECKED */
    if (count < x->x_bufsize)
    {
	float *bp = x->x_buffer;
	int col = 0;
	while (count--)
	    col = capture_appendfloat(x, *bp++, buf, col);
    }
    else
    {
	float *bp = x->x_buffer + x->x_head;
	int col = 0;
	count = x->x_bufsize - x->x_head;
	while (count--)
	    col = capture_appendfloat(x, *bp++, buf, col);
	bp = x->x_buffer;
	count = x->x_head;
	while (count--)
	    col = capture_appendfloat(x, *bp++, buf, col);
    }
}

/* CHECKED without asking and storing the changes */
static void capture_wclose(t_capture *x)
{
    hammereditor_close(x->x_filehandle, 0);
}

static void capture_click(t_capture *x, t_floatarg xpos, t_floatarg ypos,
			  t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    capture_open(x);
}

static void capture_free(t_capture *x)
{
    hammerfile_free(x->x_filehandle);
    if (x->x_buffer)
	freebytes(x->x_buffer, x->x_bufsize * sizeof(*x->x_buffer));
}

static void *capture_new(t_symbol *s, t_floatarg f)
{
    t_capture *x = 0;
    float *buffer;
    int bufsize = (int)f;  /* CHECKME */
    if (bufsize <= 0)  /* CHECKME */
	bufsize = CAPTURE_DEFSIZE;
    if (buffer = getbytes(bufsize * sizeof(*buffer)))
    {
	x = (t_capture *)pd_new(capture_class);
	x->x_canvas = canvas_getcurrent();
	if (s && s != &s_)
	{
	    if (s == gensym("x"))
		x->x_intmode = 'x';
	    else if (s == gensym("m"))
		x->x_intmode = 'm';
	    else
		x->x_intmode = 'd';  /* ignore floats */
	}
	x->x_buffer = buffer;
	x->x_bufsize = bufsize;
	outlet_new((t_object *)x, &s_float);
	x->x_filehandle = hammerfile_new((t_pd *)x, 0, 0, capture_writehook, 0);
	capture_clear(x);
    }
    return (x);
}

void capture_setup(void)
{
    capture_class = class_new(gensym("capture"),
			      (t_newmethod)capture_new,
			      (t_method)capture_free,
			      sizeof(t_capture), 0, A_DEFFLOAT, A_DEFSYM, 0);
    class_addfloat(capture_class, capture_float);
    class_addlist(capture_class, capture_list);
    class_addmethod(capture_class, (t_method)capture_clear,
		    gensym("clear"), 0);
    class_addmethod(capture_class, (t_method)capture_count,
		    gensym("count"), 0);
    class_addmethod(capture_class, (t_method)capture_dump,
		    gensym("dump"), 0);
    class_addmethod(capture_class, (t_method)capture_write,
		    gensym("write"), A_DEFSYM, 0);
    class_addmethod(capture_class, (t_method)capture_open,
		    gensym("open"), 0);
    class_addmethod(capture_class, (t_method)capture_wclose,
		    gensym("wclose"), 0);
    class_addmethod(capture_class, (t_method)capture_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    hammerfile_setup(capture_class, 0);
}
