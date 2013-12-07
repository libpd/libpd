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

#ifndef NDEBUG
# define NDEBUG
#endif
#include "libfidtrack_fidtrackX.h"

#include <math.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#define NOT_TRAVERSED       UNKNOWN_REGION_LEVEL
#define TRAVERSED           (NOT_TRAVERSED-1)
#define TRAVERSING          (NOT_TRAVERSED-2)


#define LEAF_GATE_SIZE      0

static void set_depth( FidSegRegion *r, short depth );


/* -------------------------------------------------------------------------- */


static double calculate_angle( double dx, double dy )
{
    double result;
    
    if( fabs(dx) < 0.000001 )
        result = M_PI * .5;
    else
        result = fabs( atan(dy/dx) );

    if( dx < 0 )
        result = M_PI - result;

    if( dy < 0 )
        result = -result;

    if( result < 0 )
        result += 2. * M_PI;

    return result;
}


static void sum_leaf_centers( FidtrackerX *ft, FidSegRegion *r, int width, int height )
{
    int i;

    double radius = .5 + r->depth;
    double n = radius * radius * M_PI;  // weight according to depth circle area

    if( r->adjacent_region_count == 1 ){
        float x, y;
        int w = r->right - r->left;
        int h = r->bottom - r->top;
        if( w < ft->min_leaf_width_or_height )
            ft->min_leaf_width_or_height = w;
        if( h < ft->min_leaf_width_or_height )
            ft->min_leaf_width_or_height = h;

        x = ((r->left + r->right) * .5f);
        y = ((r->top + r->bottom) * .5f);

        if( ft->pixelwarp ){
          int xx = static_cast<int>(x);
            float xf = x - xx;
            int yy = static_cast<int>(y);
            float yf = y - yy;

            // bilinear interpolated pixel warp
            FloatPoint *a = &ft->pixelwarp[ width * yy + xx ];
            FloatPoint *b = &ft->pixelwarp[ width * yy + (xx+1) ];
            FloatPoint *c = &ft->pixelwarp[ width * (yy+1) + xx ];
            FloatPoint *d = &ft->pixelwarp[ width * (yy+1) + (xx+1) ];

            float x1 = a->x + ((b->x - a->x) * xf);
            float x2 = c->x + ((d->x - c->x) * xf);

            float y1 = a->y + ((c->y - a->y) * yf);
            float y2 = b->y + ((d->y - b->y) * yf);

            x = x1 + ((x2 - x1) * yf);
            y = y1 + ((y2 - y1) * xf);


            // truncated lookup of pixel warp:
            // x = ft->pixelwarp[ width * yy + xx ].x;
            // y = ft->pixelwarp[ width * yy + xx ].y;
        }

        if( r->colour == 0 ){
            ft->black_x_sum += x * n;
            ft->black_y_sum += y * n;
            ft->black_leaf_count += n;
        }else{
            ft->white_x_sum += x * n;
            ft->white_y_sum += y * n;
            ft->white_leaf_count += n;
        }
    }else{
        for( i=0; i < r->adjacent_region_count; ++i ){
            FidSegRegion *adjacent = r->adjacent_regions[i];
            if( adjacent->level == TRAVERSED
                    && adjacent->descendent_count < r->descendent_count )
                sum_leaf_centers( ft, adjacent, width, height );
        }
    }
}


// for bcb compatibility
#ifndef _USERENTRY
#define _USERENTRY
#endif

int _USERENTRY depth_string_cmp(const void *a, const void *b)
{
  // FIXXME const_cast supidness
  const FidSegRegion **aa = reinterpret_cast<const FidSegRegion**>(const_cast<void*>(a));
  const FidSegRegion **bb = reinterpret_cast<const FidSegRegion**>(const_cast<void*>(b));

    if( !(*aa)->depth_string ){
        if( !(*bb)->depth_string )
            return 0;
        else
            return 1;
    }else if( !(*bb)->depth_string ){
        return -1;
    }else{
        return -strcmp( (*aa)->depth_string, (*bb)->depth_string ); // left heavy order
    }
}


static char *build_left_heavy_depth_string( FidtrackerX *ft, FidSegRegion *r )
{
    int i;
    char *result;
    FidSegRegion *adjacent;
    char *p, *p2;
    
    assert( ft->next_depth_string < ft->depth_string_count );
    result = &ft->depth_strings[ ft->depth_string_length * (ft->next_depth_string++) ];

    result[0] = static_cast<char>('0' + r->depth);
    result[1] = '\0';
    p = &result[1];

    if( r->adjacent_region_count != 1 ){

        for( i=0; i < r->adjacent_region_count; ++i ){
            adjacent = r->adjacent_regions[i];
            if( adjacent->level == TRAVERSED
                    && adjacent->descendent_count < r->descendent_count ){

                adjacent->depth_string = build_left_heavy_depth_string( ft, adjacent );
            }else{
                adjacent->depth_string = 0;
            }
        }

        qsort( r->adjacent_regions, r->adjacent_region_count, sizeof(FidSegRegion*), depth_string_cmp ); 
        
        for( i=0; i < r->adjacent_region_count; ++i ){
            FidSegRegion *adjacent = r->adjacent_regions[i];
            if( adjacent->depth_string ){
                p2 = adjacent->depth_string;
                while( *p2 )
                    *p++ = *p2++;

                adjacent->depth_string = 0;
            }
        }

        *p = '\0';        
    }

    return result;
}


static void print_unordered_depth_string( FidSegRegion *r )
{
    int i;
    FidSegRegion *adjacent;

    //printf( "(%d", r->depth );
    if( r->adjacent_region_count != 1 ){
        
        for( i=0; i < r->adjacent_region_count; ++i ){
            adjacent = r->adjacent_regions[i];
            if( adjacent->level == TRAVERSED
                    && adjacent->descendent_count < r->descendent_count ){

                print_unordered_depth_string( adjacent );
            }
        }
    }

    //printf( ")" );
}


static void compute_fiducial_statistics( FidtrackerX *ft, FiducialX *f,
        FidSegRegion *r, int width, int height )
{
    double all_x, all_y;
    double black_x, black_y;
    char *depth_string;
    
    ft->black_x_sum = 0.;
    ft->black_y_sum = 0.;
    ft->black_leaf_count = 0.;
    ft->white_x_sum = 0.;
    ft->white_y_sum = 0.;
    ft->white_leaf_count = 0.;

    ft->min_leaf_width_or_height = 0x7FFFFFFF;

    set_depth( r, 0 );
    
    sum_leaf_centers( ft, r, width, height );

    all_x = static_cast<double>(ft->black_x_sum + ft->white_x_sum) / static_cast<double>(ft->black_leaf_count + ft->white_leaf_count);
    all_y = static_cast<double>(ft->black_y_sum + ft->white_y_sum) / static_cast<double>(ft->black_leaf_count + ft->white_leaf_count);

    black_x = static_cast<double>(ft->black_x_sum) / static_cast<double>(ft->black_leaf_count);
    black_y = static_cast<double>(ft->black_y_sum) / static_cast<double>(ft->black_leaf_count);

    f->x = static_cast<float>(all_x);
    f->y = static_cast<float>(all_y);
    f->angle = static_cast<float>((M_PI * 2) - calculate_angle( all_x - black_x, all_y - black_y ));

/*
    print_unordered_depth_string( r );
    printf( "\n" );
    fflush( stdout );
*/

    assert( r->depth == 0 );
    assert( r->descendent_count >= ft->min_target_root_descendent_count );
    assert( r->descendent_count <= ft->max_target_root_descendent_count );

    if( ft->min_leaf_width_or_height >= LEAF_GATE_SIZE ){
        ft->next_depth_string = 0;
        depth_string = build_left_heavy_depth_string( ft, r );

        ft->temp_coloured_depth_string[0] = ( r->colour ? 'w' : 'b' );
        ft->temp_coloured_depth_string[1] = '\0';
        strcat( ft->temp_coloured_depth_string, depth_string );

        f->id = treestring_to_id( ft->treeidmap, ft->temp_coloured_depth_string );
                
    }else{
        f->id = INVALID_FIDUCIAL_ID;
    }
}


/* -------------------------------------------------------------------------- */


#define MAX( a, b ) (((a)>(b))?(a):(b))

// traverse downwards from r setting the depth value of each visited node
// depends on all nodes having assigned adjacent->descendent_count values
// to know which nodes to visit
static void set_depth( FidSegRegion *r, short depth )
{
    int i;
    short child_depth = static_cast<short>(depth + 1);

    r->depth = depth;
    
    if( r->adjacent_region_count != 1 ){  // if not a leaf
        for( i=0; i < r->adjacent_region_count; ++i ){
            FidSegRegion *adjacent = r->adjacent_regions[i];
            if( adjacent->descendent_count < r->descendent_count )
               set_depth( adjacent, child_depth );
        }
    }
}


#ifndef NDEBUG
static int r1_adjacent_contains_r2( FidSegRegion* r1, FidSegRegion* r2 )
{
    int i;

    for( i=0; i < r1->adjacent_region_count; ++i ){
        FidSegRegion *adjacent = r1->adjacent_regions[i];
        if( adjacent == r2 )
            return 1;
    }
    return 0;
}
#endif


// recursively propagate descendent count and max depth upwards
// r->visit_counter is incremented each time we have an opportunity
// to traverse to a parent. we only actually visit it once the visit
// counter reaches adjacent_region_count - 1 i.e. that we have already
// visited all of its other children
// as we travel upwards we collect nodes with the constraints defined by
// min_target_root_descendent_count, max_target_root_descendent_count,
// min_depth and max_depth
// we never traverse above a node which exceeds these constraints. we also
// never traverse nodes which are saturated or fragmented because it is
// ambiguous whether such a node has a parent, or if all it's children are
// attched, and we can't determine this in a single pass (we could save a list
// of these nodes for a later pass but we don't bother.)
// during the calls to this function we store the maximum leaf-to-node depth
// in r->depth, later this field has a different meaning
static void propagate_descendent_count_and_max_depth_upwards(
        Segmenter *s, FidSegRegion *r, FidtrackerX *ft )
{
    int i;
    FidSegRegion *parent = 0;
    assert( r->level == NOT_TRAVERSED );
    assert( r->children_visited_count == (r->adjacent_region_count - 1) );
        
    r->descendent_count = 0;
    r->depth = 0;
    r->level = TRAVERSING;
    
    for( i=0; i < r->adjacent_region_count; ++i ){
        FidSegRegion *adjacent = r->adjacent_regions[i];
        assert( r1_adjacent_contains_r2( adjacent, r ) );
        
        if( adjacent->level == TRAVERSED ){
            r->descendent_count += static_cast<short>(adjacent->descendent_count + 1);
            r->depth = static_cast<short>(MAX( r->depth, (adjacent->depth + 1) ));
        }else{
            assert( parent == 0 );
            parent = adjacent;
        }
    }

    r->level = TRAVERSED;






    if( r->descendent_count == ft->max_target_root_descendent_count
            && r->depth >= ft->min_depth && r->depth <= ft->max_depth ){

        // found fiducial candidate
        link_region( &ft->root_regions_head, r );
    
    }else{

        if( r->descendent_count >= ft->min_target_root_descendent_count
            && r->descendent_count < ft->max_target_root_descendent_count
            && r->depth >= ft->min_depth && r->depth <= ft->max_depth ){

            // found fiducial candidate
            link_region( &ft->root_regions_head, r );
        }


        if( parent
                && !(r->flags & (   SATURATED_REGION_FLAG |
                                    ADJACENT_TO_ROOT_REGION_FLAG |
                                    FREE_REGION_FLAG ) ) ){
                                    
            ++parent->children_visited_count;

            if( r->descendent_count < ft->max_target_root_descendent_count
                    && r->depth < ft->max_depth ){

                // continue propagating depth and descendent count upwards
                // so long as parent isn't a saturated node in which case it is
                // ambiguous whether parent has a parent or not so we skip it

                if(
                    !(parent->flags & SATURATED_REGION_FLAG)
                    &&
                    ( (!(parent->flags & (ADJACENT_TO_ROOT_REGION_FLAG | FRAGMENTED_REGION_FLAG) )
                        && parent->children_visited_count == (parent->adjacent_region_count - 1))
                    ||
                    ((parent->flags & (ADJACENT_TO_ROOT_REGION_FLAG | FRAGMENTED_REGION_FLAG) )
                        && parent->children_visited_count == parent->adjacent_region_count) )
                        ){
                
                    assert( r1_adjacent_contains_r2( r, parent ) );
                    assert( r1_adjacent_contains_r2( parent, r ) );

                    propagate_descendent_count_and_max_depth_upwards( s, parent, ft );
                }
            }
        }
    }
}





#ifndef NDEBUG
void sanity_check_region_initial_values( Segmenter *s )
{
    int i;
    for( i=0; i < s->region_count; ++i ){
        FidSegRegion *r = LOOKUP_SEGMENTER_REGION( s, i );

        assert( r->level == NOT_TRAVERSED );
        assert( r->children_visited_count == 0 );
        assert( r->descendent_count == 0x7FFF );
    }
}
#endif


static void find_roots( Segmenter *s, FidtrackerX *ft )
{
    int i;

    // we depend on the segmenter initializing certain region fields for us
    // check that here
#ifndef NDEBUG
    sanity_check_region_initial_values( s );
#endif

    // find fiducial roots beginning at leafs
    
    for( i=0; i < s->region_count; ++i ){
        FidSegRegion *r = LOOKUP_SEGMENTER_REGION( s, i );

        if( r->adjacent_region_count == 1
                && !(r->flags & (   SATURATED_REGION_FLAG |
                                    FRAGMENTED_REGION_FLAG |
                                    ADJACENT_TO_ROOT_REGION_FLAG |
                                    FREE_REGION_FLAG ) )
                ){

            assert( r->level == NOT_TRAVERSED );
            assert( r->children_visited_count == 0 );
            propagate_descendent_count_and_max_depth_upwards( s, r, ft );
        }
    }
}       


/* -------------------------------------------------------------------------- */


void initialize_fidtrackerX( FidtrackerX *ft, TreeIdMap *treeidmap, FloatPoint *pixelwarp )
{
    ft->min_target_root_descendent_count = treeidmap->min_node_count - 1;
    ft->max_target_root_descendent_count = treeidmap->max_node_count - 1;
    ft->min_depth = treeidmap->min_depth;
    ft->max_depth = treeidmap->max_depth;
    
    ft->depth_string_count = treeidmap->max_node_count;
    ft->depth_string_length = treeidmap->max_node_count + 1;
    ft->depth_strings = (char*)malloc( ft->depth_string_count * ft->depth_string_length );

    ft->temp_coloured_depth_string = (char*)malloc( ft->depth_string_length + 1 ); // includes space for colour prefix

    ft->treeidmap = treeidmap;
    ft->pixelwarp = pixelwarp;
}


void terminate_fidtrackerX( FidtrackerX *ft )
{
    free( ft->depth_strings );
    free( ft->temp_coloured_depth_string );
}


int find_fiducialsX( FiducialX *fiducials, int count,
        FidtrackerX *ft, Segmenter *segments, int width, int height )
{
    int i = 0;
    FidSegRegion *next;

    initialize_head_region( &ft->root_regions_head );

    find_roots( segments, ft );

    next = ft->root_regions_head.next;
    while( next != &ft->root_regions_head ){
        
        compute_fiducial_statistics( ft, &fiducials[i], next, width, height );

        next = next->next;
        ++i;
        if( i >= count )
            return i;
    }

    return i;
}

























