////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "part_info.h"

#include <string.h>


#include "papi.h"

CPPEXTERN_NEW(part_info);

/////////////////////////////////////////////////////////
//
// part_info
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
part_info :: part_info()
{
  m_number=1000;
  m_pos    = new float[m_number*3];
  m_colors = new float[m_number*4];
  m_velo   = new float[m_number*3];
  m_sizes  = new float[m_number*3];
  m_ages   = new float[m_number];

  out_num = outlet_new(this->x_obj, 0);
  out_pos = outlet_new(this->x_obj, 0);
  out_col = outlet_new(this->x_obj, 0);
  out_vel = outlet_new(this->x_obj, 0);
  out_siz = outlet_new(this->x_obj, 0);
  out_age = outlet_new(this->x_obj, 0);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
part_info :: ~part_info()
{ 
  outlet_free(out_num);
  outlet_free(out_pos);
  outlet_free(out_col);
  outlet_free(out_vel);
  outlet_free(out_siz);
  outlet_free(out_age);
  if(m_pos)delete[]m_pos;
  if(m_colors)delete[]m_colors;
  if(m_velo)delete[]m_velo;
  if(m_sizes)delete[]m_sizes;
  if(m_ages)delete[]m_ages;
}

/////////////////////////////////////////////////////////
// renderParticles
//
/////////////////////////////////////////////////////////
void part_info :: renderParticles(GemState *state)
{
  if (m_tickTime > 0.f)    {
    pMove();
  }
  //	pDrawGroupp();
  int cnt = pGetGroupCount();
  if(cnt < 1)return;
  if (cnt>m_number){
    if(m_colors)delete[]m_colors;
    if(m_sizes )delete[]m_sizes;
    if(m_pos   )delete[]m_pos;
    if(m_velo  )delete[]m_velo;
    if(m_ages  )delete[]m_ages;
    m_number = cnt;
    m_pos    = new float[m_number * 3];
    m_colors = new float[m_number * 4];
    m_velo   = new float[m_number * 3];
    m_sizes  = new float[m_number * 3];
    m_ages   = new float[m_number];
  }
  float *position = m_pos;
  float *color    = m_colors;
  float *velo     = m_velo;
  float *size     = m_sizes;
  float *age      = m_ages;
  pGetParticles(0, cnt, position, color, velo, size, age);
  for(int i = 0; i < cnt; i++)	{
    SETFLOAT(m_alist+0, position[0]);
    SETFLOAT(m_alist+1, position[1]);
    SETFLOAT(m_alist+2, position[2]);
    position+=3;
    SETFLOAT(m_alist+3, color[0]);
    SETFLOAT(m_alist+4, color[1]);
    SETFLOAT(m_alist+5, color[2]);
    SETFLOAT(m_alist+6, color[3]);
    color+=4;
    SETFLOAT(m_alist+7, velo[0]);
    SETFLOAT(m_alist+8, velo[1]);
    SETFLOAT(m_alist+9, velo[2]);
    velo+=3;
    SETFLOAT(m_alist+10, size[0]);
    SETFLOAT(m_alist+11, size[1]);
    SETFLOAT(m_alist+12, size[2]);
    size+=3;

    outlet_float(out_age, age[i]);
    outlet_list (out_siz, &s_list, 3, m_alist+10);
    outlet_list (out_vel, &s_list, 3, m_alist+7);
    outlet_list (out_col, &s_list, 4, m_alist+3);
    outlet_list (out_pos, &s_list, 3, m_alist+0);
    outlet_float(out_num, i);
    continueRender(state);
  }
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void part_info :: obj_setupCallback(t_class *classPtr)
{}
