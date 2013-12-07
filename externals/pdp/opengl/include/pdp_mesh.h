/*
 *   Pure Data Packet module. mesh object specification
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* a very naive approach to triangular meshes */

#ifndef PDP_MESH_H
#define PDP_MESH_H
#include "pdp_list.h"


/* VERTEX type 
   a vertex has a coordinate and normal vector
   a list of triangles it is connected to 
   and a list of vertexes it is connected to (these count as links) */

typedef struct _vertex
{
    float c[3]; // coordinates
    float n[3]; // normal (or force vector)
    t_pdp_list *trilist;
    t_pdp_list *vertlist;
} t_vertex;


/* TRIANGLE type:
   a triangle consist of 
   - 3 vertices
   - 3 optional median vertices (subdivision intermediates) */
typedef struct _triangle
{
    t_vertex *v[3]; // vertices
    t_vertex *m[3]; // median vertices
    float n[3]; //triangle normal
} t_triangle;


/* MESH type:
   a mesh is a list of vertices
   and a list of triangles (connections) */
typedef struct _mesh
{
    t_pdp_list *triangles;
    t_pdp_list *vertices;
    int normal_type;
    int refine_type;
} t_mesh;



/* object configuratie */
#define MESH_NORMAL_SPHERE  1  // normal = origin -> vertex
#define MESH_NORMAL_PRISM   2  // normal = center of gravity of prism base -> vertex
#define MESH_NORMAL_RANDOM  3  // normal = random vector
#define MESH_NORMAL_AVERAGE 4  // normal = average of surrounding triangles

/* refinement method */
#define MESH_REFINE_THREE 1   // triangle -> 3 triangles connecting center of gravity
#define MESH_REFINE_FOUR  2   // triangle -> 4 triangles connecting link medians

// vector utility stuff

// fixed size iterators for the lazy
#define I3(i) for(i=0; i<3; i++)
#define I4(i) for(i=0; i<4; i++)

static inline float _vector3_dot(float *v0, float *v1)
{
    float d;
    d =  v0[0] * v1[0];
    d += v0[1] * v1[1];
    d += v0[2] * v1[2];
    return d;
}

static inline void _vector3_scale(float *v, float s)
{
    int k;
    I3(k) v[k] *= s;
}

static inline float _vector3_normalize(float *v)
{
    float length = 0;
    float scale = 0;
    int k;
    length = sqrt(_vector3_dot(v,v));
    scale = 1.0f / length;
    _vector3_scale(v, scale);
    return length;
}

static inline void _vector3_cross(float *a, float *b, float *r)
{
    r[0] = a[1]*b[2] - a[2]*b[1];
    r[1] = a[2]*b[0] - a[0]*b[2];
    r[2] = a[0]*b[1] - a[1]*b[0];
}

static inline float _rand(void)
{
    long int r = random();
    float f;
    r -= (RAND_MAX >> 1);
    f = (float)r;
    f *= (2.0f / (float)RAND_MAX);
    return f;
}




/* VERTEX methods */
void vertex_add_triangle(t_vertex *v, t_triangle *t);
void vertex_remove_triangle(t_vertex *v, t_triangle *t);
void vertex_add_neighbour(t_vertex *v, t_vertex *neighbour);


/* constructor/destructors are "private"
   they may only be called by the mesh object to ensure
   the vector list stays sound (i.e. without duplicates) */
void _vertex_free(t_vertex *v);
t_vertex *_vertex_new(float *c, float *n);


void vertex_compute_normal_random(t_vertex *v);
void vertex_compute_normal_sphere(t_vertex *v);
void vertex_compute_normal_prism(t_vertex *v);
float vertex_normalize(t_vertex *v);


/* TRIANGLE methods */

/* create triangle (a connection between 3 vertices): 
   counterclockwize with facing front
   this method is "private"
   you can only create triangles as part of a mesh */
t_triangle *_triangle_new(t_vertex *v0, t_vertex *v1, t_vertex *v2);

/* delete a triangle, disconnecting the vertices */
void _triangle_free(t_triangle *t);

/* get triangle that shares the link between v0 and v1 */
t_triangle *triangle_neighbour(t_triangle *t, t_vertex *v0, t_vertex *v1);

/* add a median vector to a link in a triangle 
   note: vertices must be in triangle, or behaviour is undefined */
void triangle_add_median(t_triangle *t, t_vertex *v0, t_vertex *v1, t_vertex *median);

/* MESH methods */

/* add and remove methods for vertices and triangles */
t_vertex *mesh_vertex_add(t_mesh *m, float *c, float *n);
void mesh_vertex_remove(t_mesh *m, t_vertex *v);
t_triangle *mesh_triangle_add(t_mesh *m, t_vertex *v0, t_vertex *v1, t_vertex *v2);
void mesh_triangle_remove(t_mesh *m, t_triangle *t);

/* calculate normals */
void mesh_calculate_normals(t_mesh *m);

/* split a triangle in 4, using the intermedia median vertex storage */
void mesh_split_four(t_mesh *m, t_triangle *old_t);

/* split a triangle in 3 */
void mesh_split_three(t_mesh *m, t_triangle *old_t);

void mesh_split_all_four(t_mesh *m);

void mesh_split_all_three(t_mesh *m);

void mesh_split_random_three(t_mesh *m);

void mesh_free(t_mesh *m);

t_mesh *_mesh_new(void);

/* new tetra */
t_mesh *mesh_new_tetra(void);



void _mesh_relax_compute_resultant_spring(t_mesh *m, float *center, float d0, float r0);
void _mesh_relax_apply_force(t_mesh *m, float k);
void mesh_compute_center(t_mesh *m, float *c);
void mesh_translate(t_mesh *m, float *c);

/* relax a mesh (move toward equal link length) */
void mesh_relax(t_mesh *m, float step, float d0, float r0);

/* print some debug information */
void mesh_debug(t_mesh *m);


#endif
