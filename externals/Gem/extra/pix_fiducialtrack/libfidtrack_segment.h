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

#ifndef INCLUDED_SEGMENT_H
#define INCLUDED_SEGMENT_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/*
    usage:

    Segmenter s;

    ...
    
    initialize_segmenter( &s, WIDTH, HEIGHT, 8 );

    ...

    step_segmenter( &s, thresholded_image, WIDTH, HEIGHT );

    ...

    terminate_segmenter( &s );
*/

#define NO_REGION_FLAG                  (0)

#define FREE_REGION_FLAG                (1)

#define ADJACENT_TO_ROOT_REGION_FLAG    (2)

/*
    saturated regions are those whose adjacency list became full and other nodes
    couldn't be made adjacent, either during a make_adjacent operation or
    during a merge operation.
*/
#define SATURATED_REGION_FLAG           (4)

/*
    fragmented regions are those which couldn't be made adjacent to another
    region (or had to be detached from another region) because the other region
    was saturated.
*/
#define FRAGMENTED_REGION_FLAG          (8)

#define UNKNOWN_REGION_LEVEL            (-1)

typedef struct FidSegRegion{
    struct FidSegRegion *previous, *next;
    int colour;
    short left, top, right, bottom;

    int flags;

    short level;                            /* initialized to UNKNOWN_REGION_LEVEL */
    short depth;                            /* initialized to 0 */
    short children_visited_count;           /* initialized to 0 */
    short descendent_count;                 /* initialized to 0x7FFF */
    
    char *depth_string;                     /* not initialized by segmenter */
    
    short adjacent_region_count;
    struct FidSegRegion *adjacent_regions[ 1 ];   /* variable length array of length max_adjacent_regions */

} FidSegRegion;


typedef struct FidSegRegionReference{
    FidSegRegion *region;
    struct FidSegRegionReference *redirect;
} FidSegRegionReference;



#define REGIONREF_IS_REDIRECTED(r) ((r)->redirect != (r))

#define RESOLVE_REGIONREF_REDIRECTS( x, r )                                    \
{                                                                              \
    if( r->redirect != r ){                                                    \
        FidSegRegionReference *result = r;                                           \
        do{                                                                    \
            result = result->redirect;                                         \
        }while( result->redirect != result );                                  \
        r->redirect = result;                                                  \
        x = result;                                                            \
    }else{                                                                     \
        x = r;                                                                 \
    }                                                                          \
}
                 

void initialize_head_region( FidSegRegion *r );
void link_region( FidSegRegion *head, FidSegRegion* r );
void unlink_region( FidSegRegion* r );


typedef struct Segmenter{
    FidSegRegionReference *region_refs;
    int region_ref_count;
    unsigned char *regions;     /* buffer containing raw region ptrs */
    int region_count;
    FidSegRegion *freed_regions_head;

    int sizeof_region;
    int max_adjacent_regions;

    FidSegRegionReference **regions_under_construction;
}Segmenter;

#define LOOKUP_SEGMENTER_REGION( s, index )\
    (FidSegRegion*)(s->regions + (s->sizeof_region * (index)))

void initialize_segmenter( Segmenter *segments, int width, int height, int max_adjacent_regions );
void terminate_segmenter( Segmenter *segments );

void step_segmenter( Segmenter *segments, const unsigned char *source,
        int width, int height );


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* INCLUDED_SEGMENT_H */

