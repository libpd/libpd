////////////////////////////////////////////////////////
//
// pix_mano - an object to track a hand and its fingers
//
// Designed to work with the MANO Controller
// www.jaimeoliver.pe/mano
// 
// Jaime Oliver
// jaime.oliver2@gmail.com
//
// this is still a testing version, no guarantees...
// rev: 23-sep-2010
//
// the license for this object is GNU 
//
// for more information: www.jaimeoliver.pe
// Silent Percussion Project
//
// GEM - Graphics Environment for Multimedia
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "LICENSE.txt" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_mano.h"

CPPEXTERN_NEW(pix_mano)

  class pix_mano::PIMPL {
  public:
    struct tipData {
    public:
      unsigned int size;

      int*t_t,*v_t;

      float*tip_coord_x, *tip_coord_y, *valley_coord_x, *valley_coord_y;
 
      // index x y magnitude angle state:
      float*t_i, *t_x, *t_y, *t_m, *t_a, *t_s;
      // index x y magnitude angle state:
      float*v_i, *v_x, *v_y, *v_m, *v_a, *v_s;
	
      float*tn_i, *tn_x, *tn_y, *tn_m, *tn_a, *tn_s; 
      float*free_i;

      tipData(unsigned int size_) : 
        size(size_),
        t_t(new int[size_]), v_t(new int[size_]), 
        tip_coord_x(new float[size_]), tip_coord_y(new float[size_]), valley_coord_x(new float[size_]), valley_coord_y(new float[size_]), 
        t_i(new float[size_]), t_x(new float[size_]), t_y(new float[size_]), t_m(new float[size_]), t_a(new float[size_]), t_s(new float[size_]), 
        v_i(new float[size_]), v_x(new float[size_]), v_y(new float[size_]), v_m(new float[size_]), v_a(new float[size_]), v_s(new float[size_]), 
        tn_i(new float[size_]), tn_x(new float[size_]), tn_y(new float[size_]), tn_m(new float[size_]), tn_a(new float[size_]), tn_s(new float[size_]), 
        free_i(new float[size_])
      {

      }
      ~tipData(void) {
        size=0;
        delete[]t_t; delete[]v_t;
        delete[]tip_coord_x; delete[]tip_coord_y; delete[]valley_coord_x; delete[]valley_coord_y;
        delete[]t_i; delete[]t_x; delete[]t_y; delete[]t_m; delete[]t_a; delete[]t_s;
        delete[]v_i; delete[]v_x; delete[]v_y; delete[]v_m; delete[]v_a; delete[]v_s;
        delete[]tn_i; delete[]tn_x; delete[]tn_y; delete[]tn_m; delete[]tn_a; delete[]tn_s;
        delete[]free_i;
      }

    };

    class enteringPoint {
    public:
      unsigned int size;
      int *e_x1, *e_y1, *e_x2, *e_y2, *e_ctrx, *e_ctry, *e_no, *e_size, *e_state, *e_orient;
  
      enteringPoint(unsigned int size_) : 
        size(size_),
        e_x1(new int[size_]), 
        e_y1(new int[size_]), 
        e_x2(new int[size_]), 
        e_y2(new int[size_]), 
        e_ctrx(new int[size_]), 
        e_ctry(new int[size_]), 
        e_no(new int[size_]), 
        e_size(new int[size_]), 
        e_state(new int[size_]), 
        e_orient(new int[size_])
      { }
      ~enteringPoint(void) {
        size=0;
        delete[]e_x1; delete[]e_y1; 
        delete[]e_x2; delete[]e_y2; 
        delete[]e_ctrx; delete[]e_ctry; 
        delete[]e_no; 
        delete[]e_size; 
        delete[]e_state; 
        delete[]e_orient;
      }
    };
    class delayXY {
    public:
      unsigned int size;
      int *d_x, *d_y;
  
      delayXY(unsigned int size_) : 
        size(size_),
        d_x(new int[size_]), 
        d_y(new int[size_])
      { }
      ~delayXY(void) {
        size=0;
        delete[]d_x;
        delete[]d_y;
      }
    };

    tipData*p_tipData;
    enteringPoint*p_enteringPoint;
    delayXY*p_delayXY;

    int dummy;

    PIMPL(unsigned int tips=128, unsigned int points=4096, unsigned int delays=20) :
      p_tipData(new tipData(tips)),
      p_enteringPoint(new enteringPoint(points)),
      p_delayXY(new delayXY(delays)),
      dummy(0)
      {    }
    ~PIMPL(void) {
      if(p_tipData)delete p_tipData; p_tipData=NULL;
      if(p_enteringPoint)delete p_enteringPoint; p_enteringPoint=NULL;
      if(p_delayXY)delete p_delayXY; p_delayXY=NULL;
    }
  };


// #####################################################
//
// pix_mano
//
// #####################################################
// Constructor
// #####################################################
pix_mano :: pix_mano() :
  m_pimpl(new PIMPL())
{
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"),	gensym("moth"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"),	gensym("vec_thresh"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("param"));
  outlet1 = outlet_new(this->x_obj, gensym("list"));
  outlet2 = outlet_new(this->x_obj, gensym("list"));
  outlet3 = outlet_new(this->x_obj, gensym("list"));
  outlet4 = outlet_new(this->x_obj, gensym("list"));
  outlet5 = outlet_new(this->x_obj, gensym("list"));
  outlet6 = outlet_new(this->x_obj, gensym("list"));
  thresh = 0;
  tip_scalar = 0.4;
  head = mode = 0;
  bottom = 240;
  min_entry_size = 10;
  min_perim = 50;
  pixsamp = 4;
  pixavg = 20;
  int i;
  for (i=0; i<50; i++) {
    tp_i[i] = i;
    tp_x[i] = tp_y[i] = tp_m[i] = tp_a[i] = tp_s[i] = 0;
  }
  prev_tip = 0;
}

// #####################################################
// Destructor
// #####################################################
pix_mano :: ~pix_mano()
{ 
  if(m_pimpl)
    delete m_pimpl;
  m_pimpl=NULL;
}

// #####################################################
// processGrayImage
// #####################################################
void pix_mano :: processGrayImage(imageStruct &image)	{

  int xsize = image.xsize;
  int ysize = image.ysize;
  unsigned char *base = image.data;
  int screen_perimeter = xsize*2 + ysize*2;
  int xcount, ycount, xcoord, ycoord, ycoordm1, ycoordm2;
  int i, c, j, n, valley, edge, start=0, tempint2=0, tempint=0, temp_x, temp_y;
  int contourx[50000], contoury[50000], nindex[50000]; 
  float  partialx[10000], partialy[10000], xn, xnm1, yn, ynm1;
  double angle[10000], tempangle, xval, yval, maxangle, minangle;
  t_atom ap[5], as[6], at[7], ar[5], aq[18]; // outlets
  int pix12, pix21, pix22, pix23, pix32, difx, dify;
  int prevx, prevy, linecheck; 
  // entering point
  int cx, cy, t, foundhand=0; 
  int*enterx=new int[xsize];
  int*entery=new int[ysize];
  // hand center, area
  int hx, hy, xval_int, yval_int;		
  float hand_area;
  //fingertips
  double max_tip, min_tip;
  unsigned int tip, maxtips=100;
  bool tiptest=false;
  float tipx, tipy, tip_mag_ctr;

  if(m_pimpl->p_tipData->size < maxtips) {
    delete m_pimpl->p_tipData;
    m_pimpl->p_tipData=NULL;
  }

  if(!m_pimpl->p_tipData)
    m_pimpl->p_tipData=new PIMPL::tipData(maxtips);

  int mt = 20; // maximum number of tips
  int totalcycles=0;

  //	float tip_coord_x[maxtips], tip_coord_y[maxtips], valley_coord_x[maxtips], valley_coord_y[maxtips];

  int*t_t=m_pimpl->p_tipData->t_t;
  int*v_t=m_pimpl->p_tipData->v_t;

  float*tip_coord_x=m_pimpl->p_tipData->tip_coord_x;
  float*tip_coord_y=m_pimpl->p_tipData->tip_coord_y;
  float*valley_coord_x=m_pimpl->p_tipData->valley_coord_x;
  float*valley_coord_y=m_pimpl->p_tipData->valley_coord_y;
    
  // index x y magnitude angle state:
  float*t_i=m_pimpl->p_tipData->t_i;
  float*t_x=m_pimpl->p_tipData->t_x;
  float*t_y=m_pimpl->p_tipData->t_y;
  float*t_m=m_pimpl->p_tipData->t_m;
  float*t_a=m_pimpl->p_tipData->t_a;
  float*t_s=m_pimpl->p_tipData->t_s;
  // index x y magnitude angle state:
  float*v_i=m_pimpl->p_tipData->v_i;
  float*v_x=m_pimpl->p_tipData->v_x;
  float*v_y=m_pimpl->p_tipData->v_y;
  float*v_m=m_pimpl->p_tipData->v_m;
  float*v_a=m_pimpl->p_tipData->v_a;
  float*v_s=m_pimpl->p_tipData->v_s;
	
  float*tn_i=m_pimpl->p_tipData->tn_i;
  float*tn_x=m_pimpl->p_tipData->tn_x;
  float*tn_y=m_pimpl->p_tipData->tn_y;
  float*tn_m=m_pimpl->p_tipData->tn_m;
  float*tn_a=m_pimpl->p_tipData->tn_a;
  float*tn_s=m_pimpl->p_tipData->tn_s;
  float*free_i=m_pimpl->p_tipData->free_i;

  int tot_area = xsize * ysize;
  int perimeter = xsize * 2 + ysize * 2;
  int h;
  int*hand_areax=new int[xsize];
  int*hand_areay=new int[ysize];

  // Limits for search	
  if (left < 1) left = 1;				if (head < 1) head = 1;
  if (right > xsize - 1) right = xsize - 1;	if (bottom > ysize - 1) bottom = ysize - 1;
	
  if (mode == 0 || mode == 1 || mode == 2) {
    h = t = 0;
    // ****** CLEAR HISTOGRAMS
    for (i = 0; i < ysize; i++)		{hand_areay[i] = 0; entery[i] = 0; }
    for (i = 0; i < xsize; i++)		{hand_areax[i] = 0; enterx[i] = 0; }

    // IMAGE TREATMENT, border, thresh and edge detection

    // ****************** make BLACK BORDER....
    for (ycount = head; ycount <= bottom; ycount++) {
      ycoord = image.csize * xsize * ycount;
      if ( (ycount <= (head + 3)) || (ycount >= (bottom - 3)) ) {
        for (xcount = left; xcount <= right; xcount++) {
          xcoord = image.csize * xcount + ycoord;
          base[chGray + xcoord] =		0;
        }
      }
      else {
        for (xcount = left; xcount <= left + 3; xcount++) {
          xcoord = image.csize * xcount + ycoord;
          base[chGray + xcoord] =		0;
        }
        for (xcount = right - 3; xcount <= right; xcount++) {
          xcoord = image.csize * xcount + ycoord;
          base[chGray + xcoord] =		0;
        }
      }
    }
    // ****************** RASTER		
    for (ycount = head; ycount <= bottom; ycount++) {
      ycoord		= image.csize * xsize * ycount;
      ycoordm1	= image.csize * xsize * (ycount - 1);
      ycoordm2	= image.csize * xsize * (ycount - 2);  // only specifying coords for further calculations
      for (xcount = left; xcount <= right; xcount++) {
        xcoord  	= image.csize * xcount + ycoord;
        // THRESH the IMAGE
        if (base[chGray + xcoord] < thresh)		base[chGray + xcoord] =		0; 
        else {
          base[chGray + xcoord] =	    80;
          hand_areax[xcount] += 1;
          hand_areay[ycount] += 1;
          h++;
        }
        // prevent calculations outside area			
        if (	(xcount > (left	  + 2))	&& 
              (xcount < (right  - 1))	&& 
              (ycount > (head	  + 2))	&& 
              (ycount < (bottom - 2))		)	{
          // matrix, EDGE. 
          pix12	=	base[chGray + image.csize * (xcount - 2) + ycoordm2];	
          pix21	=	base[chGray + image.csize * (xcount - 3) + ycoordm1];
          pix22	=	base[chGray + image.csize * (xcount - 2) + ycoordm1];
          pix23	=	base[chGray + image.csize * (xcount - 1) + ycoordm1];	
          pix32	=	base[chGray + image.csize * (xcount - 2) + ycoord  ];	
			
          edge = ((pix12 * -1) + (pix21 * -1) + (pix22 * 4) + (pix23 * -1) + (pix32 * -1));
          if (edge >= 80)	edge = 80;
          else		edge =  0;
          base[chGray + image.csize * (xcount - 2) + ycoordm2] =		edge;
        }
      }
    }


    /// HAND LOGIC, find hands/ or entering points and their attributes...

      int n_hand, e;
      n_hand = 0;
      e = 1;

      foundhand = 0;
      totalcycles = 0;

      while (e > 0) { 

        // ************** Find and Check which one is the biggest entering point in the borders
        i = screen_perimeter / 2;

        //        int e_x1[i], e_y1[i], e_x2[i], e_y2[i], e_ctrx[i], e_ctry[i], e_no[i], e_size[i], e_state[i], e_orient[i];
        if(m_pimpl->p_enteringPoint->size < i) {
          delete m_pimpl->p_enteringPoint;
          m_pimpl->p_enteringPoint=NULL;
        }

        if(!m_pimpl->p_enteringPoint)
          m_pimpl->p_enteringPoint=new PIMPL::enteringPoint(i);

        int*e_x1=m_pimpl->p_enteringPoint->e_x1; 
        int*e_y1=m_pimpl->p_enteringPoint->e_y1; 
        int*e_x2=m_pimpl->p_enteringPoint->e_x2; 
        int*e_y2=m_pimpl->p_enteringPoint->e_y2; 
        int*e_ctrx=m_pimpl->p_enteringPoint->e_ctrx; 
        int*e_ctry=m_pimpl->p_enteringPoint->e_ctry; 
        int*e_no=m_pimpl->p_enteringPoint->e_no; 
        int*e_size=m_pimpl->p_enteringPoint->e_size; 
        int*e_state=m_pimpl->p_enteringPoint->e_state; 
        int*e_orient=m_pimpl->p_enteringPoint->e_orient;

        e = i = 0;
        //left border
        xcount	= left + 4;
        for (ycount = head + 3; ycount <= bottom - 4; ycount++) {	
          ycoord		= image.csize * xsize * ycount;
          xcoord  	= image.csize * xcount + ycoord;
          if (base[chGray + xcoord] == 80) {
            tempint = 1;
            if (i == 0) { 
              e_x1[e] = xcount;						
              e_y1[e] = ycount;
              e_orient[e] = 0;
            }
            i++;
          }	
          else {
            if (tempint == 1 && i > min_entry_size) {
              tempint = 0;
              e_x2[e] = xcount;						
              e_y2[e] = ycount - 1;
              e_ctrx[e] = (e_x1[e] + e_x2[e]) / 2;	
              e_ctry[e] = (e_y1[e] + e_y2[e]) / 2;
              e_state[e] = 1;
              e_size[e] = (int) sqrtf((float)(((e_x1[e]-e_x2[e])*(e_x1[e]-e_x2[e]))+((e_y1[e]-e_y2[e])*(e_y1[e]-e_y2[e]))));
              i = 0;
              e_no[e] = e;
              e++;
              foundhand = 1;
            }
            else {
              tempint = 0;
              i = 0;
            }
          }
        }// bottom border
        ycount		= bottom - 4;
        ycoord		= image.csize * xsize * ycount;
        for (xcount = left + 4; xcount <= right - 4; xcount++) {	
          xcoord  	= image.csize * xcount + ycoord;
          if (base[chGray + xcoord] == 80) {
            tempint = 1;
            if (i == 0) { 
              e_x1[e] = xcount;						
              e_y1[e] = ycount;
              e_orient[e] = 1;
            }
            i++;
          }	
          else {
            if (tempint == 1 && i > min_entry_size) {
              tempint = 0;
              e_x2[e] = xcount - 1;					
              e_y2[e] = ycount;
              e_ctrx[e] = (e_x1[e] + e_x2[e]) / 2;	
              e_ctry[e] = (e_y1[e] + e_y2[e]) / 2;
              e_state[e] = 1;
              e_size[e] = (int) sqrtf((float)(((e_x1[e]-e_x2[e])*(e_x1[e]-e_x2[e]))+((e_y1[e]-e_y2[e])*(e_y1[e]-e_y2[e]))));
              i = 0;
              e_no[e] = e;
              e++;
              foundhand = 1;
            }
            else {
              tempint = 0;
              i = 0;
            }
          }
        }// right border
        xcount		= right - 4;
        for (ycount = bottom - 4; ycount >= head + 3; ycount--) {	
          ycoord		= image.csize * xsize * ycount;
          xcoord  	= image.csize * xcount + ycoord;
          if (base[chGray + xcoord] == 80) {
            tempint = 1;
            if (i == 0) { 
              e_x1[e] = xcount;						
              e_y1[e] = ycount;
              e_orient[e] = 2;
            }
            i++;
          }	
          else {
            if (tempint == 1 && i > min_entry_size) {
              tempint = 0;
              e_x2[e] = xcount;						
              e_y2[e] = ycount + 1;
              e_ctrx[e] = (e_x1[e] + e_x2[e]) / 2;	
              e_ctry[e] = (e_y1[e] + e_y2[e]) / 2;
              e_state[e] = 1;
              e_size[e] = (int) sqrtf((float)(((e_x1[e]-e_x2[e])*(e_x1[e]-e_x2[e]))+((e_y1[e]-e_y2[e])*(e_y1[e]-e_y2[e]))));
              i = 0;
              e_no[e] = e;
              e++;
              foundhand = 1;
            }
            else {
              tempint = 0;
              i = 0;
            }
          }
        }// top border
        ycount		= head + 3;
        ycoord		= image.csize * xsize * ycount;
        for (xcount = right - 4; xcount >= left + 4; xcount--) {	
          xcoord  	= image.csize * xcount + ycoord;
          if (base[chGray + xcoord] == 80) {
            tempint = 1;
            if (i == 0) { 
              e_x1[e] = xcount;						
              e_y1[e] = ycount;
              e_orient[e] = 3;
            }
            i++;
          }	
          else {
            if (tempint == 1 && i > min_entry_size) {
              tempint = 0;
              e_x2[e] = xcount + 1;					
              e_y2[e] = ycount;
              e_ctrx[e] = (e_x1[e] + e_x2[e]) / 2;	
              e_ctry[e] = (e_y1[e] + e_y2[e]) / 2;
              e_state[e] = 1;
              e_size[e] = (int) sqrtf((float)(((e_x1[e]-e_x2[e])*(e_x1[e]-e_x2[e]))+((e_y1[e]-e_y2[e])*(e_y1[e]-e_y2[e]))));
              i = 0;
              e_no[e] = e;
              e++;
              foundhand = 1;
            }
            else {
              tempint = 0;
              i = 0;
            }
          }
        }
        if (tempint == 1) {	// if the end of the top is the same as the begginning in the left...	
          if ( (e_x1[0] == left + 4) && (e_y1[0] == head + 3) ) {
            tempint = 0;
            e_x1[0] = e_x1[e];						
            e_y1[0] = e_y1[e];
            e_ctrx[e] = (e_x1[e] + e_x2[e]) / 2;	
            e_ctry[e] = (e_y1[e] + e_y2[e]) / 2;
            e_size[0] += i;
            e_orient[0] = 3;
          }
          else {
            if ( i > min_entry_size) {
              tempint = 0;
              e_x2[e] = xcount;						
              e_y2[e] = ycount;
              e_ctrx[e] = (e_x1[e] + e_x2[e]) / 2;	
              e_ctry[e] = (e_y1[e] + e_y2[e]) / 2;
              e_state[e] = 1;
              e_size[e] = (int) sqrtf((float)(((e_x1[e]-e_x2[e])*(e_x1[e]-e_x2[e]))+((e_y1[e]-e_y2[e])*(e_y1[e]-e_y2[e]))));
              i = 0;
              e_no[e] = e;
              e++;
              foundhand = 1;
            }
            else {
              tempint = 0;
              i = 0;
            }
          }
        }
        if (e == 0) break; 
        if (mode == 2) post("Entries = %d", e);

        // ************** Finds POINTS and Create arrays with the coordinates of the CONTOUR
        // Find biggest enetering point and center
        tempint = tempint2 = c = 0;
        for (i = 0; i < e; i++) {
          if (e_size[i] > tempint) {
            tempint  = e_size[i];
            tempint2 = e_no[i];
          }
        }
        e_state[tempint2] = 0;
        // ******* find CENTRAL ENTERING POINT
        cx = e_ctrx[tempint2];
        cy = e_ctry[tempint2];
        // ************** Finds POINTS and Create arrays with the coordinates of the CONTOUR
        // (1) make first point it's e_x1, e_y1 coordinates...
        xcount = e_x1[tempint2];
        ycount = e_y1[tempint2];
        ycoord = image.csize * xsize * ycount;
        xcoord = image.csize * xcount + ycoord;
        base[chGray + xcoord] = 255;
        contourx[c] = xcount; //record in contour
        contoury[c] = ycount;
        if (mode ==2) if (n_hand == 0) { SETFLOAT(&ap[0], c);	SETFLOAT(&ap[1], contourx[c]);	SETFLOAT(&ap[2], contoury[c]);
          outlet_list(outlet1, 0, 3, ap);	
        }
        c++;		
        // (2) Find Second Point
        // if the first one is on the left....
        tempint = 0;
        if	(e_orient[tempint2] == 0) { //check if it starts on the left...
          xcount = contourx[c-1] + 1; // check to the right of previous (first) point
          if (mode ==2) post ("start(%d) x=%d y=%d ////////////////////////////////", n_hand, contourx[c-1], contoury[c-1]);
          for (ycount = (contoury[c-1] - 1); ycount <= bottom - 4; ycount++) { //in a vertical descending line towards the bottom
            ycoord = image.csize * xsize * ycount;	xcoord = image.csize * xcount + ycoord;
            if (mode ==2)post ("problems? x=%d y=%d", xcount, ycount);
            if (base[chGray + xcoord] == 80) { //if there is another point.
              base[chGray + xcoord] = 255; //record it in contour
              contourx[c] = xcount; //record in contour
              contoury[c] = ycount;
              if (mode ==2)post ("found one! x=%d y=%d", xcount, ycount);
              if ( AbsInt(contoury[c-1] - ycount) != 1 ) {
                temp_x = contourx[c-1];
                temp_y = contoury[c-1];						
                contourx[c-1] = xcount - 1; // overwrite first point in contour (in case first point is not adjacent)
                contoury[c-1] = ycount - 1;
                if (mode ==2)post ("overwrite x=%d y=%d", xcount - 1, ycount - 1);
                xcount = temp_x;
                for (ycount = temp_y; ycount < contoury[c-1]; ycount++) { //from old origin to new origin,
                  ycoord = image.csize * xsize * ycount;	xcoord = image.csize * xcount + ycoord;
                  base[chGray + xcoord] = 0; //make them all black
                  if (mode ==2)post ("black them x=%d y=%d", xcount, ycount);
                }
              }
              if (mode ==2)if (n_hand >= 0) { SETFLOAT(&ap[0], c);	SETFLOAT(&ap[1], contourx[c]);	
                SETFLOAT(&ap[2], contoury[c]); SETFLOAT(&ap[3], 1); outlet_list(outlet1, 0, 4, ap);	//spit it out
              }					
              c++;
              break;
            }
          }
        }
        // on the bottom
        else if (e_orient[tempint2] == 1) { //check if it starts on the bottom...
          ycount = contoury[c-1] - 1; // look in the next row
          ycoord = image.csize * xsize * ycount;
          if (mode ==2) post ("start(%d) x=%d y=%d ////////////////////////////////", n_hand, contourx[c-1], contoury[c-1]);
          for (xcount = (contourx[c-1] - 1); xcount <= right - 4; xcount++) { //  on a horizontal line towards the right
            xcoord  = image.csize * xcount + ycoord;
            if (mode ==2)post ("problems? x=%d y=%d", xcount, ycount);
            if (base[chGray + xcoord] == 80) {
              base[chGray + xcoord] = 255;
              contourx[c] = xcount; //record in contour
              contoury[c] = ycount;
              if (mode ==2)post ("found one! x=%d y=%d", xcount, ycount);
              if ( contourx[c-1] != (xcount - 1) && contoury[c-1] != (ycount + 1) ) {
                temp_x = contourx[c-1];
                temp_y = contoury[c-1];						
                contourx[c-1] = xcount - 1; // overwrite first point in contour (in case first point is not adjacent)
                contoury[c-1] = ycount + 1;
                ycount = temp_y;
                if (mode ==2)post ("overwrite x=%d y=%d", xcount, ycount);
                for (xcount = temp_x; xcount < contourx[c-1]; xcount++) { //from old origin to new origin,
                  ycoord = image.csize * xsize * ycount;	xcoord = image.csize * xcount + ycoord;
                  base[chGray + xcoord] = 0; //make them all black
                  if (mode ==2)post ("black them x=%d y=%d", xcount, ycount);
                }
              }
              if (n_hand >= 0) { 
                if (mode ==2) SETFLOAT(&ap[0], c);	SETFLOAT(&ap[1], contourx[c]);	SETFLOAT(&ap[2], contoury[c]);
                outlet_list(outlet1, 0, 3, ap);	//spit it out.
              }
              c++;
              break;
            }
          }
        }
        // right...
        else if	(e_orient[tempint2] == 2) {
          xcount = contourx[c-1] - 1;
          if (mode ==2) post ("start(%d) x=%d y=%d ////////////////////////////////", n_hand, contourx[c-1], contoury[c-1]);
          for (ycount = (contoury[c-1] + 1); ycount >= (head + 3); ycount--) {
            ycoord = image.csize * xsize * ycount;	xcoord = image.csize * xcount + ycoord;
            if (mode ==2)post ("problems? x=%d y=%d", xcount, ycount);
            if (base[chGray + xcoord] == 80) {
              base[chGray + xcoord] = 255;
              contourx[c] = xcount; //record in contour
              contoury[c] = ycount;
              if (mode ==2)post ("found one! x=%d y=%d", xcount, ycount);
              if ( AbsInt(contoury[c-1] - ycount) != 1 ) {
                temp_x = contourx[c-1];
                temp_y = contoury[c-1];						
                contourx[c-1] = xcount + 1; // overwrite first point in contour (in case first point is not adjacent)
                contoury[c-1] = ycount + 1;
                if (mode ==2)post ("overwrite x=%d y=%d", xcount + 1, ycount + 1);
                xcount = temp_x;
                for (ycount = temp_y; ycount > contoury[c-1]; ycount--) { //from old origin to new origin,
                  ycoord = image.csize * xsize * ycount;	xcoord = image.csize * xcount + ycoord;
                  base[chGray + xcoord] = 0; //make them all black
                  if (mode ==2) post ("black them x=%d y=%d", xcount, ycount);
                }
              }
              if (n_hand == 0) 
                if (mode ==2) { SETFLOAT(&ap[0], c);	SETFLOAT(&ap[1], contourx[c]);	SETFLOAT(&ap[2], contoury[c]);
                  outlet_list(outlet1, 0, 3, ap);	
                }					
              c++;
              break;
            }
          }
        }
        // and top...
        else if (e_orient[tempint2] == 3) {
          ycount = contoury[c-1] + 1; // look in the next row
          ycoord = image.csize * xsize * ycount;
          for (xcount = (contourx[c-1] + 1); xcount >= left + 4; xcount--) {
            xcoord  = image.csize * xcount + ycoord;
            if (base[chGray + xcoord] == 80) {
              base[chGray + xcoord] = 255;
              contourx[c] = xcount; //record in contour
              contoury[c] = ycount;
              if ( contourx[c-1] != (xcount + 1) && contoury[c-1] != (ycount - 1) ) {
                temp_x = contourx[c-1];
                temp_y = contoury[c-1];						
                contourx[c-1] = xcount + 1; // overwrite first point in contour (in case first point is not adjacent)
                contoury[c-1] = ycount - 1;
                ycount = temp_y;
                for (xcount = temp_x; xcount < contourx[c-1]; xcount--) { //from old origin to new origin,
                  ycoord = image.csize * xsize * ycount;	xcoord = image.csize * xcount + ycoord;
                  base[chGray + xcoord] = 0; //make them all black
                }
              }
              if (n_hand == 0) { SETFLOAT(&ap[0], c);	SETFLOAT(&ap[1], contourx[c]);	SETFLOAT(&ap[2], contoury[c]);
                outlet_list(outlet1, 0, 3, ap);	
              }
              c++;
              break;
            }
          }
        }
        // (3->n) Find the rest of points down the line...
        int done, old_c;
        old_c = done = 0;
        for (linecheck = 0; linecheck <= 5000; linecheck++) { //linecheck is just a temporal anti-crash measure...
          difx = abs(contourx[c-1] - contourx[0] );
          dify = abs(contoury[c-1] - contoury[0] );
          // Test to see if it closes to the origin.
          if (c > min_perim && difx < 3 && dify < 3) break; // check if it has a minimum_perimeter
          else if (done == 1) break;
          else {
            // establish relative position of last two points:
            prevx = contourx[c-1] - contourx[c-2]; //  1 (right) -1 (left)
            prevy = contoury[c-1] - contoury[c-2]; //  1 (below) -1 (top)
            // cases clockwise
            if	(prevx == -1 && prevy ==  -1) start = 0;	
            else if (prevx ==  0 && prevy ==  -1) start = 1;
            else if (prevx ==  1 && prevy ==  -1) start = 2;
            else if (prevx ==  1 && prevy ==   0) start = 3;
            else if (prevx ==  1 && prevy ==   1) start = 4;
            else if (prevx ==  0 && prevy ==   1) start = 5;
            else if (prevx == -1 && prevy ==   1) start = 6;
            else if (prevx == -1 && prevy ==   0) start = 7;
            // relative positions to center pixel clockwise too
            int orderx[] = {0, -1, -1, -1,  0,  1, 1, 1}; 
            int ordery[] = {1,  1,  0, -1, -1, -1, 0, 1};
            for (i=0; i<8; i++) {
              ycount = contoury[c-1] + ordery[ (start + i) % 8 ];
              xcount = contourx[c-1] + orderx[ (start + i) % 8 ];
              xcoord = image.csize * xcount + image.csize * xsize * ycount;
              //	if (c > min_perim && (abs(xcount - contourx[0]) > 4) && (abs(ycount - contoury[0]) > 4) ) done = 1;
              if (base[chGray + xcoord] == 80) { // if we find a point
                contourx[c] = xcount; //record in contour
                contoury[c] = ycount;							
                base[chGray + xcoord] =	255;
                if (mode ==2)if (n_hand >= 0) { SETFLOAT(&ap[0], c);	SETFLOAT(&ap[1], contourx[c]);	
                  SETFLOAT(&ap[2], contoury[c]); SETFLOAT(&ap[3], 2); outlet_list(outlet1, 0, 4, ap);	
                }						
                c++;
                break;
              }
              else if (base[chGray + xcoord] == 255) { //if we end up in a previous point
                old_c = 0;						
                for (j=0; j<c; j++) {
                  if (xcount == contourx[j] && ycount == contoury[j]) {
                    old_c = j;
                  }
                }
                if (abs(c-old_c)> 10) {
                  if (mode ==2)post("no backing oldc=%d and c=%d", old_c, c);
                  contourx[c] = xcount; //record in contour
                  contoury[c] = ycount;							
                  base[chGray + xcoord] =	255;
                  if (mode ==2) if (n_hand >= 0) { SETFLOAT(&ap[0], c);	SETFLOAT(&ap[1], contourx[c]);	
                    SETFLOAT(&ap[2], contoury[c]); SETFLOAT(&ap[3], 2); outlet_list(outlet1, 0, 4, ap);	
                  }						
                  c++;
                  break;
                }
                else {
                  if (mode ==2)post ("backing up!!! xcount = %d ycount = %d x[0] = %d y[0] = %d", xcount, ycount, contourx[0], contoury[0]);	
                  ycount = contoury[c-1];	//go to last point in countour
                  xcount = contourx[c-1];
                  xcoord = image.csize * xcount + image.csize * xsize * ycount;
                  base[chGray + xcoord] =	0; //make it black
                  c--; // and move back one point in the contour...
                }
              }
            } // close clockwise search
          } // close search for rest of points
        } // close linecheck
        if (mode ==2)post("linecheck =%d", linecheck);
        // ******** find BOUNDS...
        int maxx, maxy, mayy, mayx, mixx, mixy, miyy, miyx, nmax, nmay, nmix, nmiy;
        maxx = maxy = mayy = mayx = 0;
        nmax = nmay = nmix = nmiy = 1;
        mixx = mixy = miyy = miyx = 2000;
        for (i = 0; i < c; i++) {
          if (contourx[i] >= maxx) {
            maxx = contourx[i];
            maxy = contoury[i];
          } 
          if (contourx[i] <= mixx) {
            mixx = contourx[i];
            mixy = contoury[i];
          }
          if (contoury[i] >= mayy) {
            mayx = contourx[i];
            mayy = contoury[i];
          }
          if (contoury[i] <= miyy) {
            miyx = contourx[i];
            miyy = contoury[i];
          }
        }

        // ******** take PARTIAL CONTOUR MEASURES
        if(m_pimpl->p_delayXY->size < pixavg) { delete m_pimpl->p_delayXY; m_pimpl->p_delayXY=NULL; }  
        if(!m_pimpl->p_delayXY)m_pimpl->p_delayXY=new PIMPL::delayXY(pixavg);

        int hand_perimeter;
        hand_perimeter = c;
        int avgcontx, avgconty, ii, tx, ty, tx1, ty1, diffx, diffy, iii, ix, iy;
        int*delay_x=m_pimpl->p_delayXY->d_x;
        int*delay_y=m_pimpl->p_delayXY->d_y;


        n = tempint = 0;		
        xval = yval = 0;
        int rap, ram; rap = ram = 0;
        float refang_plus, refang_minus;	refang_plus = refang_minus = 0;
        maxangle = minangle = 0.;
        avgcontx = avgconty = tx = ty = tx1 = ty1 = ii = 0;
        for (i = 0; i < c; i++) { //low pass filter avg
          tx += contourx[i];
          ty += contoury[i];
          iii = i % pixavg;
          ix = delay_x[iii];
          iy = delay_y[iii];
          delay_x[iii] = contourx[i];
          delay_y[iii] = contoury[i];
          if (i < pixavg) ii++;
          if (i >= pixavg) {
            tx1 += ix;
            ty1 += iy;
          }
          diffx = tx - tx1;
          diffy = ty - ty1;
          avgcontx = (tx - tx1) / ii;
          avgconty = (ty - ty1) / ii;
          //	post("avgx=%d, avgy=%d, i=%d, ii=%d, tx = %d, %d ty = %d, %d", diffx, diffy, i, ii, tx, tx1, ty, ty1);
          //	post("avgx=%d, avgy=%d, i=%d, ii=%d, tx = %d, %d ty = %d, %d", avgcontx, avgconty, i, ii, tx, tx1, ty, ty1);
          if (i % pixsamp == 0) { // sample filtered contour
            partialx[n] = avgcontx; //(partialx_prev[n] + avgcontx ) / 2;
            partialy[n] = avgconty; //(partialy_prev[n] + avgconty ) / 2;
            nindex[n] = i;
            if ( n >= 2) { // bring both vectors to the origin	
              xnm1 =	partialx[n-1]	- partialx[n-2];
              xn =	partialx[n]	- partialx[n-2] - xnm1;
              ynm1 =	partialy[n-1]	- partialy[n-2];
              yn =	partialy[n]	- partialy[n-2] - ynm1;
              tempangle = atan2 ((double) ynm1, (double) xnm1); // calculate angle of first vector
              xval = ((double) xn * cos(tempangle)	 ) + ((double) yn * sin(tempangle));
              yval = ((double) xn * sin(tempangle) * -1) + ((double) yn * cos(tempangle)); // rotate second vector according to the previous one
              angle[n] = atan2 (yval, xval); // calculate the angle	
              if (angle[n] > maxangle) maxangle = angle[n];
              if (angle[n] < minangle) minangle = angle[n];
              //				if (angle[n] > 0) { rap++; refang_plus = angle[n] / rap; }
              //				if (angle[n] < 0) { ram++; refang_minus = angle[n] / ram; }					
					
            }
            else 	angle[n] = 0; 
            n++;
            if (n_hand == 0) { 
              SETFLOAT(&as[0], n);	SETFLOAT(&as[1], partialx[n]);	SETFLOAT(&as[2], partialy[n]);	
              SETFLOAT(&as[3], (float) angle[n]); outlet_list(outlet2, 0, 4, as);	
            }			
          }
        }
        //	refang_plus  += 0.001;
        //	refang_minus -= 0.001;
        //	post ("rap = %f ram = %f", refang_plus, refang_minus);	
        // ******* find (Hand center)
        xval_int = yval_int = 0;
        for (i = 0; i < n; i++) {
          xval_int  +=  (int) partialx[i]; 
          yval_int  +=  (int) partialy[i]; 
        }
        hx =  (xval_int / n); // + e_ctrx[tempint2]) / 2 ; // * (e_size[tempint2] / 3))) / (n + (e_size[tempint2] / 3));
        hy =  (yval_int / n); // + e_ctry[tempint2]) / 2 ; // * (e_size[tempint2] / 3))) / (n + (e_size[tempint2] / 3));
        // ******* Determine TOTAL HAND AREA
        hand_area = (float) h / (float) tot_area;
        // ******* find HAND DIRECTION
        double tempx, tempy, tot_magni, direction;
        tot_magni = direction = tempx = tempy = 0;
        for (i = (n / 3); i < (n * 2 / 3); i++) {	
          tempx += ((double) partialx[i] - (double) hx);
          tempy += ((double) partialy[i] - (double) hy);
          direction = atan2 ((double) tempy, (double) tempx);
        }
        if (maxx == cx) {
          maxx = cx;
          maxy = cy;
        }
        else if (mixx == cx) {
          mixx = cx;
          mixy = cy;
        }
        else if (mayy == cy) {
          mayx = cx;
          mayy = cy;
        }
        else if (miyy == cy) {
          miyx = cx;
          miyy = cy;
        }

        SETFLOAT(&aq[0], n_hand);
        SETFLOAT(&aq[1], hx);					
        SETFLOAT(&aq[2], hy);	
        SETFLOAT(&aq[3], hand_area);
        SETFLOAT(&aq[4], (float) direction);	
        SETFLOAT(&aq[5], cx);	
        SETFLOAT(&aq[6], cy);
        SETFLOAT(&aq[7], hand_perimeter);
        SETFLOAT(&aq[8], e_size[tempint2]);
        SETFLOAT(&aq[9],  maxx);					
        SETFLOAT(&aq[10], maxy);	
        SETFLOAT(&aq[11], mixx);
        SETFLOAT(&aq[12], mixy);	
        SETFLOAT(&aq[13], mayx);	
        SETFLOAT(&aq[14], mayy);
        SETFLOAT(&aq[15], miyx);
        SETFLOAT(&aq[16], miyy);
        outlet_list(outlet5, 0, 17, aq);	
        // ******* Find FINGER TIPS
        max_tip = min_tip = 0;	tip = valley = tempint = 0;
        tiptest = false;
        j = 0;
        //	refang_plus = maxangle / refang_plus;
        //	post ("rap = %f", refang_plus);
        for (i = 0; i < n; i++) {
          j++;
          if (tip > maxtips - 1) break;
          else {
            if (angle[i] > (maxangle * tip_scalar)) {
              tiptest = true;
              if (angle[i] > max_tip) {
                max_tip = angle[i];
                tempint = i;
              }
            }
            else {
              if (tiptest) {
                tiptest = false;
                if	(j > pixtip || tip == 0) {		
                  t_a[tip] = max_tip;
                  max_tip = 0;
                  tipx = ((float) (contourx[nindex[tempint-1]] + contourx[nindex[tempint-2]])) / 2.;
                  t_x[tip] = tipx;
                  tipy = ((float) (contoury[nindex[tempint-1]] + contoury[nindex[tempint-2]])) / 2.;
                  t_y[tip] = tipy;
                  tip_mag_ctr = (tipx - hx)*(tipx - hx) + (tipy - hy)*(tipy - hy);
                  tip_mag_ctr = sqrt( tip_mag_ctr );
                  t_m[tip] = tip_mag_ctr;
                  t_i[tip] = tip;
                  t_t[tip] = nindex[(tempint-1 + tempint-2) / 2]; 
                  tip++;
                  j = 0;
                }
                else if	(j <= pixtip) {	
						
                  t_a[tip - 1] = ((t_a[tip - 1] + max_tip) / 2);		max_tip = 0;
                  tipx = ((float) (contourx[nindex[tempint-3]] + contourx[nindex[tempint-1]] + contourx[nindex[tempint-2]])) / 3.;
                  t_x[tip - 1] = ((t_x[tip - 1] + tipx) / 2);
                  tipy = ((float) (contoury[nindex[tempint-3]] + contoury[nindex[tempint-1]] + contoury[nindex[tempint-2]])) / 3.;
                  t_y[tip - 1] = ((t_y[tip - 1] + tipy) / 2);
                  tip_mag_ctr = (t_x[tip - 1] - hx)*(t_x[tip - 1] - hx) + (t_y[tip - 1] - hy)*(t_y[tip - 1] - hy);
                  tip_mag_ctr = sqrt( tip_mag_ctr );
                  t_m[tip - 1] = tip_mag_ctr;
                  t_t[tip] = nindex[(tempint-3 + tempint-2 + tempint-1)/3];
                  j = 0;	
                }
              }
            }
          }
        }
        //botamos todos de golpe
        ii = 0;
        unsigned int index;
        for (index = 0; index < tip; index++) {	
          if ( ((fabs(t_x[index] - e_x1[tempint2]) > 10) && (fabs(t_y[index] - e_y1[tempint2]) > 10)) || 
               ((fabs(t_x[index] - e_x2[tempint2]) > 10) && (fabs(t_y[index] - e_y2[tempint2]) > 10)) ){
            SETFLOAT(&at[0], n_hand);	
            SETFLOAT(&at[1], ii);	
            SETFLOAT(&at[2], t_x[index]);	
            SETFLOAT(&at[3], t_y[index]);
            SETFLOAT(&at[4], t_m[index]);
            SETFLOAT(&at[5], t_a[index]);
            SETFLOAT(&at[6], t_t[index]);
            ii++;
            outlet_list(outlet3, 0, 7, at);
          }
        }
        for (index = 0; index < n; index++) {
          partialx_prev[index] = partialx[index];
          partialy_prev[index] = partialy[index];
        }
        for (index = n; index < 10000; index++) {
          partialx_prev[index] = 0;
          partialy_prev[index] = 0;
        }

		
        // ******* Find VALLEYS
        j = 0;
        //	refang_minus = maxangle / refang_minus;
        //	post ("ram = %f", refang_minus);
        n_hand++;
        totalcycles++;
      } // close mode 1
	
  } // close while
  if (foundhand == 0) SETFLOAT(&ar[0], 0);
  else 		SETFLOAT(&ar[0], 1);
  outlet_list(outlet6, 0, 1, ar);
  if (mode == 2) post("totalcycles=%d", totalcycles);

  delete[]enterx; delete[]entery;
  delete[]hand_areax; delete[]hand_areay;
} // close graymethod
// #####################################################
// floatHopMess
// #####################################################
void pix_mano :: vecParamsMess(t_symbol*s,int argc, t_atom *argv) {
  pixavg		= (int)atom_getfloat(&argv[0]);
  pixtip		= (int)atom_getfloat(&argv[1]);
  min_entry_size  = (int)atom_getfloat(&argv[2]);
  min_perim	= (int)atom_getfloat(&argv[3]);
  pixsamp		= (int)atom_getfloat(&argv[4]);
	
  post("pixavg = %d   pixtip = %d", pixavg, pixtip);
}
// #####################################################
// vecBoundsMess
// #####################################################
void pix_mano :: vecBoundsMess(t_symbol*s,int argc, t_atom *argv) {
  bottom		= (int)atom_getfloat(&argv[0]);
  head		= (int)atom_getfloat(&argv[1]);
  right		= (int)atom_getfloat(&argv[2]);
  left		= (int)atom_getfloat(&argv[3]);

  post("bottom = %d   head = %d	left = %d	right = %d", bottom, head, left, right);
  setPixModified();
}
// #####################################################
// vecThreshMess
// #####################################################
void pix_mano :: vecThreshMess(t_symbol*s,int argc, t_atom *argv) {
  mode		= (int)atom_getfloat(&argv[0]);
  thresh		= (float)atom_getfloat(&argv[1]) * 255.;
  tip_scalar	= (float)atom_getfloat(&argv[2]);
  post("mode = %d, thresh = %f, tip_scalar = %f", mode, thresh, tip_scalar);
  setPixModified();
}
// #####################################################
// static member function
// #####################################################
void pix_mano :: obj_setupCallback(t_class *classPtr)
{
  CPPEXTERN_MSG(classPtr, "vec_thresh", vecBoundsMess);
  CPPEXTERN_MSG(classPtr, "moth", vecThreshMess);
  CPPEXTERN_MSG(classPtr, "param", vecParamsMess);
}
