/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */


#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


/* ---------------------------- iem_pbank_csv ------------------------------- */
/* -- is a list storage and management object, can store an array of lists -- */
/* ------------------------------- as an csv file --------------------------- */

/* read and write method needs 2 symbols,
1. symbol is a filename,
2. symbol is a 2 character descriptor

1.char: 'b'...for blank as ITEM_SEPARATOR (" ")
1.char: 's'...for semicolon as ITEM_SEPARATOR (";")
1.char: 't'...for tabulator as ITEM_SEPARATOR ("	" = 0x09)

2.char: 'b'...for blank,return as END_OF_LINE (" \n")
2.char: 's'...for semicolon,return as END_OF_LINE (";\n")
2.char: 't'...for tabulator,return as END_OF_LINE ("     \n")
2.char: 'r'...for return-only as END_OF_LINE ("\n")

change: recall + offset + number
*/

#define IEMLIB2_DEBUG 0

#define IEM_PBANK_ITEM_SEPARATOR 0
#define IEM_PBANK_END_OF_LINE 1
#define IEM_PBANK_FORMAT_SIZE 2
#define IEM_PBANK_UNIFIED_RET 0x01
#define IEM_PBANK_UNIFIED_SEP 0x02
#define IEM_PBANK_UNIFIED_EOL 0x03

static t_class *iem_pbank_csv_class;

typedef struct _iem_pbank_csv
{
  t_object  x_obj;
  int       x_nr_para;
  int       x_nr_line;
  int       x_line;
  t_atom    *x_atbegmem;
  t_atom    *x_atbegbuf;
  t_atom    *x_atbegout;
  t_canvas  *x_canvas;
  void      *x_list_out;
  void      *x_offset_list_out;
} t_iem_pbank_csv;

static void iem_pbank_csv_write(t_iem_pbank_csv *x, t_symbol *filename, t_symbol *format)
{
  char completefilename[400], eol[8], sep, mode[4], string[200];
  int size, p, l, nrl=x->x_nr_line, nrp=x->x_nr_para;
  int state, max=nrl*nrp, org_size, eol_offset;
  FILE *fh;
  t_atom *ap=x->x_atbegmem;
  char formattext[100];
  
  strcpy(mode, "br"); // default: blank-separator, return-eol, return depends on operating system
  sep = ' ';
  sprintf(eol, ";\n");
  eol_offset = 1;
  
  if(filename->s_name[0] == '/')// linux, OSX
  {
    strcpy(completefilename, filename->s_name);
  }
  else if(((filename->s_name[0] >= 'A')&&(filename->s_name[0] <= 'Z')||
           (filename->s_name[0] >= 'a')&&(filename->s_name[0] <= 'z'))&&
          (filename->s_name[1] == ':')&&(filename->s_name[2] == '/'))// windows, backslash becomes slash in pd
  {
            strcpy(completefilename, filename->s_name);
  }
  else
  {
    strcpy(completefilename, canvas_getdir(x->x_canvas)->s_name);
    strcat(completefilename, "/");
    strcat(completefilename, filename->s_name);
  }
  
  fh = sys_fopen(completefilename,"wb");
  if(!fh)
  {
    post("iem_pbank_csv_write: cannot create %s !!\n", completefilename);
  }
  else
  {
    if(strlen(format->s_name) >= IEM_PBANK_FORMAT_SIZE)
    {
      for(p=0; p<IEM_PBANK_FORMAT_SIZE; p++)
      {
        if((format->s_name[p] >= 'A')&&(format->s_name[p] <= 'Z'))
          format->s_name[p] += 'a' - 'A';
      }
      
      if((format->s_name[IEM_PBANK_ITEM_SEPARATOR] == 'b')
         ||(format->s_name[IEM_PBANK_ITEM_SEPARATOR] == 's')
         ||(format->s_name[IEM_PBANK_ITEM_SEPARATOR] == 't'))
        mode[IEM_PBANK_ITEM_SEPARATOR] = format->s_name[IEM_PBANK_ITEM_SEPARATOR];
      
      if((format->s_name[IEM_PBANK_END_OF_LINE] == 'b')
         ||(format->s_name[IEM_PBANK_END_OF_LINE] == 's')
         ||(format->s_name[IEM_PBANK_END_OF_LINE] == 't')
         ||(format->s_name[IEM_PBANK_END_OF_LINE] == 'r'))
        mode[IEM_PBANK_END_OF_LINE] = format->s_name[IEM_PBANK_END_OF_LINE];
    }
    else
      post("iem_pbank_csv_write: use default format %s !!\n", mode);
    
    if(mode[IEM_PBANK_ITEM_SEPARATOR] == 'b')
    {
      sep = ' ';
      strcpy(formattext, "item-separator = BLANK; ");
    }
    else if(mode[IEM_PBANK_ITEM_SEPARATOR] == 's')
    {
      sep = ';';
      strcpy(formattext, "item-separator = SEMICOLON; ");
    }
    else if(mode[IEM_PBANK_ITEM_SEPARATOR] == 't')
    {
      sep = 0x09;
      strcpy(formattext, "item-separator = TABULATOR; ");
    }
    
    eol_offset = 0;
    if(mode[IEM_PBANK_END_OF_LINE] == 'b')
    {
      eol[0] = ' ';
      strcat(formattext, "end_of_line_terminator = BLANK-RETURN.");
    }
    else if(mode[IEM_PBANK_END_OF_LINE] == 's')
    {
      eol[0] = ';';
      strcat(formattext, "end_of_line_terminator = SEMICOLON-RETURN.");
    }
    else if(mode[IEM_PBANK_END_OF_LINE] == 't')
    {
      eol[0] = 0x09;
      strcat(formattext, "end_of_line_terminator = TABULATOR-RETURN.");
    }
    else if(mode[IEM_PBANK_END_OF_LINE] == 'r')
    {
      eol_offset = 1;
      strcat(formattext, "end_of_line_terminator = RETURN.");
    }
    
    ap = x->x_atbegmem;
    for(l=0; l<nrl; l++)
    {
      for(p=1; p<nrp; p++)
      {
        if(IS_A_FLOAT(ap, 0))
          fprintf(fh, "%g%c", ap->a_w.w_float, sep);
        else if(IS_A_SYMBOL(ap, 0))
          fprintf(fh, "%s%c", ap->a_w.w_symbol->s_name, sep);
        ap++;
      }
      if(IS_A_FLOAT(ap, 0))
        fprintf(fh, "%g%s", ap->a_w.w_float, eol+eol_offset);
      else if(IS_A_SYMBOL(ap, 0))
        fprintf(fh, "%s%s", ap->a_w.w_symbol->s_name, eol+eol_offset);
      ap++;
    }
    fclose(fh);
    post("iem_pbank_csv: wrote %d parameters x %d lines to file:\n%s\nwith following format:\n%s\n", nrp, nrl, completefilename, formattext);
  }
}

static int iem_pbank_csv_text2atom(char *text, int text_size, t_atom **at_beg,
                                   int *nalloc, char sep, char eol)
{
  char buf[MAXPDSTRING+1], *bufp, *ebuf = buf+MAXPDSTRING;
  const char *textp = text, *etext = text + text_size;
  int natom = 0;
  t_atom *ap = *at_beg;
  t_float f;
  
  while(1)
  {
    int type;
    
    if(textp == etext)
      break;
    if(*textp == eol)
    {
      SETSEMI(ap);
      textp++;
    }
    else if(*textp == sep)
    {
      SETCOMMA(ap);
      textp++;
    }
    else
    {
      char c;
      int flst = 0, slash = 0, lastslash = 0;
      int firstslash = (*textp == '\\');
      
      bufp = buf;
      do
      {
        c = *bufp = *textp++;
        lastslash = slash;
        slash = (c == '\\');
        
        if (flst >= 0)
        {
          int digit = (c >= '0' && c <= '9'),
          dot = (c == '.'), minus = (c == '-'),
          plusminus = (minus || (c == '+')),
          expon = (c == 'e' || c == 'E');
          if (flst == 0)  /* beginning */
          {
            if (minus) flst = 1;
            else if (digit) flst = 2;
            else if (dot) flst = 3;
            else flst = -1;
          }
          else if (flst == 1) /* got minus */
          {
            if (digit) flst = 2;
            else if (dot) flst = 3;
            else flst = -1;
          }
          else if (flst == 2) /* got digits */
          {
            if (dot) flst = 4;
            else if (expon) flst = 6;
            else if (!digit) flst = -1;
          }
          else if (flst == 3) /* got '.' without digits */
          {
            if (digit) flst = 5;
            else flst = -1;
          }
          else if (flst == 4) /* got '.' after digits */
          {
            if (digit) flst = 5;
            else if (expon) flst = 6;
            else flst = -1;
          }
          else if (flst == 5) /* got digits after . */
          {
            if (expon) flst = 6;
            else if (!digit) flst = -1;
          }
          else if (flst == 6) /* got 'e' */
          {
            if (plusminus) flst = 7;
            else if (digit) flst = 8;
            else flst = -1;
          }
          else if (flst == 7) /* got plus or minus */
          {
            if (digit) flst = 8;
            else flst = -1;
          }
          else if (flst == 8) /* got digits */
          {
            if (!digit) flst = -1;
          }
        }
        if (!slash) bufp++;
      }
      while (textp != etext && bufp != ebuf && *textp != ' ' &&
             (slash || (*textp != sep && *textp != eol)));
      *bufp = 0;
      
      if(*buf == '$' && buf[1] >= '0' && buf[1] <= '9' && !firstslash)
      {
        for (bufp = buf+2; *bufp; bufp++)
          if (*bufp < '0' || *bufp > '9')
          {
            SETDOLLSYM(ap, gensym(buf+1));
            goto iem_pbank_csv_didit;
          }
            SETDOLLAR(ap, atoi(buf+1));
iem_pbank_csv_didit: ;
      }
      else
      {
        if(flst == 2 || flst == 4 || flst == 5 || flst == 8)
        {
          f = atof(buf);
          if((f < 1.0e-20)&&(f > -1.0e-20))
            f = 0.0;
          SETFLOAT(ap, f);
        }
        else
          SETSYMBOL(ap, gensym(buf));
      }
    }
    
    ap++;
    natom++;
    if(natom == *nalloc)
    {
      *at_beg = t_resizebytes(*at_beg, *nalloc * sizeof(t_atom),
                              *nalloc * (2*sizeof(t_atom)));
      *nalloc = *nalloc * 2;
      ap = *at_beg + natom;
    }
    if(textp == etext)
      break;
  }
  return(natom);
}

static void iem_pbank_csv_debug(char *buf, int n)
{
  while(n >= 16)
  {
    post("%x %x %x %x | %x %x %x %x | %x %x %x %x | %x %x %x %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
    n -= 16;
    buf += 16;
  }
  switch(n)
  {
  case 15:
    post("%x %x %x %x | %x %x %x %x | %x %x %x %x | %x %x %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14]);
    break;
  case 14:
    post("%x %x %x %x | %x %x %x %x | %x %x %x %x | %x %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13]);
    break;
  case 13:
    post("%x %x %x %x | %x %x %x %x | %x %x %x %x | %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12]);
    break;
  case 12:
    post("%x %x %x %x | %x %x %x %x | %x %x %x %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
    break;
  case 11:
    post("%x %x %x %x | %x %x %x %x | %x %x %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10]);
    break;
  case 10:
    post("%x %x %x %x | %x %x %x %x | %x %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
    break;
  case 9:
    post("%x %x %x %x | %x %x %x %x | %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]);
    break;
  case 8:
    post("%x %x %x %x | %x %x %x %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    break;
  case 7:
    post("%x %x %x %x | %x %x %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
    break;
  case 6:
    post("%x %x %x %x | %x %x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    break;
  case 5:
    post("%x %x %x %x | %x", buf[0], buf[1], buf[2], buf[3], buf[4]);
    break;
  case 4:
    post("%x %x %x %x", buf[0], buf[1], buf[2], buf[3]);
    break;
  case 3:
    post("%x %x %x", buf[0], buf[1], buf[2]);
    break;
  case 2:
    post("%x %x", buf[0], buf[1]);
    break;
  case 1:
    post("%x", buf[0]);
    break;
   }
}

static void iem_pbank_csv_read(t_iem_pbank_csv *x, t_symbol *filename, t_symbol *format)
{
  char completefilename[400], eol, sep, mode[4], *txbuf1, *txbuf2, *txvec_src, *txvec_dst;
  int size, p, l, i, j, nrl=x->x_nr_line, nrp=x->x_nr_para, atlen=0;
  int txlen, txalloc, hat_alloc, max, eol_length;
  FILE *fh;
  t_atom *ap, *hap, *at;
  char formattext[100], str_format[8];
  
  strcpy(mode, "br"); // blank-separator, return-eol
  sep = ' '; // default SEP = space
  eol = ';'; // default any char
  eol_length = 1; // default: EOL = return only
  
  if(filename->s_name[0] == '/')/*make complete path + filename*/
     {
       strcpy(completefilename, filename->s_name);
     }
     else if(((filename->s_name[0] >= 'A')&&(filename->s_name[0] <= 'Z')||
              (filename->s_name[0] >= 'a')&&(filename->s_name[0] <= 'z'))&&
             (filename->s_name[1] == ':')&&(filename->s_name[2] == '/'))
     {
       strcpy(completefilename, filename->s_name);
     }
     else
     {
       strcpy(completefilename, canvas_getdir(x->x_canvas)->s_name);
       strcat(completefilename, "/");
       strcat(completefilename, filename->s_name);
     }
     
     fh = sys_fopen(completefilename,"rb");
     if(!fh)
     {
       post("iem_pbank_csv_read: cannot open %s !!\n", completefilename);
     }
     else
     {
       if(strlen(format->s_name) >= IEM_PBANK_FORMAT_SIZE)
       {
         strncpy(str_format, format->s_name, IEM_PBANK_FORMAT_SIZE);
         str_format[IEM_PBANK_FORMAT_SIZE] = 0;
         for(p=0; p<IEM_PBANK_FORMAT_SIZE; p++)
         {
           if((str_format[p] >= 'A')&&(str_format[p] <= 'Z'))
             str_format[p] += 'a' - 'A';
         }
         if((str_format[IEM_PBANK_ITEM_SEPARATOR] == 'b')
            ||(str_format[IEM_PBANK_ITEM_SEPARATOR] == 's')
            ||(str_format[IEM_PBANK_ITEM_SEPARATOR] == 't'))
           mode[IEM_PBANK_ITEM_SEPARATOR] = str_format[IEM_PBANK_ITEM_SEPARATOR];
         
         if((str_format[IEM_PBANK_END_OF_LINE] == 'b')
            ||(str_format[IEM_PBANK_END_OF_LINE] == 's')
            ||(str_format[IEM_PBANK_END_OF_LINE] == 't')
            ||(str_format[IEM_PBANK_END_OF_LINE] == 'r'))
           mode[IEM_PBANK_END_OF_LINE] = str_format[IEM_PBANK_END_OF_LINE];
       }
       else
         post("iem_pbank_csv_read: use default format %s !!\n", mode);
       if(mode[IEM_PBANK_ITEM_SEPARATOR] == 'b')
       {
         sep = ' ';
         strcpy(formattext, "item-separator = BLANK; ");
       }
       else if(mode[IEM_PBANK_ITEM_SEPARATOR] == 's')
       {
         sep = ';';
         strcpy(formattext, "item-separator = SEMICOLON; ");
       }
       else if(mode[IEM_PBANK_ITEM_SEPARATOR] == 't')
       {
         sep = 0x09;
         strcpy(formattext, "item-separator = TABULATOR; ");
       }
       
       eol_length = 2; // if EOL are 2 char
       if(mode[IEM_PBANK_END_OF_LINE] == 'b')
       {
         eol = ' ';
         strcat(formattext, "end_of_line_terminator = BLANK-RETURN.");
       }
       else if(mode[IEM_PBANK_END_OF_LINE] == 's')
       {
         eol = ';';
         strcat(formattext, "end_of_line_terminator = SEMICOLON-RETURN.");
       }
       else if(mode[IEM_PBANK_END_OF_LINE] == 't')
       {
         eol = 0x09;
         strcat(formattext, "end_of_line_terminator = TABULATOR-RETURN.");
       }
       else if(mode[IEM_PBANK_END_OF_LINE] == 'r')
       {
         eol_length = 1; // if EOL is only 1 char = return oly
         strcat(formattext, "end_of_line_terminator = RETURN.");
       }
       
       fseek(fh, 0, SEEK_END);
       txalloc = ftell(fh);
       fseek(fh,0,SEEK_SET);
       txbuf1 = (char *)getbytes((2 * txalloc + 256) * sizeof(char));
       txbuf2 = (char *)getbytes((2 * txalloc + 256) * sizeof(char));
       if(fread(txbuf1, sizeof(char), txalloc, fh) < sizeof(char)*txalloc)
         post("pbank.csv:435: warning read error (not specified)");
       fclose(fh);
       
	// 1.)  allow only readable ASCII (0x09, 0x0a, 0x0d, 0x20...0x7e = 
  //      = TAB, LF, CR, ' ' ... '~' = 
  //      = Tabulator, LineFeed, CarridgeReturn, Space ... Tilde), else drop
       txvec_src = txbuf1;
       txvec_dst = txbuf2;
       txlen = txalloc;
       p = 0;
       
       for(l=0; l<txlen; l++)
       {
         if(((*txvec_src >= ' ') && (*txvec_src <= '~')) || (*txvec_src == 0x09) || (*txvec_src == 0x0a) || (*txvec_src == 0x0d))
         {
           *txvec_dst++ = *txvec_src++;  // copy the same char
           p++;
         }
         else
           txvec_src++;// drop anything else
       }
       txlen = p; // dst is 2
       
  // 2.) unify windows return
       txvec_src = txbuf2;
       txvec_dst = txbuf1;
       p = 0;
       
       txlen--;  // because we seek 2 char
       for(l=0; l<txlen; l++)
       {
         if((txvec_src[0] == 0x0d)&&(txvec_src[1] == 0x0a))  // windows return
         {
           *txvec_dst++ = IEM_PBANK_UNIFIED_RET;
           txvec_src += 2;
           l++;
           p++;
         }
         else
         {
           if(l == (txlen-1))
           {
             *txvec_dst++ = *txvec_src++;
             p++;
           }
           *txvec_dst++ = *txvec_src++;
           p++;
         }
       }
       txlen = p; // dst is 1
       
  // 3.) unify any other return
       txvec_src = txbuf1;
       txvec_dst = txbuf2;
       
       p = 0;
       for(l=0; l<txlen; l++)
       {
         if((*txvec_src == 0x0d) || (*txvec_src == 0x0a))  //  return
           *txvec_dst++ = IEM_PBANK_UNIFIED_RET;
         else
           *txvec_dst++ = *txvec_src;
         txvec_src++;
         p++;
       }
       txlen = p; // dst is 2
       
  // 4.) unify separator
       txvec_src = txbuf2;
       txvec_dst = txbuf1;
       
       p = 0;
       for(l=0; l<txlen; l++)
       {
         if(*txvec_src == sep)  // replace 'sep' by IEM_PBANK_UNIFIED_SEP
           *txvec_dst++ = IEM_PBANK_UNIFIED_SEP;
         else
           *txvec_dst++ = *txvec_src;
         txvec_src++;
         p++;
       }
       txlen = p; // dst is 1
       
  // 5.) unify EndOfLine
       txvec_src = txbuf1;
       txvec_dst = txbuf2;
       
       p = 0;
       if(eol_length == 2) // EndOfLine are 2 char
       {
         txlen--;  // because we seek 2 char
         for(l=0; l<txlen; l++)
         {
           if((txvec_src[0] == eol)&&(txvec_src[1] == IEM_PBANK_UNIFIED_RET))
           {
             *txvec_dst++ = IEM_PBANK_UNIFIED_EOL;
             txvec_src += 2;
             l++;
             p++;
           }
           else
           {
             if(l == (txlen-1))
             {
               *txvec_dst++ = *txvec_src++;
               p++;
             }
             *txvec_dst++ = *txvec_src++;
             p++;
           }
         }
       }
       else // EndOfLine is only 1 char
       {
         for(l=0; l<txlen; l++)
         {
           if(*txvec_src == IEM_PBANK_UNIFIED_RET)
             *txvec_dst++ = IEM_PBANK_UNIFIED_EOL;
           else
             *txvec_dst++ = *txvec_src;
           txvec_src++;
           p++;
         }
       }
       txlen = p; // dst is 2
       
  // 6.) now correct the decimal comma to point (sometimes it happens with MS Excel)
       txvec_src = txbuf2;
       txvec_dst = txbuf1;
       
       p = 0;
       for(l=0; l<txlen; l++)
       {
         if(*txvec_src == ',')  // replace ',' by '.'
           *txvec_dst++ = '.';
         else
           *txvec_dst++ = *txvec_src;
         txvec_src++;
         p++;
       }
       txlen = p; // dst is 1
       
       
       
       
    // 7.) fill between 2 separators a zero
    // 7.) fill between separator and eol a zero
    // 7.) fill between eol and separator a zero
       txvec_src = txbuf1;
       txvec_dst = txbuf2;
       
       p = 0;
       txlen--;
       i = 0;
       for(l=0; l<txlen; )
       {
         if((txvec_src[0] == IEM_PBANK_UNIFIED_SEP)&&(txvec_src[1] == IEM_PBANK_UNIFIED_SEP))  // fill between 2 sep a zero
         {
           txvec_dst[0] = IEM_PBANK_UNIFIED_SEP;
           txvec_dst[1] = '0';
           txvec_dst[2] = IEM_PBANK_UNIFIED_SEP;
           p += 2;
           i = 1;
           txvec_dst += 2;
           l++;
           txvec_src++;
         }
         else if((txvec_src[0] == IEM_PBANK_UNIFIED_SEP)&&(txvec_src[1] == IEM_PBANK_UNIFIED_EOL))  // fill between sep and eol a zero
         {
           txvec_dst[0] = IEM_PBANK_UNIFIED_SEP;
           txvec_dst[1] = '0';
           txvec_dst[2] = IEM_PBANK_UNIFIED_EOL;
           p += 2;
           i = 1;
           txvec_dst += 2;
           l++;
           txvec_src++;
         }
         else if((txvec_src[0] == IEM_PBANK_UNIFIED_EOL)&&(txvec_src[1] == IEM_PBANK_UNIFIED_SEP))  // fill between sep and eol a zero
         {
           txvec_dst[0] = IEM_PBANK_UNIFIED_EOL;
           txvec_dst[1] = '0';
           txvec_dst[2] = IEM_PBANK_UNIFIED_SEP;
           p += 2;
           i = 1;
           txvec_dst += 2;
           l++;
           txvec_src++;
         }
         else // copy the same char
         {
           if(l == (txlen-1))
           {
             *txvec_dst++ = *txvec_src++;
             p++;
             l++;
           }
           *txvec_dst++ = *txvec_src++;
           p++;
           l++;
         }
       }
       if(i)
         p++;
       txlen = p; // dst is 2
       
       txvec_src = txbuf2;
       
       
       hat_alloc = 200;
       hap = t_getbytes(hat_alloc * sizeof(t_atom));
       
       atlen = iem_pbank_csv_text2atom(txbuf2, txlen, &hap, &hat_alloc, IEM_PBANK_UNIFIED_SEP, IEM_PBANK_UNIFIED_EOL);
       
       at = x->x_atbegmem;
       for(l=0; l<nrl; l++)  /*reset all*/
       {
         for(p=0; p<nrp; p++)
         {
           SETFLOAT(at, 0.0f);
           at++;
         }
       }
       
       at = x->x_atbegmem;
       ap = hap;
       nrp++;
       i = 0; /* atom-counter */
       j = 0;
       for(l=0; l<nrl; l++)/* nrl line times */
       {
         for(p=1; p<=nrp;)
         {
           if((p == nrp) && !(IS_A_SEMI(ap,0)))
           {
             /*post("too long");*/
             while(!(IS_A_SEMI(ap,0)))
             {
               ap++;
               atlen--;
               /*post("ignore");*/
               j++;
               if(atlen <= 0)
               {
                 goto iem_pbank_csv_end;
               }
             }
           }
           else
           {
             if(IS_A_FLOAT(ap,0))
             {
               SETFLOAT(at, ap->a_w.w_float);
               /*post("float");*/
               p++;
               i++;
               at++;
             }
             else if(IS_A_SYMBOL(ap,0))
             {
               SETSYMBOL(at, ap->a_w.w_symbol);
               /*post("sym");*/
               p++;
               i++;
               at++;
             }
             else if(IS_A_SEMI(ap,0))
             {
               /*post("semi");*/
               for(; p<nrp;)
               {
                 SETFLOAT(at,0.0);
                 /*post("zero");*/
                 p++;
                 i++;
                 at++;
               }
               p=nrp + 1;
             }
             ap++;
             atlen--;
             j++;
           }
           if(atlen <= 0)
           {
             goto iem_pbank_csv_end;
           }
         }
       }
       
iem_pbank_csv_end:
       freebytes(hap, hat_alloc * sizeof(t_atom));
       freebytes(txbuf1, (2 * txalloc + 256) * sizeof(char));
       freebytes(txbuf2, (2 * txalloc + 256) * sizeof(char));
       post("iem_pbank_csv: read %d parameters x %d lines from file:\n%s\nwith following format:\n%s\n", nrp-1, nrl, completefilename, formattext);
  }
}

static void iem_pbank_csv_recall(t_iem_pbank_csv *x, t_symbol *s, int ac, t_atom *av)
{
  int i, n, beg=0, nrp=x->x_nr_para;
  t_atom *atbuf=x->x_atbegbuf, *atmem=x->x_atbegmem;
  t_atom *atout=x->x_atbegout;
  
  if(ac >= 2)
    nrp = atom_getintarg(1, ac, av);
  if(ac >= 1)
    beg = atom_getintarg(0, ac, av);
  if(beg < 0)
    beg = 0;
  else if(beg >= x->x_nr_para)
    beg = x->x_nr_para - 1;
  if(nrp < 0)
    nrp = 0;
  else if((beg+nrp) > x->x_nr_para)
    nrp = x->x_nr_para - beg;
  atmem += x->x_nr_para * x->x_line + beg;
  atbuf += beg;
  SETFLOAT(atout, (t_float)beg);
  atout++;
  for(i=0; i<nrp; i++)
  {
    *atbuf++ = *atmem;
    *atout++ = *atmem++;
  }
  outlet_list(x->x_offset_list_out, &s_list, nrp+1, x->x_atbegout);
  outlet_list(x->x_list_out, &s_list, nrp, x->x_atbegout+1);
}

static void iem_pbank_csv_bang(t_iem_pbank_csv *x)
{
  int i, nrp=x->x_nr_para;
  t_atom *atbuf=x->x_atbegbuf;
  t_atom *atout=x->x_atbegout;
  
  SETFLOAT(atout, 0.0f);
  atout++;
  for(i=0; i<nrp; i++)
    *atout++ = *atbuf++;
  outlet_list(x->x_offset_list_out, &s_list, nrp+1, x->x_atbegout);
  outlet_list(x->x_list_out, &s_list, nrp, x->x_atbegout+1);
}

static void iem_pbank_csv_store(t_iem_pbank_csv *x, t_symbol *s, int ac, t_atom *av)
{
  int i, beg=0, nrp=x->x_nr_para;
  t_atom *atbuf=x->x_atbegbuf, *atmem=x->x_atbegmem;
  
  if(ac >= 2)
    nrp = atom_getintarg(1, ac, av);
  if(ac >= 1)
    beg = atom_getintarg(0, ac, av);
  if(beg < 0)
    beg = 0;
  else if(beg >= x->x_nr_para)
    beg = x->x_nr_para - 1;
  if(nrp < 0)
    nrp = 0;
  else if((beg+nrp) > x->x_nr_para)
    nrp = x->x_nr_para - beg;
  atmem += x->x_nr_para * x->x_line;
  atmem += beg;
  atbuf += beg;
  for(i=0; i<nrp; i++)
    *atmem++ = *atbuf++;
}

static void iem_pbank_csv_list(t_iem_pbank_csv *x, t_symbol *s, int ac, t_atom *av)
{
  if(ac >= 2)
  {
    int para_index = atom_getintarg(0, ac, av);
    
    if(para_index >= 0)
    {
      if((para_index+ac-1) <= x->x_nr_para)
      {
        int i;
        
        for(i=1; i<ac; i++)
        {
          x->x_atbegbuf[para_index] = av[i];
          para_index++;
        }
      }
    }
  }
}

static void iem_pbank_csv_ft1(t_iem_pbank_csv *x, t_floatarg fline_nr)
{
  int line = (int)fline_nr;
  
  if(line < 0)
    line = 0;
  else if(line >= x->x_nr_line)
    line = x->x_nr_line - 1;
  x->x_line = line;
}

static void iem_pbank_csv_free(t_iem_pbank_csv *x)
{
  freebytes(x->x_atbegmem, x->x_nr_para * x->x_nr_line * sizeof(t_atom));
  freebytes(x->x_atbegbuf, x->x_nr_para * sizeof(t_atom));
  freebytes(x->x_atbegout, (x->x_nr_para+1) * sizeof(t_atom));
}

static void *iem_pbank_csv_new(t_symbol *s, int ac, t_atom *av)
{
  t_iem_pbank_csv *x = (t_iem_pbank_csv *)pd_new(iem_pbank_csv_class);
  int nrpp=0, nrp=10, nrl=10, p, l, i;
  t_atom *ap;
  
  if((ac >= 1) && IS_A_FLOAT(av,0))
    nrp = atom_getintarg(0, ac, av);
  if((ac >= 2) && IS_A_FLOAT(av,1))
    nrl = atom_getintarg(1, ac, av);
  if(nrp <= 0)
    nrp = 10;
  if(nrl <= 0)
    nrl = 10;
  x->x_line = 0;
  x->x_nr_para = nrp;
  x->x_nr_line = nrl;
  x->x_atbegmem = (t_atom *)getbytes(x->x_nr_para * x->x_nr_line * sizeof(t_atom));
  x->x_atbegbuf = (t_atom *)getbytes(x->x_nr_para * sizeof(t_atom));
  x->x_atbegout = (t_atom *)getbytes((x->x_nr_para+1) * sizeof(t_atom));
  ap = x->x_atbegmem;
  for(l=0; l<nrl; l++)
  {
    for(p=0; p<nrp; p++)
    {
      SETFLOAT(ap, 0.0f);
      ap++;
    }
  }
  ap = x->x_atbegbuf;
  for(p=0; p<nrp; p++)
  {
    SETFLOAT(ap, 0.0f);
    ap++;
  }
  x->x_list_out = outlet_new(&x->x_obj, &s_list);     /*left out*/
  x->x_offset_list_out = outlet_new(&x->x_obj, &s_list);  /*right out*/
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  x->x_canvas = canvas_getcurrent();
  return (x);
}

/* ---------------- global setup function -------------------- */

void iem_pbank_csv_setup(void )
{
  iem_pbank_csv_class = class_new(gensym("iem_pbank_csv"), (t_newmethod)iem_pbank_csv_new,
                                  (t_method)iem_pbank_csv_free, sizeof(t_iem_pbank_csv), 0, A_GIMME, 0);
  class_addmethod(iem_pbank_csv_class, (t_method)iem_pbank_csv_recall, gensym("recall"), A_GIMME, 0);
  class_addmethod(iem_pbank_csv_class, (t_method)iem_pbank_csv_store, gensym("store"), A_GIMME, 0);
  class_addmethod(iem_pbank_csv_class, (t_method)iem_pbank_csv_read, gensym("read"), A_SYMBOL, A_DEFSYM, 0);
  class_addmethod(iem_pbank_csv_class, (t_method)iem_pbank_csv_write, gensym("write"), A_SYMBOL, A_DEFSYM, 0);
  class_addlist(iem_pbank_csv_class, iem_pbank_csv_list);
  class_addbang(iem_pbank_csv_class, iem_pbank_csv_bang);
  class_addmethod(iem_pbank_csv_class, (t_method)iem_pbank_csv_ft1, gensym("ft1"), A_FLOAT, 0);
//  class_sethelpsymbol(iem_pbank_csv_class, gensym("iemhelp/help-iem_pbank_csv"));
}
