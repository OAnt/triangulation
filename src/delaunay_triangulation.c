#include "private/vector.h"
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <private/array.h>
#include <private/debug.h>
#include <private/delaunay_triangulation.h>
#include <private/mesh.h>
#include <private/spatial_hash.h>
#include <private/triangle.h>
#include <public/common.h>

#define UNINDEXED_DELAUNAY
#ifndef UNINDEXED_DELAUNAY
#define INDEXED_DELAUNAY

struct indexed_mesh_st{
    mesh_st * mesh;
    size_t * handles;
    struct spatial_hash_st * sph;
    int32_t x_index;
    int32_t y_index;
};

void indexed_mesh_face_increment_bounds(
        struct indexed_mesh_st * mesh,
        size_t face_index,
        struct vector_st * min,
        struct vector_st * max)
{
    for(int32_t v = 0; v < FACE_SIZE; v++){
        size_t vertex_index = mesh->mesh->faces[face_index].f[v];
        struct vector_st * point = &mesh->mesh->vertices[vertex_index].point;
        if(point->v[mesh->x_index] > max->v[0])
            max->v[0] = point->v[mesh->x_index];
        if(point->v[mesh->y_index] > max->v[1])
            max->v[1] = point->v[mesh->y_index];
        if(point->v[mesh->x_index] < min->v[0])
            min->v[0] = point->v[mesh->x_index];
        if(point->v[mesh->y_index] < min->v[1])
            min->v[1] = point->v[mesh->y_index];
    }
}

enum error_code_e indexed_mesh_insert_into_spatial_hash(
        struct indexed_mesh_st * mesh,
        size_t face_index)
{
    G_ASSERT(face_index < array_length(mesh->mesh),
            "Face is out of bounds");
    G_ASSERT(mesh->handles[face_index] == INVALID_INDEX,
            "Face is already inserted");
    struct vector_st min = {{DBL_MAX, DBL_MAX, DBL_MAX}};
    struct vector_st max = {{-DBL_MAX, -DBL_MAX, -DBL_MAX}};
    indexed_mesh_face_increment_bounds(mesh, face_index, &min, &max);
    return spatial_hash_add(
            mesh->sph, min, max, face_index, &mesh->handles[face_index]);
}

void indexed_mesh_remove_from_spatial_hash(
        struct indexed_mesh_st * mesh,
        size_t face_index)
{
    G_ASSERT(face_index < array_length(mesh->mesh),
            "Face is out of bounds");
    G_ASSERT(mesh->handles[face_index] != INVALID_INDEX,
            "Face is already removed");
    spatial_hash_remove(mesh->sph, mesh->handles[face_index]);
    mesh->handles[face_index] = INVALID_INDEX;
}

enum error_code_e indexed_mesh_init_in_place_faces_as_boundaries(
        mesh_st * mesh,
        enum projection_plane_e pp,
        struct indexed_mesh_st * indexed_mesh)
{
    get_axis_system_from_projection_plane(
            pp, &indexed_mesh->x_index, &indexed_mesh->y_index);
    indexed_mesh->mesh = mesh; 
    size_t n_faces = array_length(mesh->faces);
    struct vector_st min = {{DBL_MAX, DBL_MAX, DBL_MAX}};
    struct vector_st max = {{-DBL_MAX, -DBL_MAX, -DBL_MAX}};
    for(size_t f = 0; f < n_faces; f++){
        indexed_mesh_face_increment_bounds(indexed_mesh, f, &min, &max);
    }
    static int32_t n_bkts = 10;
    double x_cell_size = (max.v[0] - min.v[0]) / n_bkts;
    double y_cell_size = (max.v[1] - min.v[1]) / n_bkts;
    enum error_code_e err = array_new(
            size_t, n_faces, &indexed_mesh->handles);
    if(err != ec_no_error) goto fail_no_handles;
    err = spatial_hash_new(
            n_bkts, n_bkts, x_cell_size, y_cell_size, &indexed_mesh->sph);
    if(err != ec_no_error) goto fail_no_sph;
    for(size_t f = 0; f < n_faces; f++){
        indexed_mesh->handles[f] = INVALID_INDEX;
        err = indexed_mesh_insert_into_spatial_hash(indexed_mesh, f);
        if(err != ec_no_error) goto fail_face_insert;
    }
    return ec_no_error;
fail_face_insert:
    spatial_hash_delete(&indexed_mesh->sph);
fail_no_sph:
    array_delete(&indexed_mesh->handles);
fail_no_handles:
    return err;
}

void indexed_mesh_cleanup(
        struct indexed_mesh_st * mesh)
{
    spatial_hash_delete(&mesh->sph);
    array_delete(&mesh->handles);
    memset(mesh, 0, sizeof(struct indexed_mesh_st));
}

enum error_code_e indexed_mesh_add_face(
        struct indexed_mesh_st * mesh,
        struct face_st face,
        size_t * index)
{
    G_ASSERT(index != NULL, "This function needs the index");
    enum error_code_e err = mesh_add_face(mesh->mesh, face, index);
    if(err == ec_no_error){
        return indexed_mesh_insert_into_spatial_hash(
                mesh, *index);
    }else{
        return err;
    }
}

enum error_code_e indexed_mesh_remove_face(
        struct indexed_mesh_st * mesh,
        size_t face_index)
{
    indexed_mesh_remove_from_spatial_hash(mesh, face_index);
    return mesh_remove_face(mesh->mesh, face_index);
}

enum error_code_e indexed_mesh_swap_edge(
        struct indexed_mesh_st * mesh,
        size_t face_index_0,
        size_t face_index_1)
{
    enum error_code_e err = mesh_swap_edge(
            mesh->mesh, face_index_0, face_index_1);
    if(err == ec_no_error){
        indexed_mesh_remove_from_spatial_hash(mesh, face_index_0);
        indexed_mesh_remove_from_spatial_hash(mesh, face_index_1);
        err = indexed_mesh_insert_into_spatial_hash(mesh, face_index_0);
        if(err != ec_no_error) return err;
        return indexed_mesh_insert_into_spatial_hash(mesh, face_index_1);
    }else{
        return err;
    }
}

enum error_code_e indexed_mesh_replace_face(
        struct indexed_mesh_st * mesh,
        struct face_st face,
        size_t index)
{
    enum error_code_e err = mesh_replace_face(mesh->mesh, face, index);
    if(err != ec_no_error) return err;
    indexed_mesh_remove_from_spatial_hash(mesh, index);
    return indexed_mesh_insert_into_spatial_hash(mesh, index);
}

struct indexed_mesh_hash_get_callback_data_st{
    const struct indexed_mesh_st * mesh;
    struct vector_st * point;
    enum projection_plane_e pp;
    enum point_polygon_position_e pos;
    size_t * face_index;
};

bool indexed_mesh_hash_get_callback(
        size_t face_index,
        void * _data)
{
    struct indexed_mesh_hash_get_callback_data_st * data = 
        (struct indexed_mesh_hash_get_callback_data_st *) _data;
    const struct indexed_mesh_st * mesh = data->mesh;
    data->pos = projected_face_point_position(
            mesh->mesh,
            face_index,
            data->point,
            data->pp);
    if(data->pos == ppol_in){
        *data->face_index = face_index;
        return true;
    }else{
        return false;
    }
}

enum error_code_e indexed_mesh_find_first_enclosing_triangular_face(
        const struct indexed_mesh_st * mesh,
        struct vector_st point,
        enum projection_plane_e pp,
        size_t * face_index)
{
    struct indexed_mesh_hash_get_callback_data_st data = {
        mesh, &point, pp, ppol_out, face_index};
    spatial_hash_get(
            mesh->sph,
            point,
            point,
            indexed_mesh_hash_get_callback,
            &data);
    if(data.pos == ppol_in){
        return ec_no_error;
    }else{
        return ec_error;
    }
}

#define delaunay_mesh_add_face(mesh, face, index) \
    indexed_mesh_add_face((mesh), (face), (index))
#define delaunay_mesh_remove_face(mesh, index) \
    indexed_mesh_remove_face((mesh), (index))
#define delaunay_mesh_swap_edge(mesh, index_0, index_1) \
    indexed_mesh_swap_edge((mesh), (index_0), (index_1))
#define delaunay_mesh_replace_face(mesh, face, index) \
    indexed_mesh_replace_face((mesh), (face), (index)) 
#define delaunay_mesh_find_first_enclosing_triangular_face(mesh, p, pp, i) \
    indexed_mesh_find_first_enclosing_triangular_face((mesh), (p), (pp), (i))
#define delaunay_mesh_st indexed_mesh_st
#define delaunay_mesh_faces(mesh_) (mesh_)->mesh->faces
#define delaunay_mesh_neighbor(mesh_, index) (mesh_)->mesh->neighbors[(index)]
#define delaunay_mesh_vertices(mesh_) (mesh_)->mesh->vertices

#else

#define delaunay_mesh_add_face(mesh, face, index) \
    mesh_add_face((mesh), (face), (index))
#define delaunay_mesh_remove_face(mesh, index) \
    mesh_remove_face((mesh), (index))
#define delaunay_mesh_swap_edge(mesh, index_0, index_1) \
    mesh_swap_edge((mesh), (index_0), (index_1))
#define delaunay_mesh_replace_face(mesh, face, index) \
    mesh_replace_face((mesh), (face), (index)) 
#define delaunay_mesh_find_first_enclosing_triangular_face(mesh, p, pp, i) \
    unindexed_mesh_find_first_enclosing_triangular_face((mesh), (p), (pp), (i))
#define delaunay_mesh_st mesh_st
#define delaunay_mesh_faces(mesh) (mesh)->faces
#define delaunay_mesh_neighbor(mesh, index) (mesh)->neighbors[(index)]
#define delaunay_mesh_vertices(mesh) (mesh)->vertices

#endif

#define delaunay_mesh_vertex(mesh, index) delaunay_mesh_vertices((mesh))[(index)]
#define delaunay_mesh_face(mesh, index) delaunay_mesh_faces((mesh))[(index)]

struct quad_st{
    size_t face_index_0;
    size_t face_index_1;
};

typedef struct quad_st * face_stack_t;

enum error_code_e face_stack_init(face_stack_t * s){
    return array_new(struct quad_st, 0, s);
}   

void face_stack_cleanup(face_stack_t * s){
    array_delete(s);
}

enum error_code_e face_stack_push(face_stack_t * s, struct quad_st v){
    size_t n = array_length(*s);
    enum error_code_e err = array_resize(s, n + 1);
    if(err != ec_no_error) return err;
    (*s)[n] = v;
    return ec_no_error;
}

enum error_code_e face_stack_pop(face_stack_t * s, struct quad_st * v){
    size_t n = array_length(*s);
    if(n == 0) return ec_error;
    n -= 1;
    *v = (*s)[n];
    array_resize(s, n);
    return ec_no_error;
}

struct face_test_st {
    struct face_st face;
    bool is_regular;
};

bool mesh_triangle_would_be_regular(
        struct delaunay_mesh_st * mesh,
        struct face_test_st * face)
{
    size_t p[FACE_SIZE] = {0, 1, 2};
    struct vector_st v[FACE_SIZE] = {
        delaunay_mesh_vertex(mesh, face->face.f[0]).point,
        delaunay_mesh_vertex(mesh, face->face.f[1]).point,
        delaunay_mesh_vertex(mesh, face->face.f[2]).point};
    struct vector_st normal;
    enum error_code_e err = planar_polygon_normal(p, FACE_SIZE, v, &normal);
    if(err == ec_no_error){
        face->is_regular = true;
        return true;
    }else{
        face->is_regular = false;
        return false;
    }
}

#define N_ADDED_FACES 2
#ifndef HANDLE_VERTEX_ON_EDGES
#define HANDLE_VERTEX_ON_EDGES 1
#endif
#if HANDLE_VERTEX_ON_EDGES
#define N_NEW_FACES_MAX 4

enum error_code_e handle_vertex_on_edge(
        struct delaunay_mesh_st * mesh,
        size_t face_index,
        struct face_test_st new_triangles[N_NEW_FACES_MAX],
        size_t * n_new_faces,
        size_t * new_face_indexes)
{
    G_ASSERT(*n_new_faces == 3, "There must me 3 faces");
    size_t neighbor_index = INVALID_INDEX;
    size_t non_regular_index = INVALID_INDEX;
    // at this *n_new_faces is 3
    for(int32_t i = 0; i < *n_new_faces; i++){
        if(!new_triangles[i].is_regular){
            neighbor_index = delaunay_mesh_neighbor(mesh, face_index).f[i];
            non_regular_index = i;
        }
    }
    // No neighbor, this face should, in fact, be ignored
    if(neighbor_index == INVALID_INDEX) return ec_no_error;
    // offset of face index in the neighbor's neighboring face array
    int32_t offest = 0;
    for(; offest < FACE_SIZE; offest++){
        if(delaunay_mesh_neighbor(mesh, neighbor_index).f[offest] == face_index)
            break;
    }
    // two new faces are already known, this is the third
    struct face_test_st new_face_3 = {{{
        delaunay_mesh_face(mesh, neighbor_index).f[(offest + 2) % FACE_SIZE],
        delaunay_mesh_face(mesh, neighbor_index).f[offest],
        // the new vertex is on position 2 in all three first new faces
        new_triangles[0].face.f[2],}}, true};
    new_triangles[non_regular_index] = new_face_3;
    // three new faces are already known, this is the fourth
    struct face_test_st new_face_4 = {{{
        delaunay_mesh_face(mesh, neighbor_index).f[(offest + 1) % FACE_SIZE],
        delaunay_mesh_face(mesh, neighbor_index).f[(offest + 2) % FACE_SIZE],
        // the new vertex is on position 2 in all three first new faces
        new_triangles[0].face.f[2],}}, true};
    new_triangles[N_NEW_FACES_MAX - 1] = new_face_4;
    new_face_indexes[N_NEW_FACES_MAX - 1] = neighbor_index;
    *n_new_faces = N_NEW_FACES_MAX;
    return delaunay_mesh_replace_face(mesh, new_face_4.face, neighbor_index);
}

#else
#define N_NEW_FACES_MAX 3
#endif

enum error_code_e insert_vertex_in_triangulation(
        struct delaunay_mesh_st * mesh,
        size_t vertex_index,
        enum projection_plane_e pp)
{
    size_t face_index;
    struct vector_st point = delaunay_mesh_vertex(mesh, vertex_index).point;
    enum error_code_e err = delaunay_mesh_find_first_enclosing_triangular_face(
            mesh, point, pp, &face_index);
    // This should not happen because of the super triangle.
    // Checking nonetheless
    if(err != ec_no_error)
        return err;
    struct face_st old_face = delaunay_mesh_face(mesh, face_index);
    size_t n_new_faces = 3;
    // Three faces is the standard case (-1 + 3), when a point 
    // is on a edge, there will -2 + 4 triangles
    struct face_test_st new_triangles[N_NEW_FACES_MAX] = {
        {{{old_face.f[0], old_face.f[1], vertex_index}}, false},
        {{{old_face.f[1], old_face.f[2], vertex_index}}, false},
        {{{old_face.f[2], old_face.f[0], vertex_index}}, false},
#if HANDLE_VERTEX_ON_EDGES
        {{{INVALID_INDEX, INVALID_INDEX, INVALID_INDEX}}, false},
#endif
    };
    size_t regular_count = 0;
    for(size_t i = 0; i < n_new_faces; i++){
        bool regular = mesh_triangle_would_be_regular(
                mesh, &new_triangles[i]);
        if(regular) regular_count++;
    }
    size_t new_face_indexes[N_NEW_FACES_MAX] = {
        INVALID_INDEX, INVALID_INDEX, INVALID_INDEX,
#if HANDLE_VERTEX_ON_EDGES
        INVALID_INDEX
#endif
    };
    // Doing anything would create invalid triangles, vertex index
    // is a duplicated vertex
    if(regular_count < 2) return ec_no_error;
#if HANDLE_VERTEX_ON_EDGES
    else if(regular_count == 2){
        // Note that adding an invalid third face is not always a problem
        // It will may be swapped as the point is always
        // within the circumcenter. This transforms the new_triangles
        // in order to handle all cases of vertex on edge.
        err = handle_vertex_on_edge(
                mesh,
                face_index,
                new_triangles,
                &n_new_faces,
                new_face_indexes);
        if(err != ec_no_error) return err;
    }
#endif
    // Iterating over the list of triangles,
    // the first one replace the face that contains
    // the new vertex as it must be removed.
    // There are regular_count regular faces.
    bool replaced = false;
    // the fourth new face is handled by handle_vertex_on_edge
    for(int32_t i = 0; i < N_ADDED_FACES + 1; i++){
#if HANDLE_VERTEX_ON_EDGES
        // degenerate face, ignoring it.
        // The version that does not handle vertices on edge
        // needs the degenerate triangle, hopefully, the point
        // on it will appear in the circumcenter of an adjacent
        // face and the edge will be swapped ending with a
        // valid triangle, this does not always work though.
        if(!new_triangles[i].is_regular) continue;
#endif
        if(!replaced){
            err = delaunay_mesh_replace_face(
                    mesh,
                    new_triangles[i].face,
                    face_index);
            new_face_indexes[i] = face_index;
            replaced = true;
        }else{
            err = delaunay_mesh_add_face(
                    // the first face is replaced
                    mesh, new_triangles[i].face,
                    new_face_indexes + i);
        }
        if(err != ec_no_error) return err;
    }
    face_stack_t face_stack;
    err = face_stack_init(&face_stack);
    if(err != ec_no_error) goto failure;
    for(int32_t i = 0; i < n_new_faces; i++){
        size_t face = new_face_indexes[i];
        // the face was degenerate and ignored, this is a leftover
        // ignoring
        if(face == INVALID_INDEX) continue;
        // The position of the new vertex is the same in all faces
        // the opposite face is at the fixed offset 0
        if(delaunay_mesh_neighbor(mesh, face).f[0] != INVALID_INDEX){
            struct quad_st quad = {face, delaunay_mesh_neighbor(mesh, face).f[0]};
            err = face_stack_push(&face_stack, quad);
            if(err != ec_no_error) goto failure;
        }
    }
    struct quad_st quad;
    while(face_stack_pop(&face_stack, &quad) == ec_no_error){
        struct face_st * face = &delaunay_mesh_face(mesh, quad.face_index_1);
        struct triangle_st tr = {{
            delaunay_mesh_vertex(mesh, face->f[0]).point,
            delaunay_mesh_vertex(mesh, face->f[1]).point,
            delaunay_mesh_vertex(mesh, face->f[2]).point,
        }};
        struct vector_st cc_center, cc_to_vertex, cc_to_triangle_vertex;
        err = triangle_compute_circumcircle_center(&tr, &cc_center);
        if(err != ec_no_error){
            // division by zero, some points are too close
            // for the algorithm to work
            return err;
        }
        vector_subtraction(tr.t, &cc_center, &cc_to_triangle_vertex);
        double sq_cc_radius = vector_dot_product(
                &cc_to_triangle_vertex, &cc_to_triangle_vertex);
        vector_subtraction(&point, &cc_center, &cc_to_vertex);
        double sq_dist = vector_dot_product(&cc_to_vertex, &cc_to_vertex);
        if(sq_dist < sq_cc_radius){
            // swap make sure that the vertex we are inserting is 
            // still in third position
            err = delaunay_mesh_swap_edge(
                    mesh, quad.face_index_0, quad.face_index_1);
            if(err != ec_no_error) break;
            if(delaunay_mesh_neighbor(mesh, quad.face_index_0).f[0] != INVALID_INDEX){
                struct quad_st quad_0 = {quad.face_index_0,
                    delaunay_mesh_neighbor(mesh, quad.face_index_0).f[0]};
                err = face_stack_push(&face_stack, quad_0);
                if(err != ec_no_error) break;
            }
            if(delaunay_mesh_neighbor(mesh, quad.face_index_1).f[0] != INVALID_INDEX){
                struct quad_st quad_1 = {quad.face_index_1,
                    delaunay_mesh_neighbor(mesh, quad.face_index_1).f[0]};
                err = face_stack_push(&face_stack, quad_1);
                if(err != ec_no_error) break;
            }
        }
    }
failure:
    face_stack_cleanup(&face_stack);
    return err;
}

bool is_super_face(
        struct mesh_st * mesh,
        size_t face_index)
{
    size_t n_vertices = array_length(mesh->vertices) - 3;
    for(int32_t i = 0; i < FACE_SIZE; i++){
        if(mesh->faces[face_index].f[i] >= n_vertices){
            return true;
        }
    }
    return false;
}

void mesh_super_triangle_cleanup(
        struct mesh_st * mesh)
{
    size_t index = array_length(mesh->faces) - 1;
    while(1){
        if(is_super_face(mesh, index)){
            mesh_remove_face(mesh, index);
        }
        if(index == 0) break;
        else index--;
    };
    size_t n_vertices = array_length(mesh->vertices);
    // all the adjacent faces have been removed, the vertex are isolated
    // feature, rewinding the vertex array will finish the
    // removal
    array_resize(&mesh->vertices, n_vertices - 3);
}

struct triangle_st compute_triangulation_super_triangle(
        struct mesh_st * mesh,
        enum projection_plane_e pp)
{
    G_ASSERT(pp == pp_xy || pp == pp_yz || pp == pp_zx, "Unknown projection plane");
    size_t n_vertices = array_length(mesh->vertices);
    // Initializing super triangle
    struct vector_st min = {DBL_MAX, DBL_MAX, DBL_MAX};
    struct vector_st max = {-DBL_MAX, -DBL_MAX, -DBL_MAX};
    for(size_t v = 0; v < n_vertices; v++){
        for(int32_t i = 0; i < 3; i++){
            if(mesh->vertices[v].point.v[i] > max.v[i])
                max.v[i] = mesh->vertices[v].point.v[i];
            if(mesh->vertices[v].point.v[i] < min.v[i])
                min.v[i] = mesh->vertices[v].point.v[i];
        }
    }
    struct vector_st sizes = {
        {max.v[0] - min.v[0], max.v[1] - min.v[1], max.v[2] - min.v[2]}
    };
    struct triangle_st infinite_vertices = {{min, min, min}};
    double safe_offset = 1.0;
    if(pp == pp_xy){
        double side_len = (sizes.v[0] + sizes.v[1] + safe_offset) * 10;
        infinite_vertices.t[1].v[0] = min.v[0] + side_len;
        infinite_vertices.t[2].v[1] = min.v[1] + side_len;
        infinite_vertices.t[0].v[0] = min.v[0] - side_len;
        infinite_vertices.t[0].v[1] = min.v[1] - side_len;
    }else if(pp == pp_yz){
        double side_len = (sizes.v[1] + sizes.v[2] + safe_offset) * 10;
        infinite_vertices.t[1].v[1] = min.v[1] + side_len;
        infinite_vertices.t[2].v[2] = min.v[2] + side_len;
        infinite_vertices.t[0].v[1] = min.v[1] - side_len;
        infinite_vertices.t[0].v[2] = min.v[2] - side_len;
    }else if(pp == pp_zx){
        double side_len = (sizes.v[0] + sizes.v[2] + safe_offset) * 10;
        infinite_vertices.t[1].v[0] = min.v[0] + side_len;
        infinite_vertices.t[2].v[2] = min.v[2] + side_len;
        infinite_vertices.t[0].v[0] = min.v[0] - side_len;
        infinite_vertices.t[0].v[2] = min.v[2] - side_len;
    }
    // The super triangle is meant to be big enough so that all points
    // are inside
    return infinite_vertices;
}

enum error_code_e make_triangulation_super_triangle(
        struct mesh_st * mesh,
        enum projection_plane_e pp)
{
    struct triangle_st infinite_vertices = 
        compute_triangulation_super_triangle(
                mesh, pp);
    enum error_code_e err = ec_no_error;
    struct face_st super_triangle;
    for(int32_t i = 0; i < FACE_SIZE; i++){
        err = mesh_add_vertex(
                mesh, infinite_vertices.t[i], &super_triangle.f[i]);
        if(err != ec_no_error) return err;
    }
    err = mesh_add_face(mesh, super_triangle, NULL);
    return err;
}

enum error_code_e __mesh_delaunay_triangulation(
        struct delaunay_mesh_st * mesh,
        enum projection_plane_e pp)
{
    size_t n_vertices = array_length(delaunay_mesh_vertices(mesh));
    enum error_code_e err = ec_no_error;
    for(size_t i = 0; i < n_vertices; i++){
        // vertex is already in triangulation, possible
        // if boundaries are user defined
        if(delaunay_mesh_vertex(mesh, i).adjacent_faces != INVALID_INDEX)
            continue;
        err = insert_vertex_in_triangulation(mesh, i, pp);
        // still try to clean something upon failure, this does
        // not allocates memory, it may work. At this point the
        // mesh is beyond repair anyway (in case of error).
        // If there is a problem with the geometry, there is
        // high chance the cleanup will make thing worse.
        if(err != ec_no_error) return err;
    }
    return err;
}

#ifndef UNINDEXED_DELAUNAY
enum error_code_e _mesh_delaunay_triangulation(
        struct mesh_st * mesh,
        enum projection_plane_e pp)
{
    struct indexed_mesh_st indexed_mesh;
    enum error_code_e err = indexed_mesh_init_in_place_faces_as_boundaries(
            mesh, pp, &indexed_mesh);
    if(err != ec_no_error) return err;
    err = __mesh_delaunay_triangulation(&indexed_mesh, pp);
    indexed_mesh_cleanup(&indexed_mesh);
    return err;
}
#else
#define _mesh_delaunay_triangulation(mesh, pp) __mesh_delaunay_triangulation((mesh), (pp))
#endif

enum error_code_e mesh_delaunay_triangulation(
        struct mesh_st * mesh,
        enum projection_plane_e pp)
{
    size_t n_vertices = array_length(mesh->vertices);
    if(n_vertices < 3){
        return ec_topology_error;
    }else if(array_length(mesh->faces) != 0){
        return ec_out_of_bound_error;
    }
    G_ASSERT(pp == pp_xy || pp == pp_yz || pp == pp_zx, "Unknown projection plane");
    enum error_code_e err = make_triangulation_super_triangle(
            mesh, pp);
    if(err != ec_no_error) goto failure;
    err = _mesh_delaunay_triangulation(mesh, pp);
    // still try to clean something upon failure, this does
    // not allocates memory, it may work. At this point the
    // mesh is beyond repair anyway (in case of error).
    // If there is a problem with the geometry, there is
    // high chance the cleanup will make thing worse.
    G_ASSERT(err != ec_topology_error && err != ec_out_of_bound_error,
            "The triangulation failed, this is a bug");
failure:
    mesh_super_triangle_cleanup(mesh);
    return err;
}

enum error_code_e mesh_delaunay_triangulation_user_defined_boundaries(
        struct mesh_st * mesh,
        enum projection_plane_e pp)
{
    size_t n_vertices = array_length(mesh->vertices);
    if(n_vertices < 3) return ec_topology_error;
    G_ASSERT(pp == pp_xy || pp == pp_yz || pp == pp_zx, "Unknown projection plane");
    return _mesh_delaunay_triangulation(mesh, pp);
}

