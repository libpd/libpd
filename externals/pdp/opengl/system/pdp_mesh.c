/*
 *   Pure Data Packet module. mesh implementation
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


// $$TODO: some serious memory corruption in this file our the list implementation


#include <math.h>

#include "pdp.h"
#include "pdp_mesh.h"


/* VERTEX methods */
void vertex_add_triangle(t_vertex *v, t_triangle *t)
{
    pdp_list_add_pointer(v->trilist, t);
}
void vertex_remove_triangle(t_vertex *v, t_triangle *t)
{
    pdp_list_remove_pointer(v->trilist, t);
}
void vertex_add_neighbour(t_vertex *v, t_vertex *neighbour)
{
    pdp_list_add_pointer_to_set(v->vertlist, neighbour);
};


/* constructor/destructors are "private"
   they may only be called by the mesh object to ensure
   the vector list stays sound (i.e. without duplicates) */
void _vertex_free(t_vertex *v)
{
    if (!v->trilist) post("WARNING: vertex %x has empty trilist", v);
    else{
        pdp_list_free(v->trilist);
	v->trilist = 0;
    }
    if (!v->vertlist) post("WARNING: vertex %x has empty vertlist", v);
    {
	pdp_list_free(v->vertlist);
	v->vertlist = 0;
    }
    pdp_dealloc(v);
}

t_vertex *_vertex_new(float *c, float *n)
{
    int k;
    t_vertex *v = (t_vertex *)  pdp_alloc(sizeof(t_vertex));
    I3(k) v->c[k] = c[k];
    I3(k) v->n[k] = n[k];
    v->trilist = pdp_list_new(0);
    v->vertlist = pdp_list_new(0);
    return v;
}


void vertex_compute_normal_random(t_vertex *v){int k; I3(k) v->n[k] = _rand();}
void vertex_compute_normal_sphere(t_vertex *v){int k; I3(k) v->n[k] = v->c[k];}
void vertex_compute_normal_prism(t_vertex *v)
{
    float scale = 0.0f;
    float sum[] = {0.0f, 0.0f, 0.0f};
    int k;
    t_pdp_atom* i;
    t_pdp_list *vl = v->vertlist;
    t_vertex *vtx;

    PDP_POINTER_IN(vl, i, vtx) {
	I3(k) sum[k] += vtx->c[k];
	scale = scale + 1.0f;
    }
    scale = 1.0f / scale;
    I3(k) sum[k] *= scale;
    I3(k) v->n[k] = v->c[k] - sum[k];
	
    //post("computed normal (%f, %f, %f) of vertex (%f, %f, %f)", v->n[0], v->n[1], v->n[2], v->c[0], v->c[1], v->c[2]);
};
void vertex_compute_normal_average(t_vertex *v)
{
    int triangles = pdp_list_size(v->trilist);
    float scale = 1.0f / ((float)triangles);
    t_pdp_atom* i;
    int k;
    t_triangle *t;

    I3(k) v->n[k] = 0; //reset normal
    PDP_POINTER_IN(v->trilist, i, t){
	I3(k) v->n[k] += t->n[k];
    }
    _vector3_scale(v->n, scale);
    
    
}


float vertex_normalize(t_vertex *v)
{
    return _vector3_normalize(v->c);
}


/* TRIANGLE methods */

/* create triangle (a connection between 3 vertices): 
   counterclockwize with facing front
   this method is "private"
   you can only create triangles as part of a mesh */
t_triangle *_triangle_new(t_vertex *v0, t_vertex *v1, t_vertex *v2)
{
    int k;

    t_triangle *t = (t_triangle *)pdp_alloc(sizeof(t_triangle));

    /* store vertex references */
    t->v[0] = v0;
    t->v[1] = v1;
    t->v[2] = v2;

    /* reset median vertices */
    I3(k) t->m[k] = 0;

    /* connect triangle to vertices */
    vertex_add_triangle(v0, t);
    vertex_add_triangle(v1, t);
    vertex_add_triangle(v2, t);

    /* connect vertices to vertices */
    vertex_add_neighbour(v0, v1);
    vertex_add_neighbour(v0, v2);
    vertex_add_neighbour(v1, v0);
    vertex_add_neighbour(v1, v2);
    vertex_add_neighbour(v2, v0);
    vertex_add_neighbour(v2, v1);
    
    return t;
}

/* delete a triangle, disconnecting the vertices */
void _triangle_free(t_triangle *t)
{
    int k;

    /* remove the triangle reference of the vertices */
    I3(k) vertex_remove_triangle(t->v[k], t);

    /* set references to zero (bug catcher) */
    I3(k) t->v[k] = 0;
    I3(k) t->m[k] = 0;

    /* free struct */
    pdp_dealloc(t);
    
}

/* get triangle that shares the link between v0 and v1 */
t_triangle *triangle_neighbour(t_triangle *t, t_vertex *v0, t_vertex *v1)
{
    t_pdp_atom* it;
    t_triangle *tri;
    PDP_POINTER_IN(v1->trilist, it, tri){
	if (tri != t && pdp_list_contains_pointer(v0->trilist, tri)) return tri;
    }
    return 0;
}

/* add a median vector to a link in a triangle 
   note: vertices must be in triangle, or behaviour is undefined */
void triangle_add_median(t_triangle *t, t_vertex *v0, t_vertex *v1, t_vertex *median)
{

    /* link 0 1 */
    if (!((v0 == t->v[2]) || (v1 == t->v[2]))) t->m[0] = median;

    /* link 1 2 */
    else if (!((v0 == t->v[0]) || (v1 == t->v[0]))) t->m[1] = median;

    /* link 2 0 */
    else t->m[2] = median;
}

void triangle_compute_normal(t_triangle *t)
{
    int k;
    float v0[3];
    float v1[3];
    I3(k) v0[k] = t->v[1]->c[k] - t->v[0]->c[k];
    I3(k) v1[k] = t->v[2]->c[k] - t->v[0]->c[k];
    _vector3_cross(v0,v1,t->n);
}

void triangle_compute_unit_normal(t_triangle *t)
{
    triangle_compute_normal(t);
    _vector3_normalize(t->n);
}

/* MESH methods */

/* add and remove methods for vertices and triangles */
t_vertex *mesh_vertex_add(t_mesh *m, float *c, float *n)
{
    t_vertex *v = _vertex_new(c, n);
    pdp_list_add_pointer(m->vertices, v);
    return v;
}

void mesh_vertex_remove(t_mesh *m, t_vertex *v)
{
    pdp_list_remove_pointer(m->vertices, v);
    _vertex_free(v);
}

t_triangle *mesh_triangle_add(t_mesh *m, t_vertex *v0, t_vertex *v1, t_vertex *v2)
{
    t_triangle *t = _triangle_new(v0,v1,v2);
    pdp_list_add_pointer(m->triangles, t);
    return t;
}

void mesh_triangle_remove(t_mesh *m, t_triangle *t)
{
    pdp_list_remove_pointer(m->triangles, t);
    _triangle_free(t);
}

/* calculate normals */
void mesh_calculate_normals(t_mesh *m)
{
    t_pdp_atom* it;
    t_pdp_atom* it_tri;
    t_pdp_list *l = m->vertices;
    t_pdp_list *l_tri = m->triangles;
    t_vertex *v;
    t_triangle *t;
    //while (v = pdp_list_getnext_pointer(l, &it)) vertex_compute_normal_sphere(v);
    switch(m->normal_type){
    default:
    case MESH_NORMAL_SPHERE:  PDP_POINTER_IN(l, it, v) vertex_compute_normal_sphere(v); break;
    case MESH_NORMAL_PRISM:   PDP_POINTER_IN(l, it, v) vertex_compute_normal_prism(v); break;
    case MESH_NORMAL_RANDOM:  PDP_POINTER_IN(l, it, v) vertex_compute_normal_random(v); break;
    case MESH_NORMAL_AVERAGE: 
	PDP_POINTER_IN(l_tri, it_tri, t) triangle_compute_unit_normal(t); 
	PDP_POINTER_IN(l, it, v) vertex_compute_normal_average(v); 
	break;
    }
}

/* split a triangle in 4, using the intermedia median vertex storage */
void mesh_split_four(t_mesh *m, t_triangle *old_t)
{
    int k;
    t_vertex *v[6];

    /* some intermediates */
    t_triangle *neighbour;
    t_float newv[] = {0,0,0};
    t_float nullvect[] = {0,0,0};


    /* get main vertices */
    I3(k) v[k] = old_t->v[k];

    /* get median vertices inserted by neighbouring triangles */
    I3(k) v[k+3] = old_t->m[k];

#define GET_MEDIAN(v, v0, v1)					\
    if (!v){							\
	I3(k) newv[k] = 0.5f * (v0->c[k] + v1->c[k]);		\
	v = mesh_vertex_add(m, newv, nullvect);			\
	/*vertex_normalize(v);*/				\
	if (neighbour = triangle_neighbour(old_t, v0, v1)){	\
	    triangle_add_median(neighbour, v0, v1, v);		\
	}							\
    }
    
    GET_MEDIAN(v[3], v[0], v[1])
    GET_MEDIAN(v[4], v[1], v[2])
    GET_MEDIAN(v[5], v[2], v[0])

#undef GET_MEDIAN

    /* remove the old triangle */
    mesh_triangle_remove(m, old_t);

    /* create 4 new triangles */
    mesh_triangle_add(m, v[0], v[3], v[5]);
    mesh_triangle_add(m, v[1], v[4], v[3]);
    mesh_triangle_add(m, v[2], v[5], v[4]);
    mesh_triangle_add(m, v[3], v[4], v[5]);

}

/* split a triangle in 3 */
void mesh_split_three(t_mesh *m, t_triangle *old_t)
{
    int k, l;
    t_vertex *v[4];
    t_float newv[] = {0,0,0};
    t_float nullvect[] = {0,0,0};

    /* get vertices */
    I3(k) v[k] = old_t->v[k];

    /* remove a triangle */
    mesh_triangle_remove(m, old_t);

    /* compute new vertex coordinates */
    I3(k) I3(l) newv[k] += 0.33333f * v[l]->c[k];
    
    /* create new vertex */
    v[3] = mesh_vertex_add(m, newv, nullvect);
    //vertex_normalize(v[3]);

    /* create 3 new triangles */
    mesh_triangle_add(m, v[0], v[1], v[3]);
    mesh_triangle_add(m, v[1], v[2], v[3]);
    mesh_triangle_add(m, v[2], v[0], v[3]);

}



void mesh_split_all_four(t_mesh *m)
{
    t_triangle *t;
    t_pdp_list *l = pdp_list_copy(m->triangles);

    //post("split_all_four: nb triangles %d", pdp_list_size(m->triangles));

    while (l->elements){
	t = pdp_list_pop(l).w_pointer;
	mesh_split_four(m, t);
    }
    mesh_calculate_normals(m);
    pdp_list_free(l);
}


void mesh_split_all_three(t_mesh *m)
{
    t_triangle *t;
    t_pdp_list *l = pdp_list_copy(m->triangles);

    //post("split_all_three: nb triangles %d", pdp_list_size(m->triangles));

    while (l->elements){
	t = pdp_list_pop(l).w_pointer;
	mesh_split_three(m, t);
    }
    mesh_calculate_normals(m);
    pdp_list_free(l);
}

void mesh_split_random_three(t_mesh *m)
{
    int size = pdp_list_size(m->triangles);
    t_triangle *t = pdp_list_index(m->triangles, (random() % size)).w_pointer;
    mesh_split_three(m, t);
    mesh_calculate_normals(m);
}



void mesh_free(t_mesh *m)
{
    t_pdp_list *l;
    t_triangle *t;
    t_vertex *v;

    /* delete all triangles */
    while (m->triangles->elements){
	t = pdp_list_pop(m->triangles).w_pointer;
	//post("freeing triangle %x", t);
	_triangle_free(t);
    }
    pdp_list_free(m->triangles);
    m->triangles = 0;

    /* delete all vertices */
    while (m->vertices->elements){
	v = pdp_list_pop(m->vertices).w_pointer;
	//post("freeing vertex %x", v);
	_vertex_free(v);
    }
    pdp_list_free(m->vertices);
    m->vertices = 0;

    pdp_dealloc(m);

}


t_mesh *_mesh_new(void)
{
    t_mesh *m = (t_mesh *)pdp_alloc(sizeof(t_mesh));

    /* create main vertex and triangle lists */
    m->triangles = pdp_list_new(0);
    m->vertices = pdp_list_new(0);

    /* set normal type */
    m->normal_type = MESH_NORMAL_PRISM;

    return m;
}

/* init tetra */
t_mesh *mesh_new_tetra(void)
{
    int k;
    t_triangle *t[4];
    t_vertex *v[4];
    t_pdp_atom* it;
    t_triangle *tri;
    t_mesh *m = _mesh_new();

    float n[] = {0,0,0};
    float fv[4][3] = {{2,0,0},{0,2,0},{0,0,2}, {-1,-1,-1}};

    /* add vertices */
    I4(k) v[k] = mesh_vertex_add(m, &fv[k][0], n);
    I4(k) vertex_normalize(v[k]);

    /* add triangles */
    mesh_triangle_add(m, v[0], v[1], v[2]);
    mesh_triangle_add(m, v[1], v[0], v[3]);
    mesh_triangle_add(m, v[0], v[2], v[3]);
    mesh_triangle_add(m, v[1], v[3], v[2]);


    /* compute normals */
    mesh_calculate_normals(m);

    return m;
}


void _mesh_relax_compute_resultant_spring(t_mesh *m, float *center, float d0, float r0)
{
    int k;
    t_pdp_atom *i, *j;
    t_vertex *v, *w;
    
    PDP_POINTER_IN(m->vertices, i, v){
	float scale = 0.0f;
	float r;
	
	/* compute contribution of origin link */
	I3(k) v->n[k] = v->c[k] - center[k];
	r = _vector3_normalize(v->n);
	I3(k) v->n[k] *= (r0 - r);

	PDP_POINTER_IN(v->vertlist, j, w){
	    int k;
	    float f[3];
	    float d, l;

	    /* compute force contribution of one link (model: spring with rest length == d0) */
	    I3(k) f[k] = w->c[k] - v->c[k];        // PC: f == distance vector
	    d = _vector3_normalize(f);               // PC: d == distance, vector == unit norm
	    I3(k) v->n[k] += (d - d0) * f[k];      // PC: n == n_prev + fource resultant
	}
    }
}

void _mesh_relax_apply_force(t_mesh *m, float k)
{
    t_pdp_atom* it;
    t_vertex *v;
    
    PDP_POINTER_IN(m->vertices, it, v){
	int i;
	/* apply fource vector with step */
	I3(i) v->c[i] += k * v->n[i];
    }

}

void mesh_compute_center(t_mesh *m, float *c)
{
    t_pdp_atom*(it);
    t_vertex *v;
    float scale;
    int k;

    I3(k) c[k] = 0;
    PDP_POINTER_IN(m->vertices, it, v){
	I3(k) c[k] += v->c[k];
    }
    scale = 1.0f / ((float)pdp_list_size(m->vertices));
    I3(k) c[k] *= scale;

}

void mesh_translate(t_mesh *m, float *c)
{
    t_pdp_atom *it;
    t_vertex *v;
    int k;

    PDP_POINTER_IN(m->vertices, it, v){
	I3(k) v->c[k] += c[k];
    }
}

/* relax a mesh (move toward equal link length) */
void mesh_relax(t_mesh *m, float step, float d0, float r0)
{
    int k;
    float c[3];
    mesh_compute_center(m, c);
    I3(k) c[k] = -c[k];
    mesh_translate(m, c);
    I3(k) c[k] = 0;
    _mesh_relax_compute_resultant_spring(m, c, d0, r0); /* compute force resultant */
    _mesh_relax_apply_force(m, step);    /* apply "time step towards desired distance" */
    mesh_calculate_normals(m);        /* restore normals */
}



/* print some debug information */
void mesh_debug(t_mesh *m)
{
    int k;
    int boundary_edges = 0;
    t_pdp_atom* it;
    t_triangle *t;
    post("mesh info");
    post("\tnumber of vertices = %d", pdp_list_size(m->vertices));
    post("\tnumber of triangles = %d", pdp_list_size(m->triangles));

    PDP_POINTER_IN(m->triangles, it, t){
	I3(k) if (!triangle_neighbour(t, t->v[k], t->v[(k+1)%3])) boundary_edges++;
    }
    post("\tnumber of boundaray edges = %d", boundary_edges);

    
}
