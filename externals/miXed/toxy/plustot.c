/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "common/grow.h"
#include "unstable/forky.h"
#include "hammer/file.h"
#include "common/props.h"
#include "toxy/scriptlet.h"
#include "toxy/plusbob.h"
#include "plustot.h"
#include "build_counter"

#define PLUSTOT_VERBOSE

#ifdef PLUSTOT_DEBUG
//#   define PLUSTOT_DEBUGREFCOUNTS
#   define PLUSDEBUG_ENDPOST(fn)  fputc('\n', stderr)
#else
#   define PLUSDEBUG_ENDPOST(fn)
#endif

#ifdef PLUSTOT_DEBUGREFCOUNTS
#   define PLUSDEBUG_INCRREFCOUNT(ob, fn) \
	{fprintf(stderr, "++ %x "fn"\n", (int)(ob)); Tcl_IncrRefCount(ob);}
#   define PLUSDEBUG_DECRREFCOUNT(ob, fn) \
	{fprintf(stderr, "-- %x "fn"\n", (int)(ob)); Tcl_DecrRefCount(ob);}
#else
#   define PLUSDEBUG_INCRREFCOUNT(ob, fn)  Tcl_IncrRefCount(ob)
#   define PLUSDEBUG_DECRREFCOUNT(ob, fn)  Tcl_DecrRefCount(ob)
#endif

static t_symbol *plusps_ar;
static t_symbol *plusps_env;
static t_symbol *plusps_in;
static t_symbol *plusps_var;
static t_symbol *plusps_out;
static t_symbol *plusps_qlist;
static t_symbol *plusps_print;
static t_symbol *totps_query;

static void plussymbols_create(void)
{
    /* public */
    totps_plustot = gensym("plustot");
    plusps_tot = gensym("+tot");
    plusps_Ti = gensym("+Ti");
    plusps_To = gensym("+To");
    plusps_Tv = gensym("+Tv");

    /* private */
    plusps_ar = gensym("+ar");
    plusps_env = gensym("+env");
    plusps_in = gensym("+in");
    plusps_var = gensym("+var");
    plusps_out = gensym("+out");
    plusps_qlist = gensym("+qlist");
    plusps_print = gensym("+print");
    totps_query = gensym("query");
}

static void plusloud_tcldirty(t_pd *caller, char *fnname)
{
    loud_warning((caller == PLUSBOB_OWNER ? 0 : caller), "+tot",
		 "(%s) tcl plays dirty tricks, sorry", fnname);
}

void plusloud_tclerror(t_pd *caller, Tcl_Interp *interp, char *msg)
{
    Tcl_Obj *ob = Tcl_GetObjResult(interp);
    loud_error((caller == PLUSBOB_OWNER ? 0 : caller), msg);
    if (ob)
    {
	int len;
	char *res = Tcl_GetStringFromObj(ob, &len);
	if (res && len > 0)
	{
	    char buf[MAXPDSTRING];
	    if (len > (MAXPDSTRING-2))
	    {
		len = (MAXPDSTRING-2);
		buf[MAXPDSTRING-2] = '*';
		buf[MAXPDSTRING-1] = 0;
	    }
	    else buf[len] = 0;
	    strncpy(buf, res, len);
	    loud_errand((caller == PLUSBOB_OWNER ? 0 : caller),
			"(tcl) %s", buf);
	}
	else ob = 0;
	Tcl_ResetResult(interp);
    }
    if (!ob) loud_errand((caller == PLUSBOB_OWNER ? 0 : caller),
			 "unknown error (probably a bug)");
}

/* Plustin (aka +Ti) is a Tcl_Interp wrapped as a +bob.
   This is a glist-based flavor of Plusenv. */

struct _plustin
{
    t_plusenv     tin_env;
    t_glist      *tin_glist;
    Tcl_Interp   *tin_interp;
    Tcl_CmdInfo  *tin_cinfop;
};

static t_plustype *plustin_basetype;
static t_plustype *plustin_type;
static t_plustin *plustin_default = 0;

static int plustin_testCmd(ClientData cd, Tcl_Interp *interp,
			   int objc,  Tcl_Obj **objv)
{
    Tcl_Obj *result;
    post("this is a test");
    if (objc != 2)
    {
	Tcl_WrongNumArgs(interp, 1, objv, "anyValue");
	return (TCL_ERROR);
    }

    post("in refcount: %d", objv[1]->refCount);
    result = Tcl_DuplicateObj(objv[1]);
    post("out refcount: %d", result->refCount);

    if (result == NULL)
	return (TCL_ERROR);
    Tcl_SetObjResult(interp, result);
    post("exit refcount: %d", result->refCount);
    return (TCL_OK);
}

/* To be called from derived constructors or plustin's provider. */
t_plustin *plustin_create(t_plustype *tp, t_plusbob *parent, t_symbol *id)
{
    t_plustin *tin = 0;
    Tcl_Interp *interp = Tcl_CreateInterp();
    if (interp && (tin = (t_plustin *)plusenv_create(tp, parent, id)))
    {
#ifdef PLUSTOT_DEBUG
	loudbug_post("plustin_create '%s' over %x",
		     (id ? id->s_name : "default"), (int)interp);
#endif
	tin->tin_interp = interp;
	tin->tin_cinfop = 0;
	Tcl_Preserve(interp);
	if (Tcl_Init(interp) == TCL_ERROR)
	    plusloud_tclerror(0, interp, "interpreter initialization failed");
	else
	{
	    Tcl_CmdInfo cinfo;
	    /* store Tcl_CmdInfo for off-API Tcl_InfoObjCmd() */
	    if (Tcl_GetCommandInfo(interp, "info", &cinfo)
		&& cinfo.isNativeObjectProc)
		tin->tin_cinfop = copybytes(&cinfo, sizeof(*tin->tin_cinfop));
	    /* create custom commands */
	    Tcl_CreateObjCommand(interp, "test::test",
				 (Tcl_ObjCmdProc*)plustin_testCmd,
				 (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
	}
	Tcl_Release(interp);
    }
    else loud_error(0, "failed attempt to create an interpreter");
    return (tin);
}

/* To be registered for calling from plusbob_release().
   Should never be called explicitly. */
static void plustin_delete(t_plustin *tin)
{
#ifdef PLUSTOT_DEBUG
    t_symbol *id = plusenv_getid((t_plusenv *)tin);
    loudbug_post("plustin_delete '%s' over %x",
		 (id ? id->s_name : "default"), (int)tin->tin_interp);
#endif
    if (tin->tin_cinfop)
	freebytes(tin->tin_cinfop, sizeof(*tin->tin_cinfop));
    Tcl_Preserve(tin->tin_interp);
    if (!Tcl_InterpDeleted(tin->tin_interp))
	Tcl_DeleteInterp(tin->tin_interp);
    Tcl_Release(tin->tin_interp);
}

Tcl_Interp *plustin_getinterp(t_plustin *tin)
{
    return (tin->tin_interp);
}

int plustin_procargc(t_plustin *tin, char *pname)
{
    int result = -1;
    if (tin->tin_cinfop)
    {
	/* FIXME preallocate */
	Tcl_Obj *argv[3];
	argv[0] = Tcl_NewStringObj("info", -1);
	PLUSDEBUG_INCRREFCOUNT(argv[0], "plustin_procargc");
	argv[1] = Tcl_NewStringObj("args", -1);
	PLUSDEBUG_INCRREFCOUNT(argv[0], "plustin_procargc");
	argv[2] = Tcl_NewStringObj(pname, -1);
	PLUSDEBUG_INCRREFCOUNT(argv[1], "plustin_procargc");
	if ((*tin->tin_cinfop->objProc)(tin->tin_cinfop->objClientData,
					tin->tin_interp,
					3, argv) == TCL_OK)
	{
	    Tcl_Obj *rob;
	    if (rob = Tcl_GetObjResult(tin->tin_interp))
	    {
		PLUSDEBUG_INCRREFCOUNT(rob, "plustin_procargc");
		if (Tcl_ListObjLength(tin->tin_interp, rob, &result) != TCL_OK)
		{
		    result = -1;
		    plusloud_tcldirty(0, "plustin_procargc");
		}
		Tcl_ResetResult(tin->tin_interp);
		PLUSDEBUG_DECRREFCOUNT(rob, "plustin_procargc");
	    }
	    else plusloud_tcldirty(0, "plustin_procargc");
	}
	PLUSDEBUG_DECRREFCOUNT(argv[0], "plustin_procargc");
	PLUSDEBUG_DECRREFCOUNT(argv[1], "plustin_procargc");
	PLUSDEBUG_DECRREFCOUNT(argv[2], "plustin_procargc");
    }
    return (result);
}

t_symbol *plustin_glistid(t_glist *gl)
{
    char buf[32];
    sprintf(buf, "+ti%x", (int)gl);
    return (gensym(buf));
}

t_plustin *plustin_glistfind(t_glist *gl, int mode)
{
    t_plustin *tin = 0;
    if (mode == PLUSTIN_GLIST_UP)
    {
	gl = gl->gl_owner;
	mode = PLUSTIN_GLIST_ANY;
    }
    if (mode == PLUSTIN_GLIST_THIS)
	return ((t_plustin *)plusenv_find(plustin_glistid(gl),
					  (t_plusenv *)plustin_default));
    else
    {
	while (gl)
	{
	    char buf[32];
	    sprintf(buf, "+ti%x", (int)gl);
	    if (tin = (t_plustin *)plusenv_find(gensym(buf),
						(t_plusenv *)plustin_default))
		break;
	    gl = gl->gl_owner;
	}
	return (tin ? tin : plustin_default);
    }
}

/* To be called from client code, instead of plustin_create().
   Preserving is caller's responsibility.
   Never returns null, even when called with create == 0:
   if requested id not found, default returned, created if necessary. */
t_plustin *plustin_glistprovide(t_glist *gl, int mode, int create)
{
    t_plustin *tin = 0;
    t_plusbob *parent = plusenv_getparent(plustin_type);
    if (mode == PLUSTIN_GLIST_UP)
    {
	gl = gl->gl_owner;
	mode = PLUSTIN_GLIST_ANY;
    }
    tin = plustin_glistfind(gl, mode);
    if (!tin && create)
    {
	if (tin = plustin_create(plustin_type, parent, plustin_glistid(gl)))
	    tin->tin_glist = gl;
    }
    if (!tin)
    {
	if (!plustin_default)
	    plustin_default = plustin_create(plustin_type, parent, 0);
	tin = plustin_default;
    }
    return (tin);
}

t_symbol *plustin_getglistname(t_plustin *tin)
{
    return (tin->tin_glist ? tin->tin_glist->gl_name : 0);
}

/* Plustob (aka +To) is a Tcl_Obj wrapped as a +bob. */

/* LATER rethink the plustob/plusvar rules, measure performance.
   There are two cases:

   `Bobbing' is taking an object from its wrapping bob, wrapping it into
   another bob and, optionally, setting a variable to it.

   The main deal of bobbing design is not to Tcl_DuplicateObj while passing
   bobs around.

   `Messing' is converting a Pd message (float, symbol or list) to an object,
   wrapping it and, optionally, setting a variable to it.

   The obvious sequence of {Decr(old), New, Incr(new), SetVar(new)}, which
   is currently used, involves picking a new object (New), while returning
   an old one to the pool (by SetVar or Decr, depending on a third party
   changing or not the tcl variable's value in the meantime).  I guess
   the overhead is negligible, unless we hit at the bottom of the pool.
   Moreover, we can reduce the sequence to just {Set(old), SetVar(old)},
   in the case when old is not shared (referenced neither by a variable,
   nor by a third party).  The main advantage is being consistent with
   the way Tcl itself was designed.

   An alternative:  in the original messing design, the trick was to:

   . call Set on a prepicked object, instead of New
   . call SetVar on a preserved object, as usual (var would not own its value)
   . alternate between two prepicked objects in order to avoid calling UnsetVar

   So, the sequence was just {Set(v1), SetVar(v1)}, then {Set(v2), SetVar(v2)},
   again {Set(v1), SetVar(v1)}, and so on, unless a third party (other than
   plusvar and a tcl variable) referenced our prepicked object. */

#define PLUSTOB_INIELBUFSIZE  128  /* LATER rethink */

struct _plustob
{
    t_plusbob         tob_bob;
    Tcl_Obj          *tob_value;
    t_plustin        *tob_tin;  /* redundant, LATER rethink */
    t_plusifsharedfn  tob_ifsharedfn;
    int               tob_elbufsize;
    Tcl_Obj         **tob_elbuf;
    Tcl_Obj          *tob_elbufini[PLUSTOB_INIELBUFSIZE];
};

static t_plustype *plustob_type;

/* To be called from derived constructors.
   Preserving is caller's responsibility. */
t_plustob *plustob_create(t_plustype *tp, t_plustin *tin, Tcl_Obj *ob)
{
    t_plustob *tob = 0;
    if (tin && (ob != PLUSTOB_MAKEIT || (ob = Tcl_NewObj()))
	&& (tob = (t_plustob *)plusbob_create(tp, (t_plusbob *)tin)))
    {
	if (ob) PLUSDEBUG_INCRREFCOUNT(ob, "plustob_create");
	plusbob_preserve((t_plusbob *)tin);
	tob->tob_value = ob;
	tob->tob_tin = tin;
	tob->tob_ifsharedfn = 0;
	tob->tob_elbufsize = PLUSTOB_INIELBUFSIZE;
	tob->tob_elbuf = tob->tob_elbufini;
    }
    return (tob);
}

/* To be registered for calling from plusbob_release().
   Should never be called explicitly. */
static void plustob_delete(t_plustob *tob)
{
    if (tob->tob_tin)
	plusbob_release((t_plusbob *)tob->tob_tin);
    if (tob->tob_value)
	PLUSDEBUG_DECRREFCOUNT(tob->tob_value, "plustob_delete");
    if (tob->tob_elbuf != tob->tob_elbufini)
	freebytes(tob->tob_elbuf, tob->tob_elbufsize * sizeof(*tob->tob_elbuf));
}

/* To be registered for calling from plusbob_attach().
   Should never be called explicitly. */
static void plustob_attach(t_plustob *tob)
{
    t_plustin *tin;
    if (tin = (t_plustin *)plusbob_getparent((t_plusbob *)tob))
    {
	if (tob->tob_tin)
	    plusbob_release((t_plusbob *)tob->tob_tin);
	tob->tob_tin = tin;
	plusbob_preserve((t_plusbob *)tin);
    }
    else loudbug_bug("plustob_attach");
}

/* To be called from client code.
   Preserving is caller's responsibility. */
t_plustob *plustob_new(t_plustin *tin, Tcl_Obj *ob)
{
    return (plustob_create(plustob_type, tin, ob));
}

void plustob_setifshared(t_plustob *tob, t_plusifsharedfn ifsharedfn)
{
    tob->tob_ifsharedfn = ifsharedfn;
}

int plustob_isshared(t_plustob *tob)
{
    return (tob->tob_value && Tcl_IsShared(tob->tob_value));
}

Tcl_Obj *plustob_getvalue(t_plustob *tob)
{
    return (tob->tob_value);
}

/* silent, if caller is empty */
t_plustin *plustag_tobtin(t_symbol *tag, t_pd *caller)
{
    t_plusbob *bob = plustag_validroot(tag, plusps_To, caller);
    return (bob ? ((t_plustob *)bob)->tob_tin : 0);
}

/* silent, if caller is empty */
Tcl_Obj *plustag_tobvalue(t_symbol *tag, t_pd *caller)
{
    t_plusbob *bob = plustag_validroot(tag, plusps_To, caller);
    return (bob ? ((t_plustob *)bob)->tob_value : 0);
}

/* silent, if caller is empty */
Tcl_Obj *plusatom_tobvalue(t_atom *ap, t_pd *caller)
{
    if (ap->a_type == A_SYMBOL)
	return (plustag_tobvalue(ap->a_w.w_symbol, caller));
    else if (caller)
    {
    	char buf[80];
    	atom_string(ap, buf, 80);
	loud_error((caller == PLUSBOB_OWNER ? 0 : caller),
 "+tot does not understand '%s' (check object connections)", buf);
    }
    return (0);
}

int plustob_clear(t_plustob *tob)
{
    if (!tob->tob_tin)
    {
	/* FIXME */
	loud_warning(0, "+tot", "+To: environment missing");
	return (0);
    }
    if (tob->tob_value)
    {
	PLUSDEBUG_DECRREFCOUNT(tob->tob_value, "plustob_clear");
	tob->tob_value = 0;
	return (1);
    }
    else return (0);
}

Tcl_Obj *plustob_set(t_plustob *tob, t_plustin *tin, Tcl_Obj *ob)
{
    if (tin != tob->tob_tin)
    {
	/* FIXME */
	loud_warning(0, "+tot", "+To: environment mismatch");
	return (0);
    }
    if (ob != tob->tob_value)
    {
	if (tob->tob_value)
	    PLUSDEBUG_DECRREFCOUNT(tob->tob_value, "plustob_set");
	if (ob)
	{
	    PLUSDEBUG_INCRREFCOUNT(ob, "plustob_set");
	    if (Tcl_IsShared(ob))
	    {
		/* FIXME */
	    }
	}
	tob->tob_value = ob;
    }
    return (ob);
}

Tcl_Obj *plustob_setfloat(t_plustob *tob, t_float f)
{
    Tcl_Obj *ob = tob->tob_value;
    if (!ob || Tcl_IsShared(ob))
    {
	Tcl_Obj *tmp;
	int i = (int)f;
	if (ob && tob->tob_ifsharedfn)
	{
	    if ((*tob->tob_ifsharedfn)((t_plusbob *)tob, ob) == 0)
		return (0);
	}
	if (f == i)  /* LATER rethink */
	    tmp = Tcl_NewIntObj(i);
	else
	    tmp = Tcl_NewDoubleObj((double)f);
	if (tmp)
	{
	    if (ob) PLUSDEBUG_DECRREFCOUNT(ob, "plustob_setfloat");
	    tob->tob_value = ob = tmp;
	    PLUSDEBUG_INCRREFCOUNT(ob, "plustob_setfloat");
	}
	else return (0);
    }
    else
    {
	int i = (int)f;
	if (f == i)  /* LATER rethink */
	    Tcl_SetIntObj(ob, i);
	else
	    Tcl_SetDoubleObj(ob, (double)f);
    }
    return (ob);
}

Tcl_Obj *plustob_setsymbol(t_plustob *tob, t_symbol *s)
{
    if (plustag_isvalid(s, 0))
    {
	t_plusbob *bob;
	if (bob = plustag_validroot(s, plusps_To, PLUSBOB_OWNER))
	{
	    t_plustob *from = (t_plustob *)bob;
	    return (plustob_set(tob, from->tob_tin, from->tob_value));
	}
	else return (0);
    }
    else
    {
	Tcl_Obj *ob = tob->tob_value;
	if (!ob || Tcl_IsShared(ob))
	{
	    Tcl_Obj *tmp;
	    if (ob && tob->tob_ifsharedfn)
	    {
		if ((*tob->tob_ifsharedfn)((t_plusbob *)tob, ob) == 0)
		    return (0);
	    }
	    if (tmp = Tcl_NewStringObj(s->s_name, -1))
	    {
		if (ob) PLUSDEBUG_DECRREFCOUNT(ob, "plustob_setsymbol");
		tob->tob_value = ob = tmp;
		PLUSDEBUG_INCRREFCOUNT(ob, "plustob_setsymbol");
	    }
	    else return (0);
	}
	else Tcl_SetStringObj(ob, s->s_name, -1);
	return (ob);
    }
}

Tcl_Obj *plustob_setlist(t_plustob *tob, int ac, t_atom *av)
{
    if (ac == 1)
    {
	if (av->a_type == A_FLOAT)
	    return (plustob_setfloat(tob, av->a_w.w_float));
	else if (av->a_type == A_SYMBOL)
	    return (plustob_setsymbol(tob, av->a_w.w_symbol));
    }
    else if (ac > 1)
    {
	Tcl_Obj *ob = tob->tob_value;
	int count;
	t_atom *ap;
	for (count = 0, ap = av; count < ac; count++, ap++)
	    if (ap->a_type != A_FLOAT && ap->a_type != A_SYMBOL)
		break;
	if (count > tob->tob_elbufsize)
	{
#ifdef PLUSTOT_DEBUG
	    loudbug_post("growing +To %d -> %d", tob->tob_elbufsize, count);
#endif
	    tob->tob_elbuf =
		grow_nodata(&count, &tob->tob_elbufsize, tob->tob_elbuf,
			    PLUSTOB_INIELBUFSIZE, tob->tob_elbufini,
			    sizeof(*tob->tob_elbuf));
	}
	if (count > 0)
	{
	    int i;
	    Tcl_Obj **elp;
	    for (i = 0, elp = tob->tob_elbuf; i < count; i++, elp++, av++)
	    {
		if (av->a_type == A_FLOAT)
		    *elp = Tcl_NewDoubleObj((double)av->a_w.w_float);
		else if (av->a_type == A_SYMBOL)
		    *elp = Tcl_NewStringObj(av->a_w.w_symbol->s_name, -1);
	    }
	    if (!ob || Tcl_IsShared(ob))
	    {
		Tcl_Obj *tmp;
		if (ob && tob->tob_ifsharedfn)
		{
		    if ((*tob->tob_ifsharedfn)((t_plusbob *)tob, ob) == 0)
			return (0);
		}
		if (tmp = Tcl_NewListObj(count, tob->tob_elbuf))
		{
		    if (ob) PLUSDEBUG_DECRREFCOUNT(ob, "plustob_setlist");
		    tob->tob_value = ob = tmp;
		    PLUSDEBUG_INCRREFCOUNT(ob, "plustob_setlist");
		}
		else return (0);
	    }
	    else Tcl_SetListObj(ob, count, tob->tob_elbuf);
	    return (ob);
	}
    }
    return (0);  /* count == 0, LATER rethink */
}

static int plustob_parseatoms(int ac, t_atom *av, int *natomsp, int *nlistsp,
			      Tcl_Obj **listobs, Tcl_Obj **atomobs)
{
    int i, natoms = 0, nlists = 0, start = 1;
    t_atom *ap;
    int atomcnt = 0;
    Tcl_Obj **atomptr = atomobs;
    for (i = 0, ap = av; i < ac; i++, ap++)
    {
	if (ap->a_type == A_SEMI || ap->a_type == A_COMMA)
	{
	    /* empty lists are filtered out, LATER rethink */
	    if (!start)
	    {
		if (listobs)
		{
		    if (listobs[nlists] = Tcl_NewListObj(atomcnt, atomptr))
		    {
			atomptr += atomcnt;
			atomcnt = 0;
		    }
		    else goto parsefailed;
		}
		nlists++;
	    }
	    start = 1;
	}
	else
	{
	    /* other types are ignored, LATER rethink */
	    start = 0;
	    if (ap->a_type == A_FLOAT || ap->a_type == A_SYMBOL)
	    {
		if (atomobs)
		{
		    if (!(atomobs[natoms] =
			  (ap->a_type == A_FLOAT ?
			   Tcl_NewDoubleObj((double)ap->a_w.w_float) :
			   Tcl_NewStringObj(ap->a_w.w_symbol->s_name, -1))))
			goto parsefailed;
		    atomcnt++;
		}
		natoms++;
	    }
	}
    }
    if (natoms && !start)
    {
	if (listobs &&
	    !(listobs[nlists] = Tcl_NewListObj(atomcnt, atomptr)))
	    goto parsefailed;
	nlists++;
    }
    if (natomsp) *natomsp = natoms;
    if (nlistsp) *nlistsp = nlists;
    return (1);
parsefailed:
    /* FIXME cleanup */
    return (0);
}

Tcl_Obj *plustob_setbinbuf(t_plustob *tob, t_binbuf *bb)
{
    int ac = binbuf_getnatom(bb);
    if (ac)
    {
	t_atom *av = binbuf_getvec(bb);
	Tcl_Obj *ob = tob->tob_value;
	int count, natoms, nlists;
	plustob_parseatoms(ac, av, &natoms, &nlists, 0, 0);
	count = natoms + nlists;
	if (count > tob->tob_elbufsize)
	{
	    int n = count;
#ifdef PLUSTOT_DEBUG
	    loudbug_post("growing +To %d -> %d", tob->tob_elbufsize, count);
#endif
	    tob->tob_elbuf =
		grow_nodata(&n, &tob->tob_elbufsize, tob->tob_elbuf,
			    PLUSTOB_INIELBUFSIZE, tob->tob_elbufini,
			    sizeof(*tob->tob_elbuf));
	    if (n < count)
		goto setbbfailed;
	}
	if (!plustob_parseatoms(ac, av, 0, 0,
				tob->tob_elbuf, tob->tob_elbuf + nlists))
	    goto setbbfailed;
	if (!ob || Tcl_IsShared(ob))
	{
	    Tcl_Obj *tmp;
	    if (ob && tob->tob_ifsharedfn)
	    {
		if ((*tob->tob_ifsharedfn)((t_plusbob *)tob, ob) == 0)
		    goto setbbfailed;
	    }
	    if (tmp = Tcl_NewListObj(nlists, tob->tob_elbuf))
	    {
		if (ob) PLUSDEBUG_DECRREFCOUNT(ob, "plustob_setbinbuf");
		tob->tob_value = ob = tmp;
		PLUSDEBUG_INCRREFCOUNT(ob, "plustob_setbinbuf");
	    }
	    else goto setbbfailed;
	}
	else Tcl_SetListObj(ob, nlists, tob->tob_elbuf);
	return (ob);
    }
setbbfailed:
    return (0);
}

Tcl_Obj *plustob_grabresult(t_plustob *tob)
{
    Tcl_Interp *interp = tob->tob_tin->tin_interp;
    Tcl_Obj *rob;
    if (rob = Tcl_GetObjResult(interp))
    {
	if (rob == tob->tob_value)
	    Tcl_ResetResult(interp);
	else
	{
	    PLUSDEBUG_INCRREFCOUNT(rob, "plustob_grabresult");
	    Tcl_ResetResult(interp);
	    if (Tcl_IsShared(rob))
	    {
		/* FIXME */
	    }
	    if (tob->tob_value)
		PLUSDEBUG_DECRREFCOUNT(tob->tob_value, "plustob_grabresult");
	    tob->tob_value = rob;
	}
    }
    else plusloud_tcldirty(plusbob_getowner((t_plusbob *)tob),
			   "plustob_grabresult");
    return (rob);
}

Tcl_Obj *plustob_evalob(t_plustob *tob, Tcl_Obj *ob)
{
    Tcl_Interp *interp = tob->tob_tin->tin_interp;
    Tcl_Obj *rob;
    Tcl_Preserve(interp);
    if (Tcl_EvalObj(interp, ob) == TCL_OK)
	rob = plustob_grabresult(tob);
    else
    {
	plusloud_tclerror(plusbob_getowner((t_plusbob *)tob), interp,
			  "immediate command failed");
	rob = 0;
    }
    Tcl_Release(interp);
    return (rob);
}

/* Plusvar (aka +Tv) is a plustob with a one-way link to a tcl variable.
   Whenever plusvar's value changes, the variable is set to it (the opposite
   update requires explicitly calling the plusvar_pull() request).
   This is different from one-way linking by passing TCL_LINK_READ_ONLY flag
   to Tcl_LinkVar():  plusvar's variable is not forced to be read-only,
   and its value's form and internal representation are not constrained. */

struct _plusvar
{
    t_plustob  var_tob;
    char      *var_name;
    char      *var_index;
    Tcl_Obj   *var_part1;
    Tcl_Obj   *var_part2;
};

static t_plustype *plusvar_type;

/* Since tcl always uses a hash table of string indices for array element
   lookup, there are never any gains when using integer indices. */

/* To be called from derived constructors.
   Preserving is caller's responsibility. */
t_plusvar *plusvar_create(t_plustype *tp, t_plustin *tin, Tcl_Obj *ob,
			  char *name, char *index)
{
    t_plusvar *var = 0;
    Tcl_Obj *ntob = 0;
    Tcl_Obj *itob = 0;
    if (name && *name)
    {
	if (ntob = Tcl_NewStringObj(name, -1))
	{
	    PLUSDEBUG_INCRREFCOUNT(ntob, "plusvar_create");
	}
	else goto varfailed1;
    }
    else
    {
	loudbug_bug("plusvar_create");
	goto varfailed2;
    }
    if (index)
    {
	if (itob = Tcl_NewStringObj(index, -1))
	{
	    PLUSDEBUG_INCRREFCOUNT(itob, "plusvar_create");
	}
	else goto varfailed1;
    }
    if (var = (t_plusvar *)plustob_create(tp, tin, ob))
    {
	var->var_name = getbytes(strlen(name) + 1);
	strcpy(var->var_name, name);
	if (index)
	{
	    var->var_index = getbytes(strlen(index) + 1);
	    strcpy(var->var_index, index);
	}
	else var->var_index = 0;
	var->var_part1 = ntob;
	var->var_part2 = itob;
    }
    else goto varfailed2;
    return (var);
varfailed1:
    plusloud_tcldirty(0, "plusvar_create");
varfailed2:
    if (ntob) PLUSDEBUG_DECRREFCOUNT(ntob, "plusvar_create");
    if (itob) PLUSDEBUG_DECRREFCOUNT(itob, "plusvar_create");
    return (0);
}

/* To be registered for calling from plusbob_release().
   Should never be called explicitly. */
static void plusvar_delete(t_plusvar *var)
{
    freebytes(var->var_name, strlen(var->var_name) + 1);
    if (var->var_index)
	freebytes(var->var_index, strlen(var->var_index) + 1);
    PLUSDEBUG_DECRREFCOUNT(var->var_part1, "plusvar_delete");
    if (var->var_part2)
	PLUSDEBUG_DECRREFCOUNT(var->var_part2, "plusvar_delete");
}

/* To be called from client code.
   Preserving is caller's responsibility */
t_plusvar *plusvar_new(char *name, char *index, t_plustin *tin)
{
    return (plusvar_create(plusvar_type, tin, 0, name, index));
}

/* not used yet */
static int plusvar_ifshared(t_plusbob *bob, Tcl_Obj *ob)
{
    /* Shared means either the variable still holds our value, or the value
       is referenced by a third party, or both. In either case, we have to
       pick a new object.
       LATER consider testing for illegal use of a pseudo-variable. */
    return (1);
}

/* synchronize a Tcl variable to a +var */
/* LATER try making it more efficient */
static Tcl_Obj *plusvar_postset(t_plusvar *var)
{
    Tcl_Obj *rob;
    t_plustob *tob = (t_plustob *)var;
    Tcl_Interp *interp = tob->tob_tin->tin_interp;
    Tcl_Preserve(interp);
    if (tob->tob_value)
    {
	rob = Tcl_ObjSetVar2(interp, var->var_part1, var->var_part2,
			     tob->tob_value, 0);
	if (!rob)
	{
	    if (Tcl_UnsetVar2(interp, var->var_name, var->var_index,
			      TCL_LEAVE_ERR_MSG) == TCL_OK)
		rob = Tcl_ObjSetVar2(interp, var->var_part1, var->var_part2,
				     tob->tob_value, TCL_LEAVE_ERR_MSG);
	}
	if (rob)
	{
#ifdef PLUSTOT_DEBUGREFCOUNTS
	    if (var->var_index)
		loudbug_post("vv %x plusvar_postset [%s(%s)]",
			     (int)tob->tob_value, var->var_name,
			     var->var_index);
	    else
		loudbug_post("vv %x plusvar_postset [%s]",
			     (int)tob->tob_value, var->var_name);
#endif
	}
	else plusloud_tclerror(0, interp, "cannot set variable");
    }
    else
    {
	if (Tcl_UnsetVar2(interp, var->var_name, var->var_index,
			  TCL_LEAVE_ERR_MSG) != TCL_OK)
	    plusloud_tclerror(0, interp, "cannot unset variable");
	rob = 0;
    }
    Tcl_Release(interp);
    return (rob);
}

/* move a +var's value into a Tcl variable */
Tcl_Obj *plusvar_push(t_plusvar *var)
{
    if (((t_plustob *)var)->tob_value)
	return (plusvar_postset(var));
    else
	return (0);
}

/* move a Tcl variable's value into a +var */
Tcl_Obj *plusvar_pull(t_plusvar *var)
{
    Tcl_Obj *rob;
    t_plustob *tob = (t_plustob *)var;
    Tcl_Interp *interp = tob->tob_tin->tin_interp;
    Tcl_Preserve(interp);
    if (rob = Tcl_ObjGetVar2(interp, var->var_part1, var->var_part2,
			     TCL_LEAVE_ERR_MSG))
	plustob_set(tob, tob->tob_tin, rob);
    else
	plusloud_tclerror(0, interp, "cannot read variable");
    Tcl_Release(interp);
    return (rob);
}

void plusvar_clear(t_plusvar *var, int doit)
{
    if (plustob_clear((t_plustob *)var) && doit)
	plusvar_postset(var);
}

Tcl_Obj *plusvar_set(t_plusvar *var, Tcl_Obj *ob, int doit)
{
    t_plustob *tob = (t_plustob *)var;
    if (plustob_set(tob, tob->tob_tin, ob))
	return (doit ? plusvar_postset(var) : tob->tob_value);
    else
	return (0);
}

Tcl_Obj *plusvar_setfloat(t_plusvar *var, t_float f, int doit)
{
    t_plustob *tob = (t_plustob *)var;
    if (plustob_setfloat(tob, f))
	return (doit ? plusvar_postset(var) : tob->tob_value);
    else
	return (0);
}

Tcl_Obj *plusvar_setsymbol(t_plusvar *var, t_symbol *s, int doit)
{
    t_plustob *tob = (t_plustob *)var;
    if (plustob_setsymbol(tob, s))
	return (doit ? plusvar_postset(var) : tob->tob_value);
    else
	return (0);
}

Tcl_Obj *plusvar_setlist(t_plusvar *var, int ac, t_atom *av, int doit)
{
    t_plustob *tob = (t_plustob *)var;
    if (plustob_setlist(tob, ac, av))
	return (doit ? plusvar_postset(var) : tob->tob_value);
    else
	return (0);
}

/* LATER derive +string from +bob */

struct _plusstring
{
    int    ps_len;
    char  *ps_buf;
    int    ps_refcount;
};

/* Resolving dot-separators, unless script is empty. */
static t_plusstring *plusstring_fromatoms(t_symbol *s, int ac, t_atom *av,
					  t_scriptlet *script)
{
    t_plusstring *ps = 0;
    char *buf;
    int length;
    if (script)
    {
	char *start;
	scriptlet_reset(script);
	if (s && s != &s_)
	{
	    t_atom at;
	    SETSYMBOL(&at, s);
	    scriptlet_add(script, 1, 1, 1, &at);
	}
	scriptlet_add(script, 1, 1, ac, av);
	start = scriptlet_getcontents(script, &length);
	buf = copybytes(start, length);
    }
    else
    {
	char string[MAXPDSTRING];
	char *newbuf;
	if (s && s != &s_)
	{
	    t_atom at;
	    SETSYMBOL(&at, s);
	    atom_string(&at, string, MAXPDSTRING);
	    length = strlen(string) + 1;
	    buf = getbytes(length);
	    strcpy(buf, string);
	    buf[length-1] = ' ';
	}
	else
	{
	    buf = getbytes(0);
	    length = 0;
	}
	while (ac--)
	{
	    int newlength;
	    if ((av->a_type == A_SEMI || av->a_type == A_COMMA) &&
		length && buf[length-1] == ' ') length--;
	    atom_string(av, string, MAXPDSTRING);
	    newlength = length + strlen(string) + 1;
	    if (!(newbuf = resizebytes(buf, length, newlength))) break;
	    buf = newbuf;
	    strcpy(buf + length, string);
	    length = newlength;
	    if (av->a_type == A_SEMI) buf[length-1] = '\n';
	    else buf[length-1] = ' ';
	    av++;
	}
	if (length && buf[length-1] == ' ')
	{
	    if (newbuf = resizebytes(buf, length, length-1))
	    {
		buf = newbuf;
		length--;
	    }
	}
    }
    ps = getbytes(sizeof(*ps));
    ps->ps_len = length;
    ps->ps_buf = buf;
    ps->ps_refcount = 0;
    return (ps);
}

void plusstring_preserve(t_plusstring *ps)
{
    ps->ps_refcount++;
}

void plusstring_release(t_plusstring *ps)
{
    if (--ps->ps_refcount <= 0)
    {
	if (ps->ps_refcount == 0)
	{
	    if (ps->ps_buf) freebytes(ps->ps_buf, ps->ps_len);
	    freebytes(ps, sizeof(*ps));
	}
	else loudbug_bug("plusstring_release");
    }
}

char *plusstring_get(t_plusstring *ps, int *lenp)
{
    *lenp = ps->ps_len;
    return (ps->ps_buf);
}

struct _plustot;
#define t_plustot  struct _plustot
static int plustot_doit(t_plustot *x, int sendit);

typedef struct _plusproxy
{
    t_pd        pp_pd;
    t_plustot  *pp_master;
    t_plusvar  *pp_var;
    int         pp_ndx;
    int         pp_ishot;
    int         pp_istransient;
    int         pp_warned;
} t_plusproxy;

static t_class *plusproxy_class;

/* Variable is to be created during the second parsing pass, in order to give
   it an actual name, and in order to fill only the slots that are actually
   referenced.  If ndx is negative, then this is a pseudo-scalar, otherwise
   this is a pseudo-array element. */
static t_plusproxy *plusproxy_new(t_plustot *master, int ndx,
				  int ishot, int istransient,
				  t_plustin *tin)
{
    t_plusproxy *pp = (t_plusproxy *)pd_new(plusproxy_class);
    pp->pp_master = master;
    pp->pp_var = 0;
    pp->pp_ndx = ndx;
    pp->pp_ishot = ishot;
    pp->pp_istransient = istransient;
    pp->pp_warned = 0;
    return (pp);
}

static void plusproxy_free(t_plusproxy *pp)
{
#ifdef PLUSTOT_DEBUG
    loudbug_post("plusproxy_free (%s %d)",
		 (pp->pp_var ? pp->pp_var->var_name : "deaf"), pp->pp_ndx);
#endif
    if (pp->pp_var)
	plusbob_release((t_plusbob *)pp->pp_var);
}

static void plusproxy_deafhit(t_plusproxy *pp)
{
    if (!pp->pp_warned)
    {
	loud_error((t_pd *)pp->pp_master, "deaf slot hit");
	pp->pp_warned = 1;
    }
}

static void plusproxy_clear(t_plusproxy *pp)
{
    if (pp->pp_var)
	plusvar_clear(pp->pp_var, 1);
    else
	plusproxy_deafhit(pp);
}

static void plusproxy_bang(t_plusproxy *pp)
{
    if (pp->pp_var)
	plustot_doit(pp->pp_master, 1);
    else
	plusproxy_deafhit(pp);
}

static void plusproxy_float(t_plusproxy *pp, t_float f)
{
    if (pp->pp_var)
    {
	plusvar_setfloat(pp->pp_var, f, 0);
	if (pp->pp_ishot)
	    plustot_doit(pp->pp_master, 1);
    }
    else plusproxy_deafhit(pp);
}

static void plusproxy_symbol(t_plusproxy *pp, t_symbol *s)
{
    if (pp->pp_var)
    {
	plusvar_setsymbol(pp->pp_var, s, 0);
	if (pp->pp_ishot)
	    plustot_doit(pp->pp_master, 1);
    }
    else plusproxy_deafhit(pp);
}

static void plusproxy_list(t_plusproxy *pp, t_symbol *s, int ac, t_atom *av)
{
    if (pp->pp_var)
    {
	plusvar_setlist(pp->pp_var, ac, av, 0);
	if (pp->pp_ishot)
	    plustot_doit(pp->pp_master, 1);
    }
    else plusproxy_deafhit(pp);
}

static void plusproxy_set(t_plusproxy *pp, t_symbol *s, int ac, t_atom *av)
{
    if (pp->pp_var)
    {
	if (ac == 1)
	{
	    if (av->a_type == A_FLOAT)
		plusvar_setfloat(pp->pp_var, av->a_w.w_float, 0);
	    else if (av->a_type == A_SYMBOL)
		plusvar_setsymbol(pp->pp_var, av->a_w.w_symbol, 0);
	}
	else plusvar_setlist(pp->pp_var, ac, av, 0);
    }
    else plusproxy_deafhit(pp);
}

#ifdef PLUSTOT_DEBUG
static void plusproxy_debug(t_plusproxy *pp)
{
    t_plustin *tin = ((t_plustob *)pp->pp_var)->tob_tin;
    t_symbol *id = plusenv_getid((t_plusenv *)tin);
    t_symbol *glname = plustin_getglistname(tin);
    loudbug_post("+proxy %d, glist %x",
		 pp->pp_ndx, (int)((t_plusobject *)pp->pp_master)->po_glist);
    loudbug_post("  plustin '%s' (%s) over %x", (id ? id->s_name : "default"),
		 (glname ? glname->s_name : "<anonymous>"),
		 (int)tin->tin_interp);
}
#endif

typedef struct _plusword
{
    int         pw_type;
    Tcl_Obj    *pw_ob;
    Tcl_Token  *pw_ndxv;  /* index part of this word (if array variable) */
    int         pw_ndxc;  /* numComponents of the above */
} t_plusword;

#define PLUSTOT_MAXINLETS   256  /* LATER rethink */
#define PLUSTOT_INIMAXWORDS  16

/* LATER elaborate */
#define PLUSTOT_ERRUNKNOWN  -1
#define PLUSTOT_ERROTHER    -2

struct _plustot
{
    t_plusobject   x_plusobject;
    t_plustob     *x_tob;        /* interpreter's result (after invocation) */
    t_scriptlet   *x_script;
    Tcl_Obj       *x_cname;      /* command name, main validation flag */
    Tcl_CmdInfo    x_cinfo;
    t_plusstring  *x_ctail;      /* command arguments, parse validation flag */
    Tcl_Parse      x_tailparse;
    int            x_maxwords;   /* as allocated */
    int            x_nwords;     /* as used, including command name */
    t_plusword    *x_words;      /* arguments, not evaluated */
    t_plusword     x_wordsini[PLUSTOT_INIMAXWORDS];
    int            x_maxargs;    /* == maxwords, except during growing */
    int            x_argc;       /* 0 or nwords, except during evaluation */
    Tcl_Obj      **x_argv;       /* command name and evaluated arguments */
    Tcl_Obj       *x_argvini[PLUSTOT_INIMAXWORDS];
    int            x_pseudoscalar;
    int            x_nproxies;
    t_plusproxy  **x_proxies;
    t_plusproxy   *x_mainproxy;  /* == x_proxies[0] or null if 1st slot deaf */
    t_plusproxy   *x_deafproxy;  /* dummy/x_proxies[0] if deaf, else null */
    int            x_grabwarned;
    int            x_isloud;
};

static t_class *plustot_class;

static void plustot_tclerror(t_plustot *x, Tcl_Interp *interp, char *msg)
{
    if (x->x_isloud)
	plusloud_tclerror((t_pd *)x, interp, msg);
}

/* First pass (!doit): determine number of slots.
   Second pass (doit): create variables for listening slots. */
static int plustot_usevariable(t_plustot *x, Tcl_Token *tp, int doit)
{
    int nc = tp->numComponents;
    char *errmess = 0;
    int errcode = PLUSTOT_ERRUNKNOWN;
#ifdef PLUSTOT_DEBUG
    if (!doit)
    {
	char buf[MAXPDSTRING];
	int size = tp->size;
	if (size > (MAXPDSTRING-2))
	{
	    size = (MAXPDSTRING-2);
	    buf[MAXPDSTRING-2] = '*';
	    buf[MAXPDSTRING-1] = 0;
	}
	else buf[size] = 0;
	strncpy(buf, tp->start, size);
	loudbug_startpost("%s ", buf);
    }
#endif
    tp++;
    if (nc && tp->type == TCL_TOKEN_TEXT)
    {
	int ishot = 0, iscold = 0, istransient = 0;
	if (strncmp(tp->start, "Hin", tp->size) == 0)
	    ishot = 1;
	else if (strncmp(tp->start, "Cin", tp->size) == 0)
	    iscold = 1;
	else if (strncmp(tp->start, "Tin", tp->size) == 0)
	    istransient = ishot = 1;
	if (!ishot && !iscold && strncmp(tp->start, "in", tp->size))
	{
	    /* regular variable */
	    /* LATER it should be write-traced (2nd pass, but only if there are
	       pull inputs) in order to know when the object becomes stale */
	}
	else
	{
	    /* pseudo-variable */
	    int inno = -1;
	    tp++;
	    if (nc == 1)
	    {
		if (x->x_nproxies && !x->x_pseudoscalar)
		{
		    errmess = "mixed scalar and array forms of pseudo-variable";
		    errcode = PLUSTOT_ERROTHER;
		    goto badvariable;
		}
		inno = 0;
		x->x_pseudoscalar = 1;
	    }
	    else if (nc == 2 && tp->type == TCL_TOKEN_TEXT)
	    {
		int i;
		char *p;
		if (x->x_pseudoscalar)
		{
		    errmess = "mixed scalar and array forms of pseudo-variable";
		    errcode = PLUSTOT_ERROTHER;
		    goto badvariable;
		}
		inno = 0;
		for (i = 0, p = (char *)tp->start; i < tp->size; i++, p++)
		{
		    if (*p < '0' || *p > '9')
		    {
			errmess = "invalid inlet number in pseudo-variable";
			errcode = PLUSTOT_ERROTHER;
			goto badvariable;
		    }
		    inno = inno * 10 + (int)(*p - '0');
		}
		if (inno > PLUSTOT_MAXINLETS)
		{
		    errmess = "inlet number too large in pseudo-variable";
		    errcode = PLUSTOT_ERROTHER;
		    goto badvariable;
		}
	    }
	    else
	    {
		errmess = "invalid index format in pseudo-variable";
		errcode = PLUSTOT_ERROTHER;
		goto badvariable;
	    }
	    if (inno >= 0)
	    {
		if (!doit)
		{
#ifdef PLUSTOT_DEBUG
		    loudbug_startpost("(inlet %d) ", inno);
#endif
		    if (inno >= x->x_nproxies)
			x->x_nproxies = inno + 1;
		}
		else if (inno < x->x_nproxies)
		{
		    t_plusproxy *pp = x->x_proxies[inno];
		    if (!pp->pp_var)
		    {
			t_plusvar *var;
			char buf[8], *ptr;
			if (x->x_pseudoscalar)
			    ptr = 0;
			else
			    sprintf(ptr = buf, "%d", inno);
			if (istransient)
			{
			    pp->pp_istransient = pp->pp_ishot = 1;
			    var = plusvar_new("Tin", ptr, x->x_tob->tob_tin);
			}
			else if (ishot)
			{
			    pp->pp_ishot = 1;
			    var = plusvar_new("Hin", ptr, x->x_tob->tob_tin);
			}
			else if (iscold)
			{
			    pp->pp_ishot = 0;
			    var = plusvar_new("Cin", ptr, x->x_tob->tob_tin);
			}
			/* keep defaults, as set in plustot_makeproxies(): */
			else var = plusvar_new("in", ptr, x->x_tob->tob_tin);
			plusbob_preserve((t_plusbob *)var);
			plusbob_setowner((t_plusbob *)var, (t_pd *)x);
			pp->pp_var = var;
		    }
		}
		else
		{
		    PLUSDEBUG_ENDPOST("plustot_usevariable");
		    loudbug_bug("plustot_usevariable");
		    goto badvariable;
		}
	    }
	    else
	    {
		errmess = "invalid pseudo-variable";
		errcode = PLUSTOT_ERROTHER;
		goto badvariable;
	    }
	}
	return (1);
    }
    else plusloud_tcldirty((t_pd *)x, "plustot_usevariable");
badvariable:
    if (errmess)
    {
	PLUSDEBUG_ENDPOST("plustot_usevariable");
	loud_error((t_pd *)x, errmess);
    }
    return (errcode);
}

static int plustot_doparsevariables(t_plustot *x, Tcl_Interp *interp,
				    const char *buf, int len,
				    Tcl_Parse *parsep, int doit)
{
    int nvars = 0;
    int errcode = PLUSTOT_ERRUNKNOWN;
    if (Tcl_ParseCommand(interp, buf, len, 0, parsep) == TCL_OK)
    {
	int ntok = parsep->numTokens;
	Tcl_Token *tp = parsep->tokenPtr;
	while (ntok--)
	{
	    if (tp->type == TCL_TOKEN_VARIABLE)
	    {
		int res = plustot_usevariable(x, tp, doit);
		if (res > 0)
		    nvars++;
		else
		{
		    errcode = res;
		    goto parsefailed;
		}
	    }
	    else if (tp->type == TCL_TOKEN_COMMAND)
	    {
		if (tp->size > 2)
		{
		    Tcl_Parse parse;
		    int res =
			plustot_doparsevariables(x, interp, tp->start + 1,
						 tp->size - 2, &parse, doit);
		    if (res != PLUSTOT_ERRUNKNOWN)
			Tcl_FreeParse(&parse);
		    if (res >= 0)
			nvars += res;
		    else
		    {
			errcode = res;
			goto parsefailed;
		    }
		}
	    }
	    else if (tp->type == TCL_TOKEN_SIMPLE_WORD
		     && tp->size > 2 && *tp->start == '{')
	    {
		tp++;
#if 0 && defined(PLUSTOT_DEBUG)
		if (doit && tp->size > 0)
		{
		    char buf[MAXPDSTRING+1];
		    int sz = (tp->size < MAXPDSTRING ? tp->size : MAXPDSTRING);
		    strncpy(buf, tp->start, sz);
		    buf[sz] = 0;
		    loudbug_post("simple word's text:  %s", buf);
		}
#endif
		if (ntok-- && tp->type == TCL_TOKEN_TEXT && tp->size > 0)
		{
		    Tcl_Parse parse;
		    int res =
			plustot_doparsevariables(x, interp, tp->start,
						 tp->size, &parse, doit);
		    if (res != PLUSTOT_ERRUNKNOWN)
			Tcl_FreeParse(&parse);
		    if (res >= 0)
			nvars += res;
		    else
		    {
			errcode = res;
			goto parsefailed;
		    }
		}
		else
		{
		    plusloud_tcldirty((t_pd *)x, "plustot_doparsevariables");
		    goto parsefailed;
		}
	    }
#if 0 && defined(PLUSTOT_DEBUG)
	    else if (doit && tp->size > 0)
	    {
		char buf[MAXPDSTRING+1];
		int sz = (tp->size < MAXPDSTRING ? tp->size : MAXPDSTRING);
		strncpy(buf, tp->start, sz);
		buf[sz] = 0;
		loudbug_post("other type (%d):  %s", tp->type, buf);
	    }
#endif
	    tp++;
	}
    }
    else goto parsefailed;
    return (nvars);
parsefailed:
    return (errcode);
}

static int plustot_parsevariables(t_plustot *x, Tcl_Interp *interp,
				  const char *buf, int len,
				  Tcl_Parse *parsep, int doit)
{
    int nvars;
#ifdef PLUSTOT_DEBUG
    if (!doit) loudbug_startpost("variables: ");
#endif
    nvars = plustot_doparsevariables(x, interp, buf, len, parsep, doit);
#ifdef PLUSTOT_DEBUG
    if (!doit)
    {
	if (nvars > 0)
	{
	    loudbug_post("\n%d variable substitutions", nvars);
	    loudbug_post("%d inlets requested", x->x_nproxies);
	}
	else if (nvars == 0) loudbug_post("none");
    }
#endif
    return (nvars);
}

static int plustot_makeproxies(t_plustot *x)
{
    Tcl_Interp *interp = x->x_tob->tob_tin->tin_interp;
    if (interp)
    {
	if (x->x_nproxies == 0)
	{
	    x->x_proxies = 0;
	    x->x_mainproxy = 0;
	    x->x_deafproxy = plusproxy_new(x, -2, 0, 0, x->x_tob->tob_tin);
	}
	else if (x->x_nproxies == 1
		 || (x->x_nproxies > 1 && !x->x_pseudoscalar))
	{
	    if (x->x_proxies = getbytes(x->x_nproxies * sizeof(*x->x_proxies)))
	    {
		int i;
		x->x_proxies[0] =
		    plusproxy_new(x, (x->x_pseudoscalar ? -1 : 0), 1, 0,
				  x->x_tob->tob_tin);
		for (i = 1; i < x->x_nproxies; i++)
		{
		    x->x_proxies[i] =
			plusproxy_new(x, i, 0, 0, x->x_tob->tob_tin);
		    plusinlet_new(&x->x_plusobject,
				  (t_pd *)x->x_proxies[i], 0, 0);
		}
		/* second pass: traverse listening slots, create variables */
		plustot_parsevariables(x, interp,
				       x->x_ctail->ps_buf, x->x_ctail->ps_len,
				       &x->x_tailparse, 1);
		if (x->x_proxies[0]->pp_var)
		{
		    x->x_mainproxy = x->x_proxies[0];
		    x->x_deafproxy = 0;
		}
		else
		{
		    x->x_mainproxy = 0;
		    x->x_deafproxy = x->x_proxies[0];
		}
	    }
	    else goto proxiesfailed;
	}
	else
	{
	    loudbug_bug("plustot_makeproxies");
	    goto proxiesfailed;
	}
	return (1);
    }
proxiesfailed:
    return (0);
}

static void plustot_initwords(t_plustot *x)
{
    if (x->x_words != x->x_wordsini)
	freebytes(x->x_words, x->x_maxwords * sizeof(*x->x_words));
    x->x_maxwords = PLUSTOT_INIMAXWORDS;
    x->x_nwords = 0;
    x->x_words = x->x_wordsini;
}

static void plustot_initargs(t_plustot *x)
{
    if (x->x_argv != x->x_argvini)
	freebytes(x->x_argv, x->x_maxargs * sizeof(*x->x_argv));
    x->x_maxargs = PLUSTOT_INIMAXWORDS;
    x->x_argc = 0;
    x->x_argv = x->x_argvini;
    x->x_argv[0] = x->x_cname;
}

static int plustot_resetwords(t_plustot *x)
{
    int i;
    for (i = 1; i < x->x_nwords; i++)
	PLUSDEBUG_DECRREFCOUNT(x->x_words[i].pw_ob, "plustot_resetwords");
    x->x_nwords = 0;
    if (x->x_ctail)  /* does object command exist && is parse valid? */
    {
	int nwords = x->x_tailparse.numWords + 1;
	if (nwords > x->x_maxwords)
	{
	    int n = nwords;
#ifdef PLUSTOT_DEBUG
	    loudbug_post("growing words %d -> %d", x->x_maxwords, nwords);
#endif
	    x->x_words = grow_nodata(&n, &x->x_maxwords, x->x_words,
				     PLUSTOT_INIMAXWORDS, x->x_wordsini,
				     sizeof(*x->x_words));
	    if (n != nwords)
		return (0);
	}
	return (1);
    }
    else return (0);
}

static int plustot_resetargs(t_plustot *x)
{
    int i;
    for (i = 1; i < x->x_argc; i++)
	PLUSDEBUG_DECRREFCOUNT(x->x_argv[i], "plustot_resetargs");
    x->x_argc = 0;
    x->x_argv[0] = x->x_cname;
    if (x->x_ctail)  /* does object command exist && is parse valid? */
    {
	int nargs = x->x_maxwords;
	if (nargs > x->x_maxargs)
	{
	    int n = nargs;
#ifdef PLUSTOT_DEBUG
	    loudbug_post("growing argv %d -> %d", x->x_maxargs, nargs);
#endif
	    x->x_argv = grow_nodata(&n, &x->x_maxargs, x->x_argv,
				    PLUSTOT_INIMAXWORDS, x->x_argvini,
				    sizeof(*x->x_argv));
	    x->x_argv[0] = x->x_cname;
	    if (n != nargs)
	    {
		plustot_initwords(x);
		plustot_initargs(x);
		return (0);
	    }
	}
	else if (nargs < x->x_maxargs)
	{
	    loudbug_bug("plustot_resetargs");  /* LATER rethink */
	    plustot_initwords(x);
	    plustot_initargs(x);
	    return (0);
	}
	return (1);
    }
    else return (0);
}

static int plustot_makewords(t_plustot *x)
{
    if (plustot_resetwords(x))
    {
	int i, ncomponents = 0, nwords = x->x_tailparse.numWords + 1;
	Tcl_Token *tp;
	int len;
	char buf[TCL_UTF_MAX];
#ifdef PLUSTOT_DEBUG
	loudbug_post("arguments:");
#endif
	for (i = 1, tp = x->x_tailparse.tokenPtr;
	     i < nwords; i++, tp += ncomponents)
	{
#ifdef PLUSTOT_DEBUG
	    loudbug_post("  %s token: type %d[%d], having %d[%d] component%s",
			 loud_ordinal(i), tp->type, tp[1].type,
			 tp->numComponents, tp[1].numComponents,
			 (tp->numComponents > 1 ? "s" : ""));
#endif
	    ncomponents = tp->numComponents;
	    tp++;
	    switch (x->x_words[i].pw_type = tp->type)
	    {
	    case TCL_TOKEN_TEXT:
		x->x_words[i].pw_ob = Tcl_NewStringObj(tp->start, tp->size);
		break;

	    case TCL_TOKEN_BS:
		len = Tcl_UtfBackslash(tp->start, 0, buf);
		x->x_words[i].pw_ob = Tcl_NewStringObj(buf, len);
		break;

	    case TCL_TOKEN_COMMAND:
		x->x_words[i].pw_ob = Tcl_NewStringObj(tp->start + 1,
						       tp->size - 2);
		break;

	    case TCL_TOKEN_VARIABLE:
		if (tp->numComponents > 1)
		{
		    x->x_words[i].pw_ndxv = tp + 2;
		    x->x_words[i].pw_ndxc = tp->numComponents - 1;
		}
		else x->x_words[i].pw_ndxv = 0;
		x->x_words[i].pw_ob = Tcl_NewStringObj(tp[1].start, tp[1].size);
		break;

	    default:
		plusloud_tcldirty((t_pd *)x,
				  "plustot_makewords (unexpected token type)");
		goto wordsfailed;
	    }
	    PLUSDEBUG_INCRREFCOUNT(x->x_words[i].pw_ob, "plustot_makewords");
	}
	x->x_nwords = nwords;
	return (1);
wordsfailed:
	x->x_nwords = i;
	plustot_resetwords(x);
    }
    return (0);
}

static int plustot_argsfromwords(t_plustot *x, Tcl_Interp *interp)
{
    if (plustot_resetargs(x))
    {
	t_plusword *pw;
	int i;
	for (i = 1, pw = &x->x_words[1]; i < x->x_nwords; i++, pw++)
	{
	    int result;
	    if (pw->pw_type == TCL_TOKEN_COMMAND)
	    {
		result = Tcl_EvalObjEx(interp, pw->pw_ob, 0);
		if (result == TCL_OK)
		{
		    if (x->x_argv[i] = Tcl_GetObjResult(interp))
		    {
			PLUSDEBUG_INCRREFCOUNT(x->x_argv[i],
					       "plustot_argsfromwords");
			Tcl_ResetResult(interp);
		    }
		    else
		    {
			plusloud_tcldirty((t_pd *)x, "plustot_argsfromwords");
			goto evalfailed;
		    }
		}
		else
		{
		    plustot_tclerror(x, interp, "bad word (command)");
		    goto evalfailed;
		}
	    }
	    else if (pw->pw_type == TCL_TOKEN_VARIABLE)
	    {
		Tcl_Obj *indexp;
		if (x->x_words[i].pw_ndxv)
		{
		    /* FIXME */
		    int res = Tcl_EvalTokensStandard(interp,
						     x->x_words[i].pw_ndxv,
						     x->x_words[i].pw_ndxc);
		    if (res == TCL_OK)
		    {
			indexp = Tcl_GetObjResult(interp);
			PLUSDEBUG_INCRREFCOUNT(indexp,
					       "plustot_argsfromwords");
			Tcl_ResetResult(interp);
		    }
		    else
		    {
			plustot_tclerror(x, interp, "bad index");
			goto evalfailed;
		    }
		}
		else indexp = 0;
		if (x->x_argv[i] = Tcl_ObjGetVar2(interp, pw->pw_ob, indexp,
						  TCL_LEAVE_ERR_MSG))
		{
		    PLUSDEBUG_INCRREFCOUNT(x->x_argv[i],
					   "plustot_argsfromwords");
		    Tcl_ResetResult(interp);
		}
		else
		{
		    plustot_tclerror(x, interp, "bad word (variable)");
		    goto evalfailed;
		}
	    }
	    else
	    {
		x->x_argv[i] = pw->pw_ob;
		/* refcount is 1 already (makewords), but we need to comply to
		   a general rule: args are decremented after use (resetargs) */
		PLUSDEBUG_INCRREFCOUNT(x->x_argv[i], "plustot_argsfromwords");
	    }
	}
	x->x_argc = x->x_nwords;
	return (1);
evalfailed:
	x->x_argc = i;
	plustot_resetargs(x);
    }
    return (0);  /* LATER find a proper way for passing a result */
}

static int plustot_argsfromtokens(t_plustot *x, Tcl_Interp *interp)
{
    if (plustot_resetargs(x))
    {
	int i, nwords = x->x_tailparse.numWords + 1;
	Tcl_Token *tp;
#ifdef PLUSTOT_DEBUG
	loudbug_post("arguments:");
#endif
	for (i = 1, tp = x->x_tailparse.tokenPtr;
	     i < nwords; i++, tp += (tp->numComponents + 1))
	{
	    int result;
#ifdef PLUSTOT_DEBUG
	    loudbug_startpost("  %s token: type %d[%d], having %d component%s",
			      loud_ordinal(i), tp->type, tp[1].type,
			      tp->numComponents,
			      (tp->numComponents > 1 ? "s" : ""));
#endif
	    result = Tcl_EvalTokensStandard(interp, tp + 1, tp->numComponents);
	    if (result == TCL_OK)
	    {
		if (x->x_argv[i] = Tcl_GetObjResult(interp))
		{
		    PLUSDEBUG_INCRREFCOUNT(x->x_argv[i],
					   "plustot_argsfromwords");
		    Tcl_ResetResult(interp);
#ifdef PLUSTOT_DEBUG
		    loudbug_post(", %sshared: '%s'",
				 (Tcl_IsShared(x->x_argv[i]) ? "" : "not "),
				 Tcl_GetString(x->x_argv[i]));
#endif
		}
		else
		{
		    PLUSDEBUG_ENDPOST("plustot_argsfromtokens");
		    plusloud_tcldirty((t_pd *)x, "plustot_argsfromtokens");
		}
	    }
	    else
	    {
		PLUSDEBUG_ENDPOST("plustot_argsfromtokens");
		plustot_tclerror(x, interp, "bad token");
		while (--i)
		    PLUSDEBUG_DECRREFCOUNT(x->x_argv[i],
					   "plustot_argsfromtokens");
		return (0);  /* LATER find a proper way for passing a result */
	    }
	}
	x->x_argc = nwords;
	return (1);
    }
    else return (0);
}

/* not used yet */
static int plustot_ifgrabshared(t_plustot *x, Tcl_Obj *ob)
{
    if (!x->x_grabwarned)
    {
	x->x_grabwarned = 1;
	loud_warning((t_pd *)x, 0, "shared result of a command '%s'",
		     (x->x_cname ? Tcl_GetString(x->x_cname) : "???"));
    }
    return (1);
}

static int plustot_push(t_plustot *x)
{
    if (x->x_proxies)
    {
	int i;
	for (i = 0; i < x->x_nproxies; i++)
	    if (x->x_proxies[i]->pp_var)
		if (!plusvar_push(x->x_proxies[i]->pp_var))
		    return (0);
    }
    return (1);
}

static void plustot_cleartransients(t_plustot *x)
{
    if (x->x_proxies)
    {
	int i;
	for (i = 0; i < x->x_nproxies; i++)
	    if (x->x_proxies[i]->pp_var && x->x_proxies[i]->pp_istransient)
		plusvar_clear(x->x_proxies[i]->pp_var, 1);
    }
}

/* This is the seed of it all:  if sendit == 1, this routine executes
   a full firing step, otherwise, it performs a plain evaluation. */
static int plustot_doit(t_plustot *x, int sendit)
{
    int result = 0;
    Tcl_Interp *interp = x->x_tob->tob_tin->tin_interp;
    if (x->x_cname && plustot_push(x) &&
	plustot_argsfromwords(x, interp))
    {
	if ((*x->x_cinfo.objProc)(x->x_cinfo.objClientData, interp,
				  x->x_argc, x->x_argv) == TCL_OK)
	{
	    if (plustob_grabresult(x->x_tob))
		result = 1;
	}
	else plustot_tclerror(x, interp, "command failed");
	/* Although args are to be reset in the next call to
	   plustot_argsfromwords(), however, plusvar_preset() will be called
	   first, so, unless reset is done here, $ins would be shared there.
	   LATER rethink. */
	plustot_resetargs(x);
	plustot_cleartransients(x);
    }
    if (result && sendit)
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tob);
    return (result);
}

static void plustot_eval(t_plustot *x)
{
    plustot_doit(x, 0);
}

static void plustot_get(t_plustot *x)
{
    if (x->x_tob->tob_value)
	outlet_plusbob(((t_object *)x)->ob_outlet, (t_plusbob *)x->x_tob);
}

/* set in(0), no evaluation */
static void plustot_set(t_plustot *x, t_symbol *s, int ac, t_atom *av)
{
    if (x->x_mainproxy)
	plusproxy_set(x->x_mainproxy, s, ac, av);
    else if (x->x_deafproxy)
	plusproxy_deafhit(x->x_deafproxy);
}

static void plustot_clear(t_plustot *x)
{
    if (x->x_mainproxy)
	plusproxy_clear(x->x_mainproxy);
}

static void plustot_clearall(t_plustot *x)
{
    if (x->x_proxies)
    {
	int i;
	for (i = 0; i < x->x_nproxies; i++)
	    if (x->x_proxies[i]->pp_var)
		plusvar_clear(x->x_proxies[i]->pp_var, 1);
    }
}

static void plustot_bang(t_plustot *x)
{
    if (x->x_mainproxy)
	plusproxy_bang(x->x_mainproxy);
    else
	plustot_doit(x, 1);
}

static void plustot_float(t_plustot *x, t_float f)
{
    if (x->x_mainproxy)
	plusproxy_float(x->x_mainproxy, f);
    else if (x->x_deafproxy)
	plusproxy_deafhit(x->x_deafproxy);
}

static void plustot_symbol(t_plustot *x, t_symbol *s)
{
    if (x->x_mainproxy)
	plusproxy_symbol(x->x_mainproxy, s);
    else if (x->x_deafproxy)
	plusproxy_deafhit(x->x_deafproxy);
}

static void plustot_list(t_plustot *x, t_symbol *s, int ac, t_atom *av)
{
    if (x->x_mainproxy)
	plusproxy_list(x->x_mainproxy, s, ac, av);
    else if (x->x_deafproxy)
	plusproxy_deafhit(x->x_deafproxy);
}

static void plustot_tot(t_plustot *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	Tcl_Obj *ob;
	char *start;
	int len;
	scriptlet_reset(x->x_script);
	scriptlet_add(x->x_script, 1, 1, ac, av);
	start = scriptlet_getcontents(x->x_script, &len);
	if (len > 0 && (ob = Tcl_NewStringObj(start, len)))
	{
	    /* LATER set a persistent ob, rather than create a new one */
	    PLUSDEBUG_INCRREFCOUNT(ob, "plustot_tot");
	    if (plustob_evalob(x->x_tob, ob) && s == totps_query)
		outlet_plusbob(((t_object *)x)->ob_outlet,
			       (t_plusbob *)x->x_tob);
	    PLUSDEBUG_DECRREFCOUNT(ob, "plustot_tot");
	}
    }
}

static void plustot_save(t_gobj *z, t_binbuf *bb)
{
    t_text *t = (t_text *)z;
    t_binbuf *inbb = t->te_binbuf;
    int ac = binbuf_getnatom(inbb);
    t_atom *av = binbuf_getvec(inbb);
    binbuf_addv(bb, "ssii", gensym("#X"), gensym("obj"),
                (int)t->te_xpix, (int)t->te_ypix);
    if (ac && av->a_type == A_SYMBOL)
    {
	t_symbol *s = av->a_w.w_symbol;
	if (s != totps_plustot)
	{
	    t_atom at;
	    SETSYMBOL(&at, totps_plustot);
	    binbuf_add(bb, 1, &at);
	    if (s == plusps_tot && ac > 1)
	    {
		inbb = binbuf_new();
		binbuf_add(inbb, ac - 1, av + 1);
	    }
	}
    }
    else loudbug_bug("plustot_save");
    binbuf_addbinbuf(bb, inbb);
    binbuf_addsemi(bb);
    if (inbb != t->te_binbuf)
	binbuf_free(inbb);
}

#ifdef PLUSTOT_DEBUG
static void plustot_debug(t_plustot *x)
{
    t_plusobject *po = (t_plusobject *)x;
    t_plustin *tin = x->x_tob->tob_tin;
    t_symbol *id = plusenv_getid((t_plusenv *)tin);
    t_symbol *glname = plustin_getglistname(tin);
    loudbug_post("+tot, glist %x", (int)po->po_glist);
    loudbug_post("  plustin '%s' (%s) over %x", (id ? id->s_name : "default"),
		 (glname ? glname->s_name : "<anonymous>"),
		 (int)tin->tin_interp);
    if (x->x_mainproxy)
	plusproxy_debug(x->x_mainproxy);
}
#endif

static void plustot_free(t_plustot *x)
{
    int i;
    plusbob_release((t_plusbob *)x->x_tob);
    if (x->x_cname) PLUSDEBUG_DECRREFCOUNT(x->x_cname, "plustot_free");
    if (x->x_ctail)  /* does object command exist && is parse valid? */
    {
	for (i = 1; i < x->x_nwords; i++)
	    PLUSDEBUG_DECRREFCOUNT(x->x_words[i].pw_ob, "plustot_free");
	for (i = 1; i < x->x_argc; i++)
	    PLUSDEBUG_DECRREFCOUNT(x->x_argv[i], "plustot_free");
	if (x->x_words != x->x_wordsini)
	    freebytes(x->x_words, x->x_maxwords * sizeof(*x->x_words));
	if (x->x_argv != x->x_argvini)
	    freebytes(x->x_argv, x->x_maxwords * sizeof(*x->x_argv));
	Tcl_FreeParse(&x->x_tailparse);
	plusstring_release(x->x_ctail);
    }
    if (x->x_proxies)
    {
	for (i = 0; i < x->x_nproxies; i++)
	    pd_free((t_pd *)x->x_proxies[i]);
	freebytes(x->x_proxies, x->x_nproxies * sizeof(*x->x_proxies));
    }
    else if (x->x_deafproxy)
	pd_free((t_pd *)x->x_deafproxy);
    if (x->x_script) scriptlet_free(x->x_script);
    plusobject_free(&x->x_plusobject);
}

static void *plustot_new(t_symbol *s, int ac, t_atom *av)
{
    t_plustot *x = 0;
    t_plusstring *ctail = 0;
    t_symbol *cmdname = 0;  /* command name or +selector */
    t_glist *glist = canvas_getcurrent();
    t_plustin *tin = 0;
    t_plustob *tob = 0;
    t_scriptlet *script = scriptlet_new(0, 0, 0, 0, glist, 0);
    if (s != plusps_tot && s != totps_plustot && s != &s_)
	cmdname = s;
    else if (ac && av->a_type == A_SYMBOL)
    {
	cmdname = av->a_w.w_symbol;
	ac--; av++;
    }
    if (cmdname)
    {
	if (*cmdname->s_name == '+')
	{
	    if (cmdname == plusps_ar)
		return (plustot_ar_new(cmdname, ac, av));
	    else if (cmdname == plusps_env)
		return (plustot_env_new(cmdname, ac, av));
	    else if (cmdname == plusps_in)
		return (plustot_in_new(cmdname, ac, av));
	    else if (cmdname == plusps_var)
		return (plustot_var_new(cmdname, ac, av));
	    else if (cmdname == plusps_out)
		return (plustot_out_new(cmdname, ac, av));
	    else if (cmdname == plusps_qlist)
		return (plustot_qlist_new(cmdname, ac, av));
	    else if (cmdname == plusps_print)
		return (plustot_print_new(cmdname, ac, av));
	    else
	    {
		loud_error(0, "unknown +tot's subclass");
		return (0);
	    }
	}
	/* If ac == 0, ctail is an empty plusstring, but not null.  We rely
	   on getbytes(0), copybytes(x, 0), and freebytes(x, 0) being safe.
	   LATER reconsider using a separate parse validation flag, while
	   moving tests for a null ctail to where they really belong. */
	ctail = plusstring_fromatoms(0, ac, av, script);
	plusstring_preserve(ctail);
    }
    if ((tin = plustin_glistprovide(glist, PLUSTIN_GLIST_ANY, 0)) &&
	(tob = plustob_new(tin, 0)))
    {
	t_plusstring *vistext = plusstring_fromatoms(cmdname, ac, av, script);
	plusstring_preserve(vistext);
	x = (t_plustot *)
	    plusobject_new(plustot_class, cmdname, ac, av, vistext);
	plusstring_release(vistext);
	x->x_isloud = 1;
	/* tin already preserved (plustob_new() did it) */
	plusbob_preserve((t_plusbob *)tob);
	plusbob_setowner((t_plusbob *)tob, (t_pd *)x);
	x->x_tob = tob;
	scriptlet_setowner(script, (t_pd *)x);
	x->x_script = script;
	x->x_cname = 0;
	x->x_ctail = 0;
	x->x_words = x->x_wordsini;
	plustot_initwords(x);
	x->x_argv = x->x_argvini;
	plustot_initargs(x);
	x->x_pseudoscalar = 0;
	x->x_nproxies = 0;
	x->x_proxies = 0;
	x->x_mainproxy = 0;
	x->x_deafproxy = 0;
	x->x_grabwarned = 0;
	if (cmdname && *cmdname->s_name)
	{
	    Tcl_Interp *interp = tin->tin_interp;
	    if (interp)
	    {
		if (Tcl_GetCommandInfo(interp, cmdname->s_name, &x->x_cinfo))
		{
		    if (x->x_cinfo.isNativeObjectProc)
		    {
			x->x_cname = Tcl_NewStringObj(cmdname->s_name, -1);
			PLUSDEBUG_INCRREFCOUNT(x->x_cname, "plustot_new");
			x->x_argv[0] = x->x_cname;
		    }
		    else loud_error((t_pd *)x, "'%s' is not an object command",
				    cmdname->s_name);
		}
		else loud_error((t_pd *)x, "command '%s' does not exist",
				cmdname->s_name);
		if (x->x_cname && ctail)
		{  /* object command exists, now parse the arguments: */
		    /* 1. do syntax validation and locate pseudo-variables */
		    int nvars =
			plustot_parsevariables(x, interp,
					       ctail->ps_buf, ctail->ps_len,
					       &x->x_tailparse, 0);
		    if (nvars >= 0)
		    {
			int res = 1;
			x->x_ctail = ctail;
			/* 2. create input slots */
			res = plustot_makeproxies(x);
			if (res)
			    /* 3. shallow objectifying: create a Tcl_Obj for
			       each argument; subcommand arguments will be
			       compiled to bytecode during first evaluation --
			       either below, or when the +tot object fires. */
			    res = plustot_makewords(x);
			if (res)
			{
			    /* FIXME [+tot +ar pname] */
			    int n = plustin_procargc(tin, cmdname->s_name);
			    loudbug_post("plustin_procargc: %d", n);

			    /* creation time evaluation, LATER rethink:
			       should this be immediate or scheduled? */
			    x->x_isloud = 0;
			    plustot_doit(x, 0);
			    x->x_isloud = 1;
			}
			else
			{
			    /* here we invalidate parse, but leave the command
			       valid, LATER revisit */
			    x->x_ctail = 0;
			}
			Tcl_FreeParse(&x->x_tailparse);
		    }
		    else
		    {
			if (nvars == PLUSTOT_ERRUNKNOWN)
			    plustot_tclerror(x, interp,
					    "parsing command arguments failed");
			else
			    Tcl_FreeParse(&x->x_tailparse);
			PLUSDEBUG_DECRREFCOUNT(x->x_cname, "plustot_new");
			x->x_cname = 0;
		    }
		}
	    }
	}
	plusoutlet_new(&x->x_plusobject, &s_symbol);
    }
    else
    {
	loud_error(0, "+tot: cannot initialize");
	if (tin)
	{
	    plusbob_preserve((t_plusbob *)tin);
	    plusbob_release((t_plusbob *)tin);
	}
	if (script) scriptlet_free(script);
    }
    if (ctail && !(x && x->x_ctail))
	plusstring_release(ctail);
    return (x);
}

void plusobject_widgetfree(t_plusobject *po);
void plusobject_widgetcreate(t_plusobject *po, t_symbol *s, int ac, t_atom *av,
			     t_plusstring *ps);
void plusclass_widgetsetup(t_class *c);

void plusobject_free(t_plusobject *po)
{
    plusobject_widgetfree(po);
}

t_plusobject *plusobject_new(t_class *c, t_symbol *s, int ac, t_atom *av,
			     t_plusstring *ps)
{
    t_plusobject *po = (t_plusobject *)pd_new(c);
    po->po_glist = canvas_getcurrent();
    po->po_ninlets = 1;
    po->po_noutlets = 0;
    plusobject_widgetcreate(po, s, ac, av, ps);
    return (po);
}

t_inlet *plusinlet_new(t_plusobject *po, t_pd *dest,
		       t_symbol *s1, t_symbol *s2)
{
    po->po_ninlets++;
    return (inlet_new((t_object *)po, dest, s1, s2));
}

t_outlet *plusoutlet_new(t_plusobject *po, t_symbol *s)
{
    po->po_noutlets++;
    return (outlet_new((t_object *)po, s));
}

void plusclass_inherit(t_class *c, t_symbol *s)
{
    class_addcreator((t_newmethod)plustot_new, s, A_GIMME, 0);
    forky_setsavefn(c, plustot_save);
    plusclass_widgetsetup(c);
}

void plustot_setup(void)
{
    post("beware! this is plustot %s, %s %s build...",
	 TOXY_VERSION, loud_ordinal(TOXY_BUILD), TOXY_RELEASE);
    plussymbols_create();

    plustot_class = class_new(totps_plustot,
			      (t_newmethod)plustot_new,
			      (t_method)plustot_free,
			      sizeof(t_plustot), 0, A_GIMME, 0);
    plusclass_inherit(plustot_class, plusps_tot);

    class_addbang(plustot_class, plustot_bang);
    class_addfloat(plustot_class, plustot_float);
    class_addsymbol(plustot_class, plustot_symbol);
    class_addlist(plustot_class, plustot_list);
    class_addmethod(plustot_class, (t_method)plustot_eval,
		    gensym("eval"), 0);
    class_addmethod(plustot_class, (t_method)plustot_clear,
		    gensym("clear"), 0);
    class_addmethod(plustot_class, (t_method)plustot_clearall,
		    gensym("clearall"), 0);
    class_addmethod(plustot_class, (t_method)plustot_set,
		    gensym("set"), A_GIMME, 0);
    class_addmethod(plustot_class, (t_method)plustot_get,
		    gensym("get"), 0);
    class_addmethod(plustot_class, (t_method)plustot_tot,
		    gensym("tot"), A_GIMME, 0);
    class_addmethod(plustot_class, (t_method)plustot_tot,
		    gensym("query"), A_GIMME, 0);

    plusproxy_class = class_new(gensym("+tot proxy"), 0,
				(t_method)plusproxy_free,
				sizeof(t_plusproxy), CLASS_PD, 0);
    class_addfloat(plusproxy_class, plusproxy_float);
    class_addsymbol(plusproxy_class, plusproxy_symbol);
    class_addlist(plusproxy_class, plusproxy_list);
    class_addmethod(plusproxy_class, (t_method)plusproxy_clear,
		    gensym("clear"), 0);
    class_addmethod(plusproxy_class, (t_method)plusproxy_set,
		    gensym("set"), A_GIMME, 0);

#ifdef PLUSTOT_DEBUG
    class_addmethod(plustot_class, (t_method)plustot_debug,
		    gensym("debug"), 0);
    class_addmethod(plusproxy_class, (t_method)plusproxy_debug,
		    gensym("debug"), 0);
#endif

    plustin_basetype = plusenv_setup();
    plustin_type = plustype_new(plustin_basetype, plusps_Ti,
				sizeof(t_plustin),
				(t_plustypefn)plustin_delete, 0, 0, 0);
    plustob_type = plustype_new(0, plusps_To,
				sizeof(t_plustob),
				(t_plustypefn)plustob_delete, 0, 0,
				(t_plustypefn)plustob_attach);
    plusvar_type = plustype_new(plustob_type, plusps_Tv,
				sizeof(t_plusvar),
				(t_plustypefn)plusvar_delete, 0, 0, 0);
    plustot_ar_setup();
    plustot_env_setup();
    plustot_in_setup();
    plustot_var_setup();
    plustot_out_setup();
    plustot_qlist_setup();
    plustot_print_setup();
}
