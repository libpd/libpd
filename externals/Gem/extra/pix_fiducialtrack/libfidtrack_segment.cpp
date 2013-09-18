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

#include "libfidtrack_segment.h"

#include <stdlib.h>
#include <assert.h>


/* -------------------------------------------------------------------------- */

void initialize_head_region( FidSegRegion *r )
{
    r->next = r->previous = r;
}


void link_region( FidSegRegion *head, FidSegRegion* r )
{
    r->previous = head->previous;
    r->next = head;

    r->previous->next = r;
    r->next->previous = r;
}


void unlink_region( FidSegRegion* r )
{
    r->previous->next = r->next;
    r->next->previous = r->previous;
}


/* -------------------------------------------------------------------------- */


static FidSegRegionReference* new_region( Segmenter *s, int x, int y, int colour )
{
    FidSegRegionReference *result;
    FidSegRegion *r;

    if( s->freed_regions_head ){
        r = s->freed_regions_head;
        s->freed_regions_head = r->next;
    }else{
        r = LOOKUP_SEGMENTER_REGION( s, s->region_count++ );
    }

	assert( colour == 0 || colour == 255 );

    r->colour = colour;

    r->left = r->right = static_cast<short>(x);
    r->top = r->bottom = static_cast<short>(y);

    r->flags = NO_REGION_FLAG;
    
    r->level = UNKNOWN_REGION_LEVEL;
    r->depth = 0;
    r->children_visited_count = 0;
    r->descendent_count = 0x7FFF;

    r->adjacent_region_count = 0;

    result = &s->region_refs[ s->region_ref_count++ ];
    result->redirect = result;
    result->region = r;
    
    return result;
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


static int is_adjacent( FidSegRegion* r1, FidSegRegion* r2 )
{
/*
    int i, adjacent_region_count;
    FidSegRegion **adjacent_regions, *r;

    if( r1->adjacent_region_count > r2->adjacent_region_count ){
        r = r2;
        adjacent_regions = r1->adjacent_regions;
        adjacent_region_count = r1->adjacent_region_count;
    }else{
        r = r1;
        adjacent_regions = r2->adjacent_regions;
        adjacent_region_count = r2->adjacent_region_count;
    }

    for( i=0; i < adjacent_region_count; ++i )
        if( adjacent_regions[i] == r )
            return 1;
*/

    int i;
    
    // choose the region with the shorter list for iteration
    if( r1->adjacent_region_count > r2->adjacent_region_count ){
        FidSegRegion *temp = r1;
        r1 = r2;
        r2 = temp;
    }

    for( i=0; i < r1->adjacent_region_count; ++i )
        if( r1->adjacent_regions[i] == r2 )
            return 1;

    return 0;
}


static void make_adjacent( Segmenter *s, FidSegRegion* r1, FidSegRegion* r2 )
{
    if( !is_adjacent( r1, r2 ) ){
        if( r1->adjacent_region_count < s->max_adjacent_regions ){
            if( r2->adjacent_region_count < s->max_adjacent_regions ){
                r1->adjacent_regions[ r1->adjacent_region_count++ ] = r2;
                r2->adjacent_regions[ r2->adjacent_region_count++ ] = r1;
            }else{
                r1->flags |= FRAGMENTED_REGION_FLAG;
                r2->flags |= SATURATED_REGION_FLAG;
            }
        }else{
            r1->flags |= SATURATED_REGION_FLAG;
            if( r2->adjacent_region_count < s->max_adjacent_regions )
                r2->flags |= FRAGMENTED_REGION_FLAG;
            else
                r2->flags |= SATURATED_REGION_FLAG;
        }
    }

    // postcondition, adjacency link is bi-directional or isn't created
    assert(
        (r1_adjacent_contains_r2( r1, r2 ) && r1_adjacent_contains_r2( r2, r1 ))
        ||
        (!r1_adjacent_contains_r2( r1, r2 ) && !r1_adjacent_contains_r2( r2, r1 ))
    );
}


static
#ifdef NDEBUG
void
#else
int
#endif
remove_adjacent_from( FidSegRegion *r1, FidSegRegion *r2 )
{
    int i;
    int found = 0;
    
    for( i = 0; i < r1->adjacent_region_count; ++i ){
        if( r1->adjacent_regions[i] == r2 ){
            found = 1;
            break;
        }
    }

    if( found )
        r1->adjacent_regions[i] = r1->adjacent_regions[ --r1->adjacent_region_count ];

#ifndef NDEBUG
    return found;
#endif
}


// merge r2 into r1 by first removing all common adjacencies from r2
// then transferring the remainder of adjacencies to r1 after discarding
// any excess adjacencies.
static void merge_regions( Segmenter *s, FidSegRegion* r1, FidSegRegion* r2 )
{
    int i;
    int r2_adjacent_region_count = r2->adjacent_region_count; // cache value in local in an attempt to improve efficiency
    
    assert( r1 != r2 );
    assert( !r1_adjacent_contains_r2( r1, r2 ) && !r1_adjacent_contains_r2( r2, r1 ) );
    
    // remove adjacencies of r2 which are already in r1
    for( i = 0; i < r2_adjacent_region_count; ++i ){
        FidSegRegion *a = r2->adjacent_regions[i];
        if( is_adjacent( a, r1 ) ){
#ifndef NDEBUG
            int found =
#endif
            remove_adjacent_from( a, r2 );
            assert( found );
            r2->adjacent_regions[i] = r2->adjacent_regions[ --r2_adjacent_region_count ];
        }
    }

    
    // reduce the sum of adjacent of r1 and r2 to within s->max_adjacent_regions
    // by removing adjacencies from r2
    if( r1->adjacent_region_count + r2_adjacent_region_count > s->max_adjacent_regions ){
        int new_r2_count = s->max_adjacent_regions - r1->adjacent_region_count;

        while( r2_adjacent_region_count > new_r2_count ){
            FidSegRegion *a = r2->adjacent_regions[ --r2_adjacent_region_count ];
#ifndef NDEBUG
            int found =
#endif
            remove_adjacent_from( a, r2 );
            assert( found );
            a->flags |= FRAGMENTED_REGION_FLAG;
        }

        r1->flags |= SATURATED_REGION_FLAG;
    }

    // remove the remainder of adjacencies from r2 and add them to r1

    while( r2_adjacent_region_count > 0 ){
        FidSegRegion *a = r2->adjacent_regions[ --r2_adjacent_region_count ];
        
#ifndef NDEBUG
        int found =
#endif
        remove_adjacent_from( a, r2 );
        assert( found );

        assert( r1->adjacent_region_count < s->max_adjacent_regions );
        r1->adjacent_regions[r1->adjacent_region_count++] = a;
        assert( a->adjacent_region_count < s->max_adjacent_regions );
        a->adjacent_regions[a->adjacent_region_count++] = r1;
    }

    r2->adjacent_region_count = 0;

    r1->flags |= r2->flags;

    if( r2->left < r1->left )
        r1->left = r2->left;

    if( r2->top < r1->top )
        r1->top = r2->top;

    if( r2->right > r1->right )
        r1->right = r2->right;

    if( r2->bottom > r1->bottom )
        r1->bottom = r2->bottom;

    // postcondition: all adjacencies to r1 are bidirectional

#ifndef NDEBUG
    for( i = 0; i < r1->adjacent_region_count; ++i ){
        FidSegRegion *a = r1->adjacent_regions[i];
        assert( r1_adjacent_contains_r2( r1, a ) );
    }
#endif
}


static void build_regions( Segmenter *s, const unsigned char *source, int width, int height )
{
    int x, y, i;
    FidSegRegionReference **current_row = &s->regions_under_construction[0];
    FidSegRegionReference **previous_row = &s->regions_under_construction[width];
    
    s->region_ref_count = 0;
    s->region_count = 0;
    s->freed_regions_head = 0;

    // top line

    x = 0;
    y = 0;
    current_row[0] = new_region( s, x, y, source[0] );
    current_row[0]->region->flags |= ADJACENT_TO_ROOT_REGION_FLAG;
    for( x=1, y=0, i=1 ; x < width; ++x, ++i ){

        if( source[i] == source[i-1] ){
            current_row[x] = current_row[x-1];
        }else{
            current_row[x] = new_region( s, x, y, source[i] );
            current_row[x]->region->flags |= ADJACENT_TO_ROOT_REGION_FLAG;
            make_adjacent( s, current_row[x]->region, current_row[x-1]->region );
        }
    }

    // process lines
    
    for( y=1; y < height; ++y ){

        // swap previous and current rows
        FidSegRegionReference **temp = previous_row;
        previous_row = current_row;
        current_row = temp;

        i = y * width;
        x = 0;
        
        // left edge

        RESOLVE_REGIONREF_REDIRECTS( previous_row[x], previous_row[x] );
        if( source[i] == previous_row[x]->region->colour ){
            current_row[x] = previous_row[x];
            
        }else{ // source[i] != previous_row[x]->colour

            current_row[x] = new_region( s, x, y, source[i] );
            current_row[x]->region->flags |= ADJACENT_TO_ROOT_REGION_FLAG;
            make_adjacent( s, current_row[x]->region, previous_row[x]->region );
        }
        
        ++i;
        x=1;

        // center span

        for( ; x < width; ++x, ++i ){
            //RESOLVE_REGIONREF_REDIRECTS( current_row[x-1], current_row[x-1] );   // this isn't needed because the the west cell's redirect is always up to date
            RESOLVE_REGIONREF_REDIRECTS( previous_row[x], previous_row[x] );

            if( source[i] == source[i-1] ){
            
                current_row[x] = current_row[x-1];

                if( current_row[x] != previous_row[x]
                        && source[i] == previous_row[x]->region->colour ){

                    // merge the current region into the previous one
                    // this is much more efficient than merging the previous
                    // into the current because it keeps long-lived regions
                    // alive and only frees newer (less connected?) ones
                    merge_regions( s, previous_row[x]->region, current_row[x]->region );
                    current_row[x]->region->flags = FREE_REGION_FLAG;
                    current_row[x]->region->next = s->freed_regions_head;
                    s->freed_regions_head = current_row[x]->region;
                    current_row[x]->region = 0;
                    current_row[x]->redirect = previous_row[x];
                    current_row[x] = previous_row[x];
                }


            }else{ // source_image_[i] != source_image_[i-1]

                if( current_row[x-1]->region->right < x - 1 )
                    current_row[x-1]->region->right = static_cast<short>( x - 1 );

                if( source[i] == previous_row[x]->region->colour ){
                    current_row[x] = previous_row[x];
                    current_row[x]->region->bottom = static_cast<short>(y);

                }else{
                    current_row[x] = new_region( s, x, y, source[i] );
                    make_adjacent( s, current_row[x]->region, previous_row[x]->region );
                    if( current_row[x-1]->region != previous_row[x]->region )
                        make_adjacent( s, current_row[x]->region, current_row[x-1]->region );
                }
            }
        }

        // right edge
        current_row[width-1]->region->flags |= ADJACENT_TO_ROOT_REGION_FLAG;
    }

    // make regions of bottom row adjacent or merge with root
    
    for( x = 0; x < width; ++x ){
        RESOLVE_REGIONREF_REDIRECTS( current_row[x], current_row[x] );
        current_row[x]->region->flags |= ADJACENT_TO_ROOT_REGION_FLAG;
    }
}


/* -------------------------------------------------------------------------- */


void initialize_segmenter( Segmenter *s, int width, int height, int max_adjacent_regions )
{
    s->max_adjacent_regions = max_adjacent_regions;
    s->region_refs = (FidSegRegionReference*)malloc( sizeof(FidSegRegionReference) * width * height );
    s->region_ref_count = 0;
    s->sizeof_region = sizeof(FidSegRegion) + sizeof(FidSegRegion*) * (max_adjacent_regions-1);
    s->regions = (unsigned char*)malloc( s->sizeof_region * width * height );
    s->region_count = 0;

    s->regions_under_construction = (FidSegRegionReference**)malloc( sizeof(FidSegRegionReference*) * width * 2 );
}


void terminate_segmenter( Segmenter *s )
{
    free( s->region_refs );
    free( s->regions );
    free( s->regions_under_construction );
}


void step_segmenter( Segmenter *s, const unsigned char *source, int width, int height  )
{
    if(  s->region_refs && s->regions && s->regions_under_construction )
        build_regions( s, source, width, height );
}



