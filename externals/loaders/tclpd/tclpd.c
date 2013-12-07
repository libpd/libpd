#include "tclpd.h"
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <m_imp.h>

Tcl_Interp *tclpd_interp = NULL;

void tclpd_setup(void) {
    if(tclpd_interp) {
        return;
    }

    /* verbose(-1) post to the pd window at level 3 */
    verbose(-1, "tclpd loader v" TCLPD_VERSION);

/* kludge to work around tclpd messing up the loading of UTF-8 patches.  This
 * should really be solved correctly, its probably caused by the
 * locale/encoding not being setup correctly, perhaps in pd itself */
#if defined __gnu_linux__ || defined __GNU__ || defined __FreeBSD_kernel__
    char *lang = getenv("LANG");
    setenv("LANG", "C", 1);
#endif
    proxyinlet_setup();

    tclpd_interp = Tcl_CreateInterp();
    Tcl_Init(tclpd_interp);
    Tclpd_SafeInit(tclpd_interp);
#if defined __gnu_linux__ || defined __GNU__ || defined __FreeBSD_kernel__
    setenv("LANG", lang, 1);
#endif

    Tcl_Eval(tclpd_interp, "package provide Tclpd " TCLPD_VERSION);

    t_class *foo_class = class_new(gensym("tclpd_init"), 0, 0, 0, 0, 0);
    char buf[PATH_MAX];
    snprintf(buf, PATH_MAX, "%s/tclpd.tcl", foo_class->c_externdir->s_name);
    verbose(-1, "tclpd: trying to load %s...", buf);
    int result = Tcl_EvalFile(tclpd_interp, buf);
    switch(result) {
    case TCL_ERROR:
        error("tclpd: error loading %s", buf);
        break;
    case TCL_RETURN:
        error("tclpd: warning: %s exited with code return", buf);
        break;
    case TCL_BREAK:
    case TCL_CONTINUE:
        error("tclpd: warning: %s exited with code break/continue", buf);
        break;
    }
    verbose(-1, "tclpd: loaded %s", buf);

    sys_register_loader(tclpd_do_load_lib);
}

void tclpd_interp_error(t_tcl *x, int result) {
    error("tclpd error: %s", Tcl_GetStringResult(tclpd_interp));

    logpost(x, 3, "------------------- Tcl error: -------------------");

    // Tcl_GetReturnOptions and Tcl_DictObjGet only available in Tcl >= 8.5

#if ((TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION >= 5) || (TCL_MAJOR_VERSION > 8))
    Tcl_Obj *dict = Tcl_GetReturnOptions(tclpd_interp, result);
    Tcl_Obj *errorInfo = NULL;
    Tcl_Obj *errorInfoK = Tcl_NewStringObj("-errorinfo", -1);
    Tcl_IncrRefCount(errorInfoK);
    Tcl_DictObjGet(tclpd_interp, dict, errorInfoK, &errorInfo);
    Tcl_DecrRefCount(errorInfoK);
    logpost(x, 3, "%s\n", Tcl_GetStringFromObj(errorInfo, 0));
#else
    logpost(x, 3, "Backtrace not available in Tcl < 8.5. Please upgrade Tcl.");
#endif

    logpost(x, 3, "--------------------------------------------------");
}
