////////////////////////////////////////////////////////
//
// pix_hit
//
// hit test over user defined hit_areas...
// 
// Author: Davide Morelli
// http://ww.davidemorelli.it
//
/////////////////////////////////////////////////////////

#include "pix_hit.h"
#include "Gem/PixConvert.h"


CPPEXTERN_NEW(pix_hit);

/////////////////////////////////////////////////////////
//
// pix_hit
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_hit :: pix_hit() :
  minimum_threshold(DEF_THRESHOLD),
  minimum_pixels(DEF_MINIMUM),
  min_distance(DEF_MIN_DISTANCE),
  show(false)
{
  // create the new inlet for the messages
  m_hits = outlet_new(this->x_obj, 0);
  // init the array of active hit_areas
  for (int i=0; i<NUM_hit_areas; i++)
    {
      area_active[i]=0;
      buffer[i]=0;
    }
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_hit :: ~pix_hit()
{
  outlet_free(m_hits);
}

unsigned char pix_hit :: getGreyValue(GLenum format, unsigned char *data)
{
  // is this a gray8 or RGBA?
  switch(format)
    {
      // Gray scale
    case(GL_LUMINANCE):
      return data[chGray];
      break;
      // YUV
    case(GL_YCBCR_422_GEM):
      return data[chY1];
      break;
      // RGB, RGBA
    case(GL_RGB): case (GL_RGBA):
      return (data[chRed]*RGB2GRAY_RED+data[chGreen]*RGB2GRAY_GREEN+data[chBlue]*RGB2GRAY_BLUE)>>8;
      break;
    default :
      error("GEM: pix_data: unknown image format");
      return 0;
      break;
    }
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_hit :: processImage(imageStruct &image)
{
  /* for each active rectangle test every pixel inside
     and if the number of pixles with grey value is > minimum_threshold  
     is > minimum_pixles send this rectangle id to the outlet 
     which means there has been a hit on that rectangle */
	
  unsigned char *src=image.data;
  unsigned char data[3];
  int pixels_found;
  bool go_on;

  for (int i=0; i<NUM_hit_areas; i++) {
    if (area_active[i]) {
      switch (hit_areas[i].type)  {
      case rectangle:
        {
          int xPos = static_cast<int>(hit_areas[i].x * static_cast<float>(image.xsize));
          int yPos = static_cast<int>(hit_areas[i].y * static_cast<float>(image.ysize));
	  int Width = static_cast<int>(hit_areas[i].width * static_cast<float>(image.xsize));
          int Height = static_cast<int>(hit_areas[i].height * static_cast<float>(image.ysize));
					
          go_on=true;
          bool hit=false;
          pixels_found=0;

          // for each line in this rectangle..
          for (int this_y = yPos; go_on && (this_y < yPos+Height) && (this_y<image.ysize); this_y++) {
            // for each pixle in this line
            for (int this_x = xPos; go_on && (this_x < xPos + Width) && (this_x < image.xsize); this_x++) {
              // get this pixel
              int position = (this_y * image.xsize + this_x) * image.csize;
              data[0] = src[position];
              data[1] = src[position+1];
              data[2] = src[position+2];
              if (show) {
                // src[position]=src[position+1]=src[position+2] = 0;
                src[position+0]=~src[position+0];
                src[position+1]=~src[position+1];
                src[position+2]=~src[position+2];
              }
              // get grey val
              if(getGreyValue(image.format, data) >= minimum_threshold) {
                pixels_found++;
                if (pixels_found>=minimum_pixels) {
                  // hit!
                  hit=true;
                  // exit the loop..
                  go_on=false;
                }
              }
            }
          }
          if (hit) {
            if (buffer[i]==0) {
              buffer[i]=1;
              // TODO: build list with |i 1(
              t_atom atom[2];
              SETFLOAT(&atom[0], static_cast<t_float>(i));
              SETFLOAT(&atom[1], static_cast<t_float>(1));
              outlet_list(m_hits, gensym("list"), 2, atom);
            }
          } else {
            if (buffer[i]==1) {
              buffer[i]=0;
              // TODO: build list with |i 0(
              t_atom atom[2];
              SETFLOAT(&atom[0], static_cast<t_float>(i));
              SETFLOAT(&atom[1], static_cast<t_float>(0));
              outlet_list(m_hits, gensym("list"), 2, atom);
            }
          }
          break;
        }
      case circle:
        //todo
        break;
      case line:
        {
          // i need to get the line equation f(x)
          // then 
          // for each x bewteen x1 and x2
          // get y=f(x)
          // and do the hit test in x,y
          // if grayValue > threshold
          // HIT!

          /*
            equation of the line passing from p1 and p2:
            (y-y1)/(y2-y1)=(x-x1)/(x2-x1)
            so 
            y = (x-x1)(y2-y1)/(x2-x1) + y1
            and 
            x = (y-y1(x2-x1))/(y2-y1) - x1
          */
          int x1 = static_cast<int>(hit_areas[i].x * static_cast<float>(image.xsize));
          int y1 = static_cast<int>(hit_areas[i].y * static_cast<float>(image.ysize));
          int x2 = static_cast<int>(hit_areas[i].width * static_cast<float>(image.xsize));
          int y2 = static_cast<int>(hit_areas[i].height * static_cast<float>(image.ysize));					
					
          int diffx = abs(x2-x1);
          int diffy = abs(y2-y1);

          int counter=0;

          if ((diffx==diffy)&&(diffx==0))
            break;//faulty line
          if (diffx>diffy) {
            // x leads the game
            // y = (x-x1)(y2-y1)/(x2-x1) + y1
            int incr = 1;
            if ((x2-x1)<0)
              incr = -1;
            int x=x1; 
            while (x!=x2) {
              // NB diffx can't be == 0
              // (i would not be here)
              int y = (x-x1)*(y2-y1)/(x2-x1) + y1;
              int position = y * image.xsize * image.csize +
                x * image.csize;
              data[0] = src[position];
              data[1] = src[position+1];
              data[2] = src[position+2];
              if (show){
                // src[position]=src[position+1]=src[position+2] = 0;
                src[position+0]=~src[position+0];
                src[position+1]=~src[position+1];
                src[position+2]=~src[position+2];
              }
              if(getGreyValue(image.format, data) >= minimum_threshold) {
                float where = (static_cast<float>(counter)/static_cast<float>(diffx));
                // hit!
                if (fabs(buffer[i] - where) > min_distance) {
									
                  t_atom atom[2];
                  SETFLOAT(&atom[0], static_cast<t_float>(i));
                  SETFLOAT(&atom[1], static_cast<t_float>(where));
                  outlet_list(m_hits, gensym("list"), 2, atom);
                  buffer[i]=where;
                }
              } 
              counter++;
              x+=incr;
            }
          } else {
            // y rulez
            // x = (y-y1(x2-x1))/(y2-y1) + x1
            int incr = 1;
            if ((y2-y1)<0)
              incr = -1;
            int y=y1; 
            while (y!=y2) {
              // NB diffy can't be == 0
              // (i would not be here)
              int x = (y-y1)*(x2-x1)/(y2-y1) + x1;
              int position = y * image.xsize * image.csize +
                x * image.csize;
              data[0] = src[position];
              data[1] = src[position+1];
              data[2] = src[position+2];
              if (show){
                // src[position]=src[position+1]=src[position+2] = 0;
                src[position+0]=~src[position+0];
                src[position+1]=~src[position+1];
                src[position+2]=~src[position+2];
              }
              if(getGreyValue(image.format, data) >= minimum_threshold) {
                float where = (static_cast<float>(counter)/static_cast<float>(diffy));
                // hit!
                if (fabs(buffer[i] - where) > min_distance) {
                  t_atom atom[2];
                  SETFLOAT(&atom[0], static_cast<t_float>(i));
                  SETFLOAT(&atom[1], static_cast<t_float>(where));
                  outlet_list(m_hits, gensym("list"), 2, atom);
                  buffer[i]=where;
                }
              } 
              counter++;
              y+=incr;
            }
          }
          break;
        }
      }
    }
  }
}

void pix_hit :: threshold(float thresh)
{
  if ((thresh<0) || (thresh>1))
    {
      error("threshold must be a float between 0 and 1");
      return;
    }
  minimum_threshold = (unsigned char)(thresh*255);
}

void pix_hit :: minimum(int min)
{
  if (min<=0)
    {
      error("min must be an integer > than 0 !");
      return;
    }
  minimum_pixels = min;
}

void pix_hit :: set_min_distance(float min)
{
  if ((min<0)||(min>1))
    {
      error("min_distance must be a float between 0 and 1!");
      return;
    }
  min_distance = min;
}

void pix_hit :: set_show(int val)
{
  show=(val>0);
}

void pix_hit :: createRectangle(int n, float x, float y, float w, float h)
{
  if ((n<0) || (n>NUM_hit_areas-1))
    {
      error("the id must be an integer between 0 and %i",NUM_hit_areas);
      return;
    }
  if ((x<0) || (x>1) || (y<0) || (y>1) || (w<0) || (w>1) || (h<0) || (h>1))
    {
      error("invalid input coordinates");
      return;
    }
  hit_areas[n].type = rectangle;
  hit_areas[n].x = x;
  hit_areas[n].y = y;
  hit_areas[n].width = w;
  hit_areas[n].height = h;
  area_active[n]=true;
}

void pix_hit :: createCircle(int n, float x, float y, float r)
{
  if ((n<0) || (n>NUM_hit_areas-1))
    {
      error("the id must be an integer between 0 and %i",NUM_hit_areas);
      return;
    }
  if ((x<0) || (x>1) || (y<0) || (y>1) || (r<0) || (r>1))
    {
      error("invalid input coordinates");
      return;
    }
  hit_areas[n].x = x;
  hit_areas[n].y = y;
  hit_areas[n].width = r;
  hit_areas[n].type = circle;
  area_active[n]=true;

  post("WARNING: circles not implemented.. yet.");
}

void pix_hit :: createLine(int n, float x1, float y1, float x2, float y2)
{
  if ((n<0) || (n>NUM_hit_areas-1))
    {
      error("the id must be an integer between 0 and %i",NUM_hit_areas);
      return;
    }
  if ((x1<0) || (x1>1) || (y1<0) || (y1>1) || (x2<0) || (x2>1) || (y2<0) || (y2>1))
    {
      error("invalid input coordinates");
      return;
    }
  hit_areas[n].type = line;
  hit_areas[n].x = x1;
  hit_areas[n].y = y1;
  hit_areas[n].width = x2;
  hit_areas[n].height = y2;
  area_active[n]=true;

}
void pix_hit :: move(int n, float x, float y, float w, float h)
{
  if ((n<0) || (n>NUM_hit_areas-1))
    {
      error("the id must be an integer between 0 and %i", NUM_hit_areas);
      return;
    }
  hit_areas[n].x = x;
  hit_areas[n].y = y;
  hit_areas[n].width = w;
  hit_areas[n].height = h;
  area_active[n]=true;
}

void pix_hit :: del(int n)
{
  if ((n<0) || (n>NUM_hit_areas-1))
    {
      error("the id must be an integer between 0 and %i",NUM_hit_areas);
      return;
    }
  hit_areas[n].x = 0;
  hit_areas[n].y = 0;
  hit_areas[n].width = 0;
  hit_areas[n].height = 0;
  area_active[n]=false;
  post("deleted area %i", n);
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_hit :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_hit::thresholdCallback),
                  gensym("threshold"), A_FLOAT, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_hit::minimumCallback),
                  gensym("min"), A_FLOAT, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_hit::min_distanceCallback),
                  gensym("min_distance"), A_FLOAT, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_hit::createRectangleCallback),
                  gensym("rectangle"), A_GIMME, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_hit::createCircleCallback),
                  gensym("circle"), A_GIMME, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_hit::createLineCallback),
                  gensym("line"), A_GIMME, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_hit::moveCallback),
                  gensym("move"), A_GIMME, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_hit::deleteCallback),
                  gensym("delete"), A_FLOAT, A_NULL); 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_hit::showCallback),
                  gensym("show"), A_FLOAT, A_NULL); }

void pix_hit :: thresholdCallback(void *data, t_floatarg val)
{
  GetMyClass(data)->threshold(FLOAT_CLAMP(val));
}

void pix_hit :: minimumCallback(void *data, t_floatarg val)
{
  int n = static_cast<int>(FLOAT_CLAMP(val));
  GetMyClass(data)->minimum(n);
}

void pix_hit :: min_distanceCallback(void *data, t_floatarg val)
{
  GetMyClass(data)->set_min_distance(FLOAT_CLAMP(val));
}

void pix_hit :: createLineCallback(void *data, t_symbol *sl, int argc, t_atom *argv)
{
  int n;
  float x,y,w,h;
  // check for correct number of floats...
  n = atom_getint(argv++);
  x = atom_getfloat(argv++);
  y = atom_getfloat(argv++);
  w = atom_getfloat(argv++);
  h = atom_getfloat(argv++);

  GetMyClass(data)->createLine(n,x,y,w,h);
}

void pix_hit :: createRectangleCallback(void *data, t_symbol *sl, int argc, t_atom *argv)
{
  int n;
  float x,y,w,h;
  // check for correct number of floats...
  n = atom_getint(argv++);
  x = atom_getfloat(argv++);
  y = atom_getfloat(argv++);
  w = atom_getfloat(argv++);
  h = atom_getfloat(argv++);

  GetMyClass(data)->createRectangle(n,x,y,w,h);
}

void pix_hit :: createCircleCallback(void *data, t_symbol *sl, int argc, t_atom *argv)
{
  int n;
  float x,y,r;
  // check for correct number of floats...
  n = atom_getint(argv++);
  x = atom_getfloat(argv++);
  y = atom_getfloat(argv++);
  r = atom_getfloat(argv++);

  GetMyClass(data)->createCircle(n,x,y,r);
}

void pix_hit :: moveCallback(void *data, t_symbol *sl, int argc, t_atom *argv)
{
  int n;
  float x,y,w,h;
  // check for correct number of floats...
  n = atom_getint(argv++);
  x = atom_getfloat(argv++);
  y = atom_getfloat(argv++);
  w = atom_getfloat(argv++);
  h = atom_getfloat(argv++);

  GetMyClass(data)->move(n,x,y,w,h);
}

void pix_hit :: deleteCallback(void *data, t_floatarg id)
{
  GetMyClass(data)->del(static_cast<int>(id));
}

void pix_hit :: showCallback(void *data, t_floatarg val)
{
  GetMyClass(data)->set_show(static_cast<int>(val));
}

