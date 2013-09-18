/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* The three uses of the 'hammerfile' proxy class are:
   1. providing `embedding' facility -- storing master object's state
   in a .pd file,
   2. encapsulating openpanel/savepanel management,
   3. extending the gui of Pd with a simple text editor window.

   A master class which needs embedding feature (like coll), passes
   a nonzero flag to the hammerfile setup routine, and a nonzero embedfn
   function pointer to the hammerfile constructor.  If a master needs
   access to the panels (like collcommon), then it passes nonzero readfn
   and/or writefn callback pointers to the constructor.  A master which has
   an associated text editor, AND wants to update object's state after
   edits, passes a nonzero updatefn callback in a call to the constructor.

   LATER extract the embedding stuff. */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/os.h"
#include "unstable/forky.h"
#include "hammer/file.h"

struct _hammerfile
{
    t_pd                 f_pd;
    t_pd                *f_master;
    t_canvas            *f_canvas;
    t_symbol            *f_bindname;
    t_symbol            *f_currentdir;
    t_symbol            *f_inidir;
    t_symbol            *f_inifile;
    t_hammerfilefn       f_panelfn;
    t_hammerfilefn       f_editorfn;
    t_hammerembedfn      f_embedfn;
    t_binbuf            *f_binbuf;
    t_clock             *f_panelclock;
    t_clock             *f_editorclock;
    struct _hammerfile  *f_savepanel;
    struct _hammerfile  *f_next;
};

static t_class *hammerfile_class = 0;
static t_hammerfile *hammerfile_proxies;
static t_symbol *ps__C;

static t_hammerfile *hammerfile_getproxy(t_pd *master)
{
    t_hammerfile *f;
    for (f = hammerfile_proxies; f; f = f->f_next)
	if (f->f_master == master)
	    return (f);
    return (0);
}

static void hammereditor_guidefs(void)
{
    /* if older than 0.43, create an 0.43-style pdsend */
    sys_gui("if {[llength [info procs ::pdsend]] == 0} {");
    sys_gui("proc ::pdsend {args} {::pd \"[join $args { }] ;\"}}\n");

    sys_gui("proc hammereditor_open {name geometry title sendable} {\n");
    sys_gui(" if {[winfo exists $name]} {\n");
    sys_gui("  $name.text delete 1.0 end\n");
    sys_gui(" } else {\n");
    sys_gui("  toplevel $name\n");
    sys_gui("  wm title $name $title\n");
    sys_gui("  wm geometry $name $geometry\n");
    sys_gui("  if {$sendable} {\n");
    sys_gui("   wm protocol $name WM_DELETE_WINDOW \\\n");
    sys_gui("    [concat hammereditor_close $name 1]\n");
    sys_gui("   bind $name <<Modified>> \"hammereditor_dodirty $name\"\n");
    sys_gui("  }\n");
    sys_gui("  text $name.text -relief raised -bd 2 \\\n");
    sys_gui("   -font -*-courier-medium--normal--12-* \\\n");
    sys_gui("   -yscrollcommand \"$name.scroll set\" -background lightgrey\n");
    sys_gui("  scrollbar $name.scroll -command \"$name.text yview\"\n");
    sys_gui("  pack $name.scroll -side right -fill y\n");
    sys_gui("  pack $name.text -side left -fill both -expand 1\n");
    sys_gui(" }\n");
    sys_gui("}\n");

    sys_gui("proc hammereditor_dodirty {name} {\n");
    sys_gui(" if {[catch {$name.text edit modified} dirty]} {set dirty 1}\n");
    sys_gui(" set title [wm title $name]\n");
    sys_gui(" set dt [string equal -length 1 $title \"*\"]\n");
    sys_gui(" if {$dirty} {\n");
    sys_gui("  if {$dt == 0} {wm title $name *$title}\n");
    sys_gui(" } else {\n");
    sys_gui("  if {$dt} {wm title $name [string range $title 1 end]}\n");
    sys_gui(" }\n");
    sys_gui("}\n");

    sys_gui("proc hammereditor_setdirty {name flag} {\n");
    sys_gui(" if {[winfo exists $name]} {\n");
    sys_gui("  catch {$name.text edit modified $flag}\n");
    sys_gui(" }\n");
    sys_gui("}\n");

    sys_gui("proc hammereditor_doclose {name} {\n");
    sys_gui(" destroy $name\n");
    sys_gui("}\n");

    sys_gui("proc hammereditor_append {name contents} {\n");
    sys_gui(" if {[winfo exists $name]} {\n");
    sys_gui("  $name.text insert end $contents\n");
    sys_gui(" }\n");
    sys_gui("}\n");

    /* FIXME make it more reliable */
    sys_gui("proc hammereditor_send {name} {\n");
    sys_gui(" if {[winfo exists $name]} {\n");
    sys_gui("  pdsend \"miXed$name clear\"\n");
    sys_gui("  for {set i 1} \\\n");
    sys_gui("   {[$name.text compare $i.end < end]} \\\n");
    sys_gui("  	{incr i 1} {\n");
    sys_gui("   set lin [$name.text get $i.0 $i.end]\n");
    sys_gui("   if {$lin != \"\"} {\n");
    /* LATER rethink semi/comma mapping */
    sys_gui("    regsub -all \\; $lin \"  _semi_ \" tmplin\n");
    sys_gui("    regsub -all \\, $tmplin \"  _comma_ \" lin\n");
    sys_gui("    pdsend \"miXed$name addline $lin\"\n");
    sys_gui("   }\n");
    sys_gui("  }\n");
    sys_gui("  pdsend \"miXed$name end\"\n");
    sys_gui(" }\n");
    sys_gui("}\n");

    sys_gui("proc hammereditor_close {name ask} {\n");
    sys_gui(" if {[winfo exists $name]} {\n");
    sys_gui("  if {[catch {$name.text edit modified} dirty]} {set dirty 1}\n");
    sys_gui("  if {$ask && $dirty} {\n");
    sys_gui("   set title [wm title $name]\n");
    sys_gui("   if {[string equal -length 1 $title \"*\"]} {\n");
    sys_gui("    set title [string range $title 1 end]\n");
    sys_gui("   }\n");
    sys_gui("   set answer [tk_messageBox \\-type yesnocancel \\\n");
    sys_gui("    \\-icon question \\\n");
    sys_gui("    \\-message [concat Save changes to \\\"$title\\\"?]]\n");
    sys_gui("   if {$answer == \"yes\"} {hammereditor_send $name}\n");
    sys_gui("   if {$answer != \"cancel\"} {hammereditor_doclose $name}\n");
    sys_gui("  } else {hammereditor_doclose $name}\n");
    sys_gui(" }\n");
    sys_gui("}\n");
}

/* null owner defaults to class name, pass "" to supress */
void hammereditor_open(t_hammerfile *f, char *title, char *owner)
{
    if (!owner)
	owner = class_getname(*f->f_master);
    if (!*owner)
	owner = 0;
    if (!title)
    {
	title = owner;
	owner = 0;
    }
    if (owner)
	sys_vgui("hammereditor_open .%x %dx%d {%s: %s} %d\n",
		 (int)f, 600, 340, owner, title, (f->f_editorfn != 0));
    else
	sys_vgui("hammereditor_open .%x %dx%d {%s} %d\n",
		 (int)f, 600, 340, (title ? title : "Untitled"),
		 (f->f_editorfn != 0));
}

static void hammereditor_tick(t_hammerfile *f)
{
    sys_vgui("hammereditor_close .%x 1\n", (int)f);
}

void hammereditor_close(t_hammerfile *f, int ask)
{
    if (ask && f->f_editorfn)
	/* hack: deferring modal dialog creation in order to allow for
	   a message box redraw to happen -- LATER investigate */
 	clock_delay(f->f_editorclock, 0);
    else
	sys_vgui("hammereditor_close .%x 0\n", (int)f);
}

void hammereditor_append(t_hammerfile *f, char *contents)
{
    if (contents)
    {
	char *ptr;
	for (ptr = contents; *ptr; ptr++)
	{
	    if (*ptr == '{' || *ptr == '}')
	    {
		char c = *ptr;
		*ptr = 0;
		sys_vgui("hammereditor_append .%x {%s}\n", (int)f, contents);
		sys_vgui("hammereditor_append .%x \"%c\"\n", (int)f, c);
		*ptr = c;
		contents = ptr + 1;
	    }
	}
	if (*contents)
	    sys_vgui("hammereditor_append .%x {%s}\n", (int)f, contents);
    }
}

void hammereditor_setdirty(t_hammerfile *f, int flag)
{
    if (f->f_editorfn)
	sys_vgui("hammereditor_setdirty .%x %d\n", (int)f, flag);
}

static void hammereditor_clear(t_hammerfile *f)
{
    if (f->f_editorfn)
    {
	if (f->f_binbuf)
	    binbuf_clear(f->f_binbuf);
	else
	    f->f_binbuf = binbuf_new();
    }
}

static void hammereditor_addline(t_hammerfile *f,
				 t_symbol *s, int ac, t_atom *av)
{
    if (f->f_editorfn)
    {
	int i;
	t_atom *ap;
	for (i = 0, ap = av; i < ac; i++, ap++)
	{
	    if (ap->a_type == A_SYMBOL)
	    {
		/* LATER rethink semi/comma mapping */
		if (!strcmp(ap->a_w.w_symbol->s_name, "_semi_"))
		    SETSEMI(ap);
		else if (!strcmp(ap->a_w.w_symbol->s_name, "_comma_"))
		    SETCOMMA(ap);
	    }
	}
	binbuf_add(f->f_binbuf, ac, av);
    }
}

static void hammereditor_end(t_hammerfile *f)
{
    if (f->f_editorfn)
    {
	(*f->f_editorfn)(f->f_master, 0, binbuf_getnatom(f->f_binbuf),
			 binbuf_getvec(f->f_binbuf));
	binbuf_clear(f->f_binbuf);
    }
}

static void hammerpanel_guidefs(void)
{
    sys_gui("proc hammerpanel_open {target inidir} {\n");
    sys_gui(" global pd_opendir\n");
    sys_gui(" if {$inidir == \"\"} {\n");
    sys_gui("  set $inidir $pd_opendir\n");
    sys_gui(" }\n");
    sys_gui(" set filename [tk_getOpenFile \\\n");
    sys_gui("  -initialdir $inidir]\n");
    sys_gui(" if {$filename != \"\"} {\n");
    sys_gui("  set directory [string range $filename 0 \\\n");
    sys_gui("   [expr [string last / $filename ] - 1]]\n");
    sys_gui("  if {$directory == \"\"} {set directory \"/\"}\n");
#if 1
    sys_gui("  puts stderr [concat $directory]\n");
#endif
    sys_gui("  pdsend \"$target path \\\n");
    sys_gui("   [enquote_path $filename] [enquote_path $directory] \"\n");
    sys_gui(" }\n");
    sys_gui("}\n");

    sys_gui("proc hammerpanel_save {target inidir inifile} {\n");
    sys_gui(" if {$inifile != \"\"} {\n");
    sys_gui("  set filename [tk_getSaveFile \\\n");
    sys_gui("   -initialdir $inidir -initialfile $inifile]\n");
    sys_gui(" } else {\n");
    sys_gui("  set filename [tk_getSaveFile]\n");
    sys_gui(" }\n");
    sys_gui(" if {$filename != \"\"} {\n");
    sys_gui("  set directory [string range $filename 0 \\\n");
    sys_gui("   [expr [string last / $filename ] - 1]]\n");
    sys_gui("  if {$directory == \"\"} {set directory \"/\"}\n");
    sys_gui("  pdsend \"$target path \\\n");
    sys_gui("   [enquote_path $filename] [enquote_path $directory] \"\n");
    sys_gui(" }\n");
    sys_gui("}\n");
}

/* There are two modes of -initialdir persistence:
   1. Using last reply from gui (if any, default is canvas directory):
   pass null to hammerpanel_open/save() (for explicit cd, optionally call
   hammerpanel_setopen/savedir() first).
   2. Starting always in the same directory (eg. canvasdir):
   feed hammerpanel_open/save().
   Usually, first mode fits opening better, the second -- saving. */

/* This is obsolete, but has to stay, because older versions of miXed libraries
   might overwrite new hammerpanel_guidefs().  FIXME we need version control. */
static void hammerpanel_symbol(t_hammerfile *f, t_symbol *s)
{
    if (s && s != &s_ && f->f_panelfn)
	(*f->f_panelfn)(f->f_master, s, 0, 0);
}

static void hammerpanel_path(t_hammerfile *f, t_symbol *s1, t_symbol *s2)
{
    if (s2 && s2 != &s_)
	f->f_currentdir = s2;
    if (s1 && s1 != &s_ && f->f_panelfn)
	(*f->f_panelfn)(f->f_master, s1, 0, 0);
}

static void hammerpanel_tick(t_hammerfile *f)
{
    if (f->f_savepanel)
	sys_vgui("hammerpanel_open %s {%s}\n", f->f_bindname->s_name,
		 f->f_inidir->s_name);
    else
	sys_vgui("hammerpanel_save %s {%s} {%s}\n", f->f_bindname->s_name,
		 f->f_inidir->s_name, f->f_inifile->s_name);
}

/* these are hacks: deferring modal dialog creation in order to allow for
   a message box redraw to happen -- LATER investigate */
void hammerpanel_open(t_hammerfile *f, t_symbol *inidir)
{
    if (inidir)
	f->f_inidir = inidir;
    else
	f->f_inidir = (f->f_currentdir ? f->f_currentdir : &s_);
    clock_delay(f->f_panelclock, 0);
}

void hammerpanel_setopendir(t_hammerfile *f, t_symbol *dir)
{
    if (f->f_currentdir && f->f_currentdir != &s_)
    {
	if (dir && dir != &s_)
	{
	    int length;
	    if (length = ospath_length(dir->s_name, f->f_currentdir->s_name))
	    {
		char *path = getbytes(length + 1);
		if (ospath_absolute(dir->s_name, f->f_currentdir->s_name, path))
		    /* LATER stat (think how to report a failure) */
		    f->f_currentdir = gensym(path);
		freebytes(path, length + 1);
	    }
	}
	else if (f->f_canvas)
	    f->f_currentdir = canvas_getdir(f->f_canvas);
    }
    else bug("hammerpanel_setopendir");
}

t_symbol *hammerpanel_getopendir(t_hammerfile *f)
{
    return (f->f_currentdir);
}

void hammerpanel_save(t_hammerfile *f, t_symbol *inidir, t_symbol *inifile)
{
    if (f = f->f_savepanel)
    {
	if (inidir)
	    f->f_inidir = inidir;
	else
	    /* LATER ask if we can rely on s_ pointing to "" */
	    f->f_inidir = (f->f_currentdir ? f->f_currentdir : &s_);
	f->f_inifile = (inifile ? inifile : &s_);
	clock_delay(f->f_panelclock, 0);
    }
}

void hammerpanel_setsavedir(t_hammerfile *f, t_symbol *dir)
{
    if (f = f->f_savepanel)
	hammerpanel_setopendir(f, dir);
}

t_symbol *hammerpanel_getsavedir(t_hammerfile *f)
{
    return (f->f_savepanel ? f->f_savepanel->f_currentdir : 0);
}

/* Currently embeddable hammer classes do not use the 'saveto' method.
   In order to use it, any embeddable class would have to add a creation
   method to pd_canvasmaker -- then saving could be done with a 'proper'
   sequence:  #N <master> <args>; #X <whatever>; ...; #X restore <x> <y>;
   However, this works only for -lib externals.  So, we choose a sequence:
   #X obj <x> <y> <master> <args>; #C <whatever>; ...; #C restore;
   Since the first message in this sequence is a valid creation message
   on its own, we have to distinguish loading from a .pd file, and other
   cases (editing). */

static void hammerembed_gc(t_pd *x, t_symbol *s, int expected)
{
    t_pd *garbage;
    int count = 0;
    while (garbage = pd_findbyclass(s, *x)) pd_unbind(garbage, s), count++;
    if (count != expected)
	bug("hammerembed_gc (%d garbage bindings)", count);
}

static void hammerembed_restore(t_pd *master)
{
    hammerembed_gc(master, ps__C, 1);
}

void hammerembed_save(t_gobj *master, t_binbuf *bb)
{
    t_hammerfile *f = hammerfile_getproxy((t_pd *)master);
    t_text *t = (t_text *)master;
    binbuf_addv(bb, "ssii", &s__X, gensym("obj"),
    	    	(int)t->te_xpix, (int)t->te_ypix);
    binbuf_addbinbuf(bb, t->te_binbuf);
    binbuf_addsemi(bb);
    if (f && f->f_embedfn)
	(*f->f_embedfn)(f->f_master, bb, ps__C);
    binbuf_addv(bb, "ss;", ps__C, gensym("restore"));
}

int hammerfile_ismapped(t_hammerfile *f)
{
    return (f->f_canvas->gl_mapped);
}

int hammerfile_isloading(t_hammerfile *f)
{
    return (f->f_canvas->gl_loading);
}

/* LATER find a better way */
int hammerfile_ispasting(t_hammerfile *f)
{
    int result = 0;
    t_canvas *cv = f->f_canvas;
    if (!cv->gl_loading)
    {
	t_pd *z = s__X.s_thing;
	if (z == (t_pd *)cv)
	{
	    pd_popsym(z);
	    if (s__X.s_thing == (t_pd *)cv) result = 1;
	    pd_pushsym(z);
	}
	else if (z) result = 1;
    }
#if 0
    if (result) post("pasting");
#endif
    return (result);
}

void hammerfile_free(t_hammerfile *f)
{
    t_hammerfile *prev, *next;
    hammereditor_close(f, 0);
    if (f->f_embedfn)
	/* just in case of missing 'restore' */
	hammerembed_gc(f->f_master, ps__C, 0);
    if (f->f_savepanel)
    {
	pd_unbind((t_pd *)f->f_savepanel, f->f_savepanel->f_bindname);
	pd_free((t_pd *)f->f_savepanel);
    }
    if (f->f_bindname) pd_unbind((t_pd *)f, f->f_bindname);
    if (f->f_panelclock) clock_free(f->f_panelclock);
    if (f->f_editorclock) clock_free(f->f_editorclock);
    for (prev = 0, next = hammerfile_proxies;
	 next; prev = next, next = next->f_next)
	if (next == f)
	    break;
    if (prev)
	prev->f_next = f->f_next;
    else if (f == hammerfile_proxies)
	hammerfile_proxies = f->f_next;
    pd_free((t_pd *)f);
}

t_hammerfile *hammerfile_new(t_pd *master, t_hammerembedfn embedfn,
			     t_hammerfilefn readfn, t_hammerfilefn writefn,
			     t_hammerfilefn updatefn)
{
    t_hammerfile *result = (t_hammerfile *)pd_new(hammerfile_class);
    result->f_master = master;
    result->f_next = hammerfile_proxies;
    hammerfile_proxies = result;
    if (!(result->f_canvas = canvas_getcurrent()))
    {
	bug("hammerfile_new: out of context");
	return (result);
    }

    /* 1. embedding */
    if (result->f_embedfn = embedfn)
    {
	/* just in case of missing 'restore' */
	hammerembed_gc(master, ps__C, 0);
	if (hammerfile_isloading(result) || hammerfile_ispasting(result))
	    pd_bind(master, ps__C);
    }

    /* 2. the panels */
    if (readfn || writefn)
    {
	t_hammerfile *f;
	char buf[64];
	sprintf(buf, "miXed.%x", (int)result);
	result->f_bindname = gensym(buf);
	pd_bind((t_pd *)result, result->f_bindname);
	result->f_currentdir =
	    result->f_inidir = canvas_getdir(result->f_canvas);
	result->f_panelfn = readfn;
	result->f_panelclock = clock_new(result, (t_method)hammerpanel_tick);
	f = (t_hammerfile *)pd_new(hammerfile_class);
	f->f_master = master;
	f->f_canvas = result->f_canvas;
	sprintf(buf, "miXed.%x", (int)f);
	f->f_bindname = gensym(buf);
	pd_bind((t_pd *)f, f->f_bindname);
	f->f_currentdir = f->f_inidir = result->f_currentdir;
	f->f_panelfn = writefn;	
	f->f_panelclock = clock_new(f, (t_method)hammerpanel_tick);
	result->f_savepanel = f;
    }
    else result->f_savepanel = 0;

    /* 3. editor */
    if (result->f_editorfn = updatefn)
    {
	result->f_editorclock = clock_new(result, (t_method)hammereditor_tick);
	if (!result->f_bindname)
	{
	    char buf[64];
	    sprintf(buf, "miXed.%x", (int)result);
	    result->f_bindname = gensym(buf);
	    pd_bind((t_pd *)result, result->f_bindname);
	}
    }
    return (result);
}

void hammerfile_setup(t_class *c, int embeddable)
{
    if (embeddable)
    {
	forky_setsavefn(c, hammerembed_save);
	class_addmethod(c, (t_method)hammerembed_restore,
			gensym("restore"), 0);
    }
    if (!hammerfile_class)
    {
	ps__C = gensym("#C");
	hammerfile_class = class_new(gensym("_hammerfile"), 0, 0,
				     sizeof(t_hammerfile),
				     CLASS_PD | CLASS_NOINLET, 0);
	class_addsymbol(hammerfile_class, hammerpanel_symbol);
	class_addmethod(hammerfile_class, (t_method)hammerpanel_path,
			gensym("path"), A_SYMBOL, A_DEFSYM, 0);
	class_addmethod(hammerfile_class, (t_method)hammereditor_clear,
			gensym("clear"), 0);
	class_addmethod(hammerfile_class, (t_method)hammereditor_addline,
			gensym("addline"), A_GIMME, 0);
	class_addmethod(hammerfile_class, (t_method)hammereditor_end,
			gensym("end"), 0);
	/* LATER find a way of ensuring that these are not defined yet... */
	hammereditor_guidefs();
	hammerpanel_guidefs();
    }
}
