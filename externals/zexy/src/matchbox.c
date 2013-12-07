/* 
 * matchbox: see whether a regular expression matches a symbol in the box
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


/* LATER: add a creation argument to specify the initial search mode
 *
 * LATER: bind a "name" to the [matchbox] so several objects can share the same entries
 * if no name is given at creation time, the entries are local only
 *
 * even LATER: dynamically bind to several searchlists (via "set" message)
 */


#include "zexy.h"

#define MATCHBOX_EXACT 0
#define MATCHBOX_OSC 1

#ifdef HAVE_REGEX_H
# include <sys/types.h>
# include <regex.h>
#define MATCHBOX_REGEX 2
#endif

#include <string.h>

#define FALSE 0
#define TRUE  1

/* ------------------------- matchbox ------------------------------- */

/* match the atoms of 2 lists */

static t_class *matchbox_class;


typedef struct _listlist {
  int                 argc;
  t_atom             *argv;
  struct _listlist *next;
} t_listlist;


typedef struct _matchbox
{
  t_object x_obj;

  t_listlist*x_lists;
  unsigned int x_numlists;

  int x_mode;

  t_outlet*x_outResult;
  t_outlet*x_outNumResults;
} t_matchbox;


/* ----------- here comes some infrastructure stuff -------------- */


static t_listlist* addlistlist(t_listlist*list, int argc, t_atom*argv) {
  t_listlist*ll=(t_listlist*)getbytes(sizeof(t_listlist));
  t_listlist*lp=0;
  ll->next=0;
  ll->argc=argc;
  ll->argv=(t_atom*)getbytes(argc*sizeof(t_atom));
  memcpy(ll->argv, argv, argc*sizeof(t_atom)); 

  if(0==list) {
    return ll;
  }
  
  lp=list;
  while(0!=lp->next)lp=lp->next;
  lp->next=ll;

  return list;
}

/* delete the _next_ element from the list */
static t_listlist* deletelistnext(t_listlist*list) {
  t_listlist*ll=0;

  if(!list || !list->next)return list; /* nothing to delete */

  ll=list->next;
  list->next=ll->next;
  if(ll->argv)freebytes(ll->argv, ll->argc*sizeof(t_atom));

  ll->argv=0;
  ll->argc=0;
  ll->next=0;
  freebytes(ll, sizeof(t_listlist));
  return list;
}

/* delete the entire list of lists */
static void clearlistlist(t_listlist*list) {
  if(!list)return; /* nothing to delete */
  while(list->next){
    list=deletelistnext(list);
  }
}

/* -------------- here comes the matching algorithms ----------- */


static int atommatch_exact(t_atom*pattern, t_atom*atom) {
  if(pattern->a_type==atom->a_type) {
    switch(pattern->a_type) {
    case A_FLOAT:
      return atom_getfloat(pattern)==atom_getfloat(atom);
    case A_SYMBOL:
      return atom_getsymbol(pattern)==atom_getsymbol(atom);
    default:
      return pattern==atom;
    }
  } else {
    /* post("types don't match!"); */
    return FALSE;
  }

  return TRUE;
}

#ifdef MATCHBOX_OSC /* OSC */

/*
OSC pattern matching code was written by Matt Wright, 
The Center for New Music and Audio Technologies,
University of California, Berkeley.  Copyright (c) 1998,99,2000,01,02,03,04
The Regents of the University of California (Regents).  

Permission to use, copy, modify, distribute, and distribute modified versions
of this software and its documentation without fee and without a signed
licensing agreement, is hereby granted, provided that the above copyright
notice, this paragraph and the following two paragraphs appear in all copies,
modifications, and distributions.

IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

The OSC webpage is http://cnmat.cnmat.berkeley.edu/OpenSoundControl
*/

#define OSCWarning post
static int OSC_MatchBrackets (const char *pattern, const char *test, const char*theWholePattern);
static int OSC_MatchList (const char *pattern, const char *test, const char*theWholePattern);

static int OSC_PatternMatch (const char *  pattern, const char * test, const char*theWholePattern) {
  if (pattern == 0 || pattern[0] == 0) {
    return test[0] == 0;
  } 
  
  if (test[0] == 0) {
    if (pattern[0] == '*')
      return OSC_PatternMatch (pattern+1,test, theWholePattern);
    else
      return FALSE;
  }

  switch (pattern[0]) {
  case 0      : return test[0] == 0;
  case '?'    : return OSC_PatternMatch (pattern + 1, test + 1, theWholePattern);
  case '*'    : 
    if (OSC_PatternMatch (pattern+1, test, theWholePattern)) {
      return TRUE;
    } else {
      return OSC_PatternMatch (pattern, test+1, theWholePattern);
    }
  case ']'    :
  case '}'    :
    z_verbose(1, "[matchbox]: spurious %c in OSC-pattern \".../%s/...\"",pattern[0], theWholePattern);
    return FALSE;
  case '['    :
    return OSC_MatchBrackets (pattern,test, theWholePattern);
  case '{'    :
    return OSC_MatchList (pattern,test, theWholePattern);
  case '\\'   :  
    if (pattern[1] == 0) {
      return test[0] == 0;
    } else if (pattern[1] == test[0]) {
      return OSC_PatternMatch (pattern+2,test+1, theWholePattern);
    } else {
      return FALSE;
    }
  default     :
    if (pattern[0] == test[0]) {
      return OSC_PatternMatch (pattern+1,test+1, theWholePattern);
    } else {
      return FALSE;
    }
  }
}

/* we know that pattern[0] == '[' and test[0] != 0 */

static int OSC_MatchBrackets (const char *pattern, const char *test, const char*theWholePattern) {
  int result;
  int negated = FALSE;
  const char *p = pattern;

  if (pattern[1] == 0) {
    z_verbose(1, "[matchbox]: unterminated [ in OSC-pattern \".../%s/...\"", theWholePattern);
    return FALSE;
  }

  if (pattern[1] == '!') {
    negated = TRUE;
    p++;
  }

  while (*p != ']') {
    if (*p == 0) {
      z_verbose(1, "[matchbox]: unterminated [ in OSC-pattern \".../%s/...\"", theWholePattern);
      return FALSE;
    }
    if (p[1] == '-' && p[2] != 0) {
      if (test[0] >= p[0] && test[0] <= p[2]) {
	result = !negated;
	goto advance;
      }
    }
    if (p[0] == test[0]) {
      result = !negated;
      goto advance;
    }
    p++;
  }

  result = negated;

 advance:

  if (!result)
    return FALSE;

  while (*p != ']') {
    if (*p == 0) {
      z_verbose(1, "[matchbox]: unterminated [ in OSC-pattern \".../%s/...\"", theWholePattern);
      return FALSE;
    }
    p++;
  }

  return OSC_PatternMatch (p+1,test+1, theWholePattern);
}

static int OSC_MatchList (const char *pattern, const char *test, const char* theWholePattern) {

  const char *restOfPattern, *tp = test;

  for(restOfPattern = pattern; *restOfPattern != '}'; restOfPattern++) {
    if (*restOfPattern == 0) {
      z_verbose(1, "[matchbox]: unterminated { in OSC-pattern \".../%s/...\"", theWholePattern);
      return FALSE;
    }
  }

  restOfPattern++; /* skip close curly brace */


  pattern++; /* skip open curly brace */

  while (1) {
   
    if (*pattern == ',') {
      if (OSC_PatternMatch (restOfPattern, tp, theWholePattern)) {
        return TRUE;
      } else {
        tp = test;
        ++pattern;
      }
    } else if (*pattern == '}') {
      return OSC_PatternMatch (restOfPattern, tp, theWholePattern);
    } else if (*pattern == *tp) {
      ++pattern;
      ++tp;
    } else {
      tp = test;
      while (*pattern != ',' && *pattern != '}') {
        pattern++;
      }
      if (*pattern == ',') {
        pattern++;
      }
    }
  }
}

static int atommatch_osc(t_atom*pattern, t_atom*test) {
  char*s_pattern=0;
  char*s_test=0;
  int pattern_size=0, test_size=0;

  int result = FALSE;

  if(pattern->a_type==A_SYMBOL) {
    s_pattern=pattern->a_w.w_symbol->s_name;
  } else {
    pattern_size=sizeof(char)*MAXPDSTRING;
    s_pattern=(char*)getbytes(pattern_size);
    atom_string(pattern, s_pattern, pattern_size);
  }
  if(test->a_type==A_SYMBOL) {
    s_test=test->a_w.w_symbol->s_name;
  } else {
    test_size=sizeof(char)*MAXPDSTRING;
    s_test=(char*)getbytes(test_size);
    atom_string(test, s_test, test_size);
  }


  result = OSC_PatternMatch(s_pattern, s_test, s_pattern);

  if(pattern_size>0) {
    freebytes(s_pattern, pattern_size);
    s_pattern=0; pattern_size=0;
  }
  if(test_size>0) {
    freebytes(s_test, test_size);
    s_test=0; test_size=0;
  }


  return result;
}
#endif /* OSC */


#ifdef MATCHBOX_REGEX
static int atommatch_regex(regex_t*pattern,  t_atom*test) {
  int result=FALSE;
  char*s_test=0;
  int test_size=0;

  if(0==pattern)return FALSE;
  if(0==test)   return FALSE;

  if(test->a_type==A_SYMBOL) {
    s_test=test->a_w.w_symbol->s_name;
  } else {
    test_size=sizeof(char)*MAXPDSTRING;
    s_test=(char*)getbytes(test_size);
    atom_string(test, s_test, test_size);
  }

  result=!(regexec(pattern, s_test, 0, 0, 0));

  if(test_size>0) {
    freebytes(s_test, test_size);
    s_test=0; test_size=0;
  } 
  
  return result;
}

static int listmatch_regex(int p_argc, regex_t**pattern, int t_argc, t_atom*test) {
  /* match the patterns to the test */
  int argc=p_argc;
  if(p_argc!=t_argc)
    return FALSE;

  while(argc--) {
    if(FALSE==atommatch_regex(*pattern++, test++)) {
      return FALSE;
    }
  }

  return TRUE;
}

static t_listlist*matchlistlist_regex(unsigned int*numresults, t_listlist*searchlist, int p_argc, t_atom*p_argv, int flags, int delete_results) {
  regex_t**regexpressions=0;
  t_listlist*matchinglist=0, *sl;
  int i=0;
  int num=0;

  flags|=REG_EXTENDED;

  /* 1st compile the patterns */
  regexpressions=(regex_t**)getbytes(sizeof(regex_t*)*p_argc);
  for(i=0; i<p_argc; i++) {
    char*s_pattern=0;
    int pattern_size=0;
    t_atom*pattern=p_argv+i;
    if(pattern->a_type==A_SYMBOL) {
      s_pattern=pattern->a_w.w_symbol->s_name;
    } else {
      pattern_size=sizeof(char)*MAXPDSTRING;
      s_pattern=(char*)getbytes(pattern_size);
      atom_string(pattern, s_pattern, pattern_size);
    }
    regexpressions[i]=(regex_t*)getbytes(sizeof(regex_t));
    if(regcomp(regexpressions[i], s_pattern, flags)) {
      z_verbose(1, "[matchbox]: invalid regular expression: %s", s_pattern);
      if(regexpressions[i])freebytes(regexpressions[i], sizeof(regex_t));
       regexpressions[i]=0;
    }
    if(pattern_size>0) {
      freebytes(s_pattern, pattern_size);
      s_pattern=0; pattern_size=0;
    }
  }
 
  /* match the patterns to the tests */
  if(FALSE==delete_results) {
    for(sl=searchlist; 0!=sl; sl=sl->next) {
      if(TRUE==listmatch_regex(p_argc, regexpressions, sl->argc, sl->argv)) {
        matchinglist=addlistlist(matchinglist, sl->argc, sl->argv);
        num++;
      }
    }
  } else if (TRUE==delete_results) {
    /* yummy: delete matching lists! */
    t_listlist*lastgood=searchlist;
    for(sl=searchlist; 0!=sl; sl=sl->next) {
      if(TRUE==listmatch_regex(p_argc, regexpressions, sl->argc, sl->argv)) {
        matchinglist=addlistlist(matchinglist, sl->argc, sl->argv);
        num++;

        sl=deletelistnext(lastgood); 
      } else {
        lastgood=sl;
      }
    }
  }

  /* clear the patterns */
  for(i=0; i<p_argc; i++) {
    if(regexpressions[i]){
      regfree(regexpressions[i]);
      freebytes(regexpressions[i], sizeof(regex_t));
    }
  }
  freebytes(regexpressions, sizeof(regex_t*)*p_argc);

  /* return the result */  
  if(numresults!=0)
    *numresults=num;
  return matchinglist;
}
#endif /* MATCHBOX_REGEX */





static int matchbox_atommatch(t_atom*pattern, t_atom*atom, int mode) {
  switch(mode) {
  default:
  case MATCHBOX_EXACT: return atommatch_exact(pattern, atom);
#ifdef MATCHBOX_OSC
  case MATCHBOX_OSC  : return atommatch_osc(pattern, atom);
#endif /* OSC */
  }
  return atommatch_exact(pattern, atom);
}

static int matchlist(int argc_pattern, t_atom*argv_pattern,
                     int argc, t_atom*argv, int mode) {
  int i=0;

  if(argc!=argc_pattern)
    return FALSE;

  for(i=0; i<argc; i++) {
    if(0==matchbox_atommatch(argv_pattern+i, argv+i, mode))
      return FALSE;
  }
  
  return TRUE;
}

static t_listlist*matchlistlist(unsigned int*numresults, t_listlist*searchlist, int p_argc, t_atom*p_argv, int mode, int delete_results) {
  unsigned int num=0;
  t_listlist*matchinglist=0, *sl;

  /* extra handling of regex matching (because we want to compile only once */
#ifdef MATCHBOX_REGEX
  if(MATCHBOX_REGEX==mode) {
    matchinglist=matchlistlist_regex(&num, searchlist, p_argc, p_argv, 0, delete_results);
  } else 
#endif /* MATCHBOX_REGEX */
  /* normal matching */
  if(FALSE==delete_results) {
    for(sl=searchlist->next; 0!=sl; sl=sl->next) {
      if(matchlist(p_argc, p_argv, sl->argc, sl->argv, mode)) {
        matchinglist=addlistlist(matchinglist, sl->argc, sl->argv);
        num++;
      }
    }
  } else if (TRUE==delete_results) {
    /* yummy: delete matching lists! */
    t_listlist*lastgood=searchlist;
    for(sl=searchlist->next; 0!=sl; sl=sl->next) {
      if(matchlist(p_argc, p_argv, sl->argc, sl->argv, mode)) {
        matchinglist=addlistlist(matchinglist, sl->argc, sl->argv);
        num++;

        sl=deletelistnext(lastgood);
      } else {
        lastgood=sl;
      }
    }
  }

  if(numresults!=0)
    *numresults=num;
  return matchinglist;
}


static void matchbox_list(t_matchbox*x, t_symbol*s, int argc, t_atom*argv) {
  unsigned int results=0;
  int mode=x->x_mode;
  t_listlist*resultlist=matchlistlist(&results, x->x_lists, argc, argv, mode, FALSE);
  t_listlist*dummylist;

  outlet_float(x->x_outNumResults, (t_float)results);
  
  for(dummylist=resultlist; 0!=dummylist; dummylist=dummylist->next)
    outlet_list(x->x_outResult,  gensym("list"), dummylist->argc, dummylist->argv);
}

static void matchbox_add(t_matchbox*x, t_symbol*s, int argc, t_atom*argv) {
  /* 1st match, whether we already have this entry */
  if(matchlistlist(0, x->x_lists, argc, argv, MATCHBOX_EXACT, FALSE)) {
    /* already there, skip the rest */
    z_verbose(1, "[matchbox]: refusing to add already existing list to buffer...");
    return;
  }

  /* 2nd if this is a new entry, add it */
  x->x_lists=addlistlist(x->x_lists, argc, argv);
  x->x_numlists++;
}

static void matchbox_delete(t_matchbox*x, t_symbol*s, int argc, t_atom*argv) {
  unsigned int results=0;
  int mode=x->x_mode;
  t_listlist*resultlist=matchlistlist(&results, x->x_lists, argc, argv, mode, TRUE);
  t_listlist*dummylist;
  t_symbol*delsym=gensym("deleted");

  x->x_numlists-=results;

  outlet_float(x->x_outNumResults, (t_float)results);
  
  for(dummylist=resultlist; 0!=dummylist; dummylist=dummylist->next)
    outlet_anything(x->x_outResult, delsym, dummylist->argc, dummylist->argv);
}

static void matchbox_dump(t_matchbox*x) {
  t_listlist*lp=0;

  if(0==x->x_lists || 0==x->x_lists->next){
    outlet_float(x->x_outNumResults, 0);
    return;
  }

  outlet_float(x->x_outNumResults, x->x_numlists);

  for(lp=x->x_lists->next; 0!=lp; lp=lp->next)
  {
    outlet_list(x->x_outResult,  gensym("list"), lp->argc, lp->argv);
  }
}


static void matchbox_clear(t_matchbox*x) {
  clearlistlist(x->x_lists);
  x->x_numlists=0;
}


static void matchbox_mode(t_matchbox*x, t_symbol*s) {
  if(gensym("==")==s)
    x->x_mode=MATCHBOX_EXACT;
  else if (gensym("OSC")==s) {
#ifdef MATCHBOX_OSC
    x->x_mode=MATCHBOX_OSC;
#else
     pd_error(x, "[matchbox] has been compiled without 'OSC' support; ignoring your request");
#endif /* MATCHBOX_OSC */
  } else if(gensym("regex")==s) {
#ifdef MATCHBOX_REGEX
    x->x_mode=MATCHBOX_REGEX;
#else
    pd_error(x, "[matchbox] has been compiled without 'regex' support; ignoring your request");
#endif /* MATCHBOX_REGEX */
  } else {
    pd_error(x, "mode '%s' is unknown, switching to 'exact' mode", s->s_name);
    x->x_mode=MATCHBOX_EXACT;
  }
}

static void *matchbox_new(t_symbol *s, int argc, t_atom*argv)
{
  t_matchbox *x = (t_matchbox *)pd_new(matchbox_class);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("symbol"), gensym("add"));

  x->x_outResult    =outlet_new(&x->x_obj, gensym("list"));
  x->x_outNumResults=outlet_new(&x->x_obj, gensym("float"));


  x->x_lists=(t_listlist*)getbytes(sizeof(t_listlist));
  x->x_lists->next=0;
  x->x_lists->argc=0;
  x->x_lists->argv=0;
  x->x_numlists=0;

  x->x_mode = MATCHBOX_EXACT;

  if(argc && argv->a_type==A_SYMBOL) {
    matchbox_mode(x, atom_getsymbol(argv));
  }


  return (x);
}

static void matchbox_free(t_matchbox *x)
{
  matchbox_clear(x);
  freebytes(x->x_lists, sizeof(t_listlist));  
  x->x_lists=0;
}

static void matchbox_help(t_matchbox*x)
{
  post("\n%c matchbox\t\t:: find a list in a pool of lists", HEARTSYMBOL);
}

void matchbox_setup(void)
{
#ifdef MATCHBOX_OSC
  post("matchbox: OSC-pattern matching code (c) Matt Wright, CNMAT");
#endif /* MATCHBOX_OSC */


  matchbox_class = class_new(gensym("matchbox"), (t_newmethod)matchbox_new, 
			 (t_method)matchbox_free, sizeof(t_matchbox), 0, A_GIMME, 0);

  class_addlist  (matchbox_class, matchbox_list);

  class_addmethod(matchbox_class, (t_method)matchbox_add, gensym("add"), A_GIMME, 0);
  class_addmethod(matchbox_class, (t_method)matchbox_delete, gensym("delete"), A_GIMME, 0);
  class_addmethod(matchbox_class, (t_method)matchbox_clear, gensym("clear"), A_NULL, 0);
  class_addmethod(matchbox_class, (t_method)matchbox_dump, gensym("dump"), A_NULL);

  class_addmethod(matchbox_class, (t_method)matchbox_mode, gensym("mode"), A_SYMBOL, 0);

  class_addmethod(matchbox_class, (t_method)matchbox_help, gensym("help"), A_NULL);
  zexy_register("matchbox");
}
