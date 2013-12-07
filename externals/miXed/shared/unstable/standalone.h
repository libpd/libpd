/* Copyright (c) 2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef MIXED_STANDALONE
#error MIXED_STANDALONE not defined
#else
#ifndef __STANDALONE_H__
#define __STANDALONE_H__

typedef int t_int;
typedef float t_float;

typedef struct _symbol
{
    char *s_name;
    void *s_thing;
    struct _symbol *s_next;
} t_symbol;

typedef union word
{
    t_float w_float;
    t_symbol *w_symbol;
    int w_index;
} t_word;

typedef enum
{
    A_NULL,
    A_FLOAT,
    A_SYMBOL,
    A_POINTER,
    A_SEMI,
    A_COMMA,
    A_DEFFLOAT,
    A_DEFSYM,
    A_DOLLAR, 
    A_DOLLSYM,
    A_GIMME,
    A_CANT
}  t_atomtype;

typedef struct _atom
{
    t_atomtype a_type;
    union word a_w;
} t_atom;


void *getbytes(size_t nbytes);
void *resizebytes(void *old, size_t oldsize, size_t newsize);
void freebytes(void *fatso, size_t nbytes);
t_symbol *gensym(char *s);

#endif
#endif
