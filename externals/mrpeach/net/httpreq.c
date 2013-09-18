/* httptreq.c Started by Martin Peach 20110111 */
/* httpreq will generate http 1.1 requests as lists of bytes suitable for input to tcpclient */
/* See http://www.w3.org/Protocols/rfc2616/rfc2616.html */

#include "m_pd.h"
#include <stdio.h>
#include <string.h>

static t_class *httpreq_class;

typedef struct _httpreq
{
    t_object    x_obj;
    t_outlet    *x_reqout;
    int         x_verbosity;    
} t_httpreq;

#define MAX_GETSTRING 256
static void httpreq_bang(t_httpreq *x);
static void httpreq_get(t_httpreq *x, t_symbol *s);
static void httpreq_head(t_httpreq *x, t_symbol *s);
static void httpreq_verbosity(t_httpreq *x, t_floatarg verbosity);
static void *httpreq_new (void);
void httpreq_setup(void);

static void httpreq_bang(t_httpreq *x)
{
  post("httpreq_bang %p", x);
}

static void httpreq_get(t_httpreq *x, t_symbol *s)
{
    unsigned int    i, j, len;    
    char            buf[MAX_GETSTRING];
    char            request_line[1024];
    t_atom          request_atoms[1024];

    len = strlen (s->s_name);
    if (len > MAX_GETSTRING)
    {
        pd_error(x, "httpreq_get: string too long (%d), should be less than %d", len, MAX_GETSTRING);
        return;
    }
    for (i = 0; i < strlen(s->s_name); ++i) buf[i] = s->s_name[i];
    buf[i] = 0;    

    if (0 != strncmp("http://", buf, 7))
    {
        pd_error(x, "httpreq_get: url doesn't begin with 'http://' (%d)", len);
        return;
    }
/*
5.1 Request-Line
The Request-Line begins with a method token, 
followed by the Request-URI and the protocol version, 
and ending with CRLF. 
The elements are separated by SP characters. 
No CR or LF is allowed except in the final CRLF sequence.

Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
*/
    j = sprintf(request_line, "GET ");
    for (i = 7; i < len; ++i)
    { /* skip  "http://" and the host name */
        if ('/' == buf[i]) break;
    }
    for (; i < len; ++i, ++j)
    {
        if (buf[i] <= 0x20) break;
        request_line[j] = buf[i];
    }
    j += sprintf(&request_line[j], " HTTP/1.1");
    request_line[j++] = 0xD; // <CR>
    request_line[j++] = 0xA; // <LF>
    j += sprintf(&request_line[j], "Host: ");
    for (i = 7; i < len; ++i, ++j)
    { /* copy the host name */
        if ('/' == buf[i]) break;
        request_line[j] = buf[i];
    }
    request_line[j++] = 0xD; // <CR>
    request_line[j++] = 0xA; // <LF>
    request_line[j++] = 0xD; // <CR>
    request_line[j++] = 0xA; // <LF>
    request_line[j] = 0; // terminate string

/* output the request line as a list of floats */
    for (i = 0; i < j; ++i)
    {
        SETFLOAT(&request_atoms[i], request_line[i]);
    }
    if (x->x_verbosity) post("httpreq_get: %s", request_line);
    outlet_list(x->x_reqout, &s_list, j, &request_atoms[0]);
}

static void httpreq_head(t_httpreq *x, t_symbol *s)
{ /* this is the same as get except for the method */
    unsigned int    i, j, len;    
    char            buf[MAX_GETSTRING];
    char            request_line[1024];
    t_atom          request_atoms[1024];

    len = strlen (s->s_name);
    if (len > MAX_GETSTRING)
    {
        pd_error(x, "httpreq_head: string too long (%d), should be less than %d", len, MAX_GETSTRING);
        return;
    }
    for (i = 0; i < strlen(s->s_name); ++i) buf[i] = s->s_name[i];
    buf[i] = 0;    

    if (0 != strncmp("http://", buf, 7))
    {
        pd_error(x, "httpreq_head: url doesn't begin with 'http://' (%d)", len);
        return;
    }

    j = sprintf(request_line, "HEAD ");
    for (i = 7; i < len; ++i)
    { /* skip  "http://" and the host name */
        if ('/' == buf[i]) break;
    }
    for (; i < len; ++i, ++j)
    {
        if (buf[i] <= 0x20) break;
        request_line[j] = buf[i];
    }
    j += sprintf(&request_line[j], " HTTP/1.1");
    request_line[j++] = 0xD; // <CR>
    request_line[j++] = 0xA; // <LF>
    j += sprintf(&request_line[j], "Host: ");
    for (i = 7; i < len; ++i, ++j)
    { /* copy the host name */
        if ('/' == buf[i]) break;
        request_line[j] = buf[i];
    }
    request_line[j++] = 0xD; // <CR>
    request_line[j++] = 0xA; // <LF>
    request_line[j++] = 0xD; // <CR>
    request_line[j++] = 0xA; // <LF>
    request_line[j] = 0; // terminate string

/* output the request line as a list of floats */
    for (i = 0; i < j; ++i)
    {
        SETFLOAT(&request_atoms[i], request_line[i]);
    }
    if (x->x_verbosity) post("httpreq_head: %s", request_line);
    outlet_list(x->x_reqout, &s_list, j, &request_atoms[0]);
}

static void httpreq_verbosity(t_httpreq *x, t_float verbosity)
{
    x->x_verbosity = verbosity;
    if (x->x_verbosity != 0) post ("httpreq_verbosity %d", x->x_verbosity);
}

static void *httpreq_new (void)
{
    t_httpreq *x = (t_httpreq *)pd_new(httpreq_class);
    if (NULL != x)
    {
        x->x_reqout = outlet_new(&x->x_obj, &s_anything);
    }
    return (void *)x;
}

void httpreq_setup(void)
{
    httpreq_class = class_new(gensym("httpreq"), (t_newmethod)httpreq_new, 0, sizeof(t_httpreq), CLASS_DEFAULT, 0);
    class_addbang(httpreq_class, httpreq_bang);
    class_addmethod (httpreq_class, (t_method)httpreq_get, gensym ("GET"), A_DEFSYM, 0);
    class_addmethod (httpreq_class, (t_method)httpreq_head, gensym ("HEAD"), A_DEFSYM, 0);
    class_addmethod(httpreq_class, (t_method)httpreq_verbosity, gensym("verbosity"), A_FLOAT, 0);
}
/* fin httpreq.c */
