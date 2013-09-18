/*
 * this header aims to make it easy to port Max objects to Pd
 */

/* name changes */
#define SETSYM SETSYMBOL

/* Pd doesn't have longs */
#define SETLONG SETFLOAT

/* different names for the same thing */
#define Atom t_atom
#define Symbol t_symbol

/* allocate memory */
#define sysmem_newptr(size) getbytes(128)
#define sysmem_freeptr(ptr) freebytes(ptr, 128)

/* standard object API functions */
#define atom_getlong(atom)          atom_getfloatarg(0, 1, atom)
#define atom_getsym(atom)           atom_getsymbolarg(0, 1, atom)
#define object_alloc(obj_class)     pd_new(obj_class)
#define object_free(obj)            pd_free((t_pd*)obj)
#define newobject(class)            pd_new(class)
#define outlet_int(outlet, number)  outlet_float(outlet, number)

/* debug things */
#define _enable_trace sys_verbose

/* these are NOT included here because they would cause more problems than
 * they would solve.  Usually, they are used in the setup() and new()
 * functions, where most of the differences are between the Max and PD APIs */

/* macros */
// A_DEFLONG

/* types */
// method
// Object

/* functions */
// addint()
// addmess()
// newobject()
// outlet_new()
// setup()
