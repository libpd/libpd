/* 
 * sfplay: multichannel soundfile player (use [readsf~] instead)
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
sfplay.c - Author: Winfried Ritsch - IEM Graz 10.Mai 99 - 
Modified:

  Description:
  
  Soundfile player for playing many soundfiles in single speed.
  (Made for "3 Farben Schwarz" - exhibition in Graz 99 )
  
  Filename must have the path or actual directory, since pathname
  search ist not supported to garantuee a fast open call.
      
  They idea is a state machine which handles open, skip, play, close, error
  so that a minimum intervall between OS-calls are made, to avoid peak load.
        
  It has shown, that the open call is slow if there are a lot of files
  to search for, then with the first skip the first part of a
  soundfile is also loaded by the OS.		    
          
  I experimented with asynchronous buffering with paralell
  process,which has shown no much performance hit, since more
  processes has to be handled and the modern OS's do caching anyway
  also caching is done in modern hard disk, so an additional cache
  woud be an overhead, if not special behaviour is needed (big jumps
  etc).
            
  This sfplayers should be used with an appropriate audio buffer for
  good performance, so also buffering on the object is an overhead.
              
  The sfread for linux using mmap has also not much improvement over this, if plain playing in one 
  direction is done for random access the sfread should be used, even not knowing how to mmap in
  NT.
                
Todo:
  Add the SPEED feature, but therefore there should be an own external using e.g. a 4-point interpolation.
  so overhead is reduced in this one.

  Split open to an own object called sfopen to hold more soundfiles
  then players and to enable glueless switching between soundfiles.
  
please mail problems and ideas for improvements to
ritsch@iem.kug.ac.at */

/*#define DEBUG_ME // for debugging messages */

#include "zexy.h"

#define DACBLKSIZE 64 /* in m_imp.h, but error if it is included it here*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

/* ------------------------ sfplay ----------------------------- */
#define MAX_CHANS 8 /* channels for soundfiles 1,2,4,8 */

#ifndef _WIN32
# include <unistd.h>
# include <sys/mman.h>
#endif

static t_class *sfplay_class;

typedef struct _sfplay
{
   t_object x_obj;

   t_outlet *bangout;  /* end of file */

   void*   filep;      /* pointer to file data read in mem */
   t_symbol* filename; /* filename */
   /*
      because there is no command queue,
      flags are used instead 
   */
   t_int play;         /* play: 1, stop: 0 */
   t_int please_stop;  /* can be reset only by stop-state itself */ 
   t_int please_close; /* can be reset only by close-state */
   t_int x_channels;   /* channels to play */
   t_float x_offset;   /* offsetto start reading */
   t_float offset;     /* inlet value offset in secs */
   t_float x_skip;     /* skip bytes because header */
   t_int skip;         /* pending skip if 1 */
   t_float x_speed;    /* play speed, not supported in this version */
   t_int  size;        /* size of file (if memory mapped) */
   t_int  swap;        /* swap bytes from l->b or b->m */
   FILE *fp;           /* file oper non-NULL of open */
   t_int state;        /* which state is player in */
   t_int count;        /* count for ticks before next step */

} t_sfplay;

/* states of statemachine */
#define SFPLAY_WAIT    0     /* wait for open */
#define SFPLAY_OPEN    1     
#define SFPLAY_CLOSE   2
#define SFPLAY_SKIP    3
#define SFPLAY_PLAY    4
#define SFPLAY_STOP    5
#define SFPLAY_ERROR  -1

#define SFPLAY_WAITTICKS 10 /* 1 tick of 64 Samples is ca. 1.5ms on 441000 */

/* split the os-calls in as many steps as possible 
to split them on different ticks in steps of SFPLAY_WAITTICKS
to avoid peak performance */

/* like the one from garray */
static int sfplay_am_i_big_endian(void) 
{
   unsigned short s = 1;
   unsigned char c = *(char *) (&s);
   return(c==0);
}


static void sfplay_helper(t_sfplay *x)
{
  ZEXY_USEVAR(x);
	post("\nsfplay :: a soundfile-player (c) winfried ritsch 1999");
	post("\ncreation :: sfplay <channels> <bytes> : channels set the number of channels, bytes skip fileheader");
	post("\nopen [<path>]<filename> [<endianity>]\t::open b(ig) or l(ittle) endian file"
		"\nclose\t\t\t::close file (aka eject)"
		"\nstart\t\t\t::start playing"
		"\nstop\t\t\t::stop playing"
		"\nrewind\t\t\t::rewind tape"
		"\ngoto <n>\t\t::play from byte n");
	post("\n\nyou can also start playing with a ŽbangŽ or a Ž1Ž, and stop with a Ž0Ž"
		"\nthe last outlet will do a bang after the last sample has been played");

}


/* METHOD: "open" file */

/* this dont use memory map, because I dont know about this on NT ? 
Use of the buffered functions fopen, fseek fread fclose instead the 
non buffered ones open read close */

static void sfplay_open(t_sfplay *x,t_symbol *filename,t_symbol *endian)
{

   if(x->state != SFPLAY_WAIT)
   {  
      post("sfplay: first close %s before open %s",x->filename->s_name,filename->s_name);
      return;
   }
 
/* test if big endian else asume little endian 
   should be 'l' but could be anything*/
   
   if(sfplay_am_i_big_endian())
      x->swap = !(endian->s_name[0] == 'b');
   else 
      x->swap = (endian->s_name[0] == 'b');
   
   x->skip = 1; /* skip header after open */
 
   x->filename = filename;
   
#ifdef DEBUG_ME
   post("sfplay: filename = %s",x->filename->s_name);
#endif
   
   if (x->fp != NULL)fclose(x->fp); /* should not happen */
   
   if (!(x->fp = sys_fopen(x->filename->s_name,"r"))) 
   {
      error("sfplay: can't open %s", x->filename->s_name);
   }
}



/* METHOD: close */
static void sfplay_close(t_sfplay *x)
{
   x->play = 0;
   x->please_close = 1;

   /* now in state machine 
   if(x->fp != NULL)
   {
      fclose(x->fp);
      x->fp = NULL;
   }
   */

#ifdef DEBUG_ME
   post("sfplay: close ");
#endif
   return;
}

/* for skipping header of soundfile  Dont use this for memory map */

static int sfplay_skip(t_sfplay *x)
{
   if(!x->skip) return 0;
   
   x->skip = 0;
   
   if(fseek(x->fp, (long) x->x_offset, SEEK_SET) < 0)
   {	
      error(" sfplay can't seek to byte %ld",(long) x->x_offset);
      x->x_offset = x->x_skip;
      x->skip = 1;
      return 0;
   }
   
#ifdef DEBUG_ME
   post("sfplay:skip to %f",x->x_offset);
#endif
   return 1;
}

/* Input, method for Start stop */

static void sfplay_start(t_sfplay *x)
{
   long of = x->offset * sys_getsr() * x->x_channels;
   
   if(of < 0) of = x->x_skip;
   else of += x->x_skip; /* offset in sec */

   of &= ~0x111l; /* no odds please (8 channels boundary) */
   
#ifdef DEBUG_ME
   post("sfplay: start");
#endif
   
   /* new offset postion ? (fom inlet offset) */
   if( ((t_float) of) != x->x_offset)
   {
      x->skip=1;
      x->x_offset = of;
   }
   x->play=1;
}

static void sfplay_stop(t_sfplay *x)
{
#ifdef DEBUG_ME    
   post("sfplay: stop");
#endif

   x->play=0;
   x->please_stop = 1;
}

static void sfplay_float(t_sfplay *x, t_floatarg f)
{
   int t = f;
   if (t) sfplay_start(x);
   else sfplay_stop(x);
}

/* start playing at position offset*/
static void sfplay_offset(t_sfplay *x, t_floatarg f)
{
   x->offset = f;
   x->skip = 1;
   /* correction in sfplay_play() */
   
#ifdef DEBUG_ME
   post("sfplay: offset %f",f);
#endif
	return;
}

static void sfplay_rewind(t_sfplay *x)
{
#ifdef DEBUG_ME
   post("sfplay: rewind to %f",x->x_skip);	
#endif
   
   if(!x->fp)return;
   
   x->play=0;
   fseek(x->fp,(long) x->x_skip,SEEK_SET);
}

/* restart with bang */

static void sfplay_bang(t_sfplay* x)
{
   x->skip = 1;
   sfplay_start(x);
}

static t_int *sfplay_perform(t_int *w)
{
   t_sfplay* x = (t_sfplay*)(w[1]);
   short* buf = x->filep;
   int c = x->x_channels;

   int i,j,n;
   t_float* out[MAX_CHANS];

   short s;
   int swap = x->swap;
   
   for (i=0;i<c;i++)  
      out[i] = (t_float *)(w[3+i]);

   n = (int)(w[3+c]);
   
   /* loop */
   
   
   switch(x->state){
      
      /* just wait */
   case SFPLAY_WAIT:
      
      if(x->fp != NULL){
#ifdef DEBUG_ME
         post("wait -> open");
#endif
         x->state = SFPLAY_OPEN;
         x->count = SFPLAY_WAITTICKS;
      };
      break;
      
      /* if in open state, already opened but wait for skip */
   case SFPLAY_OPEN: /* file hase opened wait some time */
      
      if(!(x->count--)){
#ifdef DEBUG_ME
         post("open -> skip");
#endif
         x->state = SFPLAY_SKIP;
         x->count = SFPLAY_WAITTICKS;
      };
      
      break;
      
      
      /* in skipmode wait until ready for stop */
   case SFPLAY_SKIP:
      
      
      if(x->count == SFPLAY_WAITTICKS)
      {
         if(!x->fp)
         {
            x->state = SFPLAY_CLOSE;
            x->count=1;
#ifdef DEBUG_ME
            post("skip -> close");
#endif
            break;
         }
         sfplay_skip(x);
      }
      if(!(x->count--))
      {
#ifdef DEBUG_ME
         post("skip -> stop");
#endif
         x->state = SFPLAY_STOP;
         x->count = SFPLAY_WAITTICKS;
      };
      break;
      
      
                    
   case SFPLAY_STOP:   /* in stop state mainly waits for play */
      
      x->please_stop = 0;

      if(x->please_close)
      {
         x->state = SFPLAY_CLOSE;
         x->count = SFPLAY_WAITTICKS;      
#ifdef DEBUG_ME
         post("stop -> close");
#endif
      }
      else if(x->skip)
      {
         x->state = SFPLAY_SKIP;
         x->count = SFPLAY_WAITTICKS;

#ifdef DEBUG_ME
         post("stop -> skip");
#endif

      }
      else if(x->play)
      {

#ifdef DEBUG_ME
         post("stop -> play");
#endif
         x->state = SFPLAY_PLAY;
      }
      break;
      
     
   case SFPLAY_PLAY:             /* yes play now */
      
      
      if(!x->play || x->please_stop)
      {	 

         /* if closing dont need o go to stop */
         if(x->please_close)
         { 
            x->state = SFPLAY_CLOSE;
            x->count = SFPLAY_WAITTICKS;
#ifdef DEBUG_ME
            post("play -> close");
#endif
         } 
         else
         { 
            x->state = SFPLAY_STOP;   
#ifdef DEBUG_ME
            post("play -> stop");
#endif
         };
         break;
      }
      
      /* should never happen */
      if(!x->filep){
         x->state = SFPLAY_ERROR;
         error("sfplay: playing but no buffer ???? play");
         return (w+4+c);
      }
      
      /* first read soundfile 16 bit*/		 
      if((j=fread(buf,sizeof(short),c*n,x->fp)) < n)
      {   

		  outlet_bang(x->bangout);
		  
		  if(feof(x->fp)){

			 while (n--) {
				for (i=0;i<c;i++)  {
				  if(--j > 0){
					 s = *buf++;
					 if(swap) s = ((s & 0xFF)<< 8) | ((s& 0xFF00) >> 8);
					 *out[i]++ = s*(1./32768.);
				  }
				  else *out[i]++ = 0;
				}
			 }

			 x->state = SFPLAY_STOP;
			 x->play = 0;
			 return(w+c+4);
		  }
         
		  /* or error if(ferror()) */
		  x->state = SFPLAY_ERROR;
		  x->count = SFPLAY_WAITTICKS;

#ifdef DEBUG_ME
		  post("play -> read error");
#endif
		  break;
      };
      
      /* copy 16 Bit to floats and swap if neccesairy */
      while (n--) {
         for (i=0;i<c;i++)  {
            s = *buf++;
            if(swap) s = ((s & 0xFF)<< 8) | ((s& 0xFF00) >> 8);
            *out[i]++ = s*(1./32768.);
         }
      }
      return (w+c+4); /* dont zero out outs */
      
      /* ok read error please close */
   case SFPLAY_ERROR:
      
      if(!(x->count--)){
         x->state = SFPLAY_CLOSE;
         sfplay_close(x);
#ifdef DEBUG_ME
         post("sfplay error reading sf: error -> close");
#endif
         x->count = SFPLAY_WAITTICKS;
      }
      break;
      
      /* in close state go to wait afterwards */
   case SFPLAY_CLOSE:
      
      x->please_close = 0;

      /* wait until ready for close operation */
      if(!(x->count--)){ 

         x->state = SFPLAY_WAIT;
         x->count = SFPLAY_WAITTICKS;
         
         /* avoid openfiles */
         if(x->fp){fclose(x->fp);x->fp = NULL;};

#ifdef DEBUG_ME
         post("sfplay: close -> wait");
#endif
      }
      break;
      
   }; /*case */
   
   /* zero out outs */
   while (n--) {
	     for (i=0;i<c;i++)
           *out[i]++ = 0.;
	  };
   
   return(w+c+4);
}


/* ---------------------- Setup junk -------------------------- */

static void sfplay_dsp(t_sfplay *x, t_signal **sp)
{

#ifdef DEBUG_ME
   post("sfplay: dsp");
#endif

   switch (x->x_channels) {
   case 1:
      dsp_add(sfplay_perform, 4, x,
         sp[0]->s_vec, 
         sp[1]->s_vec, /* out 1 */
         sp[0]->s_n);
      break;
   case 2:
      dsp_add(sfplay_perform, 5, x, 
         sp[0]->s_vec, /* out 1*/
         sp[1]->s_vec, /* out 2*/
         sp[2]->s_vec, 
         sp[0]->s_n);
      break;
   case 4:
      dsp_add(sfplay_perform, 7, x, 
         sp[0]->s_vec, 
         sp[1]->s_vec,
         sp[2]->s_vec,
         sp[3]->s_vec,
         sp[4]->s_vec,
         sp[0]->s_n);
      break;
   case 8:
        dsp_add(sfplay_perform, 11, x, 
           sp[0]->s_vec, 
           sp[1]->s_vec,
           sp[2]->s_vec,
           sp[3]->s_vec,
           sp[4]->s_vec,
           sp[5]->s_vec,
           sp[6]->s_vec,
           sp[7]->s_vec,
           sp[8]->s_vec,
           sp[0]->s_n);
        break;
   }
}


/* create sfplay with args <channels> <skip> */
static void *sfplay_new(t_floatarg chan,t_floatarg skip)
{
   t_sfplay *x = (t_sfplay *)pd_new(sfplay_class);
   t_int c = chan;
   
   switch(c){
      /* ok */
   case 1: case 2: case 4: case 8: break;
      /* try it, good luck ... */
   case 3: c = 2; break;     
   case 5: case 6: case 7: c=7; break;
   default: c=1; break;
   }

   floatinlet_new(&x->x_obj, &x->offset); /* inlet 2 */
   /*    floatinlet_new(&x->x_obj, &x->speed);  *//* inlet 3 */
   
   x->x_channels = c;
   x->x_skip = x->x_offset = skip;
   x->offset = 0.;
   x->skip = 1;
   x->x_speed = 1.0;
   x->play = 0;
   x->please_stop = 0;
   x->please_close = 0;
   x->state = SFPLAY_WAIT;
   x->count = 0;
   x->filename = NULL;
   x->fp = NULL;
   x->swap = 1;
   
   while (c--) {
      outlet_new(&x->x_obj, gensym("signal")); /* channels outlet */
   }
   x->bangout = outlet_new(&x->x_obj,  gensym("bang"));
    
   x->filep = t_getbytes(DACBLKSIZE*sizeof(short)*x->x_channels);
   
#ifdef DEBUG_ME
   post("get_bytes DACBLKSIZE*%d*%d->%ld",sizeof(short),x->x_channels,x->filep);
   post("sfplay: x_channels = %d, x_speed = %f, x_skip = %f",x->x_channels,x->x_speed,x->x_skip);
#endif
   
   return (x);
}


static void sfplay_free(t_sfplay *x)
{
   freebytes(x->filep, DACBLKSIZE*sizeof(short)*x->x_channels);
}

void sfplay_setup(void)
{
   sfplay_class = class_new(gensym("sfplay"), (t_newmethod)sfplay_new, (t_method)sfplay_free,
      sizeof(t_sfplay), 0, A_DEFFLOAT, A_DEFFLOAT,0);
   class_addmethod(sfplay_class, nullfn, gensym("signal"), 0);
   class_addmethod(sfplay_class, (t_method)sfplay_dsp, gensym("dsp"), 0);

   class_addmethod(sfplay_class, (t_method)sfplay_helper, gensym("help"), A_NULL);
   class_sethelpsymbol(sfplay_class, gensym("sf-play_record"));

   /* method open with filename */
   class_addmethod(sfplay_class, (t_method)sfplay_open, gensym("open"), A_SYMBOL,A_SYMBOL,A_NULL);
   class_addmethod(sfplay_class, (t_method)sfplay_close, gensym("close"), A_NULL);
   
   class_addmethod(sfplay_class, (t_method)sfplay_start, gensym("start"), A_NULL);
   class_addmethod(sfplay_class, (t_method)sfplay_stop,  gensym("stop"), A_NULL);
   class_addmethod(sfplay_class, (t_method)sfplay_rewind, gensym("rewind"), A_NULL);
   class_addmethod(sfplay_class, (t_method)sfplay_offset, gensym("goto"), A_DEFFLOAT, A_NULL);

   /* start stop with 0 and 1 */
   class_addfloat(sfplay_class, sfplay_float);
   /* start with bang */
   class_addbang(sfplay_class,sfplay_bang);
  zexy_register("sfplay");
}
