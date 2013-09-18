/* str.c MP 20061227 */
/* version 20070101 no more resizing memory */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "m_pd.h"

#ifndef PD_BLOBS /* PD_BLOBS is not defined in m_pd.h: No PD blob support: Make a dummy str object */
typedef struct _str
{
    t_object        x_obj;
} t_str;
static t_class *str_class;

void str_setup(void);
static void *str_new(t_symbol *s, int argc, t_atom *argv);

static void *str_new(t_symbol *s, int argc, t_atom *argv)
{
    t_str           *x;

    x = (t_str *)pd_new(str_class);
    if (x == NULL) return (x);
    post("This str is a dummy str.");
    return (x);
}

void str_setup(void)
{
    str_class = class_new(gensym("str"), (t_newmethod)str_new, 0, sizeof(t_str), 0, 0);
}
#else //ifndef PD_BLOBS
/* Make a _real_ str object: */

typedef enum
{
    string,
    nsplit,
    csplit,
    join,
    compare,
    add,
    nth,
    drip,
    to_list,
    to_symbol,
    to_float,
    n_functions
} str_function;

char *str_function_names[] = {"string", "nsplit", "csplit", "join", "compare", "add", "nth", "drip", "to_list", "to_symbol", "to_float"};

typedef struct _str
{
    t_object        x_obj;
    t_float         x_nsplit;
    str_function    x_function;
    t_blob        x_buf;
    t_blob        x_string_in1;
    t_blob        x_string_in2;
    t_blob        x_string_out1;
    t_blob        x_string_out2;
    t_atom          *x_atom_list;
    size_t          x_atom_list_end;
    size_t          x_buf_end;
    size_t          x_string_in1_end;/* the current end of the string. Must be less than x_string_in1.s_length */
    size_t          x_string_in2_end;
    size_t          x_string_out1_end;
    size_t          x_string_out2_end;
    t_outlet        *x_outlet_1;
    t_outlet        *x_outlet_2;
    t_inlet         *x_inlet_2;
    size_t          x_atom_list_length;
} t_str;

//typedef struct _blob /* pointer to a blob */
//{
//   unsigned long s_length; /* length of blob in bytes */
//   unsigned char *s_data; /* pointer to 1st byte of blob */
//} t_blob;

static t_class *str_class;

void str_setup(void);
static void *str_new(t_symbol *s, int argc, t_atom *argv);
static void str_free(t_str *x);
static void str_do_out1(t_str *x);
static void str_do_out2(t_str *x);
static void str_do_out3(t_str *x);
static void str_do_string(t_str *x);
static void str_bang(t_str *x);
static void str_float(t_str *x, t_float f);
static void str_symbol(t_str *x, t_symbol *s);
static void str_list(t_str *x, t_symbol *s, int argc, t_atom *argv);
static void str_anything(t_str *x, t_symbol *s, int argc, t_atom *argv);
static void str_string(t_str *x, t_blob *st);
static void str_set_string(t_blob *dest, t_blob *src, size_t *len);
static void str_buf_to_string(t_str *x, t_blob *dest);
static void str_float_to_buf(t_str *x, t_float f);
static void str_str_to_buf (t_str *x, t_atom *a);
static void str_symbol_to_buf(t_str *x, t_atom *a);
static void str_list_to_buf(t_str *x, t_atom *a, int n);
static void str_set(t_str *x, t_symbol *s, int argc, t_atom *argv);
static void str_fread(t_str *x, t_symbol *s, int argc, t_atom *argv);
static void str_fwrite(t_str *x, t_symbol *s, int argc, t_atom *argv);
static void str_set_second(t_str *x, t_blob *st);
static int str_equal(t_str *x);
static void str_join(t_str *x);
static void str_add(t_str *x);
static void str_drip(t_str *x);
static void str_nsplit(t_str *x);
static void str_csplit(t_str *x);
static t_symbol *str_to_symbol(t_str *x);
static t_float str_to_float(t_str *x);


static t_float str_to_float(t_str *x)
{/* return a float from an ASCII representation of a float at the start of the string, or the first character. */
    size_t  limit;
    t_float f = 0.0;

    limit = (x->x_string_in1.s_length-1 >= x->x_string_in1_end)? x->x_string_in1_end: x->x_string_in1.s_length-1;
    if (limit == f) return f;
    x->x_string_in1.s_data[limit] = '\0';
    if (sscanf((char *)x->x_string_in1.s_data, "%f", &f)) return f;
    return (f = x->x_string_in1.s_data[0]);
}

static t_symbol *str_to_symbol(t_str *x)
{
    size_t limit;

    limit = (x->x_string_in1.s_length-1 >= x->x_string_in1_end)? x->x_string_in1_end: x->x_string_in1.s_length-1;
    x->x_string_in1.s_data[limit] = '\0';
    return gensym((char *)x->x_string_in1.s_data);
}

static void str_csplit(t_str *x)
/* split input string 1 into output strings 1 and 2 at first occurrence of any character in input string 2 */
{
    size_t  i, j;

    /*post("str_csplit: x->x_string_in2_end is %lu", x->x_string_in2_end);*/
    for (i = 0; i < x->x_string_in1_end; ++i)
    {
        for (j = 0; j < x->x_string_in2_end; ++j )
        {
            if (x->x_string_in1.s_data[i] == x->x_string_in2.s_data[j])
            { /* found the first character, see if there are more */
                x->x_string_out1_end = x->x_buf_end = i;
                for (++i; i < x->x_string_in1_end; ++i)
                {
                    for (j = 0; j < x->x_string_in2_end; ++j )
                    {
                        if (x->x_string_in1.s_data[i] == x->x_string_in2.s_data[j])
                        break;
                    }
                    if(j == x->x_string_in2_end) goto found; /* 1st non-target character */
                }
            }
        }
        x->x_buf.s_data[i] = x->x_string_in1.s_data[i]; /* this string goes to the left outlet*/
    }
found:
    str_buf_to_string(x, &x->x_string_out1);
    j = i;
    for (i = 0; j < x->x_string_in1_end; ++i, ++j)
    {
       x->x_buf.s_data[i] = x->x_string_in1.s_data[j]; /* this string goes to the right outlet */
    }
    x->x_string_out2_end = x->x_buf_end = i;
    str_buf_to_string(x, &x->x_string_out2);
    return;
}

static void str_nsplit(t_str *x)
/* split input string 1 into output strings 1 and 2 at index x_nsplit */
{
    size_t len, i, j;

    if (x->x_nsplit >= 0)
    { /* split from start of data, no more than size of largest possible string */
        len = (unsigned long)x->x_nsplit;
        if (len > x->x_string_in1_end) len = x->x_string_in1_end;
        if (len > x->x_string_in1.s_length) len = x->x_string_in1.s_length;
    }
    else
    { /* split from the end */
        len = (unsigned long)-x->x_nsplit;
        if (len > x->x_string_in1_end) len = 0L;
        else len = x->x_string_in1_end - len;
    }
    for (i = 0; i < len; ++i) x->x_buf.s_data[i] = x->x_string_in1.s_data[i];
    x->x_string_out1_end = x->x_buf_end = len;
    str_buf_to_string(x, &x->x_string_out1);
    j = i;
    len = (len < x->x_string_in1_end)? x->x_string_in1_end - len: 0L;
    for (i = 0; i < len; ++i, ++j) x->x_buf.s_data[i] = x->x_string_in1.s_data[j];
    x->x_string_out2_end = x->x_buf_end = len;
    str_buf_to_string(x, &x->x_string_out2);
    x->x_buf_end = 0L;/* finished with x_buf */
    return;
}

static int str_equal(t_str *x)
/* If input string 1 is exactly equal to input string 2, return 1, else 0 */
{
    size_t len, i, j;

    if ((len = x->x_string_in1_end) != x->x_string_in2_end) return 0; /* not the same length */
    for (i = 0; i < len; ++i) if (x->x_string_in1.s_data[i] != x->x_string_in2.s_data[i]) return 0;
    return 1;

}

static void str_drip(t_str *x)
/* Send next character of input string 1 out outlet 1, bang outlet 2 if end of string */
{
    size_t              limit, i;
    unsigned char       c;

    limit = (x->x_string_in1_end > x->x_string_in1.s_length)? x->x_string_in1.s_length: x->x_string_in1_end;
    if (x->x_nsplit >= limit) x->x_nsplit = 0L;
    if (limit != 0)
    {/* x->x_nsplit points to the current output character */
        c = x->x_string_in1.s_data[(size_t)x->x_nsplit++];
        outlet_float(x->x_outlet_1, (float)c);
    }
    if (x->x_nsplit >= limit)
    { /* bang at end of string */
        x->x_nsplit = 0L;
        outlet_bang(x->x_outlet_2);
    }
}

static void str_add(t_str *x)
/* Append input string 1 to input string 2, result to input_string 2. Clear input string 1 */
{
    size_t len, i, j;

    len = x->x_string_in1_end + x->x_string_in2_end;
    if (len > x->x_string_in2.s_length) len = x->x_string_in2.s_length;
    for (i = 0, j = x->x_string_in2_end; ((i < x->x_string_in1_end) && (j < len)); ++i, ++j)
        x->x_string_in2.s_data[j] = x->x_string_in1.s_data[i];
    x->x_string_in2_end = j;
    x->x_string_in1_end = 0L;
    return;
}

static void str_join(t_str *x)
/* Append input string 1 to input string 2 in output string 1 */
{
    size_t len, i, j;

    len = x->x_string_in1_end + x->x_string_in2_end;
    if (len > x->x_string_out1.s_length) len = x->x_string_out1.s_length;
    for (i = 0; i < x->x_string_in1_end; ++i) x->x_string_out1.s_data[i] = x->x_string_in1.s_data[i];
    j = i;
    for (i = 0; i < x->x_string_in2_end; ++i, ++j) x->x_string_out1.s_data[j] = x->x_string_in2.s_data[i];
    x->x_string_out1_end = len;
    return;

}

static void str_list_to_buf(t_str *x, t_atom *a, int n)
/* Convert list of atoms to string in x->x_buf.s_data at offset x->x_buf_end,
* increment x->x_buf_end by the number of chars added to x->x_buf.s_data*/
{
    int     j;

    for (j = 0; j < n; ++j)
    { /* concatenate all arguments into a single string */
        if (a[j].a_type == A_BLOB)
        { /* already a string */
            str_str_to_buf(x, &a[j]);
        }
        else if (a[j].a_type == A_SYMBOL)
        {
            str_symbol_to_buf(x, &a[j]);
        }
        else if (a[j].a_type == A_FLOAT)
        { /* integers on [0..255] are taken directly, otherwise converted to strings */
            str_float_to_buf(x, atom_getfloat(&a[j]));
        }
    }
 }

static void str_str_to_buf (t_str *x, t_atom *a)
/* Copy string to string in x->x_buf.s_data at offset x->x_buf_end,
* increment x->x_buf_end by the number of chars added to x->x_buf.s_data*/
{
    size_t len, limit, i, j;
    char *cP = (char *)x->x_buf.s_data + x->x_buf_end;
    t_blob *str = atom_getblob(a);
    if (str == NULL)
    {
        post ("str_str_to_buf: null blob. Need a blob to point to....");
        return;
    }
    limit = x->x_buf.s_length - x->x_buf_end;
    len = (str->s_length > limit)? limit: str->s_length;
    for (i = 0, j = x->x_buf_end; ((i < len)&&(j < x->x_buf.s_length)); ++i, ++j)
        x->x_buf.s_data[j] = str->s_data[i];
    x->x_buf_end = j;
}

static void str_symbol_to_buf(t_str *x, t_atom *a)
/* Convert symbol to string in x->x_buf.s_data at offset x->x_buf_end,
* increment x->x_buf_end by the number of chars added to x->x_buf.s_data*/
{                                                                                                                               char *cP = (char *)x->x_buf.s_data + x->x_buf_end;
    atom_string(a, cP, x->x_buf.s_length-x->x_buf_end);
    x->x_buf_end += strlen(cP);
}

static void str_float_to_buf(t_str *x, t_float f)
/* Convert float to characters in x->x_buf.s_data at offset x->x_buf_end,
* increment x->x_buf_end by the number of chars added to x->x_buf.s_data. */
{
    long i;
    char j;

    if (x->x_buf_end > x->x_buf.s_length-20) /* what's the longest float? */
    {
        error("str_float_to_buf: string too long.");
        return;
    }
    /* A float is either an ascii character number or a floating-point string */
    i = (long)f;
    j = i & 0x0FF;
    if ((f == i)&&(i < 256)&&(i > -129)) /* is f an integer on [-128..255] */
        x->x_buf.s_data[x->x_buf_end++] = j;
    else
        x->x_buf_end += sprintf((char *)&x->x_buf.s_data[x->x_buf_end], "%f", f);
 }

static void str_buf_to_string(t_str *x, t_blob *dest)
/* copy x->x_buf_end bytes of x->x_buf.s_data into dest */
{
    size_t      i, limit;

    limit = (dest->s_length < x->x_buf.s_length)? dest->s_length: x->x_buf.s_length;
    if (limit > x->x_buf_end) limit = x->x_buf_end;/* normally the case */
//    post("str_buf_to_string: limit %lu", limit);
    for (i = 0; i < limit; ++i)
    {
        dest->s_data[i] = x->x_buf.s_data[i];
    }
//    post("str_buf_to_string: new length %lu", dest->s_length);
    return;
}

static void str_set_string(t_blob *dest, t_blob *src, size_t *len)
/* Copy src into dest up to the shorter of dest->s_length and src->s_length
*  and set len to number of bytes copied */
{
    size_t i, limit;

    *len = (dest->s_length < src->s_length)? dest->s_length: src->s_length;
    for (i = 0; i < *len; ++i) dest->s_data[i] = src->s_data[i];
}

static void str_fread(t_str *x, t_symbol *s, int argc, t_atom *argv)
{
 /* a [file_read( message can have any type */
    FILE    *fp = NULL;
    size_t  limit = 0L;
    int     err = 0;

    x->x_buf_end = limit;
    post("str_fread: argc %d", argc);
    str_list_to_buf(x, argv, argc);
    limit = (x->x_buf_end >= x->x_buf.s_length)? x->x_buf.s_length-1: x->x_buf_end;
    x->x_buf.s_data[limit] = '\0'; /* make buf a c string */
    if (0 == limit)
    {
        post ("str file_read: no path.");
        return;
    }
    errno = 0;
    fp = sys_fopen((char *)x->x_buf.s_data, "rb");
    if(NULL == fp)
    {
        post ("str file_read: error opening file \"%s\": %d", x->x_buf.s_data, errno);
        return;
    }
    limit = x->x_string_in1.s_length;
    limit = fread(x->x_string_in1.s_data, 1L, limit, fp);
    if (0 != (err = ferror(fp)))
    {
        post ("str file_read: error reading file \"%s\": %d", x->x_buf.s_data, errno);
        x->x_string_in1_end = 0L;
    }
    else
    {
        x->x_string_in1_end = limit;
        post ("str file_read: read %lu bytes", limit);
    }
    fclose(fp);
    return;
}

static void str_fwrite(t_str *x, t_symbol *s, int argc, t_atom *argv)
{
 /* a [file_write( message can have any type */
    FILE    *fp = NULL;
    size_t  limit = 0L;
    int     err = 0;

    if (0 == x->x_string_in1_end)
    {
        post ("str file_write: nothing to write");
        return;
    }
    x->x_buf_end = limit;
    post("str_fwrite: argc %d", argc);
    str_list_to_buf(x, argv, argc);
    limit = (x->x_buf_end >= x->x_buf.s_length)? x->x_buf.s_length: x->x_buf_end;
    if (0 == limit)
    {
        post ("str file_write: no path.");
        return;
    }
    errno = 0;
    fp = sys_fopen((char *)x->x_buf.s_data, "wb");
    if(NULL == fp)
    {
        post ("str file_write: error opening file \"%s\": %d", x->x_buf.s_data, errno);
        return;
    }
    rewind(fp);
    limit = x->x_string_in1_end;
    limit = fwrite(x->x_string_in1.s_data, 1L, limit, fp);
    if (0 != (err = ferror(fp)))
        post ("str file_write: error writing file \"%s\": %d", x->x_buf.s_data, errno);
    else post ("str file_write: wrote %lu bytes to \"%s\"", limit, x->x_buf.s_data);
    fclose(fp);
    return;
}

static void str_set(t_str *x, t_symbol *s, int argc, t_atom *argv)
{ /* a [set( message can have any type */
    x->x_buf_end = 0L;
    /*post("str_set: argc %d", argc);*/
    str_list_to_buf(x, argv, argc);
    if ((x->x_function == csplit) || (x->x_function == compare) || (x->x_function == join) || (x->x_function == add))
    {
        x->x_string_in2_end = x->x_buf_end;
        str_buf_to_string(x, &x->x_string_in2);
    }
    else
    {
        x->x_string_in1_end = x->x_buf_end;
        str_buf_to_string(x, &x->x_string_in1);
    }
    if ((x->x_function == add)||(x->x_function == join)) outlet_float(x->x_outlet_2, x->x_string_in2_end);
    else if ((x->x_function == nth) || (x->x_function == string)) outlet_float(x->x_outlet_2, x->x_string_in1_end);
}

static void str_set_second(t_str *x, t_blob *st)
{ /* Inlet 2 accepts blobs only: Set string_in2 */
    /*post("x=%p str_set_second(%p): %s %p %lu",
        x, &str_set_second, str_function_names[x->x_function], st, st->s_length);*/
    str_set_string(&x->x_string_in2, st, &x->x_string_in2_end);
    if ((x->x_function == add)||(x->x_function == join)) outlet_float(x->x_outlet_2, x->x_string_in2_end);
}

static void str_string(t_str *x, t_blob *st)
{
    /*post("x=%p str_string (%p) blob %p %lu", x, &str_string, st, st->s_length);*/
    str_set_string(&x->x_string_in1, st, &x->x_string_in1_end);
    if (x->x_function == drip) x->x_nsplit = 0L;
    str_do_string(x);
}

static void str_anything(t_str *x, t_symbol *s, int argc, t_atom *argv)
{
//    post("str_anything");
    x->x_buf_end = sprintf((char *)x->x_buf.s_data, "%s", s->s_name); /* the selector is just another word... */
    str_list_to_buf(x, argv, argc);
    x->x_string_in1_end = x->x_buf_end;
    str_buf_to_string(x, &x->x_string_in1);
    if (x->x_function == drip) x->x_nsplit = 0L;
    str_do_string(x);
}

static void str_list(t_str *x, t_symbol *s, int argc, t_atom *argv)
{
//    post("str_list");
    x->x_buf_end = 0L;
    str_list_to_buf(x, argv, argc);
    x->x_string_in1_end = x->x_buf_end;
    str_buf_to_string(x, &x->x_string_in1);
    if (x->x_function == drip) x->x_nsplit = 0L;
    str_do_string(x);
}

static void str_symbol(t_str *x, t_symbol *s)
{
//    post("str_symbol");
    x->x_buf_end = sprintf((char *)x->x_buf.s_data, "%s", s->s_name);
    x->x_string_in1_end = x->x_buf_end;
    str_buf_to_string(x, &x->x_string_in1);
    if (x->x_function == drip) x->x_nsplit = 0L;
    str_do_string(x);
}

static void str_float(t_str *x, t_float f)
{
    x->x_buf_end = 0L;

//    post("str_float");
    str_float_to_buf(x, f);
    x->x_string_in1_end = x->x_buf_end;
    str_buf_to_string(x, &x->x_string_in1);
    if (x->x_function == drip) x->x_nsplit = 0L;
    str_do_string(x);
}

static void str_bang(t_str *x)
{
//    post("str_bang");
    if((x->x_function == add) && (x->x_string_in2_end != 0))str_do_out3(x);
    else str_do_string(x);
}

/* Send string_in1 through outlet_1 */
static void str_do_out0(t_str *x)
{
/* The receiver needs to know the length of the actual data, not the size of our buffer */
/* so we temporarily replace s_length with string end. */
    size_t  true_length = x->x_string_in1.s_length;
    x->x_string_in1.s_length = x->x_string_in1_end;
//    post("str_do_out0: x->x_string_in1.s_data[0] = %d", x->x_string_in1.s_data[0]);
    outlet_blob(x->x_outlet_1, &x->x_string_in1);
    x->x_string_in1.s_length = true_length;
}

 /* send string_out_1 through outlet_1 */
 static void str_do_out1(t_str *x)
{
   size_t  true_length = x->x_string_out1.s_length;
    x->x_string_out1.s_length = x->x_string_out1_end;
//    post("str_do_out1: x->x_string_out1.s_data[0] = %d", x->x_string_out1.s_data[0]);
    outlet_blob(x->x_outlet_1, &x->x_string_out1);
    x->x_string_out1.s_length = true_length;
}

/* send string_out_2 through outlet_2 */
static void str_do_out2(t_str *x)
{
    size_t  true_length = x->x_string_out2.s_length;
    x->x_string_out2.s_length = x->x_string_out2_end;
//    post("str_do_out2: x->x_string_out2.s_data[0] = %d", x->x_string_out2.s_data[0]);
    outlet_blob(x->x_outlet_2, &x->x_string_out2);
    x->x_string_out2.s_length = true_length;
}

/* Send string_in2 through outlet_1 */
static void str_do_out3(t_str *x)
{
/* The receiver needs to know the length of the actual data, not the size of the buffer */
/* so we temporarily replace s_length with string end. */
    size_t  true_length = x->x_string_in2.s_length;
    x->x_string_in2.s_length = x->x_string_in2_end;
    outlet_blob(x->x_outlet_1, &x->x_string_in2);
    x->x_string_in2.s_length = true_length;
}

/* Perform the string function and emit the result */
static void str_do_string(t_str *x)
{
    size_t          i;
    float           f;
    unsigned char   c;

    switch (x->x_function)
    {
        case string:
            outlet_float(x->x_outlet_2, x->x_string_in1_end);
            if(x->x_string_in1_end != 0) str_do_out0(x);
            break;
        case join:
            str_join(x);
            outlet_float(x->x_outlet_2, x->x_string_out1_end);
            if(x->x_string_out1_end != 0) str_do_out1(x);
            break;
        case add:
            str_add(x);/* no output until banged */
            outlet_float(x->x_outlet_2, x->x_string_in2_end);
            break;
        case drip:
            if (x->x_string_in1_end != 0) str_drip(x);
            break;
        case nth:
            outlet_float(x->x_outlet_2, x->x_string_in1_end);
            i = x->x_nsplit; /* output the nth character as a float, or bang if none. */
            if ((x->x_string_in1_end != 0) && (i < x->x_string_in1_end))
                outlet_float(x->x_outlet_1, x->x_string_in1.s_data[i]);
            else
                outlet_bang(x->x_outlet_1);
            break;
        case compare:
            outlet_float(x->x_outlet_1, str_equal(x));
            break;
        case nsplit:
            str_nsplit(x);
            if(x->x_string_out2_end != 0) str_do_out2(x);
            if(x->x_string_out1_end != 0) str_do_out1(x);
            break;
        case csplit:
            str_csplit(x);
            if(x->x_string_out2_end != 0) str_do_out2(x);
            if(x->x_string_out1_end != 0) str_do_out1(x);
            break;
        case to_float:
            if (x->x_string_in1_end != 0)
                outlet_float(x->x_outlet_1, str_to_float(x));
            break;
        case to_symbol:
            if (x->x_string_in1_end != 0)
                outlet_symbol(x->x_outlet_1, str_to_symbol(x));
            break;
        case to_list:
            x->x_atom_list_end = (sizeof(t_atom))*(x->x_string_in1.s_length);
            if (x->x_atom_list_end > x->x_atom_list_length) x->x_atom_list_end = x->x_atom_list_length;
            for (i = 0; i < x->x_string_in1_end; ++i)
            {
                c = x->x_string_in1.s_data[i];
                f = (float)c;
                SETFLOAT(&x->x_atom_list[i], f);
            }
            if (x->x_string_in1_end != 0)
                outlet_list(x->x_outlet_1, &s_list, x->x_string_in1_end, x->x_atom_list);
            break;
    }
}

static void str_free(t_str *x)
{
//    post("str_free");
    freebytes(x->x_string_out1.s_data, x->x_string_out1.s_length);
    freebytes(x->x_string_out2.s_data, x->x_string_out2.s_length);
    freebytes(x->x_string_in1.s_data, x->x_string_in1.s_length);
    freebytes(x->x_string_in2.s_data, x->x_string_in2.s_length);
    freebytes(x->x_buf.s_data, x->x_buf.s_length);
    freebytes(x->x_atom_list, x->x_atom_list_length);
}

static void *str_new(t_symbol *s, int argc, t_atom *argv)
{
    t_str           *x;
    unsigned long   i, next;
    size_t          cLen = MAXPDSTRING;
    unsigned int    u;
    t_float         f;

    x = (t_str *)pd_new(str_class);
    if (x == NULL) return (x);
    x->x_outlet_1 = outlet_new((t_object *)x, &s_anything);
    /* This is the only place we allocate string storage */
    x->x_buf.s_data = getbytes(cLen);
    x->x_buf.s_length = cLen;
    x->x_buf_end = 0L;
    x->x_string_in1.s_data = getbytes(cLen);
    x->x_string_in1.s_length = cLen;
    x->x_string_in1_end = 0L;/* the current end of the string. Must be less than x_string_in1.s_length */
    x->x_string_in2.s_data = getbytes(cLen);
    x->x_string_in2.s_length = cLen;
    x->x_string_in2_end = 0L;
    x->x_string_out1.s_data = getbytes(cLen);
    x->x_string_out1.s_length = cLen;
    x->x_string_out1_end = 0L;
    x->x_string_out2.s_data = getbytes(cLen);
    x->x_string_out2.s_length = cLen;
    x->x_string_out2_end = 0L;
    x->x_atom_list = getbytes(cLen);
    x->x_atom_list_length = cLen;
    x->x_atom_list_end = 0L;
    x->x_function = 0; /* default = string */
    x->x_nsplit = 0L;
    next = 0; /* index of next argument */
    if (argv[0].a_type == A_SYMBOL)
    { /* the first argument may be a selector */
        atom_string(&argv[0], (char *)x->x_buf.s_data, MAXPDSTRING);
        for (i = 0; i < n_functions; ++i)
        {
            if (strcmp((char *)x->x_buf.s_data, str_function_names[i]) == 0)
            {
                x->x_function = i;
//                post("str_new: x_function is %s", str_function_names[x->x_function]);
                next = 1;
                if ((x->x_function == nsplit) || (x->x_function == csplit))
                    x->x_outlet_2 = outlet_new((t_object *)x, &s_anything);
                break;
            }
        }
    }
    if ((x->x_function == string)||(x->x_function == nth)||(x->x_function == add)||(x->x_function == join)
    || (x->x_function == drip))
       x->x_outlet_2 = outlet_new((t_object *)x, &s_anything); /* an outlet for the string length or bang */
    if ((x->x_function == nsplit)||(x->x_function == nth))
    { /* single argument must be a float, add a float inlet */
        x->x_nsplit = atom_getfloat(&argv[next]);
        x->x_inlet_2 = floatinlet_new((t_object *)x, &x->x_nsplit);
    }
    else if (x->x_function == csplit)
    { /* argument goes to string_in2 */
        str_list_to_buf(x, &argv[next], argc-next);
        x->x_string_in2_end = x->x_buf_end;
        str_buf_to_string(x, &x->x_string_in2);
    }
    else if ((x->x_function == join)||(x->x_function == compare)||(x->x_function == add))
    { /* argument goes to string_in2, add a string inlet */
        x->x_inlet_2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_blob, gensym("")); /* gensym("blob") */
        str_list_to_buf(x, &argv[next], argc-next);
        x->x_string_in2_end = x->x_buf_end;
        str_buf_to_string(x, &x->x_string_in2);
    }
    else
    { /* argument goes to string_in1 */
        str_list_to_buf(x, &argv[next], argc-next);
        x->x_string_in1_end = x->x_buf_end;
        str_buf_to_string(x, &x->x_string_in1);
    }
    return (x);
}

void str_setup(void)
{
    str_class = class_new(gensym("str"),
                    (t_newmethod)str_new,
                    (t_method)str_free,
                    sizeof(t_str), 0, A_GIMME, 0);
    class_addbang(str_class, str_bang);
    class_addfloat(str_class, str_float);
    class_addsymbol(str_class, str_symbol);
    class_addlist(str_class, str_list);
    class_addanything(str_class, str_anything);
    class_addblob(str_class, str_string);
    class_addmethod(str_class, (t_method)str_set, gensym("set"), A_GIMME, 0);
    class_addmethod(str_class, (t_method)str_fread, gensym("file_read"), A_GIMME, 0);
    class_addmethod(str_class, (t_method)str_fwrite, gensym("file_write"), A_GIMME, 0);
    class_addmethod(str_class, (t_method)str_set_second, gensym(""), A_BLOB, 0);
}
#endif // ifndef PD_BLOBS
/* end str.c */

