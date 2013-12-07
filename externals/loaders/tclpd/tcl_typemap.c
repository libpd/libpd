#include "tclpd.h"
#include <string.h>
#include <stdlib.h>

static const char *atomtype_map[] = {
  /* A_NULL */     "null",
  /* A_FLOAT */    "float",
  /* A_SYMBOL */   "symbol",
  /* A_POINTER */  "pointer",
  /* A_SEMI */     "semi",
  /* A_COMMA */    "comma",
  /* A_DEFFLOAT */ "deffloat",
  /* A_DEFSYM */   "defsym",
  /* A_DOLLAR */   "dollar",
  /* A_DOLLSYM */  "dollsym",
  /* A_GIMME */    "gimme",
  /* A_CANT */     "cant",
#ifdef A_BLOB
  /* A_BLOB */     "blob"
#endif
};

#define atomtype_map_size (sizeof(atomtype_map)/sizeof(atomtype_map[0]))

static const char * fwd_atomtype_map(t_atomtype t) {
    if(t >= atomtype_map_size) return atomtype_map[A_NULL];
    return atomtype_map[t];
}

static t_atomtype rev_atomtype_map(const char *s) {
    for(t_atomtype i = 0; i < atomtype_map_size; i++) {
        if(strcmp(s, atomtype_map[i]) == 0) return i;
    }
    return A_NULL;
}

int tcl_to_pdatom(Tcl_Obj *input, t_atom *output) {
    int llength;
    if(Tcl_ListObjLength(tclpd_interp, input, &llength) == TCL_ERROR)
        return TCL_ERROR;
    if(llength != 2)
        return TCL_ERROR;

    int i;
    Tcl_Obj *obj[2];
    for(i = 0; i < 2; i++) Tcl_ListObjIndex(tclpd_interp, input, i, &obj[i]);
    char *argv0 = Tcl_GetStringFromObj(obj[0], 0);

    t_atomtype a_type = rev_atomtype_map(argv0);

    switch(a_type) {
        case A_FLOAT:
        case A_DEFFLOAT:
        {
            double dbl;
            if(Tcl_GetDoubleFromObj(tclpd_interp, obj[1], &dbl) == TCL_ERROR)
                return TCL_ERROR;
            SETFLOAT(output, dbl);
            break;
        }
        case A_SYMBOL:
        case A_DEFSYM:
        {
            SETSYMBOL(output, gensym(Tcl_GetStringFromObj(obj[1], 0)));
            break;
        }
        case A_POINTER:
        {
            long gpointer;
            if(Tcl_GetLongFromObj(tclpd_interp, obj[1], &gpointer) == TCL_ERROR)
                return TCL_ERROR;
            SETPOINTER(output, (t_gpointer *)gpointer);
            break;
        }
        case A_SEMI:
        {
            SETSEMI(output);
            break;
        }
        case A_COMMA:
        {
            SETCOMMA(output);
            break;
        }
        case A_DOLLAR:
        {
            char *str = Tcl_GetStringFromObj(obj[1], 0);
            if(!str) {
                return TCL_ERROR;
            }
            if(*str == '$') str++;
            int ii = atoi(str);
            SETDOLLAR(output, ii);
            break;
        }
        case A_DOLLSYM:
        {
            SETSYMBOL(output, gensym(Tcl_GetStringFromObj(obj[1], 0)));
            break;
        }
        // case A_GIMME:
        // case A_CANT:
        // case A_BLOB:
        // case A_NULL:
        default:
        {
            // TODO: set error result
            return TCL_ERROR;
        }
    }

    return TCL_OK;
}

int tcl_to_pdsymbol(Tcl_Obj *input, t_symbol **output) {
    char *s = Tcl_GetStringFromObj(input, 0);
    *output = gensym(s);
    return TCL_OK;
}

int pdatom_to_tcl(t_atom *input, Tcl_Obj **output) {
    Tcl_Obj *tcl_t_atom[2];
    tcl_t_atom[0] = Tcl_NewStringObj(fwd_atomtype_map(input->a_type), -1);
    switch (input->a_type) {
        case A_FLOAT:
        case A_DEFFLOAT:
        {
            tcl_t_atom[1] = Tcl_NewDoubleObj(input->a_w.w_float);
            break;
        }
        case A_SYMBOL:
        case A_DEFSYM:
        case A_DOLLSYM:
        {
            tcl_t_atom[1] = Tcl_NewStringObj(input->a_w.w_symbol->s_name, strlen(input->a_w.w_symbol->s_name));
            break;
        }
        case A_POINTER:
        {
            tcl_t_atom[1] = Tcl_NewDoubleObj((long)input->a_w.w_gpointer);
            break;
        }
        case A_DOLLAR:
        {
            char dolbuf[8];
            snprintf(dolbuf, 8, "$%d", (int)input->a_w.w_index);
            tcl_t_atom[1] = Tcl_NewStringObj(dolbuf, -1);
            break;
        }
        case A_SEMI:
        {
            tcl_t_atom[1] = Tcl_NewStringObj(";", 1);
            break;
        }
        case A_COMMA:
        {
            tcl_t_atom[1] = Tcl_NewStringObj(",", 1);
            break;
        }
        case A_GIMME:
        case A_CANT:
#ifdef A_BLOB
        case A_BLOB:
#endif
        case A_NULL:
        default:
        {
            tcl_t_atom[1] = Tcl_NewStringObj("?", 1);
            break;
        }
    }
#if 0
    verbose(-1, "tclpd: pdatom_to_tcl: atom [type = %s, value = %s]",
        Tcl_GetStringFromObj(tcl_t_atom[0], 0),
        Tcl_GetStringFromObj(tcl_t_atom[1], 0));
#endif
    *output = Tcl_NewListObj(2, &tcl_t_atom[0]);
    Tcl_IncrRefCount(*output);
    return TCL_OK;
}

int pdsymbol_to_tcl(t_symbol *input, Tcl_Obj **output) {
#if 0
    Tcl_Obj *s[2];
    s[0] = Tcl_NewStringObj("symbol", -1);
    s[1] = Tcl_NewStringObj(input->s_name, -1);
    *output = Tcl_NewListObj(2, &s[0]);
#else
    *output = Tcl_NewStringObj(input->s_name, -1);
#endif
    Tcl_IncrRefCount(*output);
    return TCL_OK;
}

