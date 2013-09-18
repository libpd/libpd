/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2010 */

#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define SFI_HEADER_SAMPLERATE 0
#define SFI_HEADER_FILENAME 1
#define SFI_HEADER_MULTICHANNEL_FILE_LENGTH 2
#define SFI_HEADER_HEADERBYTES 3
#define SFI_HEADER_CHANNELS 4
#define SFI_HEADER_BYTES_PER_SAMPLE 5
#define SFI_HEADER_ENDINESS 6
#define SFI_HEADER_FORMAT_CODE 7

#define SFI_HEADER_SIZE 8
#define SFI_HEADER_CHUNK_SIZE_ESTIMATION 10000



/* --------------------------- soundfile_info -------------------------------- */
/* -- reads only header of a wave-file and outputs the important parameters -- */

static t_class *soundfile_info_class;

typedef struct _soundfile_info
{
  t_object  x_obj;
  long      *x_begmem;
  int       x_mem_size;
  t_atom    x_at_header[SFI_HEADER_SIZE];
  t_canvas  *x_canvas;
  void      *x_list_out;
} t_soundfile_info;

static short soundfile_info_string_to_int16(char *cvec)
{
  short ss=0;
  unsigned char *uc=(unsigned char *)cvec;
  
  ss += (short)(*uc);
  ss += (short)(*(uc+1)*256);
  return(ss);
}

static unsigned long soundfile_info_string_to_uint32(char *cvec)
{
  unsigned long ul=0;
  unsigned char *uc=(unsigned char *)cvec;
  
  ul += (unsigned long)(*uc);
  ul += (unsigned long)(*(uc+1)*256);
  ul += (unsigned long)(*(uc+2)*65536);
  ul += (unsigned long)(*(uc+3)*16777216);
  return(ul);
}

static void soundfile_info_read(t_soundfile_info *x, t_symbol *filename)
{
  char completefilename[400];
  int i, n, n2, n4, filesize, read_chars, header_size=0, ch, bytesperframe, sr, n_frames;
  FILE *fh;
  t_atom *at;
  char *cvec;
  unsigned long ul_chunk_size, ul_sr;
  short ss_format, ss_ch, ss_bytesperframe;
  
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
    post("soundfile_info_read: cannot open %s !!\n", completefilename);
  }
  else
  {
    n = x->x_mem_size; // 10000 bytes
    n2 = sizeof(short) * x->x_mem_size;
    n4 = sizeof(long) * x->x_mem_size;
    fseek(fh, 0, SEEK_END);
    filesize = ftell(fh);
    fseek(fh,0,SEEK_SET);
    read_chars = (int)fread(x->x_begmem, sizeof(char), n4, fh) / 2;
    fclose(fh);
    //    post("read chars = %d", read_chars);
    cvec = (char *)x->x_begmem;
    if(read_chars > 4)
    {
      if(strncmp(cvec, "RIFF", 4))
      {
        post("soundfile_info_read-error:  %s is no RIFF-WAVE-file", completefilename);
        goto soundfile_info_end;
      }
      header_size += 8; // jump over RIFF chunk size
      cvec += 8;
      if(strncmp(cvec, "WAVE", 4))
      {
        post("soundfile_info_read-error:  %s is no RIFF-WAVE-file", completefilename);
        goto soundfile_info_end;
      }
      header_size += 4;
      cvec += 4;
      
      for(i=header_size/2; i<read_chars; i++)
      {
        if(!strncmp(cvec, "fmt ", 4))
        {
          header_size += 4;
          cvec += 4;
          goto soundfile_info_fmt;
        }
        header_size += 2;
        cvec += 2;
      }
      post("soundfile_info_read-error:  %s has at begin no format-chunk", completefilename);
      goto soundfile_info_end;
      
soundfile_info_fmt:
      ul_chunk_size = soundfile_info_string_to_uint32(cvec);
      if(ul_chunk_size < 16)
      {
        post("soundfile_info_read-error:  %s has a format-chunk less than 16", completefilename);
        goto soundfile_info_end;
      }
      header_size += 4;
      cvec += 4;

      ss_format = soundfile_info_string_to_int16(cvec);
      if((ss_format != 1) && (ss_format != 3) && (ss_format != 6) && (ss_format != 7) && (ss_format != -2)) /* PCM = 1 ; IEEE-FLOAT = 3 ; ALAW = 6 ; MULAW = 7 ; WAVE_EX = -2 */
      {
        post("soundfile_info_read-error:  %s has unknown format code", completefilename);
        goto soundfile_info_end;
      }
      SETFLOAT(x->x_at_header+SFI_HEADER_FORMAT_CODE, (t_float)ss_format);
      header_size += 2;
      cvec += 2;

      ss_ch = soundfile_info_string_to_int16(cvec); /* channels */
      if((ss_ch < 1) || (ss_ch > 32000))
      {
        post("soundfile_info_read-error:  %s has no common channel-number", completefilename);
        goto soundfile_info_end;
      }
      SETFLOAT(x->x_at_header+SFI_HEADER_CHANNELS, (t_float)ss_ch);
      ch = (int)ss_ch;
      header_size += 2;
      cvec += 2;

      ul_sr = soundfile_info_string_to_uint32(cvec); /* samplerate */
      if((ul_sr > 2000000000) || (ul_sr < 1))
      {
        post("soundfile_info_read-error:  %s has no common samplerate", completefilename);
        goto soundfile_info_end;
      }
      SETFLOAT(x->x_at_header+SFI_HEADER_SAMPLERATE, (t_float)ul_sr);
      sr = (int)ul_sr;
      header_size += 4;
      cvec += 4;
      
      header_size += 4; /* jump over bytes_per_sec */
      cvec += 4;

      ss_bytesperframe = soundfile_info_string_to_int16(cvec); /* bytes_per_frame */
      if((ss_bytesperframe < 1) || (ss_bytesperframe > 32000))
      {
        post("soundfile_info_read-error:  %s has no common number of bytes per frame", completefilename);
        goto soundfile_info_end;
      }
      SETFLOAT(x->x_at_header+SFI_HEADER_BYTES_PER_SAMPLE, (t_float)(ss_bytesperframe / ss_ch));
      bytesperframe = (int)ss_bytesperframe;
      header_size += 2;
      cvec += 2;
      
      header_size += 2; /* jump over bits_per_sample */
      cvec += 2;
      
      for(i=header_size/2; i<read_chars; i++) // looking for data chunk
      {
        if(!strncmp(cvec, "data", 4))
          goto soundfile_info_data;
        header_size += 2;
        cvec += 2;
      }
      post("soundfile_info_read-error:  %s has at begin no data-chunk", completefilename);
      goto soundfile_info_end;
      
soundfile_info_data:
      header_size += 8; // ignore data chunk size
      cvec += 8;
      
      SETFLOAT(x->x_at_header+SFI_HEADER_HEADERBYTES, (t_float)header_size);
      
      n_frames = (filesize - header_size) / bytesperframe;
      SETFLOAT(x->x_at_header+SFI_HEADER_MULTICHANNEL_FILE_LENGTH, (t_float)n_frames);
      SETSYMBOL(x->x_at_header+SFI_HEADER_ENDINESS, gensym("l"));
      SETSYMBOL(x->x_at_header+SFI_HEADER_FILENAME, gensym(completefilename));
      
      /*      post("ch = %d", ch);
      post("sr = %d", sr);
      post("bpf = %d", bytesperframe/ch);
      post("head = %d", header_size);
      post("len = %d", n_frames);*/
      
      outlet_list(x->x_list_out, &s_list, SFI_HEADER_SIZE, x->x_at_header);
      
      
soundfile_info_end:
      
      ;
    }
  }
}

static void soundfile_info_free(t_soundfile_info *x)
{
  freebytes(x->x_begmem, x->x_mem_size * sizeof(long));
}

static void *soundfile_info_new(void)
{
  t_soundfile_info *x = (t_soundfile_info *)pd_new(soundfile_info_class);
  
  x->x_mem_size = SFI_HEADER_CHUNK_SIZE_ESTIMATION; /* try to read the first 10000 bytes of the soundfile */
  x->x_begmem = (long *)getbytes(x->x_mem_size * sizeof(long));
  x->x_list_out = outlet_new(&x->x_obj, &s_list);
  x->x_canvas = canvas_getcurrent();
  return (x);
}

/* ---------------- global setup function -------------------- */

void soundfile_info_setup(void)
{
  soundfile_info_class = class_new(gensym("soundfile_info"), (t_newmethod)soundfile_info_new,
    (t_method)soundfile_info_free, sizeof(t_soundfile_info), 0, 0);
  class_addmethod(soundfile_info_class, (t_method)soundfile_info_read, gensym("read"), A_SYMBOL, 0);
//  class_sethelpsymbol(soundfile_info_class, gensym("iemhelp/help-soundfile_info"));
}
