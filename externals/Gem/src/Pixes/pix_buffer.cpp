///////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

// we want our pd-class "pix_buffer_class" to be defined not-static
// so other pix_buffer_...-objects can bind to it
#define NO_STATIC_CLASS

#include "pix_buffer.h"
#include "Gem/ImageIO.h"

#include <string.h>
#include <stdio.h>

/////////////////////////////////////////////////////////
//
// pix_buffer
//
/////////////////////////////////////////////////////////

CPPEXTERN_NEW_WITH_TWO_ARGS(pix_buffer, t_symbol*,A_DEFSYM,t_float,A_DEFFLOAT);

/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_buffer :: pix_buffer(t_symbol *s,t_float f=100.0)
{
  if (s==&s_){
    static int buffercounter=0;
    char cbuf[16];
    buffercounter++;
    sprintf(cbuf, "pix_buffer_%04d", buffercounter);
    cbuf[15]=0;
    post("defaulting to name '%s'", cbuf);
    s=gensym(cbuf);
  }

  if (f<0)f=DEFAULT_NUM_FRAMES;
  m_bindname = s;
  m_numframes = (int)f;
  m_buffer = new imageStruct[m_numframes];
  
  pd_bind(&this->x_obj->ob_pd, m_bindname); 
  outlet_new(this->x_obj, &s_float);
}
/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_buffer :: ~pix_buffer()
{
  if(m_buffer)delete [] m_buffer;
  pd_unbind(&this->x_obj->ob_pd, m_bindname);
}
/////////////////////////////////////////////////////////
// allocateMess
//   allocate memory for m_numframes images of size x*y (with pixelsize=c)
//
/////////////////////////////////////////////////////////
void pix_buffer :: allocateMess(int x, int y, int c)
{
  int i = m_numframes;
  int format=0;

  switch (c) {
  case 1:
    format=GL_LUMINANCE;
    break;
  case 2:
    format=GL_YUV422_GEM;
    break;
  case 3:
    format=GL_RGB;
    break;
  case 4:
    format=GL_RGBA;
    break;
  default:
    format=0;
  }

  while(i--){
    m_buffer[i].xsize=x;
    m_buffer[i].ysize=y;
    m_buffer[i].csize=c;
    m_buffer[i].format=format;
    m_buffer[i].reallocate();
    m_buffer[i].setBlack();
  }
}
/////////////////////////////////////////////////////////
// allocateMess
//   allocate memory for m_numframes images of size x*y (with pixelsize=c)
//
/////////////////////////////////////////////////////////
void pix_buffer :: resizeMess(int newsize)
{
  int size=m_numframes;
  int i;
  
  if(newsize<0) {
    error("refusing to resize to <0 frames!");
    return;
  }
  
  imageStruct*buffer = new imageStruct[newsize];
  if(size>newsize)
    size=newsize;
  
  for(i=0; i<size; i++) {
    if(0!=m_buffer[i].data) {
      // copy the image
      m_buffer[i].copy2Image(buffer+i);
      m_buffer[i].xsize=1;
      m_buffer[i].ysize=1;
      // "free" the old image
      m_buffer[i].allocate();
    }
  }
  
  delete[]m_buffer;
  m_buffer=buffer;
  m_numframes=newsize;
  
  bangMess();
}


/////////////////////////////////////////////////////////
// query the number of frames in the buffer
//
/////////////////////////////////////////////////////////
void pix_buffer :: bangMess()
{
  outlet_float(this->x_obj->ob_outlet, m_numframes);
}
int pix_buffer :: numFrames()
{
  return m_numframes;
}
/////////////////////////////////////////////////////////
// put an image into the buffer @ position <pos>
//
/////////////////////////////////////////////////////////
bool pix_buffer :: putMess(imageStruct*img,int pos){
  if (pos<0 || pos>=m_numframes)return false;
  if(!img)return false;
  img->copy2Image(m_buffer+pos);
  return true;
}
/////////////////////////////////////////////////////////
// get an image from the buffer @ position <pos>
//
/////////////////////////////////////////////////////////
imageStruct*pix_buffer :: getMess(int pos){

  //post("getting frame: %d", pos);

  if (pos<0 || pos>=m_numframes)return 0;

  /* just allocated but no image */
  if(0==m_buffer[pos].format)
    return 0;

  return (m_buffer+pos);
}


/////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////
void pix_buffer :: openMess(t_symbol *filename, int pos)
{
  // GRH: muss i wie in pix_image die ganzen andern Sachen a machen ????

  // load an image into mem
  imageStruct *image = NULL;

  // some checks
  if (pos<0 || pos>=m_numframes)
  {
    error("index %d out of range (0..%d)!", pos, m_numframes);
    return;
  }
  std::string file=findFile(filename->s_name);

  image = image2mem(file.c_str());
  if(!image)
  {
    error("'%s' is no valid image!", file.c_str());
    return;
  }

  putMess(image,pos);

  // destroy the image-data
  delete image;
}

/////////////////////////////////////////////////////////
// saveMess
//
/////////////////////////////////////////////////////////
void pix_buffer :: saveMess(t_symbol *filename, int pos)
{
  // save an image from mem
  imageStruct*img=NULL;

  if(NULL==filename||NULL==filename->s_name||gensym("")==filename){
    error("no filename given!");
    return;
  }
  img=getMess(pos);
  
  if(img && img->data){
    mem2image(img, filename->s_name, 0);
  } else {
    error("index %d out of range (0..%d) or slot empty!", pos, m_numframes);
    return;
  }
}

/////////////////////////////////////////////////////////
// copyMess
//
/////////////////////////////////////////////////////////
void pix_buffer :: copyMess(int src, int dst)
{
  if(src==dst) {
    //    error("refusing to do void copying within a slot");
    return;
  }
  // copy an image from one slot to another
  if(putMess(getMess(src), dst)) {
    // fine
  } else {
    error("unable to copy image from slot:%d to slot:%d", src, dst);
    return;
  }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_buffer :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_buffer),
                   gensym("pix_depot"),
                   A_DEFSYM, A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer::allocateMessCallback),
  		  gensym("allocate"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer::resizeMessCallback),
  		  gensym("resize"), A_FLOAT, A_NULL);
  class_addbang(classPtr, reinterpret_cast<t_method>(&pix_buffer::bangMessCallback));
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer::openMessCallback),
  		  gensym("open"), A_SYMBOL, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer::openMessCallback),
  		  gensym("load"), A_SYMBOL, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer::saveMessCallback),
  		  gensym("save"), A_SYMBOL, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer::copyMessCallback),
  		  gensym("copy"), A_FLOAT, A_FLOAT, A_NULL);
}
void pix_buffer :: allocateMessCallback(void *data, t_symbol*s, int argc, t_atom*argv)
{
  int x=0;
  int y=0;
  int c=0;

  t_atom*ap=0;

  switch(argc) {
  case 3:
    ap=argv+2;
    if(A_SYMBOL==ap->a_type) {
      char c0 =*atom_getsymbol(ap)->s_name;

      switch (c0){
      case 'g': case 'G': c=1; break;
      case 'y': case 'Y': c=2; break;
      case 'r': case 'R': c=4; break;
      default:
	GetMyClass(data)->error("invalid format %s!", atom_getsymbol(ap)->s_name);
	return;
      }
    } else if(A_FLOAT==ap->a_type) {

      c=atom_getint(ap);

    } else {
      GetMyClass(data)->error("invalid format!");
      return;
    }
  case 2:
    if((A_FLOAT==argv->a_type) && (A_FLOAT==(argv+1)->a_type)) {
      x=atom_getint(argv);
      y=atom_getint(argv+1);
    } else {
      GetMyClass(data)->error("invalid dimensions!");
      return;
    }
    break;
  case 1:
    if(A_FLOAT==argv->a_type) {
      x=atom_getint(argv);
      y=1;
      c=1;
    } else {
      GetMyClass(data)->error("invalid dimension!");
      return;
    }
    break;
  default:
    GetMyClass(data)->error("usage: allocate <width> <height> <format>");
    return;
  }

  if (x<1 || y<1 || c<0){
    GetMyClass(data)->error("init-specs out of range");
    return;
  }
  if (c==0)c=4;

  GetMyClass(data)->allocateMess((int)x, (int)y, (int)c);
}
void pix_buffer :: bangMessCallback(void *data)
{
  GetMyClass(data)->bangMess();
}
void pix_buffer :: openMessCallback(void *data, t_symbol *filename, t_floatarg pos)
{
  GetMyClass(data)->openMess(filename, (int)pos);
}
void pix_buffer :: saveMessCallback(void *data, t_symbol *filename, t_floatarg pos)
{
  GetMyClass(data)->saveMess(filename, (int)pos);
}
void pix_buffer :: resizeMessCallback(void *data, t_floatarg size)
{
  GetMyClass(data)->resizeMess((int)size);
}
void pix_buffer :: copyMessCallback(void *data, t_floatarg src, t_floatarg dst)
{
  GetMyClass(data)->copyMess((int)src, (int)dst);
}
