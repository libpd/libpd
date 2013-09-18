/*
  (c) 2002:cxc@web.fm
  microtime: seconds since epoch plus microsecs
 */

#include <m_pd.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <time.h>

/* ----------------------- utime --------------------- */

static t_class *utime_class;

typedef struct _utime
{
  t_object x_obj;
  t_outlet *x_outlet1;
  t_outlet *x_outlet2;
} t_utime;

static void *utime_new(t_symbol *s, int argc, t_atom *argv) {
  t_utime *x = (t_utime *)pd_new(utime_class);
  x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
  x->x_outlet2 = outlet_new(&x->x_obj, &s_float);
  return (x);
}

#ifndef _WIN32
static void utime_bang(t_utime *x)
{
  struct timeval myutime;
  struct timezone mytz;
  
  gettimeofday(&myutime, &mytz);
  outlet_float(x->x_outlet2, (t_float)myutime.tv_usec);
  outlet_float(x->x_outlet1, (t_float)myutime.tv_sec);
}
#else
static void utime_bang(t_utime *x)
{
  FILETIME myfiletime;
  ULARGE_INTEGER ulfiletime, ulSec, uluSec;

  GetSystemTimeAsFileTime(&myfiletime);
  ulfiletime.LowPart = myfiletime.dwLowDateTime;
  ulfiletime.HighPart = myfiletime.dwHighDateTime;
  ulfiletime.QuadPart -= 116444736000000000; // number of 100ns ticks from 1601-01-01 to 1970-01-01
  ulSec.QuadPart = ulfiletime.QuadPart / (10 * 1000 * 1000);  // FILETIME uses 100ns ticks
  uluSec.QuadPart = (ulfiletime.QuadPart - ulSec.QuadPart * 10 * 1000 * 1000) / 10;  // FILETIME uses 100ns ticks

  outlet_float(x->x_outlet2, (t_float)(__int64)ulSec.QuadPart );
  outlet_float(x->x_outlet1, (t_float)(__int64)uluSec.QuadPart);
}
#endif

static void help_utime(t_utime *x)
{
  post("\n%c utime\t\t:: get the current system time", 70);
  post("\noutputs are\t:  seconds since epoch / remaining microseconds");
}

void utime_setup(void)
{
  utime_class = class_new(gensym("utime"),
			 (t_newmethod)utime_new, 0,
			 sizeof(t_utime), 0, A_GIMME, 0);
  
  class_addbang(utime_class, utime_bang);
  
  class_addmethod(utime_class, (t_method)help_utime, gensym("help"), 0);
}
