/* 
 * sfrecord: multichannel soundfile recorder (try [writesf~] instead)
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
sfrecord.c - hacked from sfplay ::: 2308:forum::für::umläute:1999 @ iem

please mail problems and ideas for improvements to
ritsch@iem.kug.ac.at
zmoelnig@iem.kug.ac.at
*/

/* TODO: deprecate this in favour of [writesf~] */

/* #define DEBUG_ME for debugging messages */

#include "zexy.h"


/* #include "m_imp.h" */

#define DACBLKSIZE 64	/* in m_imp.h, but error if it is included it here*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

/* ------------------------ sfrecord ----------------------------- */
#define MAX_CHANS 8		/* channels for soundfiles 1,2,4,8 */

#ifndef _WIN32
# include <unistd.h>
# include <sys/mman.h>
#endif

static t_class *sfrecord_class;

typedef struct _sfrecord
{
	t_object x_obj;

	void*   filep;      /* pointer to file data read in mem */
	t_symbol* filename; /* filename */

	/*
		because there is no command queue,
		flags are used instead 
	*/
	t_int write;		/* write: 1, stop: 0 */
	t_int please_stop;	/* can be reset only by stop-state itself */ 
	t_int please_close;	/* can be reset only by close-state */
	t_int x_channels;	/* channels to write */
	t_float x_offset;	/* offset to start writing */
	t_float offset;		/* inlet value offset in secs */
	t_float x_skip;		/* skip bytes because header */
	t_int skip;			/* pending skip if 1 */
	t_float x_speed;	/* write speed, not supported in this version */
	t_int size;			/* size of file (if memory mapped) */
	t_int swap;			/* swap bytes from l->b or b->m */
	FILE *fp;			/* file oper non-NULL of open */
	t_int state;		/* which state is writer in */
	t_int count;		/* count for ticks before next step */

} t_sfrecord;

/* states of statemachine */
#define SFRECORD_WAIT	0		/* wait for open */
#define SFRECORD_OPEN	1
#define SFRECORD_CLOSE	2
#define SFRECORD_SKIP	3
#define SFRECORD_WRITE	4
#define SFRECORD_STOP	5
#define SFRECORD_ERROR	-1

#define SFRECORD_WAITTICKS 10	/* 1 tick of 64 Samples is ca.1.5ms on 441000 */

/* split the os-calls in as many steps as possible 
to split them on different ticks in steps of SFRECORD_WAITTICKS
to avoid peak performance */

/* like the one from garray */
static int sfrecord_am_i_big_endian(void) 
{
	unsigned short s = 1;
	unsigned char c = *(char *) (&s);
#ifdef DEBUG_ME
	post("i am %s-endian", c?"little":"big");
#endif
	return(c==0);
}

static void state_out(t_sfrecord *x, int state)
{	/* outlet the actual state */
	outlet_float(x->x_obj.ob_outlet, state);
}


/* METHOD: "open" file */

/* this dont use memory map, because I dont know about this on NT ? 
Use of the buffered functions fopen, fseek fread fclose instead the 
non buffered ones open read close */

static void sfrecord_open(t_sfrecord *x,t_symbol *filename,t_symbol *endian)
{

	if(x->state != SFRECORD_WAIT)
	{
		post("sfrecord: first close %s before open %s",x->filename->s_name,filename->s_name);
		return;
	}
 
  /* test if big endian else asume little endian 
   * should be 'l' but could be anything
   */
	if(sfrecord_am_i_big_endian())
		x->swap = !(endian->s_name[0] == 'b');
	else
		x->swap = (endian->s_name[0] == 'b');

	/* 
   * skip header after open;; sometimes weŽll have to write a header using the x->skip; so donŽt delete it completely 
   */
  /* x->skip = 1; */

	x->filename = filename;

#ifdef DEBUG_ME
	post("sfrecord: opening %s",x->filename->s_name);
#endif

	if (x->fp != NULL)fclose(x->fp);	/* should not happen */

	if (!(x->fp = sys_fopen(x->filename->s_name, "w"))) 
	{
		error("sfrecord: can't open %s", x->filename->s_name);
	}
}



/* METHOD: close */
static void sfrecord_close(t_sfrecord *x)
{
	x->write = 0;
	x->please_close = 1;

	/* now in state machine 
	if(x->fp != NULL)
	{
		fclose(x->fp);
		x->fp = NULL;
	}
	*/

#ifdef DEBUG_ME
	post("sfrecord: closing ");
#endif
	return;
}

/* for skipping header of soundfile  DonŽt use this for memory map */

static int sfrecord_skip(t_sfrecord *x)
{
	if(!x->skip) return 0;

#ifdef DEBUG_ME
	post("sfrecord:skip to %f",x->x_skip);
#endif

	x->skip = 0;
	return 1;
}

/* Input, method for Start stop */

static void sfrecord_start(t_sfrecord *x)
{
#ifdef DEBUG_ME
	post("sfrecord: start at %d", x->x_offset);
#endif

	state_out(x, 1);
	x->write=1;
}

static void sfrecord_stop(t_sfrecord *x)
{
#ifdef DEBUG_ME
	post("sfrecord: stop");
#endif
	state_out(x, 0);

	x->write=0;
	x->please_stop = 1;
}

static void sfrecord_float(t_sfrecord *x, t_floatarg f)
{
	int t = f;
	if (t) sfrecord_start(x);
	else sfrecord_stop(x);
}

/* say what state weŽre in */
static void sfrecord_bang(t_sfrecord* x)
{
	if (x->state == SFRECORD_WRITE) state_out(x, 1); else state_out(x, 0);
}

/* ******************************************************************************** */
/*                          the work                krow eht                        */
/* ******************************************************************************** */


static t_int *sfrecord_perform(t_int *w)
{
	t_sfrecord* x = (t_sfrecord*)(w[1]);
	short* buf = x->filep;
	short* bufstart = buf;
	int c = x->x_channels;

	int i,j,n, s_n;
	t_float* in[MAX_CHANS];

	short s;
	int swap = x->swap;

	for (i=0;i<c;i++)  
		in[i] = (t_float *)(w[2+i]);

	n = s_n = (int)(w[2+c]);

	/* loop */

	switch(x->state){

		/* just wait */
	case SFRECORD_WAIT:

		if(x->fp != NULL){
#ifdef DEBUG_ME
			post("wait -> open");
#endif
			x->state = SFRECORD_OPEN;
			x->count = SFRECORD_WAITTICKS;
		};
		break;

		/* if in open state, already opened but wait for skip */
	case SFRECORD_OPEN: /* file has opened wait some time */

		if(!(x->count--)){
#ifdef DEBUG_ME
			post("open -> skip");
#endif
			x->state = SFRECORD_SKIP;
			x->count = SFRECORD_WAITTICKS;
		};

		break;

		/* in skipmode wait until ready for stop */
	case SFRECORD_SKIP:

		if(x->count == SFRECORD_WAITTICKS)
		{
			if(!x->fp)
			{
				x->state = SFRECORD_CLOSE;
				x->count=1;
#ifdef DEBUG_ME
				post("skip -> close");
#endif
				break;
			}
			sfrecord_skip(x);
		}
		if(!(x->count--))
		{
#ifdef DEBUG_ME
			post("skip -> stop");
#endif
			x->state = SFRECORD_STOP;
			x->count = SFRECORD_WAITTICKS;
		};
		break;

	case SFRECORD_STOP:		/* in stop state mainly waits for write */

		x->please_stop = 0;

		if(x->please_close)
		{
			x->state = SFRECORD_CLOSE;
			x->count = SFRECORD_WAITTICKS;      
#ifdef DEBUG_ME
			post("stop -> close");
#endif
		}
		else if(x->skip)
		{
			x->state = SFRECORD_SKIP;
			x->count = SFRECORD_WAITTICKS;

#ifdef DEBUG_ME
			post("stop -> skip");
#endif

		}
		else if(x->write)
		{

#ifdef DEBUG_ME
			post("stop -> write");
#endif
			x->state = SFRECORD_WRITE;
			state_out(x, 1);
		}
		break;

	case SFRECORD_WRITE:				/* yes write now */

		if(!x->write || x->please_stop)
		{
			/* if closing dont need to go to stop */
			if(x->please_close)	{
				x->state = SFRECORD_CLOSE;
				x->count = SFRECORD_WAITTICKS;
#ifdef DEBUG_ME
				post("write -> close");
#endif
				state_out(x, 0);

			}
			else	{
				x->state = SFRECORD_STOP;
#ifdef DEBUG_ME
				post("write -> stop");
#endif
			};
			break;
		}

		/* should never happen */
		if(!x->filep){
			x->state = SFRECORD_ERROR;
			error("sfrecord: writing but no buffer ???? write");
			return (w+4+c);
		}

		/* copy float to 16 Bit and swap if neccesairy */ /* LATER treat overflows */
		while (n--) {
			for (i=0;i<c;i++) {
			s = *in[i]++ * 32768.;
			if (swap) s = ((s & 0xFF)<< 8) | ((s& 0xFF00) >> 8);
			*buf++ = s;
			}
		}
		
		/* then write soundfile 16 bit*/		 
		if ( (j = fwrite(bufstart, sizeof(short), c*s_n, x->fp)) < 1) {
		  x->state = SFRECORD_ERROR;
		  x->count = SFRECORD_WAITTICKS;
#ifdef DEBUG_ME
			post("write -> write error\t %xd\t%xd\t%d\t%d", x->filep, buf, c*s_n*sizeof(short), j);
#endif
			break;
		}

#if 0
		if((j=fwrite(buf,sizeof(short),c*n,x->fp)) < (unsigned int) n)
		{
			if(feof(x->fp)){

				while (n--) {
					for (i=0;i<c;i++)	{
						if(--j > 0){
							s = *buf++;
							if(swap) s = ((s & 0xFF)<< 8) | ((s& 0xFF00) >> 8);
							*out[i]++ = s*(1./32768.);
						}
						else
							*out[i]++ = 0;
					}
				}
			}

			x->state = SFRECORD_STOP;
			x->write = 0;
			return(w+c+3);
			}
			/* or error if(ferror()) */
			x->state = SFRECORD_ERROR;
			x->count = SFRECORD_WAITTICKS;
#ifdef DEBUG_ME
			post("write -> write error");
#endif
			break;
		};
#endif /* 0 */
		return (w+c+3); /* writing was fine */


		/* ok :?: write error, please close */
	case SFRECORD_ERROR:

		if(!(x->count--))	{
			x->state = SFRECORD_CLOSE;
			sfrecord_close(x);
#ifdef DEBUG_ME
			post("sfrecord error writing sf: error -> close");
#endif
			x->count = SFRECORD_WAITTICKS;
		}
		break;

		/* in close state go to wait afterwards */
	case SFRECORD_CLOSE:

		x->please_close = 0;

		/* wait until ready for close operation */
		if(!(x->count--)){ 

			x->state = SFRECORD_WAIT;
			x->count = SFRECORD_WAITTICKS;

			/* avoid openfiles */
			if(x->fp){fclose(x->fp);x->fp = NULL;};

#ifdef DEBUG_ME
			post("sfrecord: close -> wait");
#endif
		}
		break;

	}; /*case */

	return(w+c+3);
}




/* ---------------------- Setup junk -------------------------- */

static void sfrecord_dsp(t_sfrecord *x, t_signal **sp)
{

#ifdef DEBUG_ME
	post("sfrecord: dsp");
	post("offset = %f\tspeed = %f\t", x->offset, x->x_speed);
#endif


	switch (x->x_channels) {
	case 1:
		dsp_add(sfrecord_perform, 3, x,
			sp[0]->s_vec, /* in 1 */
			sp[0]->s_n);
		break;
	case 2:
		dsp_add(sfrecord_perform, 4, x, 
			sp[0]->s_vec,
			sp[1]->s_vec,
			sp[0]->s_n);
		break;
	case 4:
		dsp_add(sfrecord_perform, 6, x, 
			sp[0]->s_vec, 
			sp[1]->s_vec,
			sp[2]->s_vec,
			sp[3]->s_vec,
			sp[0]->s_n);
		break;
	case 8:
		dsp_add(sfrecord_perform, 9, x, 
			sp[0]->s_vec, 
			sp[1]->s_vec,
			sp[2]->s_vec,
			sp[3]->s_vec,
			sp[4]->s_vec,
			sp[5]->s_vec,
			sp[6]->s_vec,
			sp[7]->s_vec,
			sp[0]->s_n);
		break;
	}
}


/* create sfrecord with args <channels> <skip> */
static void *sfrecord_new(t_floatarg chan)
{
	t_sfrecord *x = (t_sfrecord *)pd_new(sfrecord_class);
	t_int c = chan;

	switch(c){
		/* ok */
	case 1: case 2: case 4: case 8: break;
		/* try it, good luck ... */
	case 3: c = 2; break;     
	case 5: case 6: case 7: c=7; break;
	default: c=1; break;
	}

	outlet_new(&x->x_obj, gensym("float"));

	x->x_channels = c;
	x->x_skip = x->x_offset = 0;
	x->skip = 1;
	x->offset = 0.;
	x->x_speed = 1.0;
	x->write = 0;
	x->please_stop = 0;
	x->please_close = 0;
	x->state = SFRECORD_WAIT;
	x->count = 0;
	x->filename = NULL;
	x->fp = NULL;
	x->swap = 1;

	c--;

	while (c--) {
#ifdef DEBUG_ME
		post("create extra channel #%d", c);
#endif
		inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal")); /* channels inlet */
	}

	x->filep = t_getbytes(DACBLKSIZE*sizeof(short)*x->x_channels);

#ifdef DEBUG_ME
	post("get_bytes DACBLKSIZE*%d*%d->%ld",sizeof(short),x->x_channels,x->filep);
	post("sfrecord: x_channels = %d, x_speed = %f, x_skip = %f",x->x_channels,x->x_speed,x->x_skip);
#endif

	return (x);
}


static void sfrecord_helper(void)
{
	post("\nsfplay :: a raw-data soundfile-recorder");
	post("\ncreation :: sfrecord <channels>\t: channels set the number of channels");
	post("\nopen [<path>]<filename> [<endianity>]\t:: open b(ig) or l(ittle) endian file"
		"\nclose\t\t\t:: close file (aka eject)"
		"\nstart\t\t\t:: start playing"
		"\nstop\t\t\t:: stop playing"
		"\nbang\t\t\t:: outputs the current state (1_recording, 0_not-recording)");
		
	post("\n\nyou can also start recording with a Ž1Ž, and stop with a Ž0Ž");
}


static void sfrecord_free(t_sfrecord *x)
{
	freebytes(x->filep, DACBLKSIZE*sizeof(short)*x->x_channels);
}

void sfrecord_setup(void)
{
	sfrecord_class = class_new(gensym("sfrecord"), (t_newmethod)sfrecord_new, (t_method)sfrecord_free,
		sizeof(t_sfrecord), 0, A_DEFFLOAT, A_DEFFLOAT,0);
	class_addmethod(sfrecord_class, nullfn, gensym("signal"), 0);
	class_addmethod(sfrecord_class, (t_method)sfrecord_dsp, gensym("dsp"), 0);

	/* method open with filename */
	class_addmethod(sfrecord_class, (t_method)sfrecord_open, gensym("open"), A_SYMBOL,A_SYMBOL,A_NULL);
	class_addmethod(sfrecord_class, (t_method)sfrecord_close, gensym("close"), A_NULL);
	
	class_addmethod(sfrecord_class, (t_method)sfrecord_start, gensym("start"), A_NULL);
	class_addmethod(sfrecord_class, (t_method)sfrecord_stop,  gensym("stop"), A_NULL);

	/* start/stop with 0/1 */
	class_addfloat(sfrecord_class, sfrecord_float);

	/* bang out the current-state to the outlet*/
	class_addbang(sfrecord_class,sfrecord_bang);

	/* some help */
	class_addmethod(sfrecord_class, (t_method)sfrecord_helper,  gensym("help"), A_NULL);
	class_sethelpsymbol(sfrecord_class, gensym("sf-play_record"));
  zexy_register("sfrecord");
}
