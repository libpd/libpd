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
#include "m_pd.h"
#include "s_stuff.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXNDEV 20
#define DEVDESCSIZE 80

/**
 * find EOS of a string of tokens
 * where EOS might be indicated by \0, }
 */
unsigned int findEOT(const char*s, unsigned int length) {
  unsigned int len=0;
  do {
    char c=s[len];
    len++;
    switch(c) {
    case '{':
      len+=findEOT(s+len, length-len);
      break;
    case '}':
      if('{'!=s[0])
        return len;
      else
        return len-1;
    case '\0':
      return len;
    }
  } while(len<length);
  return len;
}

/**
 * parse a string like "jack 3" into the name "jack" and the ID '3'
 */
static
int common_parsedriver(const char*str, const int inlen, 
                       char*name, int length, 
                       int *id) {
  /* find beginning of trailing number */
  int stop=inlen-1;
  int start1=0;
  int stop1=0;
  int start=0;
  int ret=0;

  while('\0'==str[stop] && stop>=0)
    stop--;

  start=stop;
  while(start>=0) {
    char c=str[start];
    if(c<48 || c>=57) break;
    start--;
  }
  if(start==stop || start<0)
    return ret;

  stop1=start;
  while(stop1>=0) {
    char c=str[stop1];
    if(!isspace(c))
      break;
    stop1--;
  }
  if(stop1<0)
    return ret;

  if((   str[start1]=='"' && str[stop1]=='"') 
     || (str[start1]=='\''  && str[stop1]=='\'')
     || (str[start1]=='{'  && str[stop1]=='}')
     ) {
    start1++;
    stop1--;
  }
  stop1+=2;
  if(stop1>=length)stop1=length-1;
  snprintf(name, stop1-start1, "%s", str+start1);
  ret=(1==sscanf(str+start, "%d", id));

  return ret;
}
