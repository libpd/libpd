/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: flite.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: speech synthesis for PD
 *
 *  PD interface to 'flite' C libraries.
 *
 *=============================================================================*/


#include <m_pd.h>
#include "common/mooPdUtils.h"

/* black magic for Microsoft's compiler */
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <flite/flite.h>
#include <flite/cst_wave.h>

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define FLITE_DEBUG 1
//#undef FLITE_DEBUG


/*--------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------*/
extern cst_voice *PDFLITE_REGISTER_VOICE();
static cst_voice *voice;

#define DEFAULT_BUFSIZE 256
#define DEFAULT_BUFSTEP 256

/*=====================================================================
 * Structures and Types
 *=====================================================================*/

static const char *flite_description =
  "flite: Text-to-Speech external v" PACKAGE_VERSION " by Bryan Jurish\n"
  "flite: compiled on " PACKAGE_BUILD_DATE " by " PACKAGE_BUILD_USER "\n"
  ;
//static char *flite_acknowledge = "flite: based on code by ";
//static char *flite_version = "flite: PD external v%s by Bryan Jurish";



/*---------------------------------------------------------------------
 * flite
 *---------------------------------------------------------------------*/
static t_class *flite_class;
typedef struct _flite
{
  t_object x_obj;                    /* black magic (probably inheritance-related) */
  t_symbol *x_arrayname;             /* arrayname (from '_tabwrite' code in $PD_SRC/d_array.c) */
  char     *textbuf;                 /* text buffer (hack) */
  int      bufsize;                  /* text buffer size */
} t_flite;


/*--------------------------------------------------------------------
 * flite_synth : synthesize current text-buffer
 *--------------------------------------------------------------------*/
void flite_synth(t_flite *x) {
  cst_wave *wave;
  int i,vecsize;
  t_garray *a;
  t_float *vec;

# ifdef FLITE_DEBUG
  post("flite: got message 'synth'");
# endif

  // -- sanity checks
  if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class))) {
    pd_error(x,"flite: no such array '%s'", x->x_arrayname->s_name);
    return;
  }
  if (!x->textbuf) {
    pd_error(x,"flite: attempt to synthesize empty text-buffer!");
    return;
  }

# ifdef FLITE_DEBUG
  post("flite: flite_text_to_wave()");
# endif
  wave = flite_text_to_wave(x->textbuf,voice);

  if (!wave) {
    pd_error(x,"flite: synthesis failed for text '%s'", x->textbuf);
    return;
  }

  // -- resample
# ifdef FLITE_DEBUG
  post("flite: cst_wave_resample()");
# endif
  cst_wave_resample(wave,sys_getsr());

  // -- resize & write to our array
# ifdef FLITE_DEBUG
  post("flite: garray_resize(%d)", wave->num_samples);
# endif

  garray_resize(a, wave->num_samples);
  if (!garray_getfloatarray(a, &vecsize, &vec))
    pd_error(x,"flite: bad template for write to array '%s'", x->x_arrayname->s_name);

# ifdef FLITE_DEBUG
  post("flite: ->write to garray loop<-");
# endif
  for (i = 0; i < wave->num_samples; i++) {
    *vec++ = wave->samples[i]/32767.0;
  }

  // -- outlet synth-done-bang
  outlet_bang(x->x_obj.ob_outlet);

  // -- cleanup
  delete_wave(wave);

  // -- redraw
  garray_redraw(a);
}

/*--------------------------------------------------------------------
 * flite_text : set text-buffer
 *--------------------------------------------------------------------*/
void flite_text(t_flite *x, MOO_UNUSED t_symbol *s, int argc, t_atom *argv) {
  int i, alen, buffered;
  t_symbol *asym;

  // -- allocate initial text-buffer if required
  if (x->textbuf == NULL) {
    x->bufsize = DEFAULT_BUFSIZE;
    x->textbuf = getbytes(x->bufsize);
  }
  if (x->textbuf == NULL) {
    pd_error(x,"flite: allocation failed for text buffer");
    x->bufsize = 0;
    return;
  }

  // -- convert args to strings
  buffered = 0;
  for (i = 0; i < argc; i++) {
    asym = atom_gensym(argv);
    alen = 1+strlen(asym->s_name);

    // -- reallocate if necessary
    while (buffered + alen > x->bufsize) {
      x->textbuf = resizebytes(x->textbuf,x->bufsize,x->bufsize+DEFAULT_BUFSTEP);
      x->bufsize = x->bufsize+DEFAULT_BUFSTEP;
      if (x->textbuf == NULL) {
	pd_error(x,"flite: allocation failed for text buffer");
	x->bufsize = 0;
	return;
      }
    }
    
    // -- append arg-string to text-buf
    if (i == 0) {
      strcpy(x->textbuf+buffered, asym->s_name);
      buffered += alen-1;
    } else {
      *(x->textbuf+buffered) = ' ';
      strcpy(x->textbuf+buffered+1, asym->s_name);
      buffered += alen;
    }
    
    // -- increment/decrement
    argv++;
  }

#ifdef FLITE_DEBUG
  post("flite_debug: got text='%s'", x->textbuf);
#endif
}


/*--------------------------------------------------------------------
 * flite_list : parse & synthesize text in one swell foop
 *--------------------------------------------------------------------*/
void flite_list(t_flite *x, t_symbol *s, int argc, t_atom *argv) {
  flite_text(x,s,argc,argv);
  flite_synth(x);
}


/*--------------------------------------------------------------------
 * flite_set : set arrayname
 *--------------------------------------------------------------------*/
static void flite_set(t_flite *x, t_symbol *ary) {
#ifdef FLITE_DEBUG
  post("flite_set: called with arg='%s'", ary->s_name);
#endif
  x->x_arrayname = ary;
}


/*--------------------------------------------------------------------
 * constructor / destructor
 *--------------------------------------------------------------------*/
static void *flite_new(t_symbol *ary)
{
  t_flite *x;

  x = (t_flite *)pd_new(flite_class);

  // set initial arrayname
  x->x_arrayname = ary;

  // init textbuf
  x->textbuf = NULL;
  x->bufsize = 0;

  // create bang-on-done outlet
  outlet_new(&x->x_obj, &s_bang);

  return (void *)x;
}

static void flite_free(t_flite *x) {
  if (x->bufsize && x->textbuf != NULL) {
    freebytes(x->textbuf, x->bufsize);
    x->bufsize = 0;
  }
}

/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void flite_setup(void) {
  post("");
  post(flite_description);
  post("");

  // --- setup synth
  flite_init();
  voice = PDFLITE_REGISTER_VOICE();

  // --- register class
  flite_class = class_new(gensym("flite"),
			  (t_newmethod)flite_new,  // newmethod
			  (t_method)flite_free,    // freemethod
			  sizeof(t_flite),         // size
			  CLASS_DEFAULT,           // flags
			  A_DEFSYM,                // arg1: table-name
			  0);

  // --- class methods
  class_addlist(flite_class, flite_list);
  class_addmethod(flite_class, (t_method)flite_set,   gensym("set"),   A_DEFSYM, 0);
  class_addmethod(flite_class, (t_method)flite_text,  gensym("text"),  A_GIMME, 0);
  class_addmethod(flite_class, (t_method)flite_synth, gensym("synth"), 0);

  // --- help patch
  //class_sethelpsymbol(flite_class, gensym("flite-help.pd")); /* breaks pd-extended help lookup */
}
