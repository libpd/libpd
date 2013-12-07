#include "tclpd.h"

t_class *proxyinlet_class;

void proxyinlet_init(t_proxyinlet *x) {
    //x->pd = proxyinlet_class;
    x->target = NULL;
    x->sel = gensym("none");
    x->argc = 0;
    x->argv = NULL;
}

void proxyinlet_clear(t_proxyinlet *x) {
    if(x->argv) {
        freebytes(x->argv, x->argc * sizeof(*x->argv));
    }
}

#define PROXYINLET_SEL_TO_LIST 0 // 0 or 1

void proxyinlet_anything(t_proxyinlet *x, t_symbol *s, int argc, t_atom *argv) {
    proxyinlet_clear(x);

    if(!(x->argv = (t_atom *)getbytes((argc+PROXYINLET_SEL_TO_LIST) * sizeof(*x->argv)))) {
        x->argc = 0;
        error("proxyinlet: getbytes: out of memory");
        return;
    }

    x->argc = argc + PROXYINLET_SEL_TO_LIST;
    if(PROXYINLET_SEL_TO_LIST == 1) SETSYMBOL(&x->argv[0], s);
    else x->sel = s;

    int i;
    for(i = 0; i < argc; i++) {
        x->argv[i+PROXYINLET_SEL_TO_LIST] = argv[i];
    }

    proxyinlet_trigger(x);
}

void proxyinlet_trigger(t_proxyinlet *x) {
    if(x->target != NULL && x->sel != gensym("none")) {
        tclpd_inlet_anything(x->target, x->ninlet, x->sel, x->argc, x->argv);
    }
}

t_atom * proxyinlet_get_atoms(t_proxyinlet *x) {
    return x->argv;
}

void proxyinlet_clone(t_proxyinlet *x, t_proxyinlet *y) {
    y->target = x->target;
    y->sel = x->sel;

    y->argc = x->argc;
    if(!(y->argv = (t_atom *)getbytes(y->argc * sizeof(*y->argv)))) {
        y->argc = 0;
        error("proxyinlet: getbytes: out of memory");
        return;
    }

    int i;
    for(i = 0; i < x->argc; i++) {
        y->argv[i] = x->argv[i];
    }
}

void proxyinlet_setup(void) {
    proxyinlet_class = class_new(gensym("tclpd proxyinlet"),
        0, 0, sizeof(t_proxyinlet), 0, A_NULL);
    class_addanything(proxyinlet_class, proxyinlet_anything);
}
