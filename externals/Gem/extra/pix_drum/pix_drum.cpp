////////////////////////////////////////////////////////
//
// pix_mano - an object to track black pixels entering from the top of a white frame.
//
// Designed to work with the Silent Drum Controller
// www.jaimeoliver.pe/drum
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
//    Copyright (c) GÃŒnther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::fÃŒr::umlÃ€ute. IEM
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "LICENSE.txt" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_drum.h"
CPPEXTERN_NEW(pix_drum)

/////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////
pix_drum :: pix_drum()
{
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vec_params"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vec_thresh"));
	outlet1 = outlet_new(this->x_obj, gensym("list"));
	outlet2 = outlet_new(this->x_obj, gensym("list"));
	outlet3 = outlet_new(this->x_obj, gensym("list"));
	outlet4 = outlet_new(this->x_obj, gensym("list"));
    m_thresh[chRed] = m_thresh[chGreen] = m_thresh[chBlue] = m_thresh[chAlpha] = 0;
	head = left = mode = 0;
	min_width = 10;
	min_height = .14;
	bottom = 240;
	right = 320;
}
/////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////
pix_drum :: ~pix_drum()
{ }
/////////////////////////////////////////////////////////
// PROCESS IMAGE
/////////////////////////////////////////////////////////
void pix_drum :: processGrayImage(imageStruct &image)
{
	int xsize = image.xsize;
	int ysize = image.ysize;
    unsigned char *base = image.data;
	int x2, y2, xcoord, ycoord, xcount, ycount;
	int i, j, n, totalyes, totalyesx, totalyesy, xval, yval;
    int *yesx=new int[image.xsize];
    int *yesy=new int[image.ysize];
	float c2f, L_area, R_area;
	int hor, ver, L_peakx, L_peaky, R_peakx, R_peaky;
	t_atom ap[4];
	t_atom as[4];
	t_atom at[4];
	t_atom aq[4];
	
	totalyes = i = j = xval = yval = hor = ver = 0;
	L_peakx = L_peaky = R_peakx = R_peaky = 0;
	L_area = R_area = 0;	// = totalyesx = totalyesy 
	for (n=0;n<image.xsize;n++) 
		yesx[n] = 0; //clear histograms
	for (n=0;n<image.ysize;n++) 
		yesy[n] = 0;

	if (mode == 0){	
// check bounds are within limits
		if (left	< 0)		left	= 0;
		if (right	> xsize)	right	= xsize;
		if (head	< 0)		head	= 0;
		if (bottom	> ysize)	bottom	= ysize;
// thresh the image and make histograms
		for (ycount = 0; ycount < bottom; ycount++) {
			if (ycount < head)			continue;
			else if (ycount > bottom)	continue;
			else ycoord = image.csize * image.xsize * ycount;
			for (xcount = left; xcount < right; xcount++) {
				xcoord = image.csize * xcount + ycoord; 
				if (base[chGray + xcoord] < (thresh*256.)) {
					base[chGray + xcoord] = 0;
					yesx[xcount] += 1;	// make a histogram of each axis
					yesy[ycount] += 1;	//	
					totalyes++; 
				}
				else {	base[chGray + xcoord] = 255;	}
			}
		}
// find x (hor) and y (ver) coordinates
		int xval_max;
		xval_max = left;
		xval = left;
		for (i = left; i < right; i++) {
			if (yesx[i] > yesx[xval]) xval = i;
			if (yesx[i] >= yesx[xval_max]) xval_max = i; 
		}
		hor = (xval + xval_max) / 2;
		ver = yesx[hor];
// find secondary peaks
		int spL_x[100], spL_y[100], spR_x[100], spR_y[100];
		float *px=new float[right];
		int spL, spR;
		
		for (i = left; i < hor; i++) L_area += yesx[i]; 
		R_area  = (totalyes - L_area);
		L_area /= ((hor)       * (ver));
		R_area /= ((right - hor) * (ver));
		xval = yval = 0;
		
		for (i = left;	i < right; i++) {
			if		(i< pix_dist) px[i] = 0;
			else if (i>right-pix_dist-2) px[i] = 0;
			else 
			px[i] = (yesx[i-pix_dist]*-0.5 + yesx[i] + yesx[i+pix_dist]*-0.5 + yesx[i-pix_dist+1]*-0.5 + yesx[i+1] + yesx[i+pix_dist+1]*-0.5 + yesx[i-pix_dist+2]*-0.5 + yesx[i+2] + yesx[i+pix_dist+2]*-0.5)/3;
		}
		
		float highest, temp_highest; 
		int tempint, prevtempint, p, temp_highest_int, dist_ctr;
		highest = temp_highest = 0;
		tempint = p = temp_highest_int = 0;
		for (i = left;	i < right; i++) {
			if (px[i] > highest) {
				highest = px[i];
			}
		}
//		post ("highest = %f", highest);
		spL = spR = 0;
		for (i = left;	i < right; i++) {
			if		(px[i] > (highest*min_height)) {
				tempint = 1;
				if (px[i] > temp_highest) {
					temp_highest = px[i];
					temp_highest_int = i + 1;
				}
				p++;
			}
			else if (tempint = 1){
				tempint = 0;
				if (p > min_width) {
					spL_x[spL] = temp_highest_int;
					spL_y[spL] = yesx[temp_highest_int];
					dist_ctr = abs(spL_x[spL] - hor);
					if (dist_ctr > pix_dist_ctr) {
						spL++;
					}
				}
				p = 0;
				temp_highest = 0;
			}
		}

		for (i=left; i<right; i++) { 
			SETFLOAT(&aq[0], i);
			SETFLOAT(&aq[1], px[i]);
			outlet_list(outlet4, 0, 2, aq);
		}
			
// draw lines of coordinates (only to use in mode 2)
		for (ycount = 0; ycount < image.ysize; ycount++) {
			if (ycount < head)			continue;
			else if (ycount > bottom)	continue;
			else ycoord = image.csize * image.xsize * ycount;
			if (ycount == (ver + head)){
				for (xcount = 0; xcount < image.xsize; xcount++) {
					xcoord = image.csize * xcount + ycoord;
					base[chGray + xcoord] = 0;
				}
			}
			else{
				xcount = hor;
				xcoord = image.csize * xcount + ycoord;
				base[chGray + xcoord] = 0;
			}
		}
// output values through outlets
		int side;

		for(i = 0; i < spL; i++) {
			if (spL_x[i] <= hor)	{side = 0;}
			else					{side = 1;};
			SETFLOAT(&as[0], side);
			SETFLOAT(&as[1], i);
			SETFLOAT(&as[2], spL_x[i]);
			SETFLOAT(&as[3], spL_y[i]);
			outlet_list(outlet2, 0, 4, as);
		}
		
		SETFLOAT(&ap[0], hor);
		SETFLOAT(&ap[1], ver);
		SETFLOAT(&ap[2], L_area);
		SETFLOAT(&ap[3], R_area);
		outlet_list(outlet1, 0, 4, ap); 
		
	/*	for(i = 0; i < spR; i++) {
			SETFLOAT(&at[0], i);
			SETFLOAT(&at[1], spR_x[i]);
			SETFLOAT(&at[2], spR_y[i]);
			outlet_list(outlet3, 0, 3, at);
		} */
      delete[]px;
    }

/*	
// thresh the image and make histograms
		for (ycount = 0; ycount < image.ysize; ycount++) {
			if (ycount < head)			continue;
			else if (ycount > bottom)	continue;
			else ycoord = image.csize * image.xsize * ycount;
			for (xcount = 5; xcount < (image.xsize - 5); xcount++) {
				xcoord = image.csize * xcount + ycoord; 
				if (base[chGray + xcoord] < (thresh*256.)) {
					base[chGray + xcoord] = 0;
					yesx[xcount] += 1;	// make a histogram of each axis
					yesy[ycount] += 1;	//	
					totalyes++; 
				}
				else {base[chGray + xcoord] = 255;}
			}
		}

// find x (hor) and y (ver) coordinates
		xval = 5;
		for (i = 6; i < (image.xsize - 5); i++)
			if (yesx[i] > yesx[xval]) xval = i; 
		hor = xval;
		ver = yesx[xval];
// find secondary peaks
		for (i = 5; i < hor; i++) L_area += yesx[i]; 
		R_area  = (totalyes - L_area);
		L_area /= ((hor)       * (ver));
		R_area /= ((320 - hor) * (ver));
		xval = yval = 0;
		for (i = 15 ;	i < hor; i++) 
			if ((yesx[i+1]-yesx[i]) <= 0) 
				if ((yesx[i+2]-yesx[i+1]) <= 0) 
					if ((yesx[i+3]-yesx[i+2]) <= 0) 
						if ((yesx[i+4]-yesx[i+3]) <= 0) {
							L_peakx = i;
							L_peaky = yesx[i];
							break;
						}
						else {
							L_peakx = -1;
							L_peaky = -1;
						}
		
		for (i = image.xsize - 15; i > hor; i--) 
			if ((yesx[i]-yesx[i-1]) >= 0) 
				if ((yesx[i-1]-yesx[i-2]) >= 0)	
					if ((yesx[i-2]-yesx[i-3]) >= 0)
						if ((yesx[i-3]-yesx[i-4]) >= 0) {
							if ((i - hor) > 6) {
								R_peakx = i;
								R_peaky = yesx[i];
								break;
							}
						}
						else {
							R_peakx = -1;
							R_peaky = -1;
						}

// #####################################################
// draw lines of coordinates (only to use in mode 2)
// #####################################################	
		for (ycount = 0; ycount < image.ysize; ycount++) {
			if (ycount < head)			continue;
			else if (ycount > bottom)	continue;
			else ycoord = image.csize * image.xsize * ycount;
			if (ycount == (ver + head)){
				for (xcount = 0; xcount < image.xsize; xcount++) {
					xcoord = image.csize * xcount + ycoord;
					base[chGray + xcoord] = 0;
				}
			}
			else {
				xcount = hor;
				xcoord = image.csize * xcount + ycoord;
				base[chGray + xcoord] = 0;
			}
		}
// output values through outlets
		SETFLOAT(&ap[0], hor);
		SETFLOAT(&ap[1], ver);
		SETFLOAT(&ap[2], L_area);
		SETFLOAT(&ap[3], R_area);
		outlet_list(outlet1, 0, 4, ap); 
		
		SETFLOAT(&as[0], L_peakx);
		SETFLOAT(&as[1], L_peaky);
		SETFLOAT(&as[2], R_peakx);
		SETFLOAT(&as[3], R_peaky);
		outlet_list(outlet2, 0, 4, as);

	}
*/
// #####################################################
// MODE 1
// #####################################################
	else if (mode == 1) {
// check bounds are within limits
		if (left	< 0)		left	= 0;
		if (right	> xsize)	right	= xsize;
		if (head	< 0)		head	= 0;
		if (bottom	> ysize)	bottom	= ysize;
// thresh the image and make histograms
		for (ycount = 0; ycount < bottom; ycount++) {
			if (ycount < head)			continue;
			else if (ycount > bottom)	continue;
			else ycoord = image.csize * image.xsize * ycount;
			for (xcount = left; xcount < right; xcount++) {
				xcoord = image.csize * xcount + ycoord; 
				if (base[chGray + xcoord] < (thresh*256.)) {
					base[chGray + xcoord] = 0;
					yesx[xcount] += 1;	// make a histogram of each axis
					yesy[ycount] += 1;	//	
					totalyes++; 
				}
				else {	base[chGray + xcoord] = 255;	}
			}
		}
// find x (hor) and y (ver) coordinates
		xval = 5;
		for (i = 6; i < right; i++)
			if (yesx[i] > yesx[xval]) xval = i; 
		hor = xval;
		ver = yesx[xval];
// find secondary peaks
		int spL_x[100], spL_y[100], spR_x[100], spR_y[100];
		int spL, spR;
		
		spL = spR = 0;
		for (i = left; i < hor; i++) L_area += yesx[i]; 
		R_area  = (totalyes - L_area);
		L_area /= ((hor)       * (ver));
		R_area /= ((320 - hor) * (ver));
		xval = yval = 0;
		
		for (i = left;	i < hor; i++) 
			if ((yesx[i+1]-yesx[i]) <= 0) 
				if ((yesx[i+2]-yesx[i+1]) <= 0) 
					if ((yesx[i+3]-yesx[i+2]) <= 0) 
						if ((yesx[i+4]-yesx[i+3]) < 0)
							if ((hor - i) > 8) {
								spL_x[spL] = i;
								spL_y[spL] = yesx[i];
								spL++;
							}
		
		for (i = right; i > hor; i--) 
			if ((yesx[i]-yesx[i-1]) >= 0) 
				if ((yesx[i-1]-yesx[i-2]) >= 0)	
					if ((yesx[i-2]-yesx[i-3]) >= 0)
						if ((yesx[i-3]-yesx[i-4]) > 0)
							if ((i - hor) > 8) {
								spR_x[spR] = i;
								spR_y[spR] = yesx[i];
								spR++;
							}
		int tempint, prevtempint;
		tempint = 0;
		for (i=left+1; i<right; i++) {
			prevtempint = tempint;
			tempint = yesx[i] - yesx[i-1];
	//		if(sqrt(tempint*0.25 > sqrt(prevtempint*prevtempint)) tempint = 0;
			SETFLOAT(&aq[0], i);
			SETFLOAT(&aq[1], tempint);
			outlet_list(outlet4, 0, 2, aq);
		}
			
// draw lines of coordinates (only to use in mode 2)
		for (ycount = 0; ycount < image.ysize; ycount++) {
			if (ycount < head)			continue;
			else if (ycount > bottom)	continue;
			else ycoord = image.csize * image.xsize * ycount;
			if (ycount == (ver + head)){
				for (xcount = 0; xcount < image.xsize; xcount++) {
					xcoord = image.csize * xcount + ycoord;
					base[chGray + xcoord] = 0;
				}
			}
			else {
				xcount = hor;
				xcoord = image.csize * xcount + ycoord;
				base[chGray + xcoord] = 0;
			}
		}
// output values through outlets
		SETFLOAT(&ap[0], hor);
		SETFLOAT(&ap[1], ver);
		SETFLOAT(&ap[2], L_area);
		SETFLOAT(&ap[3], R_area);
		outlet_list(outlet1, 0, 4, ap); 
	
		for(i = 0; i < spL; i++) {
			SETFLOAT(&as[0], i);
			SETFLOAT(&as[1], spL_x[i]);
			SETFLOAT(&as[2], spL_y[i]);
			outlet_list(outlet2, 0, 3, as);
		}
		
		for(i = 0; i < spR; i++) {
			SETFLOAT(&at[0], i);
			SETFLOAT(&at[1], spR_x[i]);
			SETFLOAT(&at[2], spR_y[i]);
			outlet_list(outlet3, 0, 3, at);
		}
	}
// #####################################################
// MODE 2
// #####################################################
	else if (mode == 2) {
		int*yesx_s = new int[ysize];
// check bounds are within limits
		if (left	< 0)		left	= 0;
		if (right	> xsize)	right	= xsize;
		if (head	< 0)		head	= 0;
		if (bottom	> ysize)	bottom	= ysize;
// thresh the image and make histograms
		for (ycount = 0; ycount < bottom; ycount++) {
			if (ycount < head)			continue;
			else if (ycount > bottom)	continue;
			else ycoord = image.csize * image.xsize * ycount;
			for (xcount = left; xcount < right; xcount++) {
				xcoord = image.csize * xcount + ycoord; 
				if (base[chGray + xcoord] < (thresh*256.)) {
					base[chGray + xcoord] = 0;
					yesx[xcount] += 1;	// make a histogram of each axis
					yesy[ycount] += 1;	//	
					totalyes++; 
				}
				else {	base[chGray + xcoord] = 255;	}
			}
		}
// smooth values
		for (i = left; i < right; i++) {
			if 	(i == left) 	yesx_s[i] = yesx[i];
			else if (i == right)	yesx_s[i] = yesx[i];
			else 			yesx[i] = (yesx[i-1] + yesx[i] + yesx[i+1]) / 3;
		}
// find x (hor) and y (ver) coordinates
		int xval_max;
		xval_max = left;
		xval = left;
		for (i = left; i < right; i++) {
			if (yesx[i] > yesx[xval]) xval = i;
			if (yesx[i] >= yesx[xval_max]) xval_max = i; 
		}
		hor = (xval + xval_max) / 2;
		ver = yesx[hor];
// find secondary peaks
		int spL_x[100], spL_y[100], spR_x[100], spR_y[100];
		float *px=new float[right];
		int spL, spR;
		
		for (i = left; i < hor; i++) L_area += yesx[i]; 
		R_area  = (totalyes - L_area);
		L_area /= ((hor)       * (ver));
		R_area /= ((right - hor) * (ver));
		xval = yval = 0;
		
		for (i = left;	i < right; i++) {
			if		(i< pix_dist) px[i] = 0;
			else if (i>right-pix_dist-2) px[i] = 0;
			else 
			px[i] = (yesx[i-pix_dist]*-0.5 + yesx[i] + yesx[i+pix_dist]*-0.5 + yesx[i-pix_dist+1]*-0.5 + yesx[i+1] + yesx[i+pix_dist+1]*-0.5 + yesx[i-pix_dist+2]*-0.5 + yesx[i+2] + yesx[i+pix_dist+2]*-0.5)/3;
		}
		
		float highest, temp_highest; 
		int tempint, prevtempint, p, temp_highest_int, dist_ctr;
		highest = temp_highest = 0;
		tempint = p = temp_highest_int = 0;
		for (i = left;	i < right; i++) {
			if (px[i] > highest) {
				highest = px[i];
			}
		}
//		post ("highest = %f", highest);
		spL = spR = 0;
		for (i = left;	i < right; i++) {
			if		(px[i] > (highest*min_height)) {
				tempint = 1;
				if (px[i] > temp_highest) {
					temp_highest = px[i];
					temp_highest_int = i + 1;
				}
				p++;
			}
			else if (tempint = 1){
				tempint = 0;
				if (p > min_width) {
					spL_x[spL] = temp_highest_int;
					spL_y[spL] = yesx[temp_highest_int];
					dist_ctr = abs(spL_x[spL] - hor);
					if (dist_ctr > pix_dist_ctr) {
						spL++;
					}
				}
				p = 0;
				temp_highest = 0;
			}
		}

		for (i=left; i<right; i++) { 
			SETFLOAT(&aq[0], i);
			SETFLOAT(&aq[1], px[i]);
			outlet_list(outlet4, 0, 2, aq);
		}
			
// draw lines of coordinates (only to use in mode 2)
		for (ycount = 0; ycount < image.ysize; ycount++) {
			if (ycount < head)			continue;
			else if (ycount > bottom)	continue;
			else ycoord = image.csize * image.xsize * ycount;
			if (ycount == (ver + head)){
				for (xcount = 0; xcount < image.xsize; xcount++) {
					xcoord = image.csize * xcount + ycoord;
					base[chGray + xcoord] = 0;
				}
			}
			else{
				xcount = hor;
				xcoord = image.csize * xcount + ycoord;
				base[chGray + xcoord] = 0;
			}
		}
// output values through outlets
		int side;

		for(i = 0; i < spL; i++) {
			if (spL_x[i] <= hor)	{side = 0;}
			else					{side = 1;};
			SETFLOAT(&as[0], side);
			SETFLOAT(&as[1], i);
			SETFLOAT(&as[2], spL_x[i]);
			SETFLOAT(&as[3], spL_y[i]);
			outlet_list(outlet2, 0, 4, as);
		}
		
		SETFLOAT(&ap[0], hor);
		SETFLOAT(&ap[1], ver);
		SETFLOAT(&ap[2], L_area);
		SETFLOAT(&ap[3], R_area);
		outlet_list(outlet1, 0, 4, ap); 
		
	/*	for(i = 0; i < spR; i++) {
			SETFLOAT(&at[0], i);
			SETFLOAT(&at[1], spR_x[i]);
			SETFLOAT(&at[2], spR_y[i]);
			outlet_list(outlet3, 0, 3, at);
		} */
        delete[]yesx_s;
        delete[]px;
	}
    delete[]yesx; delete[]yesy;
}
/////////////////////////////////////////////////////////
// vecBoundsMess
/////////////////////////////////////////////////////////
void pix_drum :: vecBoundsMess(t_symbol*s,int argc, t_atom *argv)
{

	bottom	= (int)atom_getfloat(&argv[0]);
	head		= (int)atom_getfloat(&argv[1]);
	left		= (int)atom_getfloat(&argv[2]);
	right		= (int)atom_getfloat(&argv[3]);
	post("bottom = %d   head = %d	left = %d	right = %d", bottom, head, left, right);
	setPixModified();
}

/////////////////////////////////////////////////////////
// floatThreshMess
/////////////////////////////////////////////////////////
void pix_drum :: vecThreshMess(t_symbol*s,int argc, t_atom *argv)
{
    mode			= (int)		atom_getfloat(&argv[0]);
	thresh			= (float)	atom_getfloat(&argv[1]);
	pix_dist		= (int)		atom_getfloat(&argv[2]);
	min_height		= (float)	atom_getfloat(&argv[3]);
	min_width		= (int)		atom_getfloat(&argv[4]);
	pix_dist_ctr	= (int)		atom_getfloat(&argv[5]);
	post("mode = %d   thress = %f	pix_dist = %d", mode, thresh, pix_dist);
    setPixModified();
}

/////////////////////////////////////////////////////////
// static member function
/////////////////////////////////////////////////////////
void pix_drum :: obj_setupCallback(t_class *classPtr)
{
    CPPEXTERN_MSG(classPtr, "vec_thresh", vecBoundsMess);
    CPPEXTERN_MSG(classPtr, "vec_params", vecThreshMess);
}
