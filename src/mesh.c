#include "private/common.h"
#include <private/array.h>
#include <private/mesh.h>

enum error_code_e mesh_cleanup(
        struct mesh_st * mesh)
{
    if(!mesh) return ec_error;
    array_delete(&mesh->faces);
    array_delete(&mesh->vertices);
    free(mesh);
    memset(mesh, 0, sizeof(struct mesh_st));
    return ec_no_error;
}

enum error_code_e mesh_init(
        struct mesh_st * mesh)
{
    if(!mesh) return ec_error;
    memset(mesh, 0, sizeof(struct mesh_st));
    if(array_new(struct face_st, 0, &mesh->faces) != ec_no_error)
        goto fail_no_faces;
    if(array_new(struct vector_st, 0, &mesh->vertices) != ec_no_error)
        goto fail_no_vec;
    return ec_no_error;
fail_no_vec:
    array_delete(&mesh->faces);
fail_no_faces:
    return ec_memory_error;
}

enum error_code_e mesh_add_face(
        struct mesh_st * mesh,
        struct face_st face,
        size_t * index)
{
    size_t n_faces = array_length(mesh->faces);
    size_t n_vertices = array_length(mesh->vertices);
    for(size_t i = 0; i < FACE_SIZE; i++){
        if(face.f[i] >= n_vertices) return ec_out_of_bound_error;
    }
    if(array_resize(&mesh->faces, n_faces + 1) != ec_no_error)
        return ec_memory_error;
    mesh->faces[n_faces] = face;
    if(index) *index = n_faces;
    return ec_no_error;
}

enum error_code_e mesh_add_vertex(
        struct mesh_st * mesh,
        struct vector_st v,
        size_t * index)
{
    size_t n_vertices = array_length(mesh->vertices);
    if(array_resize(&mesh->vertices, n_vertices + 1) != ec_no_error)
        return ec_memory_error;
    mesh->vertices[n_vertices] = v;
    if(index) *index = n_vertices;
    return ec_no_error;
}

