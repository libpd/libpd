
/* in order to get strdup(), this needs to be defined */
#define _POSIX_C_SOURCE 200809L

#include "tclpd.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashtable.h"

static hash_table_t *class_table = NULL;
static hash_table_t *object_table = NULL;
static hash_table_t *source_table = NULL;

void class_table_add(const char *n, t_class *c) {
    hashtable_add(class_table, n, (void *)c);
}

void class_table_remove(const char *n) {
    hashtable_remove(class_table, n);
}

t_class * class_table_get(const char *n) {
    return (t_class *)hashtable_get(class_table, n);
}

void object_table_add(const char *n, t_tcl *o) {
    hashtable_add(object_table, n, (void *)o);
}

void object_table_remove(const char *n) {
    hashtable_remove(object_table, n);
}

t_tcl * object_table_get(const char *n) {
    return (t_tcl *)hashtable_get(object_table, n);
}

static unsigned long objectSequentialId = 0;

/* set up the class that handles loading of tcl classes */
t_class * tclpd_class_new(const char *name, int flags) {
    t_class *c = class_new(gensym(name), (t_newmethod)tclpd_new,
        (t_method)tclpd_free, sizeof(t_tcl), flags, A_GIMME, A_NULL);

    if(!class_table)
        class_table = hashtable_new(1 << 7);
    if(!class_table_get(name))
        class_table_add(name, c);

    class_addanything(c, tclpd_anything);

    // is this really necessary given that there is already a 'anything' handler?
    class_addmethod(c, (t_method)tclpd_loadbang, gensym("loadbang"), A_NULL);
    
    class_addmethod(c, (t_method)tclpd_open, gensym("menu-open"), A_NULL);

    char buf[80];
    Tcl_Obj *res;
    int res_i;

    // use properties function if exists in tcl space.
    snprintf(buf, 80, "llength [info procs ::%s::properties]", name);
    if(Tcl_Eval(tclpd_interp, buf) == TCL_OK) {
        res = Tcl_GetObjResult(tclpd_interp);
        if(Tcl_GetIntFromObj(tclpd_interp, res, &res_i) == TCL_OK && res_i > 0) {
            class_setpropertiesfn(c, tclpd_properties);
        }
    }

    // use save function if exists in tcl space.
    snprintf(buf, 80, "llength [info procs ::%s::save]", name);
    if(Tcl_Eval(tclpd_interp, buf) == TCL_OK) {
        res = Tcl_GetObjResult(tclpd_interp);
        if(Tcl_GetIntFromObj(tclpd_interp, res, &res_i) == TCL_OK && res_i > 0) {
            class_setsavefn(c, tclpd_save);
        }
    }

    return c;
}

t_class * tclpd_guiclass_new(const char *name, int flags) {
    t_class *c = tclpd_class_new(name, flags);
    t_widgetbehavior *wb = (t_widgetbehavior *)getbytes(sizeof(t_widgetbehavior));
    wb->w_getrectfn = tclpd_guiclass_getrect;
    wb->w_displacefn = tclpd_guiclass_displace;
    wb->w_selectfn = tclpd_guiclass_select;
    wb->w_activatefn = NULL;
    wb->w_deletefn = tclpd_guiclass_delete;
    wb->w_visfn = tclpd_guiclass_vis;
    wb->w_clickfn = tclpd_guiclass_click;
    class_setwidget(c, wb);
    return c;
}

t_tcl * tclpd_new(t_symbol *classsym, int ac, t_atom *at) {
    // lookup in class table
    const char *name = classsym->s_name;
    t_class *qlass = class_table_get(name);
    while(!qlass) {
        // try progressively skipping namespace/ prefixes (bug 3436716)
        name = strchr(name, '/');
        if(!name || !*++name) break;
        qlass = class_table_get(name);
    }
    if(!qlass) {
        error("tclpd: class not found: %s", name);
        return NULL;
    }

    t_tcl *x = (t_tcl *)pd_new(qlass);
    if(!x) {
        error("tclpd: failed to create object of class %s", name);
        return NULL;
    }

    /* used for numbering proxy inlets: */
    x->ninlets = 1 /* qlass->c_firstin ??? */;

    x->source_file = (char *)hashtable_get(source_table, name);
    if(!x->source_file) {
        post("tclpd: missing source file information. open command will not work.");
    }

    x->classname = Tcl_NewStringObj(name, -1);
    char so[64];
    snprintf(so, 64, "tclpd.%s.x%lx", name, objectSequentialId++);
    x->self = Tcl_NewStringObj(so, -1);
    char sd[64];
    snprintf(sd, 64, "::%s::dispatcher", name);
    x->dispatcher = Tcl_NewStringObj(sd, -1);

    // obj instance -> classname mapping
    char addmapcmd[256];
    snprintf(addmapcmd, 256, "array set ::pd::classname {{%s} {%s}}", so, name);
    Tcl_Eval(tclpd_interp, addmapcmd);

    // the lifetime of x->{classname,self,dispatcher} is greater than this
    // function, hence they get an extra Tcl_IncrRefCount here:
    //      (see tclpd_free())
    Tcl_IncrRefCount(x->classname);
    Tcl_IncrRefCount(x->self);
    Tcl_IncrRefCount(x->dispatcher);

    // store in object table (for later lookup)
    if(!object_table)
        object_table = hashtable_new(1 << 10);
    if(!object_table_get(so))
        object_table_add(so, x);

    // build constructor command
    Tcl_Obj *av[ac+3]; InitArray(av, ac+3, NULL);
    av[0] = x->dispatcher;
    Tcl_IncrRefCount(av[0]);
    av[1] = x->self;
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("constructor", -1);
    Tcl_IncrRefCount(av[2]);
    for(int i=0; i<ac; i++) {
        // NOTE: pdatom_to_tcl already calls Tcl_IncrRefCount
        //       so there is no need to call it here:

        if(pdatom_to_tcl(&at[i], &av[3+i]) == TCL_ERROR) {
#ifdef DEBUG
            post("tclpd_new: failed conversion (pdatom_to_tcl)");
#endif
            goto error;
        }
    }

    // call constructor
    if(Tcl_EvalObjv(tclpd_interp, ac+3, av, 0) != TCL_OK) {
        goto error;
    }

    // decrement reference counter
    for(int i = 0; i < (ac+3); i++)
        Tcl_DecrRefCount(av[i]);

    return x;

error:
    tclpd_interp_error(NULL, TCL_ERROR);
    for(int i = 0; i < (ac+3); i++) {
        if(!av[i]) break; // could have gone here before doing all av[]s
        Tcl_DecrRefCount(av[i]);
    }
    pd_free((t_pd *)x);
    return 0;
}

void tclpd_free(t_tcl *x) {
    // build destructor command
    Tcl_Obj *av[3]; InitArray(av, 3, NULL);
    av[0] = x->dispatcher;
    Tcl_IncrRefCount(av[0]);
    av[1] = x->self;
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("destructor", -1);
    Tcl_IncrRefCount(av[2]);

    // call destructor
    if(Tcl_EvalObjv(tclpd_interp, 3, av, 0) != TCL_OK) {
#ifdef DEBUG
        post("tclpd_free: failed to call destructor");
#endif
    }

    // decrement reference counter
    Tcl_DecrRefCount(av[0]);
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);

    // remove obj instance -> classname mapping
    char delmapcmd[256];
    snprintf(delmapcmd, 256, "unset ::pd::classname(%s)", Tcl_GetStringFromObj(x->self, NULL));
    Tcl_Eval(tclpd_interp, delmapcmd);

    // here ends the lifetime of x->classname and x->self
    Tcl_DecrRefCount(x->self);
    Tcl_DecrRefCount(x->classname);
    Tcl_DecrRefCount(x->dispatcher);
#ifdef DEBUG
    post("tclpd_free called");
#endif
}

void tclpd_anything(t_tcl *x, t_symbol *s, int ac, t_atom *at) {
    tclpd_inlet_anything(x, 0, s, ac, at);
}

void tclpd_inlet_anything(t_tcl *x, int inlet, t_symbol *s, int ac, t_atom *at) {
    // proxy method - format: <classname> <self> method <inlet#> <selector> args...
    Tcl_Obj *av[ac+5]; InitArray(av, ac+5, NULL);
    int result;

    av[0] = x->dispatcher;
    Tcl_IncrRefCount(av[0]);
    av[1] = x->self;
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("method", -1);
    Tcl_IncrRefCount(av[2]);
    av[3] = Tcl_NewIntObj(inlet);
    Tcl_IncrRefCount(av[3]);
    av[4] = Tcl_NewStringObj(s->s_name, -1);
    Tcl_IncrRefCount(av[4]);
    for(int i=0; i<ac; i++) {
        // NOTE: pdatom_to_tcl already calls Tcl_IncrRefCount
        //       so there is no need to call it here:

        if(pdatom_to_tcl(&at[i], &av[5+i]) == TCL_ERROR) {
#ifdef DEBUG
            post("pdatom_to_tcl: tclpd_inlet_anything: failed during conversion. check memory leaks!");
#endif
            goto error;
        }
    }
    result = Tcl_EvalObjv(tclpd_interp, ac+5, av, 0);
    if(result != TCL_OK) {
        goto error;
    }

    for(int i=0; i < (ac+5); i++)
        Tcl_DecrRefCount(av[i]);

    // OK
    return;

error:
    tclpd_interp_error(x, TCL_ERROR);
    for(int i=0; i < (ac+5); i++) {
        if(!av[i]) break; // could have gone here before doing all av[]s
        Tcl_DecrRefCount(av[i]);
    }
    return;
}

void tclpd_loadbang(t_tcl *x) {
    tclpd_inlet_anything(x, 0, gensym("loadbang"), 0, NULL);
}

void tclpd_open(t_tcl *x) {
    if(!x->source_file)
        return;

    sys_vgui("::pd_menucommands::menu_openfile {%s}\n", x->source_file);
}

/* Tcl glue: */

t_proxyinlet * tclpd_add_proxyinlet(t_tcl *x) {
    t_proxyinlet *proxy = (t_proxyinlet *)pd_new(proxyinlet_class);
    proxyinlet_init(proxy);
    proxy->target = x;
    proxy->ninlet = x->ninlets++;
    inlet_new(&x->o, &proxy->obj.ob_pd, 0, 0);
    return proxy;
}

/*
t_tcl * tclpd_get_instance(const char *objectSequentialId) {
    return (t_tcl *)object_table_get(objectSequentialId);
}

t_pd * tclpd_get_instance_pd(const char *objectSequentialId) {
    return (t_pd *)object_table_get(objectSequentialId);
}

t_text * tclpd_get_instance_text(const char *objectSequentialId) {
    return (t_text *)object_table_get(objectSequentialId);
}

t_object * tclpd_get_object(const char *objectSequentialId) {
    t_tcl *x = tclpd_get_instance(objectSequentialId);
    return &x->o;
}

t_pd * tclpd_get_object_pd(const char *objectSequentialId) {
    t_object *o = tclpd_get_object(objectSequentialId);
    return &o->ob_pd;
}

t_binbuf * tclpd_get_object_binbuf(const char *objectSequentialId) {
    t_object *o = tclpd_get_object(objectSequentialId);
    return o->ob_binbuf;
}

t_glist * tclpd_get_glist(const char *objectSequentialId) {
    t_tcl *x = tclpd_get_instance(objectSequentialId);
    return x->x_glist;
}

t_atom * tclpd_binbuf_get_atom(t_binbuf *b, int n) {
    if(binbuf_getnatom(b) <= n || n < 0)
        return NULL;
    return binbuf_getvec(b) + n;
}
*/

/* helper function for accessing binbuf's atoms
   cause, accessing C arrays and doing typemaps is not that easy */
t_atom * binbuf_getatom(t_binbuf *x, int index) {
    return binbuf_getvec(x) + index;
}

t_object * CAST_t_object(t_object *o) {
    return o;
}

t_pd * CAST_t_pd(t_pd *o) {
    return o;
}

t_text * CAST_t_text(t_text *o) {
    return o;
}

t_tcl * CAST_t_tcl(t_tcl *o) {
    return o;
}

void poststring2 (const char *s) {
    post("%s", s);
}

void tclpd_save(t_gobj *z, t_binbuf *b) {
    Tcl_Obj *av[3]; InitArray(av, 3, NULL);
    Tcl_Obj *res;

    t_tcl *x = (t_tcl *)z;

    av[0] = x->dispatcher;
    Tcl_IncrRefCount(av[0]);
    av[1] = x->self;
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("save", -1);
    Tcl_IncrRefCount(av[2]);

    int result = Tcl_EvalObjv(tclpd_interp, 3, av, 0);
    if(result == TCL_OK) {
        res = Tcl_GetObjResult(tclpd_interp);
        Tcl_IncrRefCount(res);
        int objc;
        Tcl_Obj **objv;
        result = Tcl_ListObjGetElements(tclpd_interp, res, &objc, &objv);
        if(result == TCL_OK) {
            if(objc == 0 && objv == NULL) {
                // call default savefn
                text_save(z, b);
            } else {
                // do custom savefn
                int i;
                double tmp;
                for(i = 0; i < objc; i++) {
                    result = Tcl_GetDoubleFromObj(tclpd_interp, objv[i], &tmp);
                    if(result == TCL_OK) {
                        binbuf_addv(b, "f", (t_float)tmp);
                    } else {
                        char *tmps = Tcl_GetStringFromObj(objv[i], NULL);
                        if(!strcmp(tmps, ";")) {
                            binbuf_addv(b, ";");
                        } else {
                            binbuf_addv(b, "s", gensym(tmps));
                        }
                    }
                }
            }
        } else {
            pd_error(x, "Tcl: object save: failed");
            tclpd_interp_error(x, result);
        }
        Tcl_DecrRefCount(res);
    } else {
        pd_error(x, "Tcl: object save: failed");
        tclpd_interp_error(x, result);
    }

    Tcl_DecrRefCount(av[0]);
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
}

void tclpd_properties(t_gobj *z, t_glist *owner) {
    Tcl_Obj *av[3]; InitArray(av, 3, NULL);

    t_tcl *x = (t_tcl *)z;

    av[0] = x->dispatcher;
    Tcl_IncrRefCount(av[0]);
    av[1] = x->self;
    Tcl_IncrRefCount(av[1]);
    av[2] = Tcl_NewStringObj("properties", -1);
    Tcl_IncrRefCount(av[2]);

    int result = Tcl_EvalObjv(tclpd_interp, 3, av, 0);
    if(result != TCL_OK) {
        //res = Tcl_GetObjResult(tclpd_interp);
        pd_error(x, "Tcl: object properties: failed");
        tclpd_interp_error(x, result);
    }

    Tcl_DecrRefCount(av[0]);
    Tcl_DecrRefCount(av[1]);
    Tcl_DecrRefCount(av[2]);
}

void tclpd_class_namespace_init(const char *classname) {
    char cmd[256];
    snprintf(cmd, 256, "if [namespace exists ::%s] "
        "{namespace delete ::%s}; "
        "namespace eval ::%s {}",
        classname, classname, classname);
    Tcl_Eval(tclpd_interp, cmd);
}

void source_table_remove(const char *object_name) {
    if(!source_table)
        source_table = hashtable_new(1 << 7);
    hashtable_remove(source_table, object_name);
}

void source_table_add(const char *object_name, const char *source_file) {
    source_table_remove(object_name);
    hashtable_add(source_table, object_name, strdup(source_file));
}
