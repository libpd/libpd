///////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_tIIRf.h"
#include "RTE/MessageCallbacks.h"

CPPEXTERN_NEW_WITH_TWO_ARGS(pix_tIIRf, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_tIIRf
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_tIIRf :: pix_tIIRf(t_floatarg fb_numf, t_floatarg ff_numf)
  : m_set(SET),
    m_ff(NULL), m_fb(NULL),
    m_ffnum(0), m_fbnum(0),
    m_buffer(NULL),
    m_bufnum(0),
    m_counter(0),
    m_inlet(NULL)
{ 
  int fb_num = (fb_numf>0.)?static_cast<int>(fb_numf):0;
  int ff_num = (ff_numf>0.)?static_cast<int>(ff_numf):0;
  fb_num++;
  ff_num++;

  m_ffnum=ff_num;
  m_fbnum=fb_num;

  m_inlet = new t_inlet*[fb_num+ff_num];
  t_inlet **inlet = m_inlet;

  m_fb = new t_float[fb_num];
  m_ff = new t_float[ff_num];

  int i=0;
  while(i<fb_num){
    m_fb[i]=0.0;
    *inlet++=floatinlet_new(this->x_obj, m_fb+i);
    i++;
  }
  m_fb[0]=1.0;
  i=0;
  while(i<ff_num){
    m_ff[i]=0.0;
    *inlet++=floatinlet_new(this->x_obj, m_ff+i);
    i++;
  }
  m_ff[0]=1.0;

  m_bufnum=1+MAX(fb_num,ff_num); // 1 for output, rest for FF/FB

  m_buffer = new t_float*[m_bufnum];
  for(i=0; i<m_bufnum; i++)
    m_buffer[i]=NULL;

  m_image.xsize=0;
  m_image.ysize=0;
  m_image.csize=4;
  m_image.format=GL_RGBA;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_tIIRf :: ~pix_tIIRf()
{
  // clean my buffer
  deallocate();
  if(m_inlet) {
    int i;
    for(i=0; i<(m_fbnum+m_ffnum); i++)inlet_free(m_inlet[i]);
    delete[]m_inlet;
    m_inlet=NULL;
  }

  if(m_fb)delete[]m_fb; m_fb=NULL;
  if(m_ff)delete[]m_ff; m_ff=NULL;

  if(m_buffer)delete[]m_buffer; m_buffer=NULL;
  m_bufnum=0;
}

static inline size_t imgSize(const imageStruct*img) {
  return (img->xsize*img->ysize*img->csize);
}

static inline bool imgCompare(const imageStruct&img1, const imageStruct&img2) {
  return ((img1.xsize==img2.xsize) &&
	  (img1.ysize==img2.ysize) &&
	  (img1.csize==img2.csize));
}

// allocate ff+fb buffers that can hold "img" like images
void pix_tIIRf ::allocate(imageStruct&img) {
  deallocate();

  m_image.xsize=img.xsize;
  m_image.ysize=img.ysize;
  m_image.csize=img.csize;
  m_image.reallocate();

  m_image.format=img.format;
  m_image.setBlack();

  size_t size=imgSize(&img);
  int i;
  for(i=0; i<m_bufnum; i++) {
    m_buffer[i]=new t_float[size*sizeof(t_float)];
  }
}
// free ff+fb buffers
void pix_tIIRf ::deallocate(void) {
  int i;
  for(i=0; i<m_bufnum; i++) {
    if(m_buffer[i])delete[]m_buffer[i];m_buffer[i]=NULL;
  }
}

// store the given image in the buffer (doing a conversion to float)
static void img2buf(imageStruct*img, t_float*fbuffer, const t_float factor=1.) {
  const t_float f=factor/255.;
  unsigned char*bbuffer=img->data;
  size_t size=imgSize(img);
  size_t i;

  for(i=0; i<size; i++) {
    *fbuffer++ = f * (*bbuffer++);
  }
}
// retrieve the current buffer into the img (doing a conversion from float)
static void buf2img(t_float*fbuffer, imageStruct*img) {
  unsigned char*bbuffer=img->data;
  size_t size=imgSize(img);
  size_t i;

  for(i=0; i<size; i++) {
    *bbuffer++ = CLAMP(static_cast<int>((*fbuffer++)*255));
  }
}

static void weightAdd(t_float*src, t_float*dest, const t_float weight, size_t len) {
  size_t i;
  for(i=0; i<len; i++) {
    *dest++ += weight* (*src++);
  }
}
static void weightSet(t_float*src, t_float*dest, const t_float weight, size_t len) {
  size_t i;
  for(i=0; i<len; i++) {
    *dest++ = weight* (*src++);
  }
}


// copy the given img to all buffers
// img img==NULL, set all buffers to black
void pix_tIIRf ::set(imageStruct*img) {
  if(img==NULL)
    img=&m_image;

  int i;
  for(i=0; i<m_bufnum; i++) {
    img2buf(img, m_buffer[i]);
  }
}

static inline unsigned int getIndex(unsigned int current, int index, unsigned int size) {
  // adding size to avoid negative numbers
  return (size+current+index)%size;
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_tIIRf :: processImage(imageStruct &image)
{
  int j;
  size_t imagesize = imgSize(&image);
  unsigned char *dest;

  if(!imgCompare(image, m_image)) {
    // LATER only reallocate if really needed
    deallocate();
    allocate(image);
    m_set=CLEAR;
  }

  switch(m_set) {
  case SET: set(&image); break;
  case CLEAR: set(); break;
  default: break;
  }
  m_set=NONE;

  dest=m_image.data;

  // do the filtering

  // feed-back
  // w[n] = x[n]*ff0 + w[n-1]*ff1 + ... + w[n-N]*ffN

  // w[n] = x[n]*ff0
  img2buf(&image, m_buffer[m_counter], m_fb[0]);
  j=m_fbnum;
  for(j=1; j<m_fbnum; j++) {
    // w[n] += w[n-J]*ffJ
    const unsigned int index=getIndex(m_counter, -j, m_bufnum-1);
    weightAdd(m_buffer[index],
	      m_buffer[m_counter], 
	      m_fb[j], 
	      imagesize);
  }

  // feed-forward
  //  y[n] = ff0*w[n] + ff1*w[n-1] + ... + ffM*w[n-M]
  weightSet(m_buffer[m_counter], 
	    m_buffer[m_bufnum-1],
	    m_ff[0],
	    imagesize);

  for(j=1; j<m_ffnum; j++) {
    const unsigned int index=getIndex(m_counter, -j, m_bufnum-1);
    weightAdd(m_buffer[index],
	      m_buffer[m_bufnum-1],
	      m_ff[j],
	      imagesize);
  }

  buf2img(m_buffer[m_bufnum-1], &image);


  m_counter = getIndex(m_counter, 1, m_bufnum-1);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_tIIRf :: obj_setupCallback(t_class *classPtr)
{
  CPPEXTERN_MSG (classPtr, "set", setMess);
  CPPEXTERN_MSG (classPtr, "list", listMess);
}

void pix_tIIRf :: setMess(t_symbol *s, int argc, t_atom* argv)
{
  m_set = SET;
  if(argc>0 && atom_getint(argv)==0)
    m_set=CLEAR;
  setPixModified();
}

void pix_tIIRf :: listMess(t_symbol *s, int argc, t_atom* argv)
{
  int i=0;
  if(argc != (m_fbnum+m_ffnum)) {
    error("need %d+%d arguments", m_fbnum, m_ffnum);
    return;
  }
  for(i=0; i<m_fbnum; i++) { m_fb[i]=atom_getfloat(argv++); }
  for(i=0; i<m_ffnum; i++) { m_ff[i]=atom_getfloat(argv++); }
    
}
