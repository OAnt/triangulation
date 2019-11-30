#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <private/array.h>
#include <private/mesh.h>
#include <public/common.h>
#include <private/delaunay_triangulation.h>

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
    return ec_no_error;
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
            struct face_st face = mesh_pop_face(mesh);
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

static struct vector_st infinite_vertices_xy[FACE_SIZE] = {
    {{-1000000.0, -1000000.0, 0.0}}, {{1000000.0, -1000000.0, 0.0}},
    {{0.0, 1000000.0, 0.0}}
};
static struct vector_st infinite_vertices_yz[FACE_SIZE] = {
    {{0.0, -1000000.0, -1000000.0}}, {{0.0, 1000000.0, -1000000.0}},
    {{0.0, 0.0, 1000000.0}}
};
static struct vector_st infinite_vertices_zx[FACE_SIZE] = {
    {{-1000000.0, 0.0, -1000000.0}}, {{-1000000.0, 0.0, 1000000.0}},
    {{1000000.0, 0.0, 0.0}}
};

enum error_code_e mesh_delaunay_triangulation(
        struct mesh_st * mesh,
        enum projection_plane_e pp)
{
    size_t n_vertices = array_length(mesh->vertices);
    if(n_vertices < 3) return ec_topology_error;
    if(array_length(mesh->faces) != 0) return ec_out_of_bound_error;
    // Initializing super triangle
    struct vector_st * infinite_vertices;
    if(pp == pp_xy){
        infinite_vertices = infinite_vertices_xy;
    }else if(pp == pp_yz){
        infinite_vertices = infinite_vertices_yz;
    }else if(pp == pp_zx){
        infinite_vertices = infinite_vertices_zx;
    }else{
        return ec_out_of_bound_error;
    }
    // The super triangle is meant to be big enough so that all points
    // are inside
    enum error_code_e err = ec_no_error;
    struct face_st super_triangle;
    for(int32_t i = 0; i < FACE_SIZE; i++){
        err = mesh_add_vertex(
                mesh, infinite_vertices[i], &super_triangle.f[i]);
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

