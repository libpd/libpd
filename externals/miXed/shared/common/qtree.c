/* Copyright (c) 2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "loud.h"
#include "qtree.h"

/* Since there is no sentinel node, the deletion routine has to have
   a few extra checks.  LATER rethink. */

/* LATER freelist */

typedef t_qnode *(*t_qtree_inserthook)(t_qnode *);

#ifdef QTREE_DEBUG
/* returns black-height or 0 if failed */
static int qnode_verify(t_qnode *np)
{
    if (np)
    {
	int bhl, bhr;
	if (((bhl = qnode_verify(np->n_left)) == 0) ||
	    ((bhr = qnode_verify(np->n_right)) == 0))
	    return (0);
	if (bhl != bhr)
	{
	    /* failure: two paths rooted in the same node
	       contain different number of black nodes */
	    loudbug_bug("qnode_verify: not balanced");
	    return (0);
	}
	if (np->n_black)
	    return (bhl + 1);
	else
	{
	    if ((np->n_left && !np->n_left->n_black) ||
		(np->n_right && !np->n_right->n_black))
	    {
		loudbug_bug("qnode_verify: adjacent red nodes");
		return (0);
	    }
	    return (bhl);
	}
    }
    else return (1);
}

/* returns black-height or 0 if failed */
static int qtree_verify(t_qtree *tree)
{
    return (qnode_verify(tree->t_root));
}

static int qnode_checkmulti(t_qnode *np1, t_qnode *np2)
{
    if (np1 && np2 && np1->n_key == np2->n_key)
    {
	if (np1 == np2)
	    loudbug_bug("qnode_checkmulti");
	else
	    return (1);
    }
    return (0);
}

static void qnode_post(t_qtree *tree, t_qnode *np,
		       t_qnode_vshowhook hook, char *message)
{
    loudbug_startpost("%g ", np->n_key);
    if (tree->t_valuetype == QTREETYPE_FLOAT)
	loudbug_startpost("%g ", QNODE_GETFLOAT(np));
    else if (tree->t_valuetype == QTREETYPE_SYMBOL)
	loudbug_startpost("%s ", QNODE_GETSYMBOL(np)->s_name);
    else if (tree->t_valuetype == QTREETYPE_ATOM)
    {
	t_atom *ap = QNODE_GETATOMPTR(np);
	if (ap->a_type == A_FLOAT)
	    loudbug_startpost("%g ", ap->a_w.w_float);
	else if (ap->a_type == A_SYMBOL)
	    loudbug_startpost("%s ", ap->a_w.w_symbol->s_name);
    }
    else if (hook)
    {
	char buf[MAXPDSTRING];
	(*hook)(np, buf, MAXPDSTRING);
	loudbug_startpost("%s ", buf);
    }
    else loudbug_startpost("0x%08x ", (int)QNODE_GETSYMBOL(np));
    loudbug_startpost("%s ", (np->n_black ? "black" : "red"));

    if (qnode_checkmulti(np, np->n_parent) ||
	qnode_checkmulti(np, np->n_left) ||
	qnode_checkmulti(np, np->n_right) ||
	qnode_checkmulti(np->n_parent, np->n_left) ||
	qnode_checkmulti(np->n_parent, np->n_right) ||
	qnode_checkmulti(np->n_left, np->n_right))
	loudbug_startpost("multi ");

    if (np->n_parent)
	loudbug_startpost("(%g -> ", np->n_parent->n_key);
    else
	loudbug_startpost("(nul -> ");
    if (np->n_left)
	loudbug_startpost("%g, ", np->n_left->n_key);
    else
	loudbug_startpost("nul, ");
    if (np->n_right)
	loudbug_startpost("%g)", np->n_right->n_key);
    else
	loudbug_startpost("nul)");
    if (message)
	loudbug_post(": %s", message);
    else
	loudbug_endpost();
}

/* Assert a standard stackless traversal producing the same sequence,
   as the auxiliary list. */
static int qtree_checktraversal(t_qtree *tree)
{
    t_qnode *treewalk = tree->t_root;
    t_qnode *listwalk = tree->t_first;
    int count = 0;
    while (treewalk)
    {
	t_qnode *prev = treewalk->n_left;
	if (prev)
	{
	    while (prev->n_right && prev->n_right != treewalk)
		prev = prev->n_right;
	    if (prev->n_right)
	    {
		prev->n_right = 0;
		count++;
		if (treewalk == listwalk)
		    listwalk = listwalk->n_next;
		else
		{
		    loudbug_bug("qtree_checktraversal 1");
		    qnode_post(tree, treewalk, 0, "treewalk");
		    if (listwalk)
			qnode_post(tree, listwalk, 0, "listwalk");
		    else
			loudbug_post("empty listwalk pointer");
		    listwalk = treewalk;
		}
		treewalk = treewalk->n_right;
	    }
	    else
	    {
		prev->n_right = treewalk;
		treewalk = treewalk->n_left;
	    }
	}
	else
	{
	    count++;
	    if (treewalk == listwalk)
		listwalk = listwalk->n_next;
	    else
	    {
		loudbug_bug("qtree_checktraversal 2");
		qnode_post(tree, treewalk, 0, "treewalk");
		if (listwalk)
		    qnode_post(tree, listwalk, 0, "listwalk");
		else
		    loudbug_post("empty listwalk pointer");
		listwalk = treewalk;
	    }
	    treewalk = treewalk->n_right;
	}
    }
    return (count);
}

static int qnode_height(t_qnode *np)
{
    if (np)
    {
	int lh = qnode_height(np->n_left);
	int rh = qnode_height(np->n_right);
	return (lh > rh ? lh + 1 : rh + 1);
    }
    else return (0);
}

void qtree_debug(t_qtree *tree, int level, t_qnode_vshowhook hook)
{
    t_qnode *np;
    int count;
    loudbug_post("------------------------");
    count = qtree_checktraversal(tree);
    if (level)
    {
	for (np = tree->t_first; np; np = np->n_next)
	    qnode_post(tree, np, hook, 0);
	if (level > 1)
	{
	    loudbug_post("************");
	    for (np = tree->t_last; np; np = np->n_prev)
		loudbug_startpost("%g ", np->n_key);
	    loudbug_endpost();
	}
    }
    if (tree->t_root)
    {
	t_qnode *first = tree->t_root, *last = tree->t_root;
	while (first->n_left && first->n_left != tree->t_root)
	    first = first->n_left;
	while (last->n_right && last->n_right != tree->t_root)
	    last = last->n_right;
	loudbug_post("count %d, height %d, root %g",
		     count, qnode_height(tree->t_root), tree->t_root->n_key);
	loudbug_post("first %g, root->left* %g, last %g, root->right* %g",
		     (tree->t_first ? tree->t_first->n_key : 0), first->n_key,
		     (tree->t_last ? tree->t_last->n_key : 0), last->n_key);
    }
    else loudbug_post("empty");
    loudbug_post("...verified (black-height is %d)", qtree_verify(tree));
    loudbug_post("------------------------");
}
#endif

/* assuming that target node (np->n_right) exists */
static void qtree_lrotate(t_qtree *tree, t_qnode *np)
{
    t_qnode *target = np->n_right;
    if (np->n_right = target->n_left)
	np->n_right->n_parent = np;
    if (!(target->n_parent = np->n_parent))
	tree->t_root = target;
    else if (np == np->n_parent->n_left)
	np->n_parent->n_left = target;
    else
	np->n_parent->n_right = target;
    target->n_left = np;
    np->n_parent = target;
}

/* assuming that target node (np->n_left) exists */
static void qtree_rrotate(t_qtree *tree, t_qnode *np)
{
    t_qnode *target = np->n_left;
    if (np->n_left = target->n_right)
	np->n_left->n_parent = np;
    if (!(target->n_parent = np->n_parent))
	tree->t_root = target;
    else if (np == np->n_parent->n_left)
	np->n_parent->n_left = target;
    else
	np->n_parent->n_right = target;
    target->n_right = np;
    np->n_parent = target;
}

static t_qnode *qtree_preinserthook(t_qnode *np)
{
    while (np->n_prev && np->n_prev->n_key == np->n_key)
	np = np->n_prev;
    if (np->n_left)
    {
	np = np->n_prev;
	if (np->n_right)
	{
	    /* LATER revisit */
	    loudbug_bug("qtree_preinserthook");
	    return (0);  /* do nothing */
	}
    }
    return (np);
}

static t_qnode *qtree_postinserthook(t_qnode *np)
{
    while (np->n_next && np->n_next->n_key == np->n_key)
	np = np->n_next;
    if (np->n_right)
    {
	np = np->n_next;
	if (np->n_left)
	{
	    /* LATER revisit */
	    loudbug_bug("qtree_postinserthook");
	    return (0);  /* do nothing */
	}
    }
    return (np);
}

/* Returns a newly inserted or already existing node (or 0 if allocation
   failed).  A caller is responsible for assigning a value.  If hook is
   supplied, it is called iff key is found.  In case of key being found
   (which means foundp returns 1), a new node is inserted, unless hook is
   either empty, or returns null.  Hook's nonempty return is the parent
   for the new node.  It is expected to have no more than one child. */
static t_qnode *qtree_doinsert(t_qtree *tree, double key, t_qnode *preexisting,
			       t_qtree_inserthook hook, int *foundp)
{
    t_qnode *np, *parent, *result;
    int leftchild;
    *foundp = 0;
    if (!(np = tree->t_root))
    {
	if (!(np = (tree->t_nodesize > 0 ?
		    getbytes(tree->t_nodesize) : preexisting)))
	{
	    if (tree->t_nodesize == 0)
		loudbug_bug("qtree_insert, node not supplied");
	    return (0);
	}
	np->n_key = key;
	np->n_black = 1;
	np->n_left = np->n_right = np->n_parent = 0;
	tree->t_root = tree->t_first = tree->t_last = np;
	np->n_prev = np->n_next = 0;
	return (np);
    }

    do
    {
	if (np->n_key == key)
	{
	    *foundp = 1;
	    if (hook && (parent = (*hook)(np)))
	    {
		if (parent->n_left && parent->n_right)
		{
		    loudbug_bug("qtree_insert, callback return 1");
		    parent = parent->n_next;
		}
		if (leftchild = (key < parent->n_key))
		{
		    if (parent->n_left)
		    {
			loudbug_bug("qtree_insert, callback return 2");
			leftchild = 0;
		    }
		}
		else if (parent->n_right)
		    leftchild = 1;
		goto addit;
	    }
	    else return (np);  /* a caller may then keep or replace the value */
	}
	else parent = np;
    }
    while (np = (key < np->n_key ? np->n_left : np->n_right));
    leftchild = (key < parent->n_key);
addit:
    /* parent has no more than one child, new node becomes
       parent's immediate successor or predecessor */
    if (!(np = (tree->t_nodesize > 0 ?
		getbytes(tree->t_nodesize) : preexisting)))
    {
	if (tree->t_nodesize == 0)
	    loudbug_bug("qtree_insert, node not supplied");
	return (0);
    }
    np->n_key = key;
    np->n_parent = parent;
    if (leftchild)
    {
	parent->n_left = np;
	/* update the auxiliary linked list structure */
	np->n_next = parent;
	if (np->n_prev = parent->n_prev)
	    np->n_prev->n_next = np;
	else
	    tree->t_first = np;
	parent->n_prev = np;
    }
    else
    {
	parent->n_right = np;
	/* update the auxiliary linked list structure */
	np->n_prev = parent;
	if (np->n_next = parent->n_next)
	    np->n_next->n_prev = np;
	else
	    tree->t_last = np;
	parent->n_next = np;
    }
    result = np;

    /* balance the tree -- LATER clean this if possible... */
    np->n_black = 0;
    while (np != tree->t_root && !np->n_parent->n_black)
    {
	t_qnode *uncle;
	/* np->n_parent->n_parent exists (we always paint root node in black) */
	if (np->n_parent == np->n_parent->n_parent->n_left)
	{
	    uncle = np->n_parent->n_parent->n_right;
	    if (!uncle  /* (sentinel not used) */
		|| uncle->n_black)
	    {
		if (np == np->n_parent->n_right)
		{
		    np = np->n_parent;
		    qtree_lrotate(tree, np);
		}
		np->n_parent->n_black = 1;
		np->n_parent->n_parent->n_black = 0;
		qtree_rrotate(tree, np->n_parent->n_parent);
	    }
	    else
	    {
		np->n_parent->n_black = 1;
		uncle->n_black = 1;
		np = np->n_parent->n_parent;
		np->n_black = 0;
	    }
	}
	else
	{
	    uncle = np->n_parent->n_parent->n_left;
	    if (!uncle  /* (sentinel not used) */
		|| uncle->n_black)
	    {
		if (np == np->n_parent->n_left)
		{
		    np = np->n_parent;
		    qtree_rrotate(tree, np);
		}
		np->n_parent->n_black = 1;
		np->n_parent->n_parent->n_black = 0;
		qtree_lrotate(tree, np->n_parent->n_parent);
	    }
	    else
	    {
		np->n_parent->n_black = 1;
		uncle->n_black = 1;
		np = np->n_parent->n_parent;
		np->n_black = 0;
	    }
	}
    }
    tree->t_root->n_black = 1;
    return (result);
}

/* assuming that requested node exists */
void qtree_delete(t_qtree *tree, t_qnode *gone)
{
    t_qnode *parent;  /* parent of gone, after relinking */
    t_qnode *child;   /* gone's only child (or null), after relinking */
    /* gone has to be the parent of no more than one child */
    if (gone->n_left && gone->n_right)
    {
	/* Successor is the new parent of gone's children, and a new child
	   of gone's parent (if any).  Successor always exists in this context,
	   and it has no left child.  The simplistic scheme is to replace
	   gone's fields with successor's fields, and delete the successor.
	   We cannot do so, however, because 1. nodes may be caller-owned
	   (nodesize == 0), 2. successor may be pointed at... */
	t_qnode *successor = gone->n_next;
	child = successor->n_right;
	successor->n_left = gone->n_left;
	successor->n_left->n_parent = successor;
	if (successor == gone->n_right)
	    parent = successor;
	else
	{
	    /* successor's parent always exists in this context,
	       successor is the left child of its parent */
	    parent = successor->n_parent;
	    parent->n_left = child;
	    if (child)  /* (sentinel not used) */
		child->n_parent = parent;
	    successor->n_right = gone->n_right;
	    successor->n_right->n_parent = successor;
	}
	if (gone->n_parent)
	{
	    int swp;
	    if (gone == gone->n_parent->n_left)
		gone->n_parent->n_left = successor;
	    else
		gone->n_parent->n_right = successor;
	    successor->n_parent = gone->n_parent;
	    swp = gone->n_black;
	    gone->n_black = successor->n_black;
	    successor->n_black = swp;
	}
	else
	{
	    tree->t_root = successor;
	    successor->n_parent = 0;
	    gone->n_black = successor->n_black;
	    successor->n_black = 1;  /* LATER rethink */
	}

	/* update the auxiliary linked list structure */
	if (successor->n_prev = gone->n_prev)
	    gone->n_prev->n_next = successor;
	else
	    tree->t_first = successor;
    }
    else
    {
	/* update the auxiliary linked list structure */
	if (gone->n_prev)
	    gone->n_prev->n_next = gone->n_next;
	else
	    tree->t_first = gone->n_next;
	if (gone->n_next)
	    gone->n_next->n_prev = gone->n_prev;
	else
	    tree->t_last = gone->n_prev;

	/* connect gone's child with gone's parent */
	if (gone->n_left)
	    child = gone->n_left;
	else
	    child = gone->n_right;
	if (parent = gone->n_parent)
	{
	    if (child)  /* (sentinel not used) */
		child->n_parent = parent;
	    if (gone == parent->n_left)
		parent->n_left = child;
	    else
		parent->n_right = child;
	}
	else
	{
	    if (tree->t_root = child)
	    {
		child->n_parent = 0;
		child->n_black = 1;  /* LATER rethink */
	    }
	    goto done;
	}
    }

    if (gone->n_black)
    {
	/* balance the tree -- LATER clean this if possible... */
	/* on entry:  tree is not empty, parent always exists, child
	   not necessarily... */
	while (child != tree->t_root &&
	       (!child ||  /* (sentinel not used) */
		child->n_black))
	{
	    t_qnode *other;  /* another child of the same parent */
	    if (child == parent->n_left)
	    {
		other = parent->n_right;
		if (other &&  /* (sentinel not used) */
		    !other->n_black)
		{
		    other->n_black = 1;
		    parent->n_black = 0;
		    qtree_lrotate(tree, parent);
		    other = parent->n_right;
		}
		if (!other ||  /* (sentinel not used) */
		    (!other->n_left || other->n_left->n_black) &&
		    (!other->n_right || other->n_right->n_black))
		{
		    if (other)  /* (sentinel not used) */
			other->n_black = 0;
		    child = parent;
		    parent = parent->n_parent;
		}
		else
		{
		    if (!other ||  /* (sentinel not used) */
			!other->n_right || other->n_right->n_black)
		    {
			if (other)  /* (sentinel not used) */
			{
			    if (other->n_left) other->n_left->n_black = 1;
			    other->n_black = 0;
			    qtree_rrotate(tree, other);
			    other = parent->n_right;
			}
		    }
		    if (other)  /* (sentinel not used) */
		    {
			if (other->n_right) other->n_right->n_black = 1;
			other->n_black = parent->n_black;
		    }
		    parent->n_black = 1;
		    qtree_lrotate(tree, parent);
		    tree->t_root->n_black = 1;  /* LATER rethink */
		    goto done;
		}
	    }
	    else  /* right child */
	    {
		other = parent->n_left;
		if (other &&  /* (sentinel not used) */
		    !other->n_black)
		{
		    other->n_black = 1;
		    parent->n_black = 0;
		    qtree_rrotate(tree, parent);
		    other = parent->n_left;
		}
		if (!other ||  /* (sentinel not used) */
		    (!other->n_left || other->n_left->n_black) &&
		    (!other->n_right || other->n_right->n_black))
		{
		    if (other)  /* (sentinel not used) */
			other->n_black = 0;
		    child = parent;
		    parent = parent->n_parent;
		}
		else
		{
		    if (!other ||  /* (sentinel not used) */
			!other->n_left || other->n_left->n_black)
		    {
			if (other)  /* (sentinel not used) */
			{
			    if (other->n_right) other->n_right->n_black = 1;
			    other->n_black = 0;
			    qtree_lrotate(tree, other);
			    other = parent->n_left;
			}
		    }
		    if (other)  /* (sentinel not used) */
		    {
			if (other->n_left) other->n_left->n_black = 1;
			other->n_black = parent->n_black;
		    }
		    parent->n_black = 1;
		    qtree_rrotate(tree, parent);
		    tree->t_root->n_black = 1;  /* LATER rethink */
		    goto done;
		}
	    }
	}
	if (child)  /* (sentinel not used) */
	    child->n_black = 1;
    }
done:
    if (tree->t_nodesize)
	freebytes(gone, tree->t_nodesize);
#ifdef QTREE_DEBUG
    qtree_verify(tree);
#endif
}

t_qnode *qtree_search(t_qtree *tree, double key)
{
    t_qnode *np = tree->t_root;
    while (np && np->n_key != key)
	np = (key < np->n_key ? np->n_left : np->n_right);
    return (np);
}

/* Returns the greatest node <= key, if any (may return null).
   If deltap is not null, it will hold the abs diff (key - node.n_key). */
t_qnode *qtree_closestunder(t_qtree *tree, double key, double *deltap)
{
    t_qnode *np, *parent;
    if (!(np = tree->t_root))
	return (0);
    do
	if (np->n_key == key)
	{
	    if (deltap)
		*deltap = 0.;
	    return (np);
	}
	else parent = np;
    while (np = (key < np->n_key ? np->n_left : np->n_right));
    if (np = (key < parent->n_key ? parent->n_prev : parent))
    {
	if (deltap)
	    *deltap = key - np->n_key;
	return (np);
    }
    else return (0);
}

/* Returns the smallest node >= key, if any (may return null).
   If deltap is not null, it will hold the abs diff (node.n_key - key). */
t_qnode *qtree_closestover(t_qtree *tree, double key, double *deltap)
{
    t_qnode *np, *parent;
    if (!(np = tree->t_root))
	return (0);
    do
	if (np->n_key == key)
	{
	    if (deltap)
		*deltap = 0.;
	    return (np);
	}
	else parent = np;
    while (np = (key < np->n_key ? np->n_left : np->n_right));
    if (np = (key > parent->n_key ? parent->n_next : parent))
    {
	if (deltap)
	    *deltap = np->n_key - key;
	return (np);
    }
    else return (0);
}

/* Returns the smallest node >= key or the greatest node <= key, whichever
   makes the smallest abs diff, |key - node.n_key|.  Returns null only for
   an empty tree.  If deltap is not null, it will hold the signed diff
   (negative for an undernode, i.e. when node < key). */
t_qnode *qtree_closest(t_qtree *tree, double key, double *deltap)
{
    t_qnode *np, *parent;
    if (!(np = tree->t_root))
	return (0);
    do
	if (np->n_key == key)
	{
	    if (deltap)
		*deltap = 0.;
	    return (np);
	}
	else parent = np;
    while (np = (key < np->n_key ? np->n_left : np->n_right));
    if (key > parent->n_key)
    {
	if (np = parent->n_next)
	{
	    double delta1 = key - parent->n_key;
	    double delta2 = np->n_key - key;
	    if (delta1 < delta2)
	    {
		if (deltap)
		    *deltap = -delta1;
		return (parent);
	    }
	    else
	    {
		if (deltap)
		    *deltap = delta2;
		return (np);
	    }
	}
	else
	{
	    if (deltap)
		*deltap = parent->n_key - key;
	    return (parent);
	}
    }
    else
    {
	if (np = parent->n_prev)
	{
	    double delta1 = key - np->n_key;
	    double delta2 = parent->n_key - key;
	    if (delta1 < delta2)
	    {
		if (deltap)
		    *deltap = -delta1;
		return (np);
	    }
	    else
	    {
		if (deltap)
		    *deltap = delta2;
		return (parent);
	    }
	}
	else
	{
	    if (deltap)
		*deltap = parent->n_key - key;
	    return (parent);
	}
    }
}

t_qnode *qtree_insert(t_qtree *tree, double key,
		      t_qnode *preexisting, int *foundp)
{
    int found;
    return (qtree_doinsert(tree, key, preexisting, 0,
			   (foundp ? foundp : &found)));
}

t_qnode *qtree_multiinsert(t_qtree *tree, double key,
			   t_qnode *preexisting, int fifoflag)
{
    int found;
    return (qtree_doinsert(tree, key, preexisting,
			   (fifoflag ?
			    qtree_postinserthook :
			    qtree_preinserthook), &found));
}

t_qnode *qtree_override(t_qtree *tree, t_qnode *oldnode, t_qnode *newnode)
{
    if (tree->t_nodesize)
    {
	loudbug_bug("qtree_override 1");
	return (0);
    }
    else
    {
	newnode->n_key = oldnode->n_key;
	newnode->n_black = oldnode->n_black;
	if (newnode->n_left = oldnode->n_left)
	    newnode->n_left->n_parent = newnode;
	if (newnode->n_right = oldnode->n_right)
	    newnode->n_right->n_parent = newnode;
	if (newnode->n_parent = oldnode->n_parent)
	{
	    if (oldnode == newnode->n_parent->n_left)
		newnode->n_parent->n_left = newnode;
	    else if (oldnode == newnode->n_parent->n_right)
		newnode->n_parent->n_right = newnode;
	    else
		loudbug_bug("qtree_override 2");
	}
	if (newnode->n_prev = oldnode->n_prev)
	    newnode->n_prev->n_next = newnode;
	if (newnode->n_next = oldnode->n_next)
	    newnode->n_next->n_prev = newnode;
	if (tree->t_root == oldnode)
	    tree->t_root = newnode;
	if (tree->t_first == oldnode)
	    tree->t_first = newnode;
	if (tree->t_last == oldnode)
	    tree->t_last = newnode;
	return (newnode);
    }
}

t_qnode *qtree_insertfloat(t_qtree *tree, double key, t_float f,
			   int replaceflag)
{
    int found;
    t_qnode *np = qtree_doinsert(tree, key, 0, 0, &found);
    if (np && (!found || replaceflag))
    {
	if (tree->t_valuetype == QTREETYPE_FLOAT)
	{
	    t_qnode_float *npf = (t_qnode_float *)np;
	    npf->nf_value = f;
	}
	else if (tree->t_valuetype == QTREETYPE_ATOM)
	{
	    t_qnode_atom *npa = (t_qnode_atom *)np;
	    t_atom *ap = &npa->na_value;
	    SETFLOAT(ap, f);
	}
	else loudbug_bug("qtree_insertfloat");
    }
    return (np);
}

t_qnode *qtree_insertsymbol(t_qtree *tree, double key, t_symbol *s,
			    int replaceflag)
{
    int found;
    t_qnode *np = qtree_doinsert(tree, key, 0, 0, &found);
    if (np && (!found || replaceflag))
    {
	if (tree->t_valuetype == QTREETYPE_SYMBOL)
	{
	    t_qnode_symbol *nps = (t_qnode_symbol *)np;
	    nps->ns_value = s;
	}
	else if (tree->t_valuetype == QTREETYPE_ATOM)
	{
	    t_qnode_atom *npa = (t_qnode_atom *)np;
	    t_atom *ap = &npa->na_value;
	    SETSYMBOL(ap, s);
	}
	else loudbug_bug("qtree_insertsymbol");
    }
    return (np);
}

t_qnode *qtree_insertatom(t_qtree *tree, double key, t_atom *ap,
			  int replaceflag)
{
    int found;
    t_qnode *np = qtree_doinsert(tree, key, 0, 0, &found);
    if (np && (!found || replaceflag))
    {
	if (tree->t_valuetype == QTREETYPE_ATOM)
	{
	    t_qnode_atom *npa = (t_qnode_atom *)np;
	    npa->na_value = *ap;
	}
	else loudbug_bug("qtree_insertatom");
    }
    return (np);
}

/* LATER preallocate 'freecount' nodes */
static void qtree_doinit(t_qtree *tree, t_qtreetype vtype,
			 size_t nodesize, int freecount)
{
    tree->t_root = tree->t_first = tree->t_last = 0;
    tree->t_valuetype = vtype;
    tree->t_nodesize = nodesize;
}

void qtree_inittyped(t_qtree *tree, t_qtreetype vtype, int freecount)
{
    size_t nsize;
    switch (vtype)
    {
    case QTREETYPE_FLOAT:
	nsize = sizeof(t_qnode_float);
	break;
    case QTREETYPE_SYMBOL:
	nsize = sizeof(t_qnode_symbol);
	break;
    case QTREETYPE_ATOM:
	nsize = sizeof(t_qnode_atom);
	break;
    default:
	loudbug_bug("qtree_inittyped");
	vtype = QTREETYPE_ILLEGAL;
	nsize = sizeof(t_qnode);
    }
    qtree_doinit(tree, vtype, nsize, freecount);
}

void qtree_initcustom(t_qtree *tree, size_t nodesize, int freecount)
{
    qtree_doinit(tree, QTREETYPE_CUSTOM, nodesize, freecount);
}

/* LATER keep and/or preallocate 'freecount' nodes (if negative, keep all) */
void qtree_clear(t_qtree *tree, int freecount)
{
    t_qnode *np, *next = tree->t_first;
    while (np = next)
    {
	next = next->n_next;
	if (tree->t_nodesize)
	    freebytes(np, tree->t_nodesize);
    }
    qtree_doinit(tree, tree->t_valuetype, tree->t_nodesize, 0);
}
