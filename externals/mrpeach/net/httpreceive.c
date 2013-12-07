/* httpreceive.c Started by Martin Peach 20110111 */
/* httpreceive will process http 1.1 responses received as lists of bytes from something like [tcpclient] */
/* See http://www.w3.org/Protocols/rfc2616/rfc2616.html */

#include "m_pd.h"
#include <stdio.h>
#include <string.h>

static t_class *httpreceive_class;

#define STATUS_BUF_LEN 4096 /* how big can the status part get? */

typedef struct _httpreceive
{
    t_object    x_obj;
    t_outlet    *x_status_out;
    t_outlet    *x_message_out;
    t_outlet    *x_statuscode_out;
    int         x_state;
    int         x_remaining;
    int         x_verbosity;
    char        *x_status_buf;
    size_t      x_status_buf_len;
    int         x_status_buf_write_index;
} t_httpreceive;

#define MAX_GETSTRING 256

static void httpreceive_bang(t_httpreceive *x);
static void httpreceive_list(t_httpreceive *x, t_symbol *s, int argc, t_atom *argv);
static void httpreceive_verbosity(t_httpreceive *x, t_floatarg verbosity);
static void httpreceive_free (t_httpreceive *x);
static void *httpreceive_new (void);
void httpreceive_setup(void);

static void httpreceive_bang(t_httpreceive *x)
{
  post("httpreceive_bang %p", x);
}

static void httpreceive_list(t_httpreceive *x, t_symbol *s, int argc, t_atom *argv)
{
    int     i, j, k, m, n, message_len = 0;
    char    buf[256];
    t_atom  status_list[2];

    /* first check that all the atoms are integer floats on 0-255 */
    for (i = 0; i < argc; ++i)
    {
        if (argv[i].a_type != A_FLOAT)
        {
            pd_error(x, "list element %d is not a float", i);
            return;
        }
        j = argv[i].a_w.w_float;
        if (j != argv[i].a_w.w_float)
        {
            pd_error(x, "list element %d is not an integer", i);
            return;
        }
        if ((j < -128) || (j > 255))
        {
            pd_error(x, "list element %d is not on [0...255]", i);
            return;
        }
    }
    if (x->x_verbosity) post ("httpreceive_list %d elements, x_state %d x_remaining %d", i, x->x_state, x->x_remaining);

 
    for (i = 0; ((i < argc) && (x->x_state == 0)); ++i)
    {
        j = argv[i].a_w.w_float;
        x->x_status_buf[x->x_status_buf_write_index] = j;
        /* status ends with CRLFCRLF */
        if
        (
            (x->x_status_buf_write_index > 3)
            && (j == 10) && (x->x_status_buf[x->x_status_buf_write_index-1] == 13)
            && (x->x_status_buf[x->x_status_buf_write_index-2] == 10)
            && (x->x_status_buf[x->x_status_buf_write_index-3] == 13)
        )
        {
            x->x_state = 1;/* complete status header is in x->x_status_buf */
            x->x_status_buf[x->x_status_buf_write_index+1] = 0;
            if (x->x_verbosity) post("httpreceive_list: status: %s", x->x_status_buf);
            /* get status code from first line */
            if ((1 != sscanf(x->x_status_buf, "HTTP/1.1 %d", &j)) && (1 != sscanf(x->x_status_buf, "HTTP/1.0 %d", &j)))
            {
                pd_error(x, "httpreceive_list: malformed status line");
                post("httpreceive_list: status: %s", x->x_status_buf);
            }
            else
            {
                outlet_float(x->x_statuscode_out, j);
                if (x->x_verbosity) post("httpreceive_list: status code: %d", j);
                for (j = 8; j < x->x_status_buf_write_index; ++j)
                if (x->x_status_buf[j] >= 'A') break; /* skip to start of reason phrase */
                for (k = 0; j < x->x_status_buf_write_index; ++j, ++k)
                {
                    if  (13 == x->x_status_buf[j]) break;
                    buf[k] = x->x_status_buf[j];/* copy reason phrase to buf */
                }
                buf[k] = 0;
                /* make buf into a symbol */
                SETSYMBOL(&status_list[0], gensym(buf));
                /* output it through status outlet */ 
                outlet_anything(x->x_status_out, gensym("reason"), 1, &status_list[0]);
                /* loop through all the response header fields */                        

                do
                {
                    /* skip to first non-whitespace on next line: */
                    for (; j < x->x_status_buf_write_index; ++j)
                    if (x->x_status_buf[j] > 32) break;
                    /* copy the field name to buf */
                    for (k = 0; j < x->x_status_buf_write_index; ++j, ++k)
                    {
                        if (':' == x->x_status_buf[j]) break;
                        buf[k] = x->x_status_buf[j];
                    }
                    j++; /* skip the colon */
                    buf[k] = 0;
                    SETSYMBOL(&status_list[0], gensym(buf)); /* field name */
                    /* skip whitespace: */
                    for (; j < x->x_status_buf_write_index; ++j)
                    if (x->x_status_buf[j] != 32) break;
                    /* copy the token to buf */
                    for (k = 0; j < x->x_status_buf_write_index; ++j, ++k)
                    {
                        if (13 == x->x_status_buf[j]) break;
                        buf[k] = x->x_status_buf[j];
                    }
                    buf[k] = 0;
                    /* if the value is a number, set it to a float, else it's a symbol */
                    for (m = 0; m < k; ++m) if ((buf[m] < '0' || buf[m] > '9')) break;
                    if (m == k)
                    {                            
                        sscanf(buf, "%d", &n);
                        SETFLOAT(&status_list[1], n);
                        /* if this is Content-Length, we know the message_length */
                        if (atom_getsymbol(&status_list[0]) == gensym("Content-Length")) x->x_remaining = n;
                    }
                    else SETSYMBOL(&status_list[1], gensym(buf));
                    outlet_anything(x->x_status_out, status_list[0].a_w.w_symbol, 1, &status_list[1]);
                } while (j < x->x_status_buf_write_index-3);

            }
        } // if end of status response
        else x->x_status_buf_write_index++;
        if (x->x_status_buf_write_index >= x->x_status_buf_len)
        {
            pd_error(x, "httpreceive_list: status buffer full");
            x->x_status_buf_write_index = 0;
            x->x_state = 0;
            x->x_remaining = 0;
        }
    } // for each byte
    if (1 == x->x_state)
    {
        /* any remaining atoms are the message body. For now just output them */
        /* but if the incoming bytes are in more than one list this won't work, */
        /*we'll need to cache them until we have Content-Length */
        if (x->x_verbosity) post ("httpreceive_list: x->x_remaining is %d", x->x_remaining);
        message_len = argc - i;
        if (x->x_verbosity) post ("httpreceive_list: message_len is %d", message_len);
        if (message_len <= x->x_remaining) x->x_remaining -= message_len;
        outlet_list(x->x_message_out, &s_list, message_len, &argv[i]);
        x->x_status_buf_write_index = 0;
        if (0 == x->x_remaining) x->x_state = 0;
    }
}

static void httpreceive_verbosity(t_httpreceive *x, t_float verbosity)
{
    x->x_verbosity = verbosity;
    if (x->x_verbosity != 0) post ("httpreceive_verbosity %d", x->x_verbosity);
}

static void httpreceive_free (t_httpreceive *x)
{
    if ((NULL != x->x_status_buf)&&(0 != x->x_status_buf_len)) freebytes(x->x_status_buf, x->x_status_buf_len);
}

static void *httpreceive_new (void)
{
    t_httpreceive *x = (t_httpreceive *)pd_new(httpreceive_class);
    if (NULL != x)
    {
        x->x_message_out = outlet_new(&x->x_obj, &s_anything);
        x->x_status_out = outlet_new(&x->x_obj, &s_anything);
        x->x_statuscode_out = outlet_new(&x->x_obj, &s_float);  /* rightmost outlet */
        x->x_state = 0; /* waiting for a list of bytes */
        x->x_status_buf_len = STATUS_BUF_LEN;
        if (NULL == (x->x_status_buf = getbytes(STATUS_BUF_LEN)))
        {
            pd_error(x, "httpreceive_new: no memory available for x_status_buf");
            x->x_status_buf_len = 0;
        }
        x->x_status_buf_write_index = 0;
        x->x_remaining = 0;
        x->x_verbosity = 0;
    }
    
    return (void *)x;
}

void httpreceive_setup(void)
{
    httpreceive_class = class_new(gensym("httpreceive"), (t_newmethod)httpreceive_new, (t_method)httpreceive_free, sizeof(t_httpreceive), CLASS_DEFAULT, 0);
    class_addbang(httpreceive_class, httpreceive_bang);
    class_addlist (httpreceive_class, (t_method)httpreceive_list);
    class_addmethod(httpreceive_class, (t_method)httpreceive_verbosity, gensym("verbosity"), A_FLOAT, 0);
}
/* fin httpreceive.c */
