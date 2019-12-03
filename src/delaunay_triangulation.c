#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <private/array.h>
#include <private/mesh.h>
#include <public/common.h>
#include <private/triangle.h>
#include <private/delaunay_triangulation.h>

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

enum error_code_e insert_vertex_in_triangulation(
        struct mesh_st * mesh,
        size_t vertex_index,
        enum projection_plane_e pp)
{
    size_t face_index;
    struct vector_st point = mesh->vertices[vertex_index].point;
    enum error_code_e err = unindexed_mesh_find_first_enclosing_triangular_face(
            mesh, point, pp, &face_index);
    // This should not happen because of the super triangle.
    // Checking nonetheless
    if(err != ec_no_error) return err;
    struct face_st * face = &mesh->faces[face_index];
    struct face_st new_triangles[3] = {
        {{face->f[0], face->f[1], vertex_index}},
        {{face->f[1], face->f[2], vertex_index}},
        {{face->f[2], face->f[0], vertex_index}},
    };
    size_t new_face_indexes[3] = {face_index, 0, 0};
    err = mesh_replace_face(mesh, new_triangles[0], face_index);
    if(err != ec_no_error) return err;
    for(int32_t i = 1; i < 3; i++){
        err = mesh_add_face(
                mesh, new_triangles[i], new_face_indexes + i);
        if(err != ec_no_error) return err;
    }
    face_stack_t face_stack;
    err = face_stack_init(&face_stack);
    if(err != ec_no_error) goto failure;
    for(int32_t i = 0; i < 3; i++){
        size_t face = new_face_indexes[i];
        // The position of the new vertex is the same in all faces
        // the opposite face is at the fixed offset 0
        if(mesh->neighbors[face].f[0] != INVALID_INDEX){
            struct quad_st quad = {face, mesh->neighbors[face].f[0]};
            err = face_stack_push(&face_stack, quad);
            if(err != ec_no_error) goto failure;
        }
    }
    struct quad_st quad;
    while(face_stack_pop(&face_stack, &quad) == ec_no_error){
        struct face_st * face = &mesh->faces[quad.face_index_1];
        struct triangle_st tr = {{
            mesh->vertices[face->f[0]].point,
            mesh->vertices[face->f[1]].point,
            mesh->vertices[face->f[2]].point,
        }};
        struct vector_st cc_center, cc_to_vertex, cc_to_triangle_vertex;
        err = triangle_compute_circumcircle_center(&tr, &cc_center);
        if(err != ec_no_error) return err;
        vector_subtraction(tr.t, &cc_center, &cc_to_triangle_vertex);
        double sq_cc_radius = vector_dot_product(
                &cc_to_triangle_vertex, &cc_to_triangle_vertex);
        vector_subtraction(&point, &cc_center, &cc_to_vertex);
        double sq_dist = vector_dot_product(&cc_to_vertex, &cc_to_vertex);
        if(sq_dist < sq_cc_radius){
            // swap make sure that the vertex we are inserting is 
            // still in third position
            err = mesh_swap_edge(mesh, quad.face_index_0, quad.face_index_1);
            if(err != ec_no_error) break;
            if(mesh->neighbors[quad.face_index_0].f[0] != INVALID_INDEX){
                struct quad_st quad_0 = {quad.face_index_0,
                    mesh->neighbors[quad.face_index_0].f[0]};
                err = face_stack_push(&face_stack, quad_0);
                if(err != ec_no_error) break;
            }
            if(mesh->neighbors[quad.face_index_1].f[0] != INVALID_INDEX){
                struct quad_st quad_1 = {quad.face_index_1,
                    mesh->neighbors[quad.face_index_1].f[0]};
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

void mesh_rewind(
        struct mesh_st * mesh,
        size_t * _index)
{
    size_t index = *_index;
    for(; index > 0; index--){
        if(!is_super_face(mesh, index)) break;
    }
    array_resize(&mesh->faces, index + 1);
    *_index = index;
}

void mesh_super_triangle_cleanup(
        struct mesh_st * mesh)
{
    size_t decr_index = array_length(mesh->faces) - 1;
    for(size_t index = 0; index < array_length(mesh->faces); index++){
        if(index == decr_index) break;
        if(is_super_face(mesh, index)){
            mesh_rewind(mesh, &decr_index);
            struct face_st face;
            mesh_pop_face(mesh, &face);
            decr_index--;
            mesh_replace_face(mesh, face, index);
        }
    }
    size_t n_vertices = array_length(mesh->vertices);
    // all the adjacent faces have been removed, the vertex are isolated
    // feature, rewinding the vertex array will finish the
    // removal
    array_resize(&mesh->vertices, n_vertices - 3);
}

enum error_code_e mesh_delaunay_triangulation(
        struct mesh_st * mesh,
        enum projection_plane_e pp)
{
    size_t n_vertices = array_length(mesh->vertices);
    if(n_vertices < 3) return ec_topology_error;
    if(array_length(mesh->faces) != 0) return ec_out_of_bound_error;
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
    struct triangle_st infinite_vertices;
    memset(&infinite_vertices, 0, sizeof(struct triangle_st));
    double safe_offset = 1.0;
    infinite_vertices.t[0].v[0] = - safe_offset;
    infinite_vertices.t[0].v[1] = - safe_offset;
    infinite_vertices.t[0].v[2] = - safe_offset;
    if(pp == pp_xy){
        double side_len = sizes.v[0] + sizes.v[1] + safe_offset;
        infinite_vertices.t[1].v[0] = side_len;
        infinite_vertices.t[2].v[1] = side_len;
    }else if(pp == pp_yz){
        double side_len = sizes.v[1] + sizes.v[2] + safe_offset;
        infinite_vertices.t[1].v[1] = side_len;
        infinite_vertices.t[2].v[2] = side_len;
    }else if(pp == pp_zx){
        double side_len = sizes.v[0] + sizes.v[2] + safe_offset;
        infinite_vertices.t[1].v[0] = side_len;
        infinite_vertices.t[2].v[2] = side_len;
    }else{
        return ec_out_of_bound_error;
    }
    // The super triangle is meant to be big enough so that all points
    // are inside
    enum error_code_e err = ec_no_error;
    struct face_st super_triangle;
    for(int32_t i = 0; i < FACE_SIZE; i++){
        err = mesh_add_vertex(
                mesh, infinite_vertices.t[i], &super_triangle.f[i]);
        if(err != ec_no_error) goto failure;
    }
    mesh_add_face(mesh, super_triangle, NULL);
    for(size_t i = 0; i < n_vertices; i++){
        err = insert_vertex_in_triangulation(mesh, i, pp);
        // still try to clean something upon failure, this does
        // not allocates memory, it may work. At this point the
        // mesh is beyond repair anyway (in case of error).
        // If there is a problem with the geometry, there is
        // high chance the cleanup will make thing worse.
        if(err == ec_memory_error) goto failure;
        else if(err != ec_no_error) return err;
    }
failure:
    mesh_super_triangle_cleanup(mesh);
    return ec_no_error;
}

