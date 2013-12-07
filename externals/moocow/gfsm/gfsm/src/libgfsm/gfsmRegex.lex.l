/*======================================================================
 * Flex Options
 */
/*%option outfile="gfsmRegex.lex.c"*/
%option header-file="gfsmRegex.lex.h"
%option outfile="lex.yy.c"
%option prefix="gfsmRegex_yy"
%option reentrant
%option 8bit
%option yylineno

%option bison-bridge

%{
/*======================================================================
 * User C Header
 */

#include <gfsmMem.h>
#include <gfsmRegexCompiler.h>

#include "gfsmRegex.tab.h"

#define my_compiler ((gfsmRegexCompiler*)yyextra)
#define my_gstr     (my_compiler->gstr)
#define my_abet     (my_compiler->abet)

%}

/*======================================================================
 * Flex Definitions
 */

DIGIT    [0-9]
SPACE    [[:space:]]
WCHAR    [^\<\[\\ \t\n\r\#]
BCHAR    [^\\\]\ \t\n\r\=]
UTF8PREFIX "Ã"

%x STATE_ESCAPE
%x STATE_BRACKETED
%x STATE_BRACKETED_SEP
%x STATE_BRACKETED_ESCAPE
%x STATE_COMMENT
%x STATE_WEIGHT
%x STATE_UINT

/*======================================================================
 * Rules
 */
%%

{UTF8PREFIX}+. {
  yylval->gs = gfsm_gstring_new_bytes(yytext,yyleng);
  return TOK_STRING;
}

"\\"     { BEGIN(STATE_ESCAPE); }

"#"      { BEGIN(STATE_COMMENT); }

"["      { 
  BEGIN(STATE_BRACKETED);
  return '[';
}

"<"      { BEGIN(STATE_WEIGHT); return '<'; }

"("      { return '('; }
")"      { return ')'; }

"*"      { return '*'; }
"+"      { return '+'; }
"^"      { BEGIN(STATE_UINT); return '|'; }
"?"      { return '?'; }
"!"      { return '!'; }
"|"      { return '|'; }
"&"      { return '&'; }
":"      { return ':'; }
"@"      { return '@'; }
"-"      { return '-';  /*"/1"{return PROJ1;}*/ /*"/2"{return PROJ2;}*/ }

"$"      { return '$'; /* non-AT&T: determinize */ }
"%"      { return '%'; /* non-AT&T: rmepsilon */ }
"~"      { return '~'; /* non-AT&T: connect */ }

{SPACE}  { /* ignore */ }

{WCHAR}  {
  yylval->c = yytext[0];
  return TOK_CHAR;
}



<STATE_WEIGHT>([\+\-])?({DIGIT}*\.)?({DIGIT}+)([Ee]([\+\-]?){DIGIT}+)? {
  yylval->w = strtod(yytext,NULL);
  return TOK_WEIGHT;
}

<STATE_WEIGHT>\> {
  BEGIN(INITIAL);
  return '>';
}



<STATE_UINT>{DIGIT}+ {
  BEGIN(INITIAL);
  yylval->u = strtol(yytext,NULL,0);
  return TOK_UINT;
}


<STATE_COMMENT>[^\n]*\n  { BEGIN(INITIAL); /* ignore comments */ }




<STATE_ESCAPE>. {
  BEGIN(INITIAL);
  yylval->c = yytext[0];
  return TOK_CHAR;
}


<STATE_BRACKETED>{UTF8PREFIX}+. {
  g_string_append_len(my_gstr,yytext,yyleng);
}
<STATE_BRACKETED>{SPACE}*"]" {
  unput(']');
  BEGIN(STATE_BRACKETED_SEP);
  if (my_gstr->len) {
    yylval->gs = gfsm_gstring_dup(my_gstr);
    return TOK_STRING;
  }
}
<STATE_BRACKETED>{BCHAR}+ {
  g_string_append(my_gstr, yytext);
}
<STATE_BRACKETED>({SPACE}*)"="({SPACE}*) {
  unput('=');
  BEGIN(STATE_BRACKETED_SEP);
  if (my_gstr->len) {
    yylval->gs = gfsm_gstring_dup(my_gstr);
    return TOK_STRING;
  }
}
<STATE_BRACKETED>{SPACE}+ {
  unput(' ');
  BEGIN(STATE_BRACKETED_SEP);
  if (my_gstr->len) {
    yylval->gs = gfsm_gstring_dup(my_gstr);
    return TOK_STRING;
  }
}
<STATE_BRACKETED>\\ {
  BEGIN(STATE_BRACKETED_ESCAPE);
}


<STATE_BRACKETED_SEP>"]" {
  BEGIN(INITIAL);
  if (my_gstr->len) {
    g_string_truncate(my_gstr,0);
    return ']';
  }
}
<STATE_BRACKETED_SEP>"=" {
  BEGIN(STATE_BRACKETED);
  g_string_truncate(my_gstr,0);
  return '=';
}
<STATE_BRACKETED_SEP>" " {
  BEGIN(STATE_BRACKETED);
  if (my_gstr->len) {
    g_string_truncate(my_gstr,0);
    return ' ';
  }
}


<STATE_BRACKETED_ESCAPE>. {
  BEGIN(STATE_BRACKETED);
  g_string_append_c(my_gstr, yytext[0]);
}



<*>. {
  gfsm_scanner_carp(yyextra, "bad character '%s'", yytext);
  return TOK_UNKNOWN;
}

%%

/*======================================================================
 * User C Code
 */

GFSM_SCANNER_YYWRAP(gfsmRegex_yy)
