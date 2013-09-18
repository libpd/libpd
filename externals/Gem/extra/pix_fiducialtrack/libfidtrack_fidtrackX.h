/*
  Fiducial tracking library.
  Copyright (C) 2004 Ross Bencina <rossb@audiomulch.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef INCLUDED_FIDTRACK_H
#define INCLUDED_FIDTRACK_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#include "libfidtrack_segment.h"
#include "libfidtrack_treeidmap.h"

  /* JMZ: taken from floatpoint.h */
typedef struct FloatPoint{
  float x, y;
}FloatPoint;

typedef struct FidtrackerX{

    int min_target_root_descendent_count;
    int max_target_root_descendent_count;
    int min_depth, max_depth;

    struct FidSegRegion root_regions_head;
    
    char *depth_strings;
    int depth_string_count;
    int depth_string_length;
    int next_depth_string;
    char *temp_coloured_depth_string;

    double black_x_sum, black_y_sum, black_leaf_count;
    double white_x_sum, white_y_sum, white_leaf_count;

    int min_leaf_width_or_height;

    TreeIdMap *treeidmap;

    FloatPoint *pixelwarp;

} FidtrackerX;

/* pixelwarp is a Width by Height array of pixel coordinates and can be NULL */

void initialize_fidtrackerX( FidtrackerX *ft, TreeIdMap *treeidmap, FloatPoint *pixelwarp );

void terminate_fidtrackerX( FidtrackerX *ft );



#define INVALID_FIDUCIAL_ID  INVALID_TREE_ID

typedef struct FiducialX{
    int id;                                 /* can be INVALID_FIDUCIAL_ID */
    
    float x, y;
    float angle;

}FiducialX;


/*
    usage:

    #define MAX_FIDUCIAL_COUNT  120
    Fiducial fiducials[ MAX_FIDUCIAL_COUNT ];
    PartialSegmentTopology pst;
    int count;

    count = find_fiducials( segments, &pst, fiducials, MAX_FIDUCIAL_COUNT );
*/

int find_fiducialsX( FiducialX *fiducials, int count,
        FidtrackerX *ft, Segmenter *segments, int width, int height );


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* INCLUDED_FIDTRACK_H */
