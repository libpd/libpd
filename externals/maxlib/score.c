/* ------------------------- score   ------------------------------------------ */
/*                                                                              */
/* Simple score following / orientation. Incoming data gets compared to a       */
/* score stored in an array or table.                                           */
/* Written by Olaf Matthes (olaf.matthes@gmx.de)                                */
/* Get source at http://www.akustische-kunst.org/puredata/maxlib/               */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"

#define MAX_NOTES 32                /* maximum number of notes that can be stored */

static char *version = "score v0.1, score follower written by Olaf Matthes <olaf.matthes@gmx.de>";
 
typedef struct score
{
  t_object x_ob;
  t_inlet  *x_invelo;               /* inlet for velocity */
  t_inlet  *x_inreset;              /* inlet to reset the object */
  t_outlet *x_outindex;             /* index :: position in given score */
  t_outlet *x_outerror;             /* indicates lost orientation */
  t_symbol *x_sym;                  /* name of array that contains the score */
  t_garray *x_buf;                  /* the above array itselfe */

  t_int    x_state;                 /* indicates state of score following: */
                                    /* running = 1, record = -1, stop = 0  */
  t_int    x_skipindex;             /* max. number of notes to skip */
  t_float  x_skiptime;              /* max time in ms to skip */
  t_int    x_index;                 /* position in array / score */
  t_int    x_lastpitch;
  t_int    x_error;

  t_int    x_notecount;
  t_int    x_pitch;
  t_int    x_velo;
	/* helpers needed to do the calculations */
  double   x_starttime[MAX_NOTES];
  double   x_laststarttime;
  t_int    x_alloctable[MAX_NOTES];

} t_score;

static void score_float(t_score *x, t_floatarg f)
{
	/* This is the score following algorhythm:

	    first, we check if the note we got is in the score. In case
		it's not the next note, we'll search 'skipnotes' in advance.
		In case that fails we go back 'skipnotes' and check them. As
		extra these notes have to be 'younger' than 'skiptime' (to 
		avoid going back too far in case of slow melodies)
		As last resort we check if we probably just got the same not 
		again (double trigger from keyboard or the like) 
	                                                                 */

	t_int velo = x->x_velo;     /* get the velocity */
	t_garray *b = x->x_buf;		/* make local copy of array */
	float *tab;                 /* we'll store notes in here */
	int items;
	int i, j, n, check;

	x->x_pitch = (t_int)f;
	x->x_error = 0;

		/* check our array */
	if (!b)
	{
		post("score: no array selected!");
		x->x_error = 1;
		goto output;
	}
	if (!garray_getfloatarray(b, &items, &tab))
	{
		post("score: couldn't read from array!");
		x->x_error = 1;
		goto output;
	}

	if (x->x_state)	/* score follower is running */
	{
		n = check = x->x_notecount;		/* make local copys */

		if (x->x_velo != 0)		/* store note-on in alloctable */
		{
				/* store note in alloctable */
			x->x_alloctable[n] = (t_int)x->x_pitch;
				/* store note-on time */
			x->x_starttime[n] = clock_getlogicaltime();
			if(++x->x_notecount >= MAX_NOTES)x->x_notecount = 0;  /* total number of notes has increased */
		} else return;	/* we don't care about note-off's */

			/* first we try to find a match within the skip area */
			/* ( probably looking ahead in the score ) */
		for (i = x->x_index + 1; i < (x->x_index + x->x_skipindex + 1); i++)
		{
			// post("%d: %d -> %d", i, x->x_alloctable[n], (t_int)tab[i]);
			if(x->x_alloctable[n] == (t_int)tab[i])
			{
				if(i - x->x_index != 1) post("score: skipped %d notes!", i - x->x_index - 1);
				x->x_alloctable[n] = -1;		/* delete note, we've matched it! */
				x->x_index = i;
				goto output;
			}
		}

			/* then we look back within the boudaries of skiptime */
		for (i = x->x_index - 1; i > (x->x_index - x->x_skipindex) - 1; i--)
		{
			check = n;	/* get current notecount */

			for (j = 0; j < MAX_NOTES; j++)	/* check with every note from our alloctable */
			{ 
				if (x->x_alloctable[check] == (t_int)tab[i])	/* this one would fit */
				{
						/* check the time restrictions */
					if (clock_gettimesince(x->x_starttime[check]) < x->x_skiptime)
					{
						if (i != x->x_index) post("score: skipped %d notes in score!", x->x_index - i);
						if (j != 0) post("score: skipped %d notes from input!", j);
						post("score: going back by %g milliseconds!", clock_gettimesince(x->x_starttime[check]));
						x->x_index = i;
							/* new notecount: we assume the notes we skipped are errors made by the */
							/* performer. new notes will be added right behind the last valid one */
						x->x_notecount = (check++) % MAX_NOTES;
						x->x_alloctable[x->x_notecount - 1] = -1;	/* delete note since we've matched it */
						goto output;
					}
					else /* ough, too old ! */
					{
						post("score: matching note is too old! (ignored)");
						x->x_alloctable[check] = 0;		/* delete note since it's too old */
						x->x_error = 1;
						goto output;	/* stop with first match as all others would be far older */
					}
				}
				if(--check < 0) check = MAX_NOTES - 1;	/* decrease counter */
														/* as we want to go back in time */
			}
		}
			/* or is it just the same note again ??? (double trigger...) */
		if(x->x_pitch == x->x_lastpitch)
		{
			post("score: repetition! (ignored)");
			x->x_alloctable[x->x_notecount - 1] = -1; /* forget this one */
			return;
		}

			/* in case we found nothing: indicate that! */
		x->x_error = 1;
		post("score: couldn't find any matches !");
		x->x_lastpitch = x->x_pitch;
		goto output;
	}
	else return;

output:
		/* output index */
	outlet_float(x->x_outindex, x->x_index);
		/* bang in case of error */
	if(x->x_error) outlet_bang(x->x_outerror);
}

static void score_ft1(t_score *x, t_floatarg f)
{
	x->x_velo = (t_int)f;
}

	/* start following the previoisly recorded score */
static void score_start(t_score *x, t_symbol *s, t_int argc, t_atom* argv)
{
	x->x_index = (t_int)atom_getfloatarg(0, argc, argv);
	if(x->x_index > 0)
	{
		post("score: starting at note %d", x->x_index);
	}
	else post("score: start following");
	x->x_index--;	/* because our array starts with 0 */
	x->x_state = 1;
}
	/* resume following the previoisly recorded score */
static void score_resume(t_score *x)
{
	x->x_state = 1;
	post("score: resume following");
}

	/* stop following the previoisly recorded score */
static void score_stop(t_score *x)
{
	x->x_state = 0;
	post("score: stop following");
}

	/* choose the array that holds the score */
void score_set(t_score *x, t_symbol *s)
{
	t_garray *b;
	
	x->x_sym = s;

	if ((b = (t_garray *)pd_findbyclass(s, garray_class)))
	{
		post("score: array set to \"%s\"", s->s_name);
		x->x_buf = b;
	} else {
		post("score: no array \"%s\" (error %d)", s->s_name, b);
		x->x_buf = 0;
	}
}

static void score_reset(t_score *x)
{
	int i;

	x->x_state = 0;				/* don't follow */
	x->x_error = 0;
	x->x_index = -1;
	x->x_notecount = 0;
	x->x_lastpitch = 0;
	for(i = 0; i < MAX_NOTES; i++)x->x_alloctable[i] = -1;

	post("score: reset");
}

static void score_free(t_score *x)
{
	// nothing to do
}

static t_class *score_class;

static void *score_new(t_symbol *s, t_floatarg fskipindex, t_floatarg fskiptime)
{
	int i;

    t_score *x = (t_score *)pd_new(score_class);
    x->x_invelo = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
    x->x_inreset = inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("bang"), gensym("reset"));
	x->x_outindex = outlet_new(&x->x_ob, gensym("float"));
	x->x_outerror = outlet_new(&x->x_ob, gensym("float"));

	x->x_sym = s;				/* get name of array */
	score_set(x,x->x_sym);      /* set array */
	if(!fskipindex)fskipindex = 2;
	if(!fskiptime)fskiptime = 300.0;
	x->x_skipindex = (t_int)fskipindex;
	x->x_skiptime = (t_float)fskiptime;
	post("score: skipindex set to %d, skiptime set to %g milliseconds", x->x_skipindex, x->x_skiptime);

	x->x_state = 0;				/* don't follow */
	x->x_error = 0;
	x->x_index = -1;
	x->x_notecount = 0;
	x->x_pitch = x->x_lastpitch = -1;
	for(i = 0; i < MAX_NOTES; i++)x->x_alloctable[i] = -1;

    return (void *)x;
}

#ifndef MAXLIB
void score_setup(void)
{
    score_class = class_new(gensym("score"), (t_newmethod)score_new,
    	(t_method)score_free, sizeof(t_score), 0, A_SYMBOL, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(score_class, (t_method)score_reset, gensym("reset"), 0);
	class_addmethod(score_class, (t_method)score_resume, gensym("resume"), 0);
    class_addmethod(score_class, (t_method)score_start, gensym("start"), A_GIMME, 0);
    class_addmethod(score_class, (t_method)score_stop, gensym("stop"), 0);
    class_addmethod(score_class, (t_method)score_ft1, gensym("ft1"), A_FLOAT, 0);
    class_addmethod(score_class, (t_method)score_reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(score_class, (t_method)score_set, gensym("set"), A_SYMBOL, 0);
    class_addfloat(score_class, score_float);
    
    logpost(NULL, 4, version);
}
#else
void maxlib_score_setup(void)
{
    score_class = class_new(gensym("maxlib_score"), (t_newmethod)score_new,
    	(t_method)score_free, sizeof(t_score), 0, A_SYMBOL, A_DEFFLOAT, A_DEFFLOAT, 0);
	class_addcreator((t_newmethod)score_new, gensym("score"), A_SYMBOL, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(score_class, (t_method)score_reset, gensym("reset"), 0);
	class_addmethod(score_class, (t_method)score_resume, gensym("resume"), 0);
    class_addmethod(score_class, (t_method)score_start, gensym("start"), A_GIMME, 0);
    class_addmethod(score_class, (t_method)score_stop, gensym("stop"), 0);
    class_addmethod(score_class, (t_method)score_ft1, gensym("ft1"), A_FLOAT, 0);
    class_addmethod(score_class, (t_method)score_reset, gensym("reset"), A_GIMME, 0);
	class_addmethod(score_class, (t_method)score_set, gensym("set"), A_SYMBOL, 0);
    class_addfloat(score_class, score_float);
    class_sethelpsymbol(score_class, gensym("maxlib/score-help.pd"));
}
#endif
