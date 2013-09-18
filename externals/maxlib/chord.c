/* ------------------------- chord   ------------------------------------------ */
/*                                                                              */
/* Tries to detect a chord (or any harmonic relations) of incoming notes.       */
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
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <stdlib.h>
#endif


#define MAX_POLY 32                 /* maximum number of notes played at a time */

#define kUnison 0
#define kMaj 1
#define kMin 2
#define kDim 3
#define kAug 4
#define kMaj7 5
#define kDom7  6
#define kMin7  7
#define kHalfDim7 8
#define kDim7  9
#define kMinMaj7 10
#define kMaj7s5  11
#define kMaj7b5  12
#define kDom7s5  13
#define kDom7b5  14
#define kDomb9 15
#define kMaj9  16
#define kDom9  17
#define kMin9  18
#define kHalfDim9 19
#define kMinMaj9  20
#define kDimMaj9  21
#define kMaj9b5  22
#define kDom9b5  23
#define kDom9b13  24
#define kMin9s11  25
#define kmM9b11  26
#define kMaj7b9  27
#define kMaj7s5b9  28
#define kDom7b9  29
#define kMin7b9  30
#define kMinb9s11  31
#define kHalfDimb9  32
#define kDim7b9 33
#define kMinMajb9 34 
#define kDimMajb9 35
#define kMaj7s9  36
#define kDom7s9  37
#define kMaj7s11  38
#define kMs9s11  39
#define kHDimb11 40
#define kMaj11  41
#define kDom11  42
#define kMin11  43
#define kHalfDim11 44  
#define kDim11  45
#define kMinMaj11 46 
#define kDimMaj11 47
#define kMaj11b5  48
#define kMaj11s5  49
#define kMaj11b9  50
#define kMaj11s9  51
#define kMaj11b13  52
#define kMaj11s13  53
#define kM11b5b9  54
#define kDom11b5  55
#define kDom11b9  56
#define kDom11s9  57
#define kHalfDim11b9 58
#define kDom7s11  59
#define kMin7s11  60
#define kDom13s11  61
#define kM7b913  62
#define kMaj7s13  63
#define kMaj9s13  64
#define kM7b9s13  65
#define kDom7b13  66
#define kChrom  67
#define kNone  68

#define kXX -1


static char *version = "chord v0.2, written by Olaf Matthes <olaf.matthes@gmx.de>";

static char* pitch_class[13] = {"C ", "Db ", "D ", "Eb ", "E ", "F ", "Gb ", "G ", "Ab ", "A ", "Bb ", "B ", "no root "};
static char name_class[7] = {'C', 'D', 'E', 'F', 'G', 'A', 'B'};

typedef struct {
	int  type;
	int  rootMember;
} t_type_root;
 
typedef struct chord
{
  t_object x_ob;
  t_outlet *x_outchordval;          /* chord as MIDI note number of base note */
  t_outlet *x_outchordclass;        /* class of chord's bass note */
  t_outlet *x_outchordname;         /* chord name, e.g. "Cmajor7" */
  t_outlet *x_outchordinversion;    /* inversion of the chord (root = 0, 1st = 1, 2nd = 2) */
  t_outlet *x_outchordnotes;        /* list with note numbers belonging to the chord */

  t_int    x_pitch;
  t_int    x_pc[12];                /* pitch class array */
  t_int    x_abs_pc[12];            /* pitch class array: absolute MIDI note numbers */
  t_int    x_velo;
  t_int    x_alloctable[MAX_POLY];  /* a table used to store all playing notes */
  t_int    x_poly;                  /* number of notes currently playing */
  t_atom   x_chordlist[12];			/* list that stores the note numbers for output */
  t_int    x_split;                 /* highes note number to process */

  t_int    x_chord_type;			/* chord's type (number between 0 and 68) */
  t_int    x_chord_root;            /* chord's root (pitch class) */
  t_int    x_chord_bass;            /* chord's bass note (MIDI note number) */
  t_int    x_chord_inversion;       /* chord's state of inversion (root, 1st, 2nd) */

} t_chord;

/* functions */
static void chord_kick_out_member(t_chord *x, t_int number, t_int *members);
static void chord_chord_finder(t_chord *x, t_int num_pcs);
static void chord_draw_chord_type(t_chord *x, t_int num_pcs);


static void chord_unison(t_chord *x)
{
	int i;
	int member = 0;
	for(i = 0; i < 12; i++)
		if(x->x_pc[i])
		{
			member = i;				// find pitch class
			break;
		}
	x->x_chord_type  = 0;
	x->x_chord_root  = member;
	chord_draw_chord_type(x, 1);				// output onto the screen
}

static void chord_dyad(t_chord *x)
{
	static t_type_root dyads[11] = 
	 {{ kMaj7,  1 }, { kDom7, 1 }, { kMin, 0 }, { kMaj, 0 }, { kMaj,  1 },
	  { kDom7 , 0 }, { kMaj,  0 }, { kMaj, 1 }, { kMin, 1 }, { kDom7, 0 }, { kMaj7,  0 }};
	register t_type_root* t;

	int members[2];
	int i, j = 0;
	int interval1;

	for(i = 0; i < 12; i++)
		if(x->x_pc[i]) members[j++] = i;		/* load members array with chord pitch classes */
	interval1 = members[1] - members[0];		/* calculate interval between first two members */
	interval1 = interval1 - 1;					/* reduce interval1 to start at zero */
	t = &(dyads[interval1]);					/* find TypeRoot struct for this interval */
	x->x_chord_type = t->type;
	if (interval1 == 5)
		x->x_chord_root = (members[0]+8)%12;
	else
		x->x_chord_root = members[t->rootMember];
	x->x_chord_inversion = t->rootMember;		/* get state of inversion */
	chord_draw_chord_type(x, 2);				/* output results */
}

static void chord_triad(t_chord *x)
{
	static t_type_root triads[10][10] = 
	{/* interval1 is a half step			*/
	 {{ kMaj7b9,   1 }, { kMaj9,     1 }, { kMinMaj7, 1 }, { kMaj7,     1 }, { kDom7s11,2 },
	  { kDomb9 ,   0 }, { kMaj7,     1 }, { kMaj7s5,  1 }, { kMin9,     2 }, { kMaj7b9, 0 }},
	 /* interval1 is a whole step			*/
	 {{ kMin9,     0 }, { kDom9,     0 }, { kMin7,    1 }, { kDom7,     1 }, { kDom9,   0 },
	  { kHalfDim7, 1 }, { kDom7,     1 }, { kDom9,    0 }, { kMaj9,     0 }},
	 /* interval1 is a minor third			*/
	 {{ kMaj7s5,   2 }, { kDom7,     2 }, { kDim,     0 }, { kMin,      0 }, { kMaj,    2 },
	  { kDim,      2 }, { kMin7,     0 }, { kMinMaj7, 0 }},
	 /* interval1 is a major third			*/
	 {{ kMaj7,     2 }, { kHalfDim7, 2 }, { kMaj,     0 }, { kAug,      0 }, { kMin,    2 },
	  { kDom7,     0 }, { kMaj7,     0 }},
	 /* interval1 is a perfect fourth		*/
	 {{ kDomb9,    1 }, { kDom9,     1 }, { kMin,     1 }, { kMaj,      1 }, { kDom9,   2 },
	  { kDom7s11,  1 }},
	 /* interval1 is an augmented fourth	*/
	 {{ kDom7s11,  0 }, { kDom7,     2 }, { kDim,     1 }, { kHalfDim7, 0 }, { kDomb9,  2 }},
	 /* interval1 is a perfect fifth		*/
	 {{ kMaj7,     2 }, { kMin7,     2 }, { kDom7,    0 }, { kMaj7,     0 }},
	 /* interval1 is a minor sixth			*/
	 {{ kMinMaj7,  2 }, { kDom9,     1 }, { kMaj7s5,  0 }},
	 /* interval1 is a major sixth			*/
	 {{ kMaj9,     2 }, { kMin9,     1 }},
	 /* interval1 is a minor seventh		*/
	 {{ kMaj7b9,   2 }}
	};
	register t_type_root* t;
	
	int members[3];
	int i, j = 0;
	int interval1, interval2;

	for(i = 0; i < 12; i++)
		if(x->x_pc[i]) members[j++] = i;		/* load members array with chord pitch classes */
	interval1 = members[1] - members[0];		/* calculate interval between first two members */
	interval2 = members[2] - members[0];		/* calculate interval between first and third */
	interval2 = interval2 - interval1 - 1;		/* reduce interval2 to start at zero */
	interval1 = interval1 - 1;					/* reduce interval1 to start at zero */
	t = &(triads[interval1][interval2]);		/* find TypeRoot struct for this interval vector */
	x->x_chord_type = t->type;
	x->x_chord_root = members[t->rootMember];
	switch(t->rootMember) {						/* get state of inversion */
			case 0: 
				x->x_chord_inversion = 0;
				break;
			case 1: 
				x->x_chord_inversion = 2;
				break;
			case 2: 
				x->x_chord_inversion = 1;
	}
	chord_draw_chord_type(x, 3);				/* output onto the screen */
}

static void chord_quartad(t_chord *x)
{
	static t_type_root quartads[9][9][9] =
	{
	 {/* interval1 is a half step			*/
	   {/* interval2 is a whole step		*/
		{ kM7b9s13, 2 }, { kMinMajb9,1 }, { kMaj7b9,   1 }, { kMaj7s13,  2 }, { kDimMajb9, 1 },
		{ kMaj7b9,  1 }, { kMaj7s13, 2 }, { kM7b913,   1 }, { kM7b9s13,  1 }},
	   {/* interval2 is a minor third		*/
		{ kMinMaj9, 1 }, { kMaj9,    1 }, { kHalfDimb9,0 }, { kMin7b9,   0 }, { kMaj9,     1 },
		{ kDim7b9,  0 }, { kMin7b9,  0 }, { kMinMajb9, 0 }},
	   {/* interval2 is a major third		*/
		{ kMaj7s9,  1 }, { kDom7s11, 3 }, { kDomb9,    0 }, { kMinMaj7,  1 }, { kDom7s9,   3 },
		{ kDomb9,   0 }, { kMaj7b9,  0 }},
	   {/* interval2 is a perfect fourth	*/
		{ kMaj11,   1 }, { kMaj7b5,  1 }, { kMaj7,     1 }, { kMaj7s5,   1 }, { kMin9,     3 },
		{ kMaj7s13, 1 }},
	   {/* interval2 is a tritone			*/
		{ kDimMaj9, 3 }, { kDom11,   3 }, { kDim7b9,   0 }, { kHalfDimb9,0 }, { kDimMajb9, 0 }},
	   {/* interval2 is a perfect fifth		*/
		{ kMaj11,   3 }, { kDom7s9,  3 }, { kDomb9,    0 }, { kMaj7b9,   0 }},
	   {/* interval2 is a minor sixth		*/
		{ kMaj7s9,  3 }, { kMin9,    3 }, { kMaj7s13,  1 }},
	   {/* interval2 is a major sixth		*/
		{ kMinMaj9, 3 }, { kM7b913,  0 }},
	   {/* interval2 is a minor seventh		*/
		{ kM7b9s13, 0 }}
	 },
	 {/* interval1 is a whole step			*/
	   {/* interval2 is a minor third		*/
		{ kM7b913,   2 }, { kMin7b9, 1 }, { kDomb9,    1 }, { kMin9,     0 }, { kHalfDimb9,1 },
		{ kDomb9,    1 }, { kMin9,   0 }, { kMinMaj9,  0 }},
	   {/* interval2 is a major third		*/
		{ kMin9,     1 }, { kDom9,   1 }, { kDom9,     0 }, { kDom7s5,   2 }, { kDom9,     1 },
		{ kDom9,     0 }, { kMaj9,   0 }},
	   {/* interval2 is a perfect fourth	*/
		{ kDom7s9,   1 }, { kDom11,  3 }, { kHalfDim7, 1 }, { kMin7,     1 }, { kDom9,     3 },
		{ kHalfDimb9,3 }},
	   {/* interval2 is a tritone			*/
		{ kDom11,    1 }, { kDom7b5, 3 }, { kDom7,     1 }, { kDom7s5,   1 }, { kMin7b9,   3 }},
	   {/* interval2 is a perfect fifth		*/
		{ kMaj7b5,   3 }, { kDom11,  1 }, { kDom9,     0 }, { kMaj9,     0 }},
	   {/* interval2 is a minor sixth		*/
		{ kDom7s11,  1 }, { kDom9,   3 }, { kDim7b9,   3 }},
	   {/* interval2 is a major sixth		*/
		{ kMaj9,     3 }, { kMin7b9, 3 }},
	   {/* interval2 is a minor seventh		*/
		{ kMinMajb9, 3 }}
	 },
	 {/* interval1 is a minor third			*/
	   {/* interval2 is a major third		*/
		{ kMaj7s13,  3 }, { kDim7b9,  1 }, { kDom7s9,   0 }, { kMaj7s5,   2 }, { kDim7b9,   1 },
		{ kDom7s9,   0 }, { kMaj7s9,  0 }},
	   {/* interval2 is a perfect fourth	*/
		{ kDomb9,    2 }, { kDom9,    2 }, { kMin7,     2 }, { kDom7,     2 }, { kDom11,    2 },
		{ kDom7s11,  2 }},
	   {/* interval2 is a tritone			*/
		{ kDim7b9,   2 }, { kDom7,    3 }, { kDim7,     0 }, { kHalfDim7, 0 }, { kDomb9,    3 }},
	   {/* interval2 is a perfect fifth		*/
		{ kMaj7,     3 }, { kHalfDim7,3 }, { kMin7,     0 }, { kMinMaj7,  0 }},
	   {/* interval2 is a minor sixth		*/
		{ kDomb9,    2 }, { kDom9,    2 }, { kDom7s9,   2 }},
	   {/* interval2 is a major sixth		*/
		{ kHalfDimb9,2 }, { kDomb9,   3 }},
	   {/* interval2 is a minor seventh		*/
		{ kMaj7b9,   3 }}
	 },
	 {/* interval1 is a major third			*/
	   {/* interval2 is a perfect fourth	*/
		{ kMaj7b9,   2 }, { kMaj9,    2 }, { kMinMaj7,   2 }, { kMaj7,     2 }, { kDom11,  0 },
		{ kMaj11,    0 }},
	   {/* interval2 is a tritone			*/
		{ kHalfDimb9,2 }, { kDom7s5,  3 }, { kHalfDim7,  2 }, { kDom7b5,   0 }, { kMaj7b5, 0 }},
	   {/* interval2 is a perfect fifth		*/
		{ kMaj7s5,   3 }, { kMin7,    3 }, { kDom7,      0 }, { kMaj7,     0 }},
	   {/* interval2 is a minor sixth		*/
		{ kMinMaj7,  3 }, { kDom7s5,  0 }, { kMaj7s5,    0 }},
	   {/* interval2 is a major sixth		*/
		{ kMin7b9,   2 }, { kMin9,    2 }},
	   {/* interval2 is a minor seventh		*/
		{ kMaj7s13,  0 }}
	 },
	 {/* interval1 is a perfect fourth		*/
	   {/* interval2 is a tritone			*/
		{ kDimMajb9, 2 }, { kMin7b9, 1 }, { kDomb9,  1 }, { kMaj7b5,  2 }, { kDimMaj9, 0 }},
	   {/* interval2 is a perfect fifth		*/
		{ kMin9,     1 }, { kDom9,   1 }, { kDom11,  0 }, { kDom11,   2 }},
	   {/* interval2 is a minor sixth		*/
		{ kDom7s9,   1 }, { kDom9,   3 }, { kDim7b9, 3 }},
	   {/* interval2 is a major sixth		*/
		{ kMaj9,     3 }, { kHalfDimb9,3 }},
	   {/* interval2 is a minor seventh		*/
		{ kDimMajb9, 3 }}
	 },
	 {/* interval1 is a tritone				*/
	   {/* interval2 is a perfect fifth		*/
		{ kMaj7s13,  3 }, { kHalfDimb9,1 }, { kDom7s11, 0 }, { kMaj11,   2 }},
	   {/* interval2 is a minor sixth		*/
		{ kDomb9,    2 }, { kDom9,     2 }, { kDom7s9,  2 }},
	   {/* interval2 is a major sixth		*/
		{ kDim7b9,   2 }, { kDomb9,    3 }},
	   {/* interval2 is a minor seventh		*/
		{ kMaj7b9,   3 }}
	 },
	 {/* interval1 is a perfect fifth		*/
	   {/* interval2 is a minor sixth		*/
		{ kMaj7b9,    2 }, { kMaj9,    2 }, { kMaj7s9,   2 }},
	   {/* interval2 is a major sixth		*/
		{ kMin7b9,    2 }, { kMin9,    2 }},
	   {/* interval2 is a minor seventh		*/
		{ kMaj7s13,   0 }}
	 },
	 {/* interval1 is a minor sixth			*/
	   {/* interval2 is a major sixth		*/
		{ kMinMajb9,  2 }, { kMinMaj9, 2 }},
	   {/* interval2 is a minor seventh		*/
		{ kM7b913,    3 }}
	 },
	 {/* interval1 is a major sixth			*/
	   {/* interval2 is a minor seventh		*/
		{ kM7b9s13,   2 }}
	 }
	};

	register t_type_root* t;
	
	int members[4];
	int interval1, interval2, interval3;
	int i, j = 0;
	for (i=0; i<12; i++)
		if (x->x_pc[i]) members[j++] = i;		/* load members array with chord pitch classes */
	interval1 = members[1] - members[0];		/* calculate interval between first two members */
	interval2 = members[2] - members[0];		/* calculate interval between first and third */
	interval3 = members[3] - members[0];		/* calculate interval between first and third */
	interval3 = interval3 - interval2 - 1;		/* reduce interval3 to start at zero */
	interval2 = interval2 - interval1 - 1;		/* reduce interval2 to start at zero */
	interval1 = interval1 - 1;					/* reduce interval1 to start at zero */

		/* find TypeRoot struct for this interval set */
	t = &(quartads[interval1][interval2][interval3]);
	x->x_chord_type = t->type;
	x->x_chord_root = members[t->rootMember];
	switch(t->rootMember) {						/* get state of inversion */
			case 0: 
				x->x_chord_inversion = 0;
				break;
			case 1: 
				x->x_chord_inversion = 2;
				break;
			case 2: 
				x->x_chord_inversion = 2;
				break;
			case 3: 
				x->x_chord_inversion = 1;
	}
	chord_draw_chord_type(x, 4);				/* output results */
}

static void chord_fatal_error(char* s1, char* s2)
{
	post("chord: error: %s : %s", s1, s2);
}

static void chord_quintad(t_chord *x)
{
	static int initialized = 0;
	static t_type_root quintads[8][8][8][8];
	register int i, j, k, l;
	register t_type_root *t;
	t_int members[5];
	int interval1, interval2, interval3, interval4;
	int *st;
	int maj9[5][4] = {{1,1,2,3}, {0,1,1,2}, {3,0,1,1}, {2,3,0,1}, {1,2,3,0}};
	int dom9[5][4] = {{1,1,2,2}, {1,1,1,2}, {2,1,1,1}, {2,2,1,1}, {1,2,2,1}};
	int min9[5][4] = {{1,0,3,2}, {1,1,0,3}, {2,1,1,0}, {3,2,1,1}, {0,3,2,1}};
	int had9[5][4] = {{1,0,2,3}, {1,1,0,2}, {3,1,1,0}, {2,3,1,1}, {0,2,3,1}};
	int miM9[5][4] = {{1,0,3,3}, {0,1,0,3}, {3,0,1,0}, {3,3,0,1}, {0,3,3,0}};
	int diM9[5][4] = {{1,0,2,4}, {0,1,0,2}, {4,0,1,0}, {2,4,0,1}, {0,2,4,0}};
	int M9b5[5][4] = {{1,1,1,4}, {0,1,1,1}, {4,0,1,1}, {1,4,0,1}, {1,1,4,0}};
	int D9b5[5][4] = {{1,1,1,3}, {1,1,1,1}, {3,1,1,1}, {1,3,1,1}, {1,1,3,1}};
	int mM91[5][4] = {{1,0,0,6}, {0,1,0,0}, {6,0,1,0}, {0,6,0,1}, {0,0,6,0}};
	int M7b9[5][4] = {{0,2,2,3}, {0,0,2,2}, {3,0,0,2}, {2,3,0,0}, {2,2,3,0}};
	int M5b9[5][4] = {{0,2,3,2}, {0,0,2,3}, {2,0,0,2}, {3,2,0,0}, {2,3,2,0}};
	int D7b9[5][4] = {{0,2,2,2}, {1,0,2,2}, {2,1,0,2}, {2,2,1,0}, {2,2,2,1}};
	int m7b9[5][4] = {{0,1,3,2}, {1,0,1,3}, {2,1,0,1}, {3,2,1,0}, {1,3,2,1}};
	int mb51[5][4] = {{0,1,2,0}, {4,0,1,2}, {0,4,0,1}, {2,0,4,0}, {1,2,0,4}};
	int d7b9[5][4] = {{0,1,2,3}, {1,0,1,2}, {3,1,0,1}, {2,3,1,0}, {1,2,3,1}};
	int mMb9[5][4] = {{0,1,3,3}, {0,0,1,3}, {3,0,0,1}, {3,3,0,0}, {1,3,3,0}};
	int dMb9[5][4] = {{0,1,2,4}, {0,0,1,2}, {4,0,0,1}, {2,4,0,0}, {1,2,4,0}};
	int dib9[5][4] = {{0,1,2,2}, {2,0,1,2}, {2,2,0,1}, {2,2,2,0}, {1,2,2,2}};
	int M7s9[5][4] = {{2,0,2,3}, {0,2,0,2}, {3,0,2,0}, {2,3,0,2}, {0,2,3,0}};
	int D7s9[5][4] = {{2,0,2,2}, {1,2,0,2}, {2,1,2,0}, {2,2,1,2}, {0,2,2,1}};
	int M7s1[5][4] = {{3,1,0,3}, {0,3,1,0}, {3,0,3,1}, {0,3,0,3}, {1,0,3,0}};
	int d9b3[5][4] = {{1,1,2,0}, {3,1,1,2}, {0,3,1,1}, {2,0,3,1}, {1,2,0,3}};
	int M9s3[5][4] = {{1,4,2,0}, {0,1,4,2}, {0,0,1,4}, {2,0,0,1}, {4,2,0,0}};
	int M9st[5][4] = {{1,1,5,0}, {0,1,1,5}, {0,0,1,1}, {5,0,0,1}, {1,5,0,0}};
	int s9s1[5][4] = {{2,0,1,0}, {4,2,0,1}, {0,4,2,0}, {1,0,4,2}, {0,1,0,4}};
	int h7b1[5][4] = {{2,0,1,3}, {1,2,0,1}, {3,1,2,0}, {1,3,1,2}, {0,1,3,1}};
	int M711[5][4] = {{3,0,1,3}, {0,3,0,1}, {3,0,3,0}, {1,3,0,3}, {0,1,3,0}};
	int M115[5][4] = {{1,1,0,5}, {0,1,1,0}, {5,0,1,1}, {0,5,0,1}, {1,0,5,0}};
	int d711[5][4] = {{3,0,1,2}, {1,3,0,1}, {2,1,3,0}, {1,2,1,3}, {0,1,2,1}};
	int d712[5][4] = {{1,1,0,1}, {4,1,1,0}, {1,4,1,1}, {0,1,4,1}, {1,0,1,4}};
	int d713[5][4] = {{1,1,0,4}, {1,1,1,0}, {4,1,1,1}, {0,4,1,1}, {1,0,4,1}};
	int m711[5][4] = {{2,1,1,2}, {1,2,1,1}, {2,1,2,1}, {1,2,1,2}, {1,1,2,1}};
	int m712[5][4] = {{1,0,1,1}, {4,1,0,1}, {1,4,1,0}, {1,1,4,1}, {0,1,1,4}};
	int di11[5][4] = {{1,0,1,0}, {5,1,0,1}, {0,5,1,0}, {1,0,5,1}, {0,1,0,5}};
	int mM11[5][4] = {{2,1,1,3}, {0,2,1,1}, {3,0,2,1}, {1,3,0,2}, {1,1,3,0}};
	int dM11[5][4] = {{2,1,0,4}, {0,2,1,0}, {4,0,2,1}, {0,4,0,2}, {1,0,4,0}};
	int Meb5[5][4] = {{3,0,0,4}, {0,3,0,0}, {4,0,3,0}, {0,4,0,3}, {0,0,4,0}};
	int Mes5[5][4] = {{3,0,2,2}, {0,3,0,2}, {2,0,3,0}, {2,2,0,3}, {0,2,2,0}};
	int Meb9[5][4] = {{0,2,0,5}, {0,0,2,0}, {5,0,0,2}, {0,5,0,0}, {2,0,5,0}};
	int Mes9[5][4] = {{2,0,0,5}, {0,2,0,0}, {5,0,2,0}, {0,5,0,2}, {0,0,5,0}};
	int Deb5[5][4] = {{3,0,0,3}, {1,3,0,0}, {3,1,3,0}, {0,3,1,3}, {0,0,3,1}};
	int Mes3[5][4] = {{3,0,4,0}, {0,3,0,4}, {0,0,3,0}, {4,0,0,3}, {0,4,0,0}};
	int Deb9[5][4] = {{0,2,0,4}, {1,0,2,0}, {4,1,0,2}, {0,4,1,0}, {2,0,4,1}};
	int De91[5][4] = {{0,2,0,1}, {4,0,2,0}, {1,4,0,2}, {0,1,4,0}, {2,0,1,4}};
	int Des9[5][4] = {{2,0,0,4}, {1,2,0,0}, {4,1,2,0}, {0,4,1,2}, {0,0,4,1}};
	int Ds11[5][4] = {{3,1,0,2}, {1,3,1,0}, {2,1,3,1}, {0,2,1,3}, {1,0,2,1}};
	int m7s1[5][4] = {{2,2,0,2}, {1,2,2,0}, {2,1,2,2}, {0,2,1,2}, {2,0,2,1}};
	int D3s1[5][4] = {{5,0,1,0}, {1,5,0,1}, {0,1,5,0}, {1,0,1,5}, {0,1,0,1}};
	int Mb9s[5][4] = {{0,2,5,0}, {0,0,2,5}, {0,0,0,2}, {5,0,0,0}, {2,5,0,0}};
	int D7b3[5][4] = {{3,2,0,1}, {1,3,2,0}, {1,1,3,2}, {0,1,1,3}, {2,0,1,1}};

	if (!initialized) {
		for (i=0; i<8; i++)
			for (j=0; j<8; j++)
				for (k=0; k<8; k++)
					for (l=0; l<8; l++) {
						quintads[i][j][k][l].type		= kNone;
						quintads[i][j][k][l].rootMember = kXX;
					}


		// major ninths
		for (i=0; i<5; i++) {
			st = maj9[i];
			t  = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			t->type		  = kMaj9;
			t->rootMember = i;
		}

		// dominant ninths
		for (i=0; i<5; i++) {
			st = dom9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "dom9");
			t->type		  = kDom9;
			t->rootMember = i;
		}

		// minor ninths
		for (i=0; i<5; i++) {
			st = min9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "min9");
			t->type		  = kMin9;
			t->rootMember = i;
		}

		// half diminished ninths
		for (i=0; i<5; i++) {
			st = had9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "had9");
			t->type		  = kHalfDim9;
			t->rootMember = i;
		}

		// minor/major ninths
		for (i=0; i<5; i++) {
			st = miM9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "miM9");
			t->type		  = kMinMaj9;
			t->rootMember = i;
		}

		// diminished/major ninths
		for (i=0; i<5; i++) {
			st = diM9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "diM9");
			t->type		  = kDimMaj9;
			t->rootMember = i;
		}

		// major ninth flat 5
		for (i=0; i<5; i++) {
			st = M9b5[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M9b5");
			t->type		  = kMaj9b5;
			t->rootMember = i;
		}

		// dominant ninth flat 5
		for (i=0; i<5; i++) {
			st = D9b5[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "D9b5");
			t->type		  = kDom9b5;
			t->rootMember = i;
		}

		// minor/major ninth flat 11
		for (i=0; i<5; i++) {
			st = mM91[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "mM91");
			t->type		  = kmM9b11;
			t->rootMember = i;
		}

		// major seventh flat nine
		for (i=0; i<5; i++) {
			st = M7b9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M7b9");
			t->type		  = kMaj7b9;
			t->rootMember = i;
		}

		// major seventh sharp five flat nine
		for (i=0; i<5; i++) {
			st = M5b9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M5b9");
			t->type		  = kMaj7s5b9;
			t->rootMember = i;
		}

		// dominant seventh flat nine
		for (i=0; i<5; i++) {
			st = D7b9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "D7b9");
			t->type		  = kDom7b9;
			t->rootMember = i;
		}

		// minor seventh flat nine
		for (i=0; i<5; i++) {
			t = &(quintads[m7b9[i][0]][m7b9[i][1]][m7b9[i][2]][m7b9[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "m7b9");
			t->type		  = kMin7b9;
			t->rootMember = i;
		}

		// minor flat nine sharp eleventh
		for (i=0; i<5; i++) {
			st = mb51[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "mb51");
			t->type		  = kMinb9s11;
			t->rootMember = i;
		}

		// half diminished seventh flat nine
		for (i=0; i<5; i++) {
			st = d7b9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "d7b9");
			t->type		  = kHalfDimb9;
			t->rootMember = i;
		}

		// minor/major  seventh flat nine
		for (i=0; i<5; i++) {
			st = mMb9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "mMb9");
			t->type		  = kMinMajb9;
			t->rootMember = i;
		}

		// diminished major seventh flat nine
		for (i=0; i<5; i++) {
			st = dMb9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "dMb9");
			t->type		  = kDimMajb9;
			t->rootMember = i;
		}

		// diminished seventh flat nine
		for (i=0; i<5; i++) {
			t = &(quintads[dib9[i][0]][dib9[i][1]][dib9[i][2]][dib9[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "dib9");
			t->type		  = kDim7b9;
			t->rootMember = i;
		}

		// major seventh sharp nine
		for (i=0; i<5; i++) {
			t = &(quintads[M7s9[i][0]][M7s9[i][1]][M7s9[i][2]][M7s9[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M7s9");
			t->type		  = kMaj7s9;
			t->rootMember = i;
		}

		// dominant seventh sharp nine
		for (i=0; i<5; i++) {
			t = &(quintads[D7s9[i][0]][D7s9[i][1]][D7s9[i][2]][D7s9[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "D7s9");
			t->type		  = kDom7s9;
			t->rootMember = i;
		}

		// major seventh sharp eleventh
		for (i=0; i<5; i++) {
			t = &(quintads[M7s1[i][0]][M7s1[i][1]][M7s1[i][2]][M7s1[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M7s1");
			t->type		  = kMaj7s11;
			t->rootMember = i;
		}

		// dominant ninth flat thirteenth
		for (i=0; i<5; i++) {
			st = d9b3[i];
			t  = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "d9b3");
			t->type		  = kDom9b13;
			t->rootMember = i;
		}

		// major ninth sharp thirteenth
		for (i=0; i<5; i++) {
			st = M9s3[i];
			t  = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M9s3");
			t->type		  = kMaj9s13;
			t->rootMember = i;
		}

		// major ninth sharp thirteenth
		for (i=0; i<5; i++) {
			t = &(quintads[M9st[i][0]][M9st[i][1]][M9st[i][2]][M9st[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M9st");
			t->type		  = kMaj9s13;
			t->rootMember = i;
		}

		// major chord sharp ninth sharp eleventh
		for (i=0; i<5; i++) {
			st = s9s1[i];
			t  = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "s9s1");
			t->type		  = kMs9s11;
			t->rootMember = i;
		}

		// half diminished seven flat 11
		for (i=0; i<5; i++) {
			st = h7b1[i];
			t  = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "h7b1");
			t->type		  = kHDimb11;
			t->rootMember = i;
		}

		// major eleventh
		for (i=0; i<5; i++) {
			t = &(quintads[M711[i][0]][M711[i][1]][M711[i][2]][M711[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M711");
			t->type		  = kMaj11;
			t->rootMember = i;
		}

		// major eleventh
		for (i=0; i<5; i++) {
			t = &(quintads[M115[i][0]][M115[i][1]][M115[i][2]][M115[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M711");
			t->type		  = kMaj11;
			t->rootMember = i;
		}

		// dominant eleventh
		for (i=0; i<5; i++) {
			t = &(quintads[d711[i][0]][d711[i][1]][d711[i][2]][d711[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "d711");
			t->type		  = kDom11;
			t->rootMember = i;
		}

		// dominant eleventh
		for (i=0; i<5; i++) {
			t = &(quintads[d712[i][0]][d712[i][1]][d712[i][2]][d712[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "d712");
			t->type		  = kDom11;
			t->rootMember = i;
		}

		// dominant eleventh
		for (i=0; i<5; i++) {
			st = d713[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "d713");
			t->type		  = kDom11;
			t->rootMember = i;
		}

		// minor eleventh
		for (i=0; i<5; i++) {
			t = &(quintads[m711[i][0]][m711[i][1]][m711[i][2]][m711[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "m711");
			t->type		  = kMin11;
			t->rootMember = i;
		}

		// minor eleventh
		for (i=0; i<5; i++) {
			t = &(quintads[m712[i][0]][m712[i][1]][m712[i][2]][m712[i][3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "m712");
			t->type		  = kMin11;
			t->rootMember = i;
		}

		// diminished eleventh
		for (i=0; i<5; i++) {
			st = di11[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "di11");
			t->type		  = kDim11;
			t->rootMember = i;
		}

		// minor/major eleventh
		for (i=0; i<5; i++) {
			st = mM11[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "mM11");
			t->type		  = kMinMaj11;
			t->rootMember = i;
		}

		// diminished major eleventh
		for (i=0; i<5; i++) {
			st = dM11[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "dM11");
			t->type		  = kDimMaj11;
			t->rootMember = i;
		}

		// major eleventh flat fifth
		for (i=0; i<5; i++) {
			st = Meb5[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "Meb5");
			t->type		  = kMaj11b5;
			t->rootMember = i;
		}

		// major eleventh sharp fifth
		for (i=0; i<5; i++) {
			st = Mes5[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "Mes5");
			t->type		  = kMaj11s5;
			t->rootMember = i;
		}

		// major eleventh flat ninth
		for (i=0; i<5; i++) {
			st = Meb9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "Meb9");
			t->type		  = kMaj11b9;
			t->rootMember = i;
		}

		// major eleventh sharp ninth
		for (i=0; i<5; i++) {
			st = Mes9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "Mes9");
			t->type		  = kMaj11s9;
			t->rootMember = i;
		}

		// major eleventh sharp thirteenth
		for (i=0; i<5; i++) {
			st = Mes3[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "Mes3");
			t->type		  = kMaj11s13;
			t->rootMember = i;
		}

		// dominant eleventh flat fifth
		for (i=0; i<5; i++) {
			st = Deb5[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "Deb5");
			t->type		  = kDom11b5;
			t->rootMember = i;
		}

		// dominant eleventh flat ninth
		for (i=0; i<5; i++) {
			st = Deb9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "Deb9");
			t->type		  = kDom11b9;
			t->rootMember = i;
		}

		// dominant eleventh flat ninth
		for (i=0; i<5; i++) {
			st = De91[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "De91");
			t->type		  = kDom11b9;
			t->rootMember = i;
		}

		// dominant eleventh sharp ninth
		for (i=0; i<5; i++) {
			st = Des9[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "Des9");
			t->type		  = kDom11s9;
			t->rootMember = i;
		}

		// dominant seventh sharp eleventh
		for (i=0; i<5; i++) {
			st = Ds11[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "Ds11");
			t->type		  = kDom7s11;
			t->rootMember = i;
		}

		// minor seventh sharp eleventh
		for (i=0; i<5; i++) {
			st = m7s1[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "m7s1");
			t->type		  = kMin7s11;
			t->rootMember = i;
		}

		// dominant thirteenth sharp eleventh
		for (i=0; i<5; i++) {
			st = D3s1[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "D3s1");
			t->type		  = kDom13s11;
			t->rootMember = i;
		}

		// major seventh flat ninth sharp thirteenth
		for (i=0; i<5; i++) {
			st = Mb9s[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "Mb9s");
			t->type		  = kM7b9s13;
			t->rootMember = i;
		}

		// dominant seventh flat thirteenth
		for (i=0; i<5; i++) {
			st = D7b3[i];
			t = &(quintads[st[0]][st[1]][st[2]][st[3]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "D7b3");
			t->type		  = kDom7b13;
			t->rootMember = i;
		}

		initialized = 1;
		return;
	}

	j = 0;
	for (i=0; i<12; i++)
		if (x->x_pc[i]) members[j++] = i;		/* load members array with chord pitch classes */
	interval1 = members[1] - members[0];		/* calculate interval between first two members */
	interval2 = members[2] - members[0];		/* calculate interval between first and third */
	interval3 = members[3] - members[0];		/* calculate interval between first and third */
	interval4 = members[4] - members[0];		/* calculate interval between first and fourth */
	interval4 = interval4 - interval3 - 1;		/* reduce interval4 to start at zero */
	interval3 = interval3 - interval2 - 1;		/* reduce interval3 to start at zero */
	interval2 = interval2 - interval1 - 1;		/* reduce interval2 to start at zero */
	interval1 = interval1 - 1;					/* reduce interval1 to start at zero */

	// find TypeRoot struct for this interval set
	t = &(quintads[interval1][interval2][interval3][interval4]);
	if (t->rootMember != kXX)
	{
		x->x_chord_type = t->type;
		x->x_chord_root = members[t->rootMember];
		switch(t->rootMember) {						/* get state of inversion */
			case 0: 
				x->x_chord_inversion = 0;
				break;
			case 1: 
				x->x_chord_inversion = 2;
				break;
			case 2: 
				x->x_chord_inversion = 2;
				break;
			case 3: 
				x->x_chord_inversion = 2;
				break;
			case 4: 
				x->x_chord_inversion = 1;
		}
		chord_draw_chord_type(x, 5);				/* output result */
	} else
		chord_kick_out_member(x, 5, members);
}

static void chord_sextad(t_chord *x)
{
	static int initialized = 0;
	static t_type_root sextads[7][7][7][7][7];
	register int i, j, k, l, m;
	register t_type_root *t;
	register int* st;
	t_int members[6];
	int interval1, interval2, interval3, interval4, interval5;

	int D9b3[6][5] =
		{{1,1,2,0,1}, {1,1,1,2,0}, {1,1,1,1,2}, {0,1,1,1,1}, {2,0,1,1,1}, {1,2,0,1,1}};
	int m9s1[6][5] =
		{{1,0,2,0,2}, {1,1,0,2,0}, {2,1,1,0,2}, {0,2,1,1,0}, {2,0,2,1,1}, {0,2,0,2,1}};
	int M711[6][5] =
		{{1,1,0,1,3}, {0,1,1,0,1}, {3,0,1,1,0}, {1,3,0,1,1}, {0,1,3,0,1}, {1,0,1,3,0}};
	int D711[6][5] =
		{{1,1,0,1,2}, {1,1,1,0,1}, {2,1,1,1,0}, {1,2,1,1,1}, {0,1,2,1,1}, {1,0,1,2,1}};
	int hd11[6][5] =
		{{1,0,1,0,3}, {1,1,0,1,0}, {3,1,1,0,1}, {0,3,1,1,0}, {1,0,3,1,1}, {0,1,0,3,1}};
	int M1b5[6][5] =
		{{1,1,0,0,4}, {0,1,1,0,0}, {4,0,1,1,0}, {0,4,0,1,1}, {0,0,4,0,1}, {1,0,0,4,0}};
	int M159[6][5] =
		{{0,2,0,0,4}, {0,0,2,0,0}, {4,0,0,2,0}, {0,4,0,0,2}, {0,0,4,0,0}, {2,0,0,4,0}};
	int M1s3[6][5] =
		{{1,1,0,4,0}, {0,1,1,0,4}, {0,0,1,1,0}, {4,0,0,1,1}, {0,4,0,0,1}, {1,0,4,0,0}};
	int hd19[6][5] =
		{{0,1,1,0,3}, {1,0,1,1,0}, {3,1,0,1,1}, {0,3,1,0,1}, {1,0,3,1,0}, {1,1,0,3,1}};
	int M1b3[6][5] =
		{{3,0,1,0,2}, {0,3,0,1,0}, {2,0,3,0,1}, {0,2,0,3,0}, {1,0,2,0,3}, {0,1,0,2,0}};
	int D1b5[6][5] =
		{{1,1,0,0,3}, {1,1,1,0,0}, {3,1,1,1,0}, {0,3,1,1,1}, {0,0,3,1,1}, {1,0,0,3,1}};
	int D1s9[6][5] =
		{{2,0,0,1,2}, {1,2,0,0,1}, {2,1,2,0,0}, {1,2,1,2,0}, {0,1,2,1,2}, {0,0,1,2,1}};
	int m791[6][5] =
		{{0,1,2,0,2}, {1,0,1,2,0}, {2,1,0,1,2}, {0,2,1,0,1}, {2,0,2,1,0}, {1,2,0,2,1}};
	int d7s1[6][5] =
		{{1,1,1,0,2}, {1,1,1,1,0}, {2,1,1,1,1}, {0,2,1,1,1}, {1,0,2,1,1}, {1,1,0,2,1}};
	int d3s1[6][5] =
		{{3,1,0,1,0}, {1,3,1,0,1}, {0,1,3,1,0}, {1,0,1,3,1}, {0,1,0,1,3}, {1,0,1,0,1}};


	if (!initialized) {
		for (i=0; i<7; i++)
			for (j=0; j<7; j++)
				for (k=0; k<7; k++)
					for (l=0; l<7; l++) 
						for (m=0; m<7; m++) {
							sextads[i][j][k][l][m].type		= kNone;
							sextads[i][j][k][l][m].rootMember = kXX;
						}

		// dominant ninth flat thirteen
		for (i=0; i<6; i++) {
			st = D9b3[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "D9b3");
			t->type		  = kDom9b13;
			t->rootMember = i;
		}

		// minor ninth sharp eleventh
		for (i=0; i<6; i++) {
			st = m9s1[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "m9s1");
			t->type		  = kMin9s11;
			t->rootMember = i;
		}

		// major eleventh
		for (i=0; i<6; i++) {
			st = M711[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M711");
			t->type		  = kMaj11;
			t->rootMember = i;
		}

		// dominant eleventh
		for (i=0; i<6; i++) {
			st = D711[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "D711");
			t->type		  = kDom11;
			t->rootMember = i;
		}

		// half diminished eleventh
		for (i=0; i<6; i++) {
			st = hd11[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "hd11");
			t->type		  = kHalfDim11;
			t->rootMember = i;
		}

		// major eleventh flat 5
		for (i=0; i<6; i++) {
			st = M1b5[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M1b5");
			t->type		  = kMaj11b5;
			t->rootMember = i;
		}

		// major eleventh flat 5 flat 9
		for (i=0; i<6; i++) {
			st = M159[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M159");
			t->type		  = kM11b5b9;
			t->rootMember = i;
		}

		// major eleventh sharp 13
		for (i=0; i<6; i++) {
			st = M1s3[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M1s3");
			t->type		  = kMaj11s13;
			t->rootMember = i;
		}

		// half diminished eleventh flat 9
		for (i=0; i<6; i++) {
			st = hd19[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "hd19");
			t->type		  = kHalfDim11b9;
			t->rootMember = i;
		}

		// major eleventh flat 13
		for (i=0; i<6; i++) {
			st = M1b3[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "M1b3");
			t->type		  = kMaj11b13;
			t->rootMember = i;
		}

		// dominant eleventh flat five
		for (i=0; i<6; i++) {
			st = D1b5[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "D1b5");
			t->type		  = kDom11b5;
			t->rootMember = i;
		}

		// dominant eleventh sharp nine
		for (i=0; i<6; i++) {
			st = D1s9[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "D1s9");
			t->type		  = kDom11s9;
			t->rootMember = i;
		}

		// minor seventh flat 9 sharp 11
		for (i=0; i<6; i++) {
			st = m791[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "m791");
			t->type		  = kMinb9s11;
			t->rootMember = i;
		}

		// dominant seventh sharp 11
		for (i=0; i<6; i++) {
			st = d7s1[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "d7s1");
			t->type		  = kDom7s11;
			t->rootMember = i;
		}

		// dominant thirteenth sharp 11
		for (i=0; i<6; i++) {
			st = d3s1[i];
			t = &(sextads[st[0]][st[1]][st[2]][st[3]][st[4]]);
			if (t->type != kNone) chord_fatal_error("redefining chord", "d3s1");
			t->type		  = kDom13s11;
			t->rootMember = i;
		}

		initialized = 1;
		return;
	}

	j = 0;
	for (i=0; i<12; i++)
		if (x->x_pc[i]) members[j++] = i;		// load members array with chord pitch classes
	interval1 = members[1] - members[0];		// calculate interval between first two members
	interval2 = members[2] - members[0];		// calculate interval between first and third
	interval3 = members[3] - members[0];		// calculate interval between first and third
	interval4 = members[4] - members[0];		// calculate interval between first and fourth
	interval5 = members[5] - members[0];		// calculate interval between first and fifth
	interval5 = interval5 - interval4 - 1;		// reduce interval5 to start at zero
	interval4 = interval4 - interval3 - 1;		// reduce interval4 to start at zero
	interval3 = interval3 - interval2 - 1;		// reduce interval3 to start at zero
	interval2 = interval2 - interval1 - 1;		// reduce interval2 to start at zero
	interval1 = interval1 - 1;					// reduce interval1 to start at zero

	// find TypeRoot struct for this interval set
	t = &(sextads[interval1][interval2][interval3][interval4][interval5]);
	if (t->rootMember != kXX) {
		x->x_chord_type = t->type;
		x->x_chord_root = members[t->rootMember];
		switch(t->rootMember) {						/* get state of inversion */
			case 0: 
				x->x_chord_inversion = 0;
				break;
			case 1: 
				x->x_chord_inversion = 2;
				break;
			case 2: 
				x->x_chord_inversion = 2;
				break;
			case 3: 
				x->x_chord_inversion = 2;
				break;
			case 4: 
				x->x_chord_inversion = 2;
				break;
			case 5: x->x_chord_inversion = 1;
		}
		chord_draw_chord_type(x, 6);						// output onto the screen
	} else
		chord_kick_out_member(x, 6, members);
}

static int chord_accidental(t_int pc)
{
	switch (pc) {
		case  0:
		case  2:
		case  4:
		case  5:
		case  7:
		case  9:
		case 11:	return 0;
		case  1:
		case  3:
		case  6:
		case  8:
		case 10:	
		default:	return 1;
	}
}

static int chord_name_third(t_chord *x, char* chord, int c, int rootName)
{
	int third = (x->x_chord_root+4)%12;		// look for major third
	if (x->x_pc[third]) {					// if one is there
		x->x_pc[third] = 0;					// erase from pcs array
		chord[c++] = name_class[(rootName+2)%7];
		if (chord_accidental(third)) {			// if it has an chord_accidental
			// make it a flat if the root also has an chord_accidental
			if (chord_accidental(x->x_chord_root)) 
				chord[c++] = 'b';
			// otherwise make it a sharp
			else					   
				chord[c++] = '#';
		}
		chord[c++] = ' ';
		return c;						// return if major third found
	}

	third = (x->x_chord_root+3)%12;			// no major, look for minor third
	if (x->x_pc[third]) {					// if one is there
		x->x_pc[third] = 0;					// erase from pcs array
		chord[c++] = name_class[(rootName+2)%7];
		if (chord_accidental(third))			// if it has an chord_accidental
			chord[c++] = 'b'; else		// make it a flat
		if (chord_accidental(x->x_chord_root)) {	// if the root has an chord_accidental
			chord[c++] = 'b';			// make the third a flat
			if (chord[0] == 'G')		// if the root is Gb
				chord[c++] = 'b';		// this must be Bbb
		}
		chord[c++] = ' ';
		return c;
	}

	return c;							// if we get here there was no third
}

static int chord_name_fifth(t_chord *x, char* chord, int c, int rootName)
{
	int fifth = (x->x_chord_root+7)%12;
	if (x->x_pc[fifth]) {
		x->x_pc[fifth] = 0;
		chord[c++] = name_class[(rootName+4)%7];
		if (chord_accidental(fifth)) {
			if (chord_accidental(x->x_chord_root)) chord[c++] = 'b';
			else					   chord[c++] = '#';
		}
		chord[c++] = ' ';
		return c;
	}

	fifth = (x->x_chord_root+6)%12;
	if (x->x_pc[fifth]) {
		x->x_pc[fifth] = 0;
		chord[c++] = name_class[(rootName+4)%7];
		if (chord[0] != 'B')	   chord[c++] = 'b';
		if (chord_accidental(x->x_chord_root)) chord[c++] = 'b';
		chord[c++] = ' ';
		return c;
	}
	
	fifth = (x->x_chord_root+8)%12;
	if (x->x_pc[fifth]) {
		x->x_pc[fifth] = 0;
		chord[c++] = name_class[(rootName+4)%7];
		if (chord_accidental(fifth)) chord[c++] = '#'; else
		if (!chord_accidental(x->x_chord_root)) {
			chord[c++] = '#';
			if (chord[0] == 'B')
				chord[c++] = '#';
		}
		chord[c++] = ' ';
		return c;
	}
	
	return c;
}

static int chord_name_seventh(t_chord *x, char* chord, int c, int rootName)
{
	int seventh = (x->x_chord_root+11)%12;
	if (x->x_pc[seventh]) {
		x->x_pc[seventh] = 0;
		chord[c++] = name_class[(rootName+6)%7];
		if (chord_accidental(seventh)) chord[c++] = '#';
		chord[c++] = ' ';
		return c;
	}
	seventh = (x->x_chord_root+10)%12;
	if (x->x_pc[seventh]) {
		x->x_pc[seventh] = 0;
		chord[c++]   = name_class[(rootName+6)%7];
		if (chord_accidental(seventh) || chord_accidental(x->x_chord_root))
			chord[c++] = 'b';
		chord[c++]   = ' ';
		return c;
	}
	seventh = (x->x_chord_root+9)%12;
	if (x->x_pc[seventh]) {
		x->x_pc[seventh] = 0;
		chord[c++] = name_class[(rootName+6)%7];
		chord[c++] = 'b';
		if (chord_accidental(x->x_chord_root))		chord[c++] = 'b'; else
		if (chord_accidental((seventh+1)%12)) chord[c++] = 'b';
		chord[c++] = ' ';
		return c;
	}
	return c;
}

static int chord_name_ninth(t_chord *x, char* chord, int c, int rootName)
{
	int ninth = (x->x_chord_root+2)%12;
	if (x->x_pc[ninth]) {
		x->x_pc[ninth] = 0;
		chord[c++] = name_class[(rootName+1)%7];
		if (chord_accidental(ninth)) {
			if (chord_accidental(x->x_chord_root)) chord[c++] = 'b';
			else					   chord[c++] = '#';
		}
		chord[c++] = ' ';
		return c;
	}

	ninth = (x->x_chord_root+1)%12;
	if (x->x_pc[ninth]) {
		x->x_pc[ninth] = 0;
		chord[c++] = name_class[(rootName+1)%7];
		if (chord_accidental(ninth)) chord[c++] = 'b';
		else {
			if (chord_accidental(x->x_chord_root)) {
				chord[c++] = 'b';
				if ((x->x_chord_root == 1) || (x->x_chord_root == 6) || (x->x_chord_root == 8))
					chord[c++] = 'b';
			}
		}
		chord[c++] = ' ';
		return c;
	}

	ninth = (x->x_chord_root+3)%12;
	if (x->x_pc[ninth]) {
		x->x_pc[ninth] = 0;
		chord[c++] = name_class[(rootName+1)%7];
		if (chord_accidental(ninth)) chord[c++] = '#'; else
		if (!chord_accidental(x->x_chord_root)) {
			chord[c++] = '#';
			if (chord_accidental((x->x_chord_root+2)%12))
				chord[c++] = '#';
		}
		chord[c++] = ' ';
		return c;
	}

	return c;
}

static int chord_name_eleventh(t_chord *x, char* chord, int c, int rootName)
{
	int eleventh = (x->x_chord_root+5)%12;
	if (x->x_pc[eleventh]) {
		x->x_pc[eleventh] = 0;
		chord[c++] = name_class[(rootName+3)%7];
		if (chord_accidental(eleventh))  chord[c++] = 'b'; else
		if (chord_accidental(x->x_chord_root)) chord[c++] = 'b';
		chord[c++] = ' ';
		return c;
	}
	
	eleventh = (x->x_chord_root+6)%12;
	if (x->x_pc[eleventh]) {
		x->x_pc[eleventh] = 0;
		chord[c++] = name_class[(rootName+3)%7];
		if (chord_accidental(eleventh))  chord[c++] = '#'; else
		if ((!chord_accidental(x->x_chord_root)) && (x->x_chord_root == 11))
			chord[c++] = '#';
		chord[c++] = ' ';
		return c;
	}

	return c;
}

static int chord_name_thirteenth(t_chord *x, char* chord, int c, int rootName)
{
	int thirteenth = (x->x_chord_root+9)%12;
	if (x->x_pc[thirteenth]) {
		x->x_pc[thirteenth] = 0;
		chord[c++] = name_class[(rootName+5)%7];
		if (chord_accidental(thirteenth)) {
			if (chord_accidental(x->x_chord_root))
				chord[c++] = 'b'; 
			else
				chord[c++] = '#';
		}
		
		chord[c++] = ' ';
		return c;
	}
	
	thirteenth = (x->x_chord_root+10)%12;
	if (x->x_pc[thirteenth]) {
		x->x_pc[thirteenth] = 0;
		chord[c++] = name_class[(rootName+5)%7];
		if (chord_accidental(thirteenth)) chord[c++] = '#'; else
		if (!chord_accidental(x->x_chord_root)) {
			chord[c++] = '#';
			if (chord_accidental((x->x_chord_root+9)%12))
				chord[c++] = '#';
		}
		chord[c++] = ' ';
		return c;
	}

	thirteenth = (x->x_chord_root+8)%12;
	if (x->x_pc[thirteenth]) {
		x->x_pc[thirteenth] = 0;
		chord[c++] = name_class[(rootName+5)%7];
		if (chord_accidental(thirteenth)) chord[c++] = 'b'; else
		if (chord_accidental(x->x_chord_root)) {
			chord[c++] = 'b';
			if (chord_accidental(x->x_chord_root+9)%12)
				chord[c++] = 'b';
		}
		chord[c++] = ' ';
		return c;
	}

	return c;
}



static void chord_spell_chord(t_chord *x, char *chord, t_int num_pcs)
{
	int rootName = 0;			// keep index of root name class
	int c     = 0;				// pointer to current character
	int named = 0;				// how many members have been named
	int mark;
	int i;

	// use chordRoot to set rootName index and store characters for name
	switch (x->x_chord_root)
	{
		case 0:		chord[c++] = name_class[rootName=0];	break;
		case 1:		chord[c++] = name_class[rootName=1];
				chord[c++] = 'b';			break;
		case 2:		chord[c++] = name_class[rootName=1];	break;
		case 3:		chord[c++] = name_class[rootName=2];
				chord[c++] = 'b';			break;
		case 4:		chord[c++] = name_class[rootName=2];	break;
		case 5:		chord[c++] = name_class[rootName=3];	break;
		case 6:		chord[c++] = name_class[rootName=4];
				chord[c++] = 'b';			break;
		case 7:		chord[c++] = name_class[rootName=4];	break;
		case 8:		chord[c++] = name_class[rootName=5];
				chord[c++] = 'b';			break;
		case 9:		chord[c++] = name_class[rootName=5];	break;
		case 10:	chord[c++] = name_class[rootName=6];
				chord[c++] = 'b';			break;
		case 11:	chord[c++] = name_class[rootName=6];	break;
		default:						break;
	}
	x->x_pc[x->x_chord_root] = 0;					/* set this member to zero */

	chord[c++] = ' ';			// insert space
	if (++named == num_pcs) {	// if everything is named
		chord[c] = '\0';		// terminate the string
		return;					// and return
	}

	mark = c;				// use mark to see if new names are added
	for (i=0; i<6; i++) {
		// advance search by thirds
		switch (i) {
			case 0:	mark = chord_name_third	 (x, chord, c, rootName); break;
			case 1:	mark = chord_name_fifth	 (x, chord, c, rootName); break;
			case 2:	mark = chord_name_seventh	 (x, chord, c, rootName); break;
			case 3:	mark = chord_name_ninth	 (x, chord, c, rootName); break;
			case 4:	mark = chord_name_eleventh	 (x, chord, c, rootName); break;
			case 5:	mark = chord_name_thirteenth(x, chord, c, rootName); break;
		}
		if (mark != c) {		// if new name is added
			++named;			// increment count of named members
			c = mark;			// update character pointer
		}
		if (named == num_pcs) {	// if everything is named
			chord[c] = '\0';	// terminate the string
			return;				// and return
		}
	}

	chord[c] = '\0';
}


static void chord_draw_chord_type(t_chord *x, t_int num_pcs)
{
	char   chord[255];				/* output string */
	int i, j;

		/* get members of chord */
	j = 0;
	for(i = 0; i < 12; i++)
	{
		if(x->x_pc[i])
		{
			SETFLOAT(x->x_chordlist+j, x->x_abs_pc[i]);
			j++;
		}
	}

	if (x->x_chord_type != kNone)
	{
		chord_spell_chord(x, chord, num_pcs);	/* spell chord members */
	}
	else
	{
		post("going...");
		chord[0] = '\0';
		for(i = 0; i < 12; i++)
			if (x->x_pc[i]) 
				strcat(chord, pitch_class[i]);	/* output single notes */
		post("did it");
	}

	strcat(chord, ": ");
	strcat(chord, pitch_class[x->x_chord_root]);
	
		/* append name of chord type */
	switch (x->x_chord_type) {
		case kUnison:	strcat(chord, "unison"); 			  break;
		case kMaj:		strcat(chord, "major"); 			  break;
		case kMin:		strcat(chord, "minor"); 			  break;
		case kDim:		strcat(chord, "diminished"); 		  break;
		case kAug:		strcat(chord, "augmented"); 		  break;

		case kMaj7:		strcat(chord, "major 7th"); 		  break;
		case kDom7:		strcat(chord, "dominant 7th"); 		  break;
		case kMin7: 	strcat(chord, "minor 7th"); 		  break;
		case kHalfDim7:	strcat(chord, "half diminished 7th"); break;
		case kDim7:		strcat(chord, "diminished 7th");	  break;
		case kMinMaj7:	strcat(chord, "minor/major 7th");	  break;

		case kMaj7s5:	strcat(chord, "major 7th #5");		  break;
		case kMaj7b5:	strcat(chord, "major 7th b5");		  break;
		case kDom7s5:	strcat(chord, "dominant 7th #5"); 	  break;
		case kDom7b5:	strcat(chord, "dominant 7th b5"); 	  break;
		case kDomb9:	strcat(chord, "dominant b9");		  break;

		case kMaj9:		strcat(chord, "major 9th");			  break;
		case kDom9:		strcat(chord, "dominant 9th");		  break;
		case kMin9:		strcat(chord, "minor 9th");			  break;
		case kHalfDim9:	strcat(chord, "half diminished 9th"); break;
		case kMinMaj9:	strcat(chord, "minor major 9th");	  break;
		case kDimMaj9:	strcat(chord, "diminished major 9th");break;
		case kMaj9b5:	strcat(chord, "major 9th b5");		  break;
		case kDom9b5:	strcat(chord, "dominant 9th b5");	  break;
		case kDom9b13:	strcat(chord, "dominant 9th b13");	  break;
		case kMin9s11:	strcat(chord, "minor 9th #11");		  break;
		case kmM9b11:	strcat(chord, "minor/maj 9th b11");	  break;

		case kMaj7b9:	strcat(chord, "major 7th b9");		  break;
		case kMaj7s5b9:	strcat(chord, "major 7th #5 b9");	  break;
		case kDom7b9:	strcat(chord, "dominant 7th b9");	  break;
		case kMin7b9:	strcat(chord, "minor 7th b9");		  break;
		case kMinb9s11:	strcat(chord, "minor b9 #11");		  break;
		case kHalfDimb9:strcat(chord, "half diminished b9");  break;
		case kDim7b9:	strcat(chord, "diminished b9");		  break;
		case kMinMajb9: strcat(chord, "minor/major b9");	  break;
		case kDimMajb9:	strcat(chord, "diminished M7 b9");	  break;

		case kMaj7s9:	strcat(chord, "major 7th #9");		  break;
		case kDom7s9:	strcat(chord, "dominant #9");		  break;
		case kMaj7s11:	strcat(chord, "major 7th #11");		  break;
		case kMaj9s13:	strcat(chord, "major 9th #13");		  break;
		case kMs9s11:	strcat(chord, "major #9 #11");		  break;
		case kHDimb11:	strcat(chord, "half diminished b11"); break;

		case kMaj11:	strcat(chord, "major 11th");		  break;
		case kDom11:	strcat(chord, "dominant 11th");		  break;
		case kMin11:	strcat(chord, "minor 11th");		  break;
		case kHalfDim11:strcat(chord, "half diminished 11th");break;
		case kDim11:	strcat(chord, "diminished 11th");	  break;
		case kMinMaj11:	strcat(chord, "minor/major 11th");	  break;
		case kDimMaj11: strcat(chord, "diminished maj 11th"); break;

		case kMaj11b5:	strcat(chord, "major 11th b5");		  break;
		case kMaj11s5:	strcat(chord, "major 11th #5");		  break;
		case kMaj11b9:	strcat(chord, "major 11th b9");		  break;
		case kMaj11s9:	strcat(chord, "major 11th #9");		  break;
		case kMaj11b13:	strcat(chord, "major 11th b13");	  break;
		case kMaj11s13:	strcat(chord, "major 11th #13");	  break;
		case kM11b5b9:	strcat(chord, "major 11th b5 b9");	  break;
		case kDom11b5:	strcat(chord, "dominant 11th b5");	  break;
		case kDom11b9:	strcat(chord, "dominant 11th b9");	  break;
		case kDom11s9:	strcat(chord, "dominant 11th #9");	  break;
		case kHalfDim11b9:strcat(chord, "half dim 11th b9");  break;
		case kDom7s11:	strcat(chord, "dominant #11");		  break;
		case kMin7s11:	strcat(chord, "minor 7th #11");		  break;

		case kDom13s11:	strcat(chord, "dominant 13th #11");	  break;
		case kM7b913:	strcat(chord, "major 7 b9 13");		  break;
		case kMaj7s13:	strcat(chord, "major 7th #13");		  break;
		case kM7b9s13:	strcat(chord, "major 7 b9 #13");	  break;
		case kDom7b13:	strcat(chord, "dominant 7th b13");	  break;
		case kChrom:	strcat(chord, "chromatic");			  break;
		case kNone:
		default:	 	strcat(chord, "unknown"); 			  break;
	}

	x->x_chord_bass = x->x_abs_pc[x->x_chord_root];	/* get MIDI note number of bass */

		/* output results */
	outlet_list(x->x_outchordnotes, NULL, j, x->x_chordlist);
	outlet_float(x->x_outchordinversion, x->x_chord_inversion);
	outlet_symbol(x->x_outchordname, gensym(chord));
	outlet_float(x->x_outchordclass, x->x_chord_root);
	outlet_float(x->x_outchordval, x->x_chord_bass);
}

static void chord_kick_out_member(t_chord *x, t_int number, t_int *members)
{
	int *distances;
	int minDistance = 1000;
	int badMember   = 0;
	int i, j, interval;

	distances = getbytes(number*sizeof(int));
	
	for (i=0; i<number; i++) {
		// initialize total distance to zero
		distances[i] = 0;
		for (j=0; j<number; j++)
			if (j != i) {
				// get absolute value of interval size
				interval = abs(members[i] - members[j]);
				// make inversions of intervals equivalent
				if (interval > 6) interval = 12 - interval;
				// add absolute interval size to total
				distances[i] += interval;
			}
		
		// if this is the smallest total distance
		if (distances[i] < minDistance) {
			// remember it
			minDistance = distances[i];
			badMember   = i;
		}
	}
	freebytes(distances, number * sizeof(int));
	x->x_pc[members[badMember]] = 0;			// cancel out most dissonant member
	chord_chord_finder(x, number-1);			// call chord finder again without it
	x->x_pc[members[badMember]] = 1;			// replace most dissonant member
}

static void chord_chord_finder(t_chord *x, t_int num_pcs)
{
	int i;
	x->x_chord_type = kNone;
	x->x_chord_root = kXX;	/* none */
	switch (num_pcs) {
		case  1:	chord_unison(x);	break;
		case  2:	chord_dyad(x);		break;
		case  3:	chord_triad(x);		break;
		case  4:	chord_quartad(x);	break;
		case  5:	chord_quintad(x);	break;
		case  6:	chord_sextad(x);	break;
		default:	x->x_chord_type = kChrom;
					for(i = 0; i < 12; i++)   // 12 was num_pcs !?
					{
						if(x->x_pc[i])
						{
							x->x_chord_root = i;
							break;
						}
					}
	}
}

static void chord_float(t_chord *x, t_floatarg f)
{
	t_int velo = x->x_velo;
	t_int allloc = 0;
	t_int num_pc = 0;	/* number of pitch classes present */
	int i, j, k, l;

	x->x_pitch = (t_int)f;

	if(x->x_pitch <= x->x_split)
	{
			/* first we need to put the note into the allocation table */
		if(velo == 0)	/* got note-off: remove from allocation table */
		{
			if(x->x_poly > 0)x->x_poly--; /* polyphony has decreased by one */
			for(i = 0; i < MAX_POLY; i++) /* search for voice allocation number */
			{
					/* search for corresponding alloc number */
				if(x->x_alloctable[i] == x->x_pitch)
				{
					x->x_alloctable[i] = -1;     /* free the alloc number */
					break;
				}
					/* couldn't find it ? */
				if(i == MAX_POLY - 1)
				{
					post("chord: no corresponding note-on found (ignored)");
					return;
				}
			}
			return;		/* no need to look for chord */
		}
		else	/* we got a note-on message */
		{
			if(x->x_poly == MAX_POLY)
			{
				post("chord: too many note-on messages (ignored)");
				return;
			}
			
			x->x_poly++;      /* number of currently playing notes has increased */
				/* assign a voice allocation number */
			for(i = 0; i < MAX_POLY; i++)
			{
					/* search for free alloc number */
				if(x->x_alloctable[i] == -1)
				{
					x->x_alloctable[i] = x->x_pitch;   /* ... and store pitch */
					break;
				}
			}
				/* copy all notes into the pitch class array */
			for(i = 0; i < 12; i++)
			{
				x->x_pc[i] = 0;						/* empty pitch class */
				x->x_abs_pc[i] = -1;				/* empty absolute values */
			}
			for(i = 0; i < MAX_POLY; i++)
			{
					/* check for presence of pitch class */
				if(x->x_alloctable[i] != -1)
				{
					if(!x->x_pc[x->x_alloctable[i]%12])	/* a new pitch class */
					{
						x->x_abs_pc[x->x_alloctable[i]%12] = x->x_alloctable[i];
					}
					else if(x->x_abs_pc[x->x_alloctable[i]%12] > x->x_alloctable[i])	/* remember lowest pitch */
					{
						x->x_abs_pc[x->x_alloctable[i]%12] = x->x_alloctable[i];
					}

					x->x_pc[x->x_alloctable[i]%12] = 1;	/* indicate presence of pc */
				}
			}
				/* count number of pitch classes */
			for(i = 0; i < 12; i++)
			{
				num_pc += x->x_pc[i];
			}
			// post("%d pitch classes", num_pc);
		}
	}

	chord_chord_finder(x, num_pc);
}

static void chord_ft1(t_chord *x, t_floatarg f)
{
	x->x_velo = (t_int)f;
}

static t_class *chord_class;

static void *chord_new(t_floatarg f)
{
	int i;
    t_chord *x = (t_chord *)pd_new(chord_class);
    inlet_new(&x->x_ob, &x->x_ob.ob_pd, gensym("float"), gensym("ft1"));
	x->x_outchordval = outlet_new(&x->x_ob, gensym("float"));
	x->x_outchordclass = outlet_new(&x->x_ob, gensym("float"));
	x->x_outchordname = outlet_new(&x->x_ob, gensym("symbol"));
	x->x_outchordinversion = outlet_new(&x->x_ob, gensym("float"));
	x->x_outchordnotes = outlet_new(&x->x_ob, gensym("float"));

	x->x_split = (t_int)f;
	if(x->x_split == 0)x->x_split = 128;
	for(i = 0; i < MAX_POLY; i++)x->x_alloctable[i] = -1;
    
    return (void *)x;
}

#ifndef MAXLIB
void chord_setup(void)
{
    chord_class = class_new(gensym("chord"), (t_newmethod)chord_new,
    	0, sizeof(t_chord), 0, A_DEFFLOAT, 0);
#else
void maxlib_chord_setup(void)
{
    chord_class = class_new(gensym("maxlib_chord"), (t_newmethod)chord_new,
    	0, sizeof(t_chord), 0, A_DEFFLOAT, 0);
#endif
    class_addfloat(chord_class, chord_float);
    class_addmethod(chord_class, (t_method)chord_ft1, gensym("ft1"), A_FLOAT, 0);
#ifndef MAXLIB
    
    logpost(NULL, 4, version);
#else
	class_addcreator((t_newmethod)chord_new, gensym("chord"), A_DEFFLOAT, 0);
    class_sethelpsymbol(chord_class, gensym("maxlib/chord-help.pd"));
#endif
}

