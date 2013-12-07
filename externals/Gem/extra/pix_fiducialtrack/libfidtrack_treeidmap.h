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

#ifndef INCLUDED_TREEIDMAP_H
#define INCLUDED_TREEIDMAP_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


typedef struct TreeIdMap{
    void *implementation_;

    int tree_count;
    int min_node_count, max_node_count;
    int min_depth, max_depth;

    int max_adjacencies;

}TreeIdMap;

void initialize_treeidmap_from_file( TreeIdMap* treeidmap, const char *file_name );

void terminate_treeidmap( TreeIdMap* treeidmap );

#define INVALID_TREE_ID     (-1)

// returns INVALID_TREE_ID for unfound id
int treestring_to_id( TreeIdMap* treeidmap, const char *treestring );


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* INCLUDED_TREEIDMAP_H */
