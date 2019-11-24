#include "private/common.h"
#include <private/array.h>
#include <private/mesh.h>

enum error_code_e mesh_cleanup(
        struct mesh_st * mesh)
{
    // Mesh is null not doing anything
    if(!mesh) return ec_error;
    // Deleting dynamically allocated arrays
    array_delete(&mesh->faces);
    array_delete(&mesh->vertices);
    // Setting everything to zero for good measure
    memset(mesh, 0, sizeof(struct mesh_st));
    return ec_no_error;
}

enum error_code_e mesh_init(
        struct mesh_st * mesh)
{
    // Mesh is null not doing anything
    if(!mesh) return ec_error;
    // Setting everything to zero for good measure
    memset(mesh, 0, sizeof(struct mesh_st));
    // Initializing arrays, in case of failure going to an
    // error handler that reverts what was done until the error.
    // This assumes that array_new does the same.
    if(array_new(struct face_st, 0, &mesh->faces) != ec_no_error)
        goto fail_no_faces;
    if(array_new(struct vector_st, 0, &mesh->vertices) != ec_no_error)
        goto fail_no_vec;
    return ec_no_error;
    // Only the faces were allocated
fail_no_vec:
    array_delete(&mesh->faces);
    // Nothing was allocated yet
fail_no_faces:
    return ec_memory_error;
}

enum error_code_e mesh_add_face(
        struct mesh_st * mesh,
        struct face_st face,
        size_t * index)
{
    // getting lengths of arrays until now
    size_t n_faces = array_length(mesh->faces);
    size_t n_vertices = array_length(mesh->vertices);
    // Ensuring we are not adding a face that references
    // unknown vertices
    for(size_t i = 0; i < FACE_SIZE; i++){
        if(face.f[i] >= n_vertices) return ec_out_of_bound_error;
    }
    // Resizing the array, now it can holds the correct number of
    // features
    if(array_resize(&mesh->faces, n_faces + 1) != ec_no_error)
        return ec_memory_error;
    // appending the face and returns its index
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
    // Resizing the array, now it can holds the correct number of
    // features
    if(array_resize(&mesh->vertices, n_vertices + 1) != ec_no_error)
        return ec_memory_error;
    // appending the vertex and returns its index
    mesh->vertices[n_vertices] = v;
    if(index) *index = n_vertices;
    return ec_no_error;
}

enum point_polygon_position_e polygon_point_position(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _IN struct vector_st * point)
{

    return ppol_in;
}

