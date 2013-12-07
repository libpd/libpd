/******************************************************
 *
 * audiosettings - get/set audio preferences from within Pd-patches
 * Copyright (C) 2010-2012 IOhannes m zmölnig
 *
 *   forum::für::umläute
 *
 *   institute of electronic music and acoustics (iem)
 *   university of music and dramatic arts, graz (kug)
 *
 *
 ******************************************************
 *
 * license: GNU General Public License v.3 or later
 *
 ******************************************************/
#include "mediasettings.h"

#define MAXAUDIOINDEV 4
#define MAXAUDIOOUTDEV 4

static void as_get_audio_params(
    int *pnaudioindev, int *paudioindev, int *pchindev,
    int *pnaudiooutdev, int *paudiooutdev, int *pchoutdev,
    int *prate, int *padvance, int *pcallback, int *pblocksize) {
#if (defined PD_MINOR_VERSION) && (PD_MINOR_VERSION >= 43)
  sys_get_audio_params(pnaudioindev , paudioindev , pchindev,
                       pnaudiooutdev, paudiooutdev, pchoutdev,
                       prate, padvance, pcallback, pblocksize);
#else
  if(pblocksize)
   *pblocksize=-1;

  sys_get_audio_params(pnaudioindev , paudioindev , pchindev,
                       pnaudiooutdev, paudiooutdev, pchoutdev,
                       prate, padvance, pcallback);


#endif



}




static t_class *audiosettings_class;

t_symbol*s_pdsym=NULL;

typedef struct _as_drivers {
  t_symbol*name;
  int      id;

  struct _as_drivers *next;
} t_as_drivers;

typedef struct _as_params {
  int naudioindev, audioindev[MAXAUDIOINDEV], chindev[MAXAUDIOINDEV];
  int naudiooutdev, audiooutdev[MAXAUDIOOUTDEV], choutdev[MAXAUDIOOUTDEV];
  int rate, advance, callback;  
  int blocksize;
} t_as_params;

t_as_drivers*as_finddriver(t_as_drivers*drivers, const t_symbol*name) {
  while(drivers) {
    if(name==drivers->name)return drivers;
    drivers=drivers->next;
  }
  return NULL;
}

t_as_drivers*as_finddriverid(t_as_drivers*drivers, const int id) {
  while(drivers) {
    if(id==drivers->id)return drivers;
    drivers=drivers->next;
  }
  return NULL;
}

t_as_drivers*as_adddriver(t_as_drivers*drivers, t_symbol*name, int id, int overwrite) {
  t_as_drivers*driver=as_finddriver(drivers, name);

  if(driver) {
    if(overwrite) {
      driver->name=name;
      driver->id  =id;
    }
    return drivers;
  }

  driver=(t_as_drivers*)getbytes(sizeof(t_as_drivers));
  driver->name=name;
  driver->id=id;
  driver->next=drivers;

  return driver;
}

t_as_drivers*as_driverparse(t_as_drivers*drivers, const char*buf) {
  int start=-1;
  int stop =-1;

  unsigned int index=0;
  int depth=0;
  const char*s;
  char substring[MAXPDSTRING];

  for(index=0, s=buf; 0!=*s; s++, index++) {
    if('{'==*s) {
      start=index;
      depth++;
    }
    if('}'==*s) {
      depth--;
      stop=index;

      if(start>=0 && start<stop) {
        char drivername[MAXPDSTRING];
        int  driverid;
        int length=stop-start;
        if(length>=MAXPDSTRING)length=MAXPDSTRING-1;
        snprintf(substring, length, "%s", buf+start+1);
        if(common_parsedriver(substring, length,
                              drivername, MAXPDSTRING,
                              &driverid)) {
          drivers=as_adddriver(drivers, gensym(drivername), driverid, 0);
        } else {
          if((start+1)!=(stop))
            post("unparseable: '%s' (%d-%d)", substring, start, stop);
        }
      }
      start=-1;
      stop=-1;
    }
  }

  return drivers;
}

static t_as_drivers*DRIVERS=NULL;

static t_symbol*as_getdrivername(const int id) {
  t_as_drivers*driver=as_finddriverid(DRIVERS, id);
  if(driver) {
    return driver->name;
  } else {
    return gensym("<unknown>");
  }
}

static int as_getdriverid(const t_symbol*id) {
  t_as_drivers*driver=as_finddriver(DRIVERS, id);
  if(driver) {
    return driver->id;
  }
  return -1; /* unknown */
}



static void as_params_print(t_as_params*parms) {
  int i=0;
  post("\n=================================<");

  post("indevs: %d", parms->naudioindev);
  for(i=0; i<MAXAUDIOINDEV; i++) {
    post("indev: %d %d", parms->audioindev[i], parms->chindev[i]);
  }
  post("outdevs: %d", parms->naudiooutdev);
  for(i=0; i<MAXAUDIOOUTDEV; i++) {
    post("outdev: %d %d", parms->audiooutdev[i], parms->choutdev[i]);
  }
  post("rate=%d", parms->rate);
  post("advance=%d", parms->advance);
  post("callback=%d", parms->callback);
  post(">=================================\n");

}
static void as_params_get(t_as_params*parms) {
  int i=0;
  memset(parms, 0, sizeof(t_as_params));
  parms->callback=-1;

  as_get_audio_params(	&parms->naudioindev,  parms->audioindev,  parms->chindev,
                       	&parms->naudiooutdev, parms->audiooutdev, parms->choutdev, 
                       	&parms->rate,        &parms->advance,
			&parms->callback,    &parms->blocksize);

  for(i=parms->naudioindev; i<MAXAUDIOINDEV; i++) {
    parms->audioindev[i]=0;
    parms->chindev[i]=0;
  }
  for(i=parms->naudiooutdev; i<MAXAUDIOOUTDEV; i++) {
    parms->audiooutdev[i]=0;
    parms->choutdev[i]=0;
  }


//  as_params_print(parms);
}



typedef struct _audiosettings
{
  t_object x_obj;
  t_outlet*x_info;


  t_as_params x_params;
} t_audiosettings;


static void audiosettings_listparams(t_audiosettings *x);
static void audiosettings_listdevices(t_audiosettings *x)
{
  int i;

  char indevlist[MAXNDEV][DEVDESCSIZE], outdevlist[MAXNDEV][DEVDESCSIZE];
  int indevs = 0, outdevs = 0, canmulti = 0, cancallback = 0;

  t_atom atoms[3];
 
  sys_get_audio_devs((char*)indevlist, &indevs, 
                     (char*)outdevlist, &outdevs, 
                     &canmulti, &cancallback, 
                     MAXNDEV, DEVDESCSIZE);

  SETSYMBOL (atoms+0, gensym("driver"));
  SETSYMBOL (atoms+1, as_getdrivername(sys_audioapi));
  outlet_anything(x->x_info, gensym("device"), 2, atoms);

  SETSYMBOL (atoms+0, gensym("multi"));
  SETFLOAT (atoms+1, (t_float)canmulti);
  outlet_anything(x->x_info, gensym("device"), 2, atoms);

  SETSYMBOL (atoms+0, gensym("callback"));
  SETFLOAT (atoms+1, (t_float)cancallback);
  outlet_anything(x->x_info, gensym("device"), 2, atoms);

  SETSYMBOL(atoms+0, gensym("in"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)indevs);
  outlet_anything(x->x_info, gensym("device"), 3, atoms);

  for(i=0; i<indevs; i++) {
    SETFLOAT (atoms+1, (t_float)i);
    SETSYMBOL(atoms+2, gensym(indevlist[i]));
    outlet_anything(x->x_info, gensym("device"), 3, atoms);
  }

  SETSYMBOL(atoms+0, gensym("out"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)outdevs);
  outlet_anything(x->x_info, gensym("device"), 3, atoms);

  for(i=0; i<outdevs; i++) {
    SETFLOAT (atoms+1, (t_float)i);
    SETSYMBOL(atoms+2, gensym(outdevlist[i]));
    outlet_anything(x->x_info, gensym("device"), 3, atoms);
  }
}

/* this is the actual settings used
 *
 */
static void audiosettings_listparams(t_audiosettings *x) {
  int i;
  t_atom atoms[4];

  t_as_params params;
  as_params_get(&params);

  SETSYMBOL (atoms+0, gensym("rate"));
  SETFLOAT (atoms+1, (t_float)params.rate);
  outlet_anything(x->x_info, gensym("params"), 2, atoms);

  SETSYMBOL (atoms+0, gensym("advance"));
  SETFLOAT (atoms+1, (t_float)params.advance);
  outlet_anything(x->x_info, gensym("params"), 2, atoms);

  SETSYMBOL (atoms+0, gensym("callback"));
  SETFLOAT (atoms+1, (t_float)params.callback);
  outlet_anything(x->x_info, gensym("params"), 2, atoms);

  SETSYMBOL(atoms+0, gensym("in"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)params.naudioindev);
  outlet_anything(x->x_info, gensym("params"), 3, atoms);

  for(i=0; i<params.naudioindev; i++) {
    SETFLOAT (atoms+1, (t_float)params.audioindev[i]);
    SETFLOAT (atoms+2, (t_float)params.chindev[i]);
    outlet_anything(x->x_info, gensym("params"), 3, atoms);
  }

  SETSYMBOL(atoms+0, gensym("out"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)params.naudiooutdev);
  outlet_anything(x->x_info, gensym("params"), 3, atoms);

  for(i=0; i<params.naudiooutdev; i++) {
    SETFLOAT (atoms+1, (t_float)params.audiooutdev[i]);
    SETFLOAT (atoms+2, (t_float)params.choutdev[i]);
    outlet_anything(x->x_info, gensym("params"), 3, atoms);
  }
}


static void audiosettings_params_init(t_audiosettings*x) {
  as_params_get(&x->x_params);
}
static void audiosettings_params_apply(t_audiosettings*x) {
/*
     "pd audio-dialog ..."
     #00: indev[0]
     #01: indev[1]
     #02: indev[2]
     #03: indev[3]
     #04: inchan[0]
     #05: inchan[1]
     #06: inchan[2]
     #07: inchan[3]
     #08: outdev[0]
     #09: outdev[1]
     #10: outdev[2]
     #11: outdev[3]
     #12: outchan[0]
     #13: outchan[1]
     #14: outchan[2]
     #15: outchan[3]
     #16: rate
     #17: advance
     #18: callback
*/

  t_atom argv [2*MAXAUDIOINDEV+2*MAXAUDIOOUTDEV+3];
  int argc=2*MAXAUDIOINDEV+2*MAXAUDIOOUTDEV+3;

  int i=0;

//  as_params_print(&x->x_params);



  for(i=0; i<MAXAUDIOINDEV; i++) {
    SETFLOAT(argv+i+0*MAXAUDIOINDEV, (t_float)(x->x_params.audioindev[i]));
    SETFLOAT(argv+i+1*MAXAUDIOINDEV, (t_float)(x->x_params.chindev   [i]));
  }
  for(i=0; i<MAXAUDIOOUTDEV; i++) {
    SETFLOAT(argv+i+2*MAXAUDIOINDEV+0*MAXAUDIOOUTDEV,(t_float)(x->x_params.audiooutdev[i]));
    SETFLOAT(argv+i+2*MAXAUDIOINDEV+1*MAXAUDIOOUTDEV,(t_float)(x->x_params.choutdev   [i]));
  }

  SETFLOAT(argv+2*MAXAUDIOINDEV+2*MAXAUDIOOUTDEV+0,(t_float)(x->x_params.rate));
  SETFLOAT(argv+2*MAXAUDIOINDEV+2*MAXAUDIOOUTDEV+1,(t_float)(x->x_params.advance));
  SETFLOAT(argv+2*MAXAUDIOINDEV+2*MAXAUDIOOUTDEV+2,(t_float)(x->x_params.callback));

  if (s_pdsym->s_thing) typedmess(s_pdsym->s_thing, 
				  gensym("audio-dialog"), 
				  argc,
				  argv);
}


/* find the beginning of the next parameter in the list */
typedef enum {
  PARAM_RATE,
  PARAM_ADVANCE,
  PARAM_CALLBACK,
  PARAM_INPUT,
  PARAM_OUTPUT,
  PARAM_INVALID
} t_paramtype;
static t_paramtype audiosettings_setparams_id(t_symbol*s) {
  if(gensym("@rate")==s) {
    return PARAM_RATE;
  } else if(gensym("@samplerate")==s) {
    return PARAM_RATE;
  } else if(gensym("@advance")==s) {
    return PARAM_ADVANCE;
  } else if(gensym("@buffersize")==s) {
    return PARAM_ADVANCE;
  } else if(gensym("@callback")==s) {
    return PARAM_CALLBACK;
  } else if(gensym("@input")==s) {
    return PARAM_INPUT;
  } else if(gensym("@output")==s) {
    return PARAM_OUTPUT;
  }
  return PARAM_INVALID;
}

/* find the beginning of the next parameter in the list */
static int audiosettings_setparams_next(int argc, t_atom*argv) {
  int i=0;
  for(i=0; i<argc; i++) {
    if(A_SYMBOL==argv[i].a_type) {
      t_symbol*s=atom_getsymbol(argv+i);
      if('@'==s->s_name[0])
	return i;
    }
  }
  return i;
}

/* <rate> ... */
static int audiosettings_setparams_rate(t_audiosettings*x, int argc, t_atom*argv) {
  if(argc<=0)return 1;
  t_int rate=atom_getint(argv);
  if(rate>0)
    x->x_params.rate=rate;
  return 1;
}

/* <advance> ... */
static int audiosettings_setparams_advance(t_audiosettings*x, int argc, t_atom*argv) {
  if(argc<=0)return 1;
  t_int advance=atom_getint(argv);
  if(advance>0)
    x->x_params.advance=advance;

  return 1;
}

/* <callback?> ... */
static int audiosettings_setparams_callback(t_audiosettings*x, int argc, t_atom*argv) {
  if(argc<=0)return 1;
  t_int callback=atom_getint(argv);
  x->x_params.callback=callback;

  return 1;
}

/* [<device> <channels>]* ... */
static int audiosettings_setparams_input(t_audiosettings*x, int argc, t_atom*argv) {
  int length=audiosettings_setparams_next(argc, argv);
  int i;
  int numpairs=length/2;

  if(length%2)return length;

  if(numpairs>MAXAUDIOINDEV)
    numpairs=MAXAUDIOINDEV;

  for(i=0; i<numpairs; i++) {
    int dev=0;
    int ch=0;

    if(A_FLOAT==argv[2*i+0].a_type) {
      dev=atom_getint(argv);
    } else if (A_SYMBOL==argv[2*i+0].a_type) {
      // LATER: get the device-id from the device-name
      continue;
    } else {
      continue;
    }
    ch=atom_getint(argv+2*i+1);

    x->x_params.audioindev[i]=dev;
    x->x_params.chindev[i]=ch;
  }

  return length;
}

static int audiosettings_setparams_output(t_audiosettings*x, int argc, t_atom*argv) {
  int length=audiosettings_setparams_next(argc, argv);
  int i;
  int numpairs=length/2;

  if(length%2)return length;

  if(numpairs>MAXAUDIOOUTDEV)
    numpairs=MAXAUDIOOUTDEV;

  for(i=0; i<numpairs; i++) {
    int dev=0;
    int ch=0;

    if(A_FLOAT==argv[2*i+0].a_type) {
      dev=atom_getint(argv);
    } else if (A_SYMBOL==argv[2*i+0].a_type) {
      // LATER: get the device-id from the device-name
      continue;
    } else {
      continue;
    }
    ch=atom_getint(argv+2*i+1);

    x->x_params.audiooutdev[i]=dev;
    x->x_params.choutdev[i]=ch;
  }

  return length;
}

static void audiosettings_setparams(t_audiosettings *x, t_symbol*s, int argc, t_atom*argv) {
/*
  PLAN:
    several messages that accumulate to a certain settings, and then "apply" them
*/
  int apply=1;
  int advance=0;
  t_paramtype param=PARAM_INVALID;

  audiosettings_params_init (x); /* re-initialize to what we got */

  advance=audiosettings_setparams_next(argc, argv);
  while((argc-=advance)>0) {
    argv+=advance;
    s=atom_getsymbol(argv);
    param=audiosettings_setparams_id(s);

    argv++;
    argc--;

    switch(param) {
    case PARAM_RATE:
      advance=audiosettings_setparams_rate(x, argc, argv);
      break;
    case PARAM_ADVANCE:
      advance=audiosettings_setparams_advance(x, argc, argv);
      break;
    case PARAM_CALLBACK:
      advance=audiosettings_setparams_callback(x, argc, argv);
      break;
    case PARAM_INPUT:
      advance=audiosettings_setparams_input(x, argc, argv);
      break;
    case PARAM_OUTPUT:
      advance=audiosettings_setparams_output(x, argc, argv);
      break;
    default:
      pd_error(x, "unknown parameter"); postatom(1, argv);endpost();
      break;
    }

    argc-=advance;
    argv+=advance;
    advance=audiosettings_setparams_next(argc, argv);
  }
  if(apply) {
    audiosettings_params_apply(x);
  }
}

static void audiosettings_testdevices(t_audiosettings *x);


/*
 */
static void audiosettings_listdrivers(t_audiosettings *x)
{
  t_as_drivers*driver=NULL;
  t_atom ap[2];

  for(driver=DRIVERS; driver; driver=driver->next) {
    SETSYMBOL(ap+0, driver->name);
    SETFLOAT (ap+1, (t_float)(driver->id));
    outlet_anything(x->x_info, gensym("driver"), 2, ap);    
  }
}

static void audiosettings_setdriver(t_audiosettings *x, t_symbol*s, int argc, t_atom*argv) {
  int id=-1;
  s=gensym("<unknown>"); /* just re-use the argument, which is not needed anyhow */
  switch(argc) {
  case 0:
    audiosettings_listdrivers(x);
    return;
  case 1:
    if(A_FLOAT==argv->a_type) {
      s=as_getdrivername(atom_getint(argv));
      break;
    } else if (A_SYMBOL==argv->a_type) {
      s=atom_getsymbol(argv);
      break;
    }
  }

  id=as_getdriverid(s);
  if(id<0) {
    pd_error(x, "invalid driver '%s'", s->s_name);
    return;
  }
  verbose(1, "setting driver '%s' (=%d)", s->s_name, id);
#ifdef HAVE_SYS_CLOSE_AUDIO
  sys_close_audio();
  sys_set_audio_api(id);
  sys_reopen_audio();
#else
  if (s_pdsym->s_thing) {
    t_atom ap[1];
    SETFLOAT(ap, id);
    typedmess(s_pdsym->s_thing, 
              gensym("audio-setapi"), 
              1,
              ap);
  }
#endif
}

static void audiosettings_bang(t_audiosettings *x) {
  audiosettings_listdrivers(x);
  audiosettings_listdevices(x);
  audiosettings_listparams(x);
}


static void audiosettings_free(t_audiosettings *x){

}


static void *audiosettings_new(void)
{
  t_audiosettings *x = (t_audiosettings *)pd_new(audiosettings_class);
  x->x_info=outlet_new(&x->x_obj, 0);

  char buf[MAXPDSTRING];
  sys_get_audio_apis(buf);

  DRIVERS=as_driverparse(DRIVERS, buf);
  audiosettings_params_init (x); 
  return (x);
}


void audiosettings_setup(void)
{
  s_pdsym=gensym("pd");

  logpost(NULL, 4, "audiosettings: audio settings manager");
  logpost(NULL, 4, "          Copyright (C) 2010 IOhannes m zmoelnig");
  logpost(NULL, 4, "          for the IntegraLive project");
  logpost(NULL, 4, "          institute of electronic music and acoustics (iem)");
  logpost(NULL, 4, "          published under the GNU General Public License version 3 or later");
  logpost(NULL, 4, "          compiled: "__DATE__"");

#if (defined PD_MINOR_VERSION) && (PD_MINOR_VERSION < 43)
  if(1) {
    int major, minor, bugfix;
    sys_getversion(&major, &minor, &bugfix);
    if(0==major && minor>=43) {
    error("[audiosettings] have been compiled against an old version of Pd");
    error("                that is incompatible with the one you are using!");
    error("                recompile [audiosettings]");
    }
    return;
  }
#endif

  audiosettings_class = class_new(gensym("audiosettings"), (t_newmethod)audiosettings_new, (t_method)audiosettings_free,
			     sizeof(t_audiosettings), 0, 0);

  class_addbang(audiosettings_class, (t_method)audiosettings_bang);
  class_addmethod(audiosettings_class, (t_method)audiosettings_listdrivers, gensym("listdrivers"), A_NULL);
  class_addmethod(audiosettings_class, (t_method)audiosettings_listdevices, gensym("listdevices"), A_NULL);
  class_addmethod(audiosettings_class, (t_method)audiosettings_listparams, gensym("listparams"), A_NULL);


  class_addmethod(audiosettings_class, (t_method)audiosettings_setdriver, gensym("driver"), A_GIMME, A_NULL);
  class_addmethod(audiosettings_class, (t_method)audiosettings_setparams, gensym("params"), A_GIMME, A_NULL);

  class_addmethod(audiosettings_class, (t_method)audiosettings_testdevices, gensym("testdevices"), A_NULL);

}


static void audiosettings_testdevices(t_audiosettings *x)
{
  int i;

  char indevlist[MAXNDEV][DEVDESCSIZE], outdevlist[MAXNDEV][DEVDESCSIZE];
  int indevs = 0, outdevs = 0, canmulti = 0, cancallback = 0;

  if(0) {
    pd_error(x, "this should never happen");
  }
 
  sys_get_audio_devs((char*)indevlist, &indevs, (char*)outdevlist, &outdevs, &canmulti,
                &cancallback, MAXNDEV, DEVDESCSIZE);

  post("%d indevs", indevs);
  for(i=0; i<indevs; i++)
    post("\t#%02d: %s", i, indevlist[i]);

  post("%d outdevs", outdevs);
  for(i=0; i<outdevs; i++)
    post("\t#%02d: %s", i, outdevlist[i]);

  post("multi: %d\tcallback: %d", canmulti, cancallback);

  endpost();

  int naudioindev, audioindev[MAXAUDIOINDEV], chindev[MAXAUDIOINDEV];
  int naudiooutdev, audiooutdev[MAXAUDIOOUTDEV], choutdev[MAXAUDIOOUTDEV];
  int rate, advance, callback, blocksize;
  as_get_audio_params(&naudioindev, audioindev, chindev,
                       &naudiooutdev, audiooutdev, choutdev, 
                       &rate, &advance, &callback, &blocksize);
  
  post("%d audioindev (parms)", naudioindev);
  for(i=0; i<naudioindev; i++) {
    post("\t#%02d: %d %d", i, audioindev[i], chindev[i]);
  }
  post("%d audiooutdev (parms)", naudiooutdev);
  for(i=0; i<naudiooutdev; i++) {
    post("\t#%02d: %d %d", i, audiooutdev[i], choutdev[i]);
  }
  post("rate=%d\tadvance=%d\tcallback=%d\tblocksize=%d", rate, advance, callback, blocksize);

}
