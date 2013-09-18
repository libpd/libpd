/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __MESSTREE_H__
#define __MESSTREE_H__

typedef int (*t_messcall)(t_pd *, t_symbol *, int, t_atom *);
typedef char *t_messarg;

typedef struct _messslot
{
    char        *ms_name;
    t_messcall   ms_call;
    char        *ms_argument;
    int          ms_flags;
    struct _messnode  *ms_subnode;
} t_messslot;

typedef struct _messnode  /* a parser's symbol definition, sort of... */
{
    t_messslot  *mn_table;
    int          mn_nslots;
    int          mn_index;
} t_messnode;

EXTERN_STRUCT _messtree;
#define t_messtree  struct _messtree

#define MESSTREE_NSLOTS(slots)  (sizeof(slots)/sizeof(*(slots)))

enum { MESSTREE_OK,        /* done current message parsing, parse next */
       MESSTREE_CONTINUE,  /* continue current message parsing */
       MESSTREE_UNKNOWN,   /* current message unknown, parse next */
       MESSTREE_CORRUPT,   /* current message corrupt, parse next */
       MESSTREE_FATAL      /* exit parsing */
};

#define MESSTREE_NONEXCLUSIVE  1

t_messtree *messtree_new(t_symbol *selector);
void messtree_add(t_messtree *mt, t_messnode *rootnode);
t_messtree *messtree_build(t_messslot *rootslot);
int messtree_doit(t_messtree *mt, t_messslot **msp, int *nargp,
		  t_pd *target, t_symbol *s, int ac, t_atom *av);

#endif
