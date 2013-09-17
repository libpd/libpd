/* 
 * regex: regular expression pattern matcher
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
#include "zexy.h"

#ifdef HAVE_REGEX_H
# include <sys/types.h>
# include <regex.h>
# include <string.h>
#endif

# define NUM_REGMATCHES 10

/*
 * regex    : see whether a regular expression matches the given symbol
 */

/* ------------------------- regex ------------------------------- */

/* match a regular expression against a string */

static t_class *regex_class;

typedef struct _regex
{
  t_object x_obj;
#ifdef HAVE_REGEX_H
  char    *x_regexstring; /* the uncompiled regular expression */
  int      x_regexstringlength; 

  regex_t *x_regexp;
  int x_matchnum;

  int x_flags; /* flags for the regex-compiler; REG_EXTENDED is always enabled */
#endif

  t_outlet*x_outResult;
  t_outlet*x_outDetails;
  t_outlet*x_outNumDetails;

} t_regex;

#ifdef HAVE_REGEX_H
static char*regex_l2s(int *reslen, t_symbol*s, int argc, t_atom*argv)
{
  char *result = 0;
  int pos=0, i=0;
  t_atom*ap;
  int length=0;
  if(reslen)*reslen=length;

  /* 1st get the length of the symbol */
  if(s)length+=strlen(s->s_name);
  else length-=1;
  length+=argc;

  i=argc;
  ap=argv;
  while(i--){
    char buffer[MAXPDSTRING];
    int len=0;
    if(A_SYMBOL==ap->a_type){
      len=strlen(ap->a_w.w_symbol->s_name);
    } else {
      atom_string(ap, buffer, MAXPDSTRING);
      len=strlen(buffer);
    }
    length+=len;
    ap++;
  }

  if(length<=0)return(0);

  result = (char*)getbytes((length+1)*sizeof(char));

  if (s) {
    char *buf = s->s_name;
    strcpy(result+pos, buf);
    pos+=strlen(buf);
    if(i){
      strcpy(result+pos, " ");
      pos += 1;
    }
  }

  ap=argv;
  i=argc;
  while(i--){
    if(A_SYMBOL==ap->a_type){
      strcpy(result+pos, ap->a_w.w_symbol->s_name);
      pos+= strlen(ap->a_w.w_symbol->s_name);
    } else {
      char buffer[MAXPDSTRING];
      atom_string(ap, buffer, MAXPDSTRING);
      strcpy(result+pos, buffer);
      pos += strlen(buffer);
    }
    ap++;
    if(i){
      strcpy(result+pos, " ");
      pos++;
    }
  }

  result[length]=0;
  if(reslen)*reslen=length;
  return result;
}

static void regex_compile(t_regex *x)
{
  int flags =  x->x_flags;
  flags |= REG_EXTENDED;

  if(0==x->x_regexstring || 0==x->x_regexstringlength){
    pd_error(x, "[regex]: no regular expression given");
    return;
  }


  if(x->x_regexp){
   regfree(x->x_regexp);
   freebytes(x->x_regexp, sizeof(t_regex));
   x->x_regexp=0;
  }
  x->x_regexp=(regex_t*)getbytes(sizeof(t_regex));

  if(regcomp(x->x_regexp, x->x_regexstring, flags)) {
    pd_error(x, "[regex]: invalid regular expression: %s", x->x_regexstring);
    if(x->x_regexp)freebytes(x->x_regexp, sizeof(t_regex));
    x->x_regexp=0;
  }
}
#endif


static void regex_case(t_regex *x, t_float f){
#if HAVE_REGEX_H
  if(f>0.f)
    x->x_flags |= REG_ICASE;
  else
    x->x_flags ^= REG_ICASE;

  regex_compile(x);
#endif
}


static void regex_regex(t_regex *x, t_symbol*s, int argc, t_atom*argv)
{
#ifdef HAVE_REGEX_H
  char*result=0;
  int length=0;

  result=regex_l2s(&length, 0, argc, argv);

  if(0==result || 0==length){
    pd_error(x, "[regex]: no regular expression given");
    return;
  }

  if(x->x_regexstring) {
    freebytes(x->x_regexstring, x->x_regexstringlength);
    x->x_regexstring=0;
    x->x_regexstringlength=0;
  }
  
  x->x_regexstring=result;
  x->x_regexstringlength=length;

  regex_compile(x);
#endif
}

/* compare the given list as string with the precompiled regex */
static void regex_symbol(t_regex *x, t_symbol *s, int argc, t_atom*argv)
{
#ifdef HAVE_REGEX_H
  char*teststring=0;
  int length=0;

  int num_matches=x->x_matchnum;
  regmatch_t*match=(regmatch_t*)getbytes(sizeof(regmatch_t)*num_matches);
  t_atom*ap=(t_atom*)getbytes(sizeof(t_atom)*(3*num_matches));

  int err=0;

  if(!x->x_regexp){
    pd_error(x, "[regex]: no regular expression!");
    goto cleanup;
  }
  teststring=regex_l2s(&length, 0, argc, argv);
  if(!teststring||!length){
    pd_error(x, "[regex]: cannot evaluate string");
    goto cleanup;
  }

  /* do the actual comparing against the regex */
  err=regexec(x->x_regexp, teststring, num_matches, match, 0);
  if(teststring){
    freebytes(teststring, length);
    teststring=0;
  }

  if(err) { /* NO match */
    if(match){
      freebytes(match, sizeof(regmatch_t)*num_matches);
      match=0;
    }

    outlet_float(x->x_outResult, 0.f);
  } else { /* match! */
    int num_results=0;
    int i=0;
    t_atom*ap2=ap;

    for(i=0; i<num_matches; i++){
      if(match[i].rm_so!=-1){
        /* output the matches */
        if(i>0 && (match[i].rm_so==match[i-1].rm_so) && (match[i].rm_eo==match[i-1].rm_eo)){
          /* duplicate matches */
        } else {
          SETFLOAT(ap2+0, (t_float)i);
          SETFLOAT(ap2+1, (t_float)match[i].rm_so);
          SETFLOAT(ap2+2, (t_float)match[i].rm_eo);
          ap2+=3;
          num_results++;
        }
      }
    }
    if(match){
      freebytes(match, sizeof(regmatch_t)*num_matches);
      match=0;
    }
    outlet_float(x->x_outNumDetails, (t_float)num_results);
    for(i=0; i<num_results; i++){
      outlet_list(x->x_outDetails, gensym("list"), 3, ap+(i*3));
    }
    outlet_float(x->x_outResult, 1.f);
  }
 cleanup:
  if(teststring)freebytes(teststring, length);
  if(match)freebytes(match, sizeof(regmatch_t)*num_matches);

  if(ap){
    freebytes(ap, sizeof(t_atom)*(1+2*num_matches));
  }
#endif
}

static void *regex_new(t_symbol *s, int argc, t_atom*argv)
{
  t_regex *x = (t_regex *)pd_new(regex_class);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("symbol"), gensym("regex"));

  x->x_outResult=outlet_new(&x->x_obj, 0);
  x->x_outDetails=outlet_new(&x->x_obj, gensym("list"));
  x->x_outNumDetails=outlet_new(&x->x_obj, gensym("float"));


#ifdef HAVE_REGEX_H
  x->x_flags=0;

  x->x_regexstring=0;
  x->x_regexstringlength=0;

  x->x_regexp=0;
  x->x_matchnum=NUM_REGMATCHES;
  if(argc)regex_regex(x, gensym(""), argc, argv);
  else{
    t_atom a;
    SETSYMBOL(&a, gensym(".*"));
    regex_regex(x, 0, 1, &a);
  }
#else
  error("[regex] non-functional: compiled without regex-support!");
#endif

  return (x);
}

static void regex_free(t_regex *x)
{
#ifdef HAVE_REGEX_H
  if(x->x_regexstring){
    freebytes(x->x_regexstring, x->x_regexstringlength);
    x->x_regexstring=0;
    x->x_regexstringlength=0;
  }

  if(x->x_regexp) {
    regfree(x->x_regexp);
    freebytes(x->x_regexp, sizeof(t_regex));
    x->x_regexp=0;
  }
#endif
}

static void regex_help(t_regex*x)
{
  post("\n%c regex\t\t:: test the input whether it matches a regular expression", HEARTSYMBOL);
}

void regex_setup(void)
{
  regex_class = class_new(gensym("regex"), (t_newmethod)regex_new, 
			 (t_method)regex_free, sizeof(t_regex), 0, A_GIMME, 0);

  class_addlist  (regex_class, regex_symbol);
  class_addmethod(regex_class, (t_method)regex_regex, gensym("regex"), A_GIMME, 0);

  class_addmethod(regex_class, (t_method)regex_case, gensym("case"), A_FLOAT, 0);

  class_addmethod(regex_class, (t_method)regex_help, gensym("help"), A_NULL);
  zexy_register("regex");
}
