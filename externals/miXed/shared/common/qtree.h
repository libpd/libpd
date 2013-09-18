/* Copyright (c) 2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __QTREE_H__
#define __QTREE_H__

#ifdef KRZYSZCZ
#define QTREE_DEBUG
#endif

typedef enum
{
    QTREETYPE_FLOAT, QTREETYPE_SYMBOL, QTREETYPE_ATOM,
    QTREETYPE_CUSTOM, QTREETYPE_ILLEGAL
} t_qtreetype;

typedef struct _qnode
{
    double          n_key;
    int             n_black;
    struct _qnode  *n_left;
    struct _qnode  *n_right;
    struct _qnode  *n_parent;
    struct _qnode  *n_prev;
    struct _qnode  *n_next;
} t_qnode;

typedef struct _qnode_float
{
    t_qnode  nf_node;
    t_float  nf_value;
} t_qnode_float;

typedef struct _qnode_symbol
{
    t_qnode    ns_node;
    t_symbol  *ns_value;
} t_qnode_symbol;

typedef struct _qnode_atom
{
    t_qnode  na_node;
    t_atom   na_value;
} t_qnode_atom;

typedef struct _qtree
{
    t_qnode     *t_root;
    t_qnode     *t_first;
    t_qnode     *t_last;
    t_qtreetype  t_valuetype;
    size_t       t_nodesize;
} t_qtree;

#define QNODE_GETFLOAT(np)    (((t_qnode_float *)(np))->nf_value)
#define QNODE_GETSYMBOL(np)   (((t_qnode_symbol *)(np))->ns_value)
#define QNODE_GETATOMPTR(np)  (&((t_qnode_atom *)(np))->na_value)

typedef void (*t_qnode_vshowhook)(t_qnode *, char *, unsigned);

t_qnode *qtree_search(t_qtree *tree, double key);
t_qnode *qtree_closestunder(t_qtree *tree, double key, double *deltap);
t_qnode *qtree_closestover(t_qtree *tree, double key, double *deltap);
t_qnode *qtree_closest(t_qtree *tree, double key, double *deltap);

t_qnode *qtree_insert(t_qtree *tree, double key,
		      t_qnode *preexisting, int *foundp);
t_qnode *qtree_multiinsert(t_qtree *tree, double key,
			   t_qnode *preexisting, int fifoflag);
t_qnode *qtree_override(t_qtree *tree, t_qnode *oldnode, t_qnode *newnode);
t_qnode *qtree_insertfloat(t_qtree *tree, double key, t_float f,
			   int replaceflag);
t_qnode *qtree_insertsymbol(t_qtree *tree, double key, t_symbol *s,
			    int replaceflag);
t_qnode *qtree_insertatom(t_qtree *tree, double key, t_atom *ap,
			  int replaceflag);
void qtree_delete(t_qtree *tree, t_qnode *np);

void qtree_inittyped(t_qtree *tree, t_qtreetype vtype, int freecount);
void qtree_initcustom(t_qtree *tree, size_t nodesize, int freecount);
void qtree_clear(t_qtree *tree, int freecount);

#ifdef QTREE_DEBUG
void qtree_debug(t_qtree *tree, int level, t_qnode_vshowhook hook);
#endif

#endif
