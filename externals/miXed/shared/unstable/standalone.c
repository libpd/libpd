/* Copyright (c) 1997-2004 Miller Puckette, krzYszcz, and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Parts of Pd API are duplicated here, as needed by standalone versions of
   Pd modules.  LATER standalones should be linked to the Pd API library. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "standalone.h"

void *getbytes(size_t nbytes)
{
    void *ret;
    if (nbytes < 1) nbytes = 1;
    ret = (void *)calloc(nbytes, 1);
    if (!ret)
	fprintf(stderr, "ERROR: getbytes() failed -- out of memory");
    return (ret);
}

void *resizebytes(void *old, size_t oldsize, size_t newsize)
{
    void *ret;
    if (newsize < 1) newsize = 1;
    if (oldsize < 1) oldsize = 1;
    ret = (void *)realloc((char *)old, newsize);
    if (newsize > oldsize && ret)
    	memset(((char *)ret) + oldsize, 0, newsize - oldsize);
    if (!ret)
	fprintf(stderr, "ERROR: resizebytes() failed -- out of memory");
    return (ret);
}

void freebytes(void *fatso, size_t nbytes)
{
    free(fatso);
}

#define HASHSIZE 1024

static t_symbol *symhash[HASHSIZE];

static t_symbol *dogensym(char *s, t_symbol *oldsym)
{
    t_symbol **sym1, *sym2;
    unsigned int hash1 = 0,  hash2 = 0;
    int length = 0;
    char *s2 = s;
    while (*s2)
    {
	hash1 += *s2;
	hash2 += hash1;
	length++;
	s2++;
    }
    sym1 = symhash + (hash2 & (HASHSIZE-1));
    while (sym2 = *sym1)
    {
	if (!strcmp(sym2->s_name, s)) return(sym2);
	sym1 = &sym2->s_next;
    }
    if (oldsym) sym2 = oldsym;
    else
    {
    	sym2 = (t_symbol *)getbytes(sizeof(*sym2));
    	sym2->s_name = getbytes(length+1);
    	sym2->s_next = 0;
    	sym2->s_thing = 0;
    	strcpy(sym2->s_name, s);
    }
    *sym1 = sym2;
    return (sym2);
}

t_symbol *gensym(char *s)
{
    return(dogensym(s, 0));
}
