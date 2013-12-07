// Copyright (c) 2004 Tim Blechmann
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "GEM.LICENSE.TERMS"  in this distribution.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// See file GEM.LICENSE.TERMS for further informations on licensing terms.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef _PIX_MEAN_COLOR_H
#define _PIX_MEAN_COLOR_H

#include "Base/GemPixObj.h"

class GEM_EXTERN pix_mean_color : public GemPixObj
{
	CPPEXTERN_HEADER(pix_mean_color, GemPixObj);

 public:
	// constructor
	pix_mean_color(int argc, t_atom *argv);
	
 protected:
	// destructor
	virtual ~pix_mean_color();

	// processing routine
	virtual void processRGBImage(imageStruct &image);
	virtual void processRGBAImage(imageStruct &image);
	virtual void processGrayImage(imageStruct &image);
	virtual void processYUVImage(imageStruct &image);
	
	t_outlet * m_list;
};


#endif /* _PIX_MEAN_COLOR_H */
