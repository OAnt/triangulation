#ifndef GEOMETRY_MESH_H
#define GEOMETRY_MESH_H

#include <stdlib.h>
#include <private/vector.h>
#include <private/common.h>

#define FACE_SIZE 3

/**
 * Structure representing a triangular face. Note that this
 * use vertex index in order to preserve topology.
 */
struct face_st {
    size_t f[FACE_SIZE];/** vertices composing the face. */
};

/**
 * Structure representing a 3D mesh.
 */
struct mesh_st {
    struct face_st * faces; /** faces of the mesh */
    struct vector_st * vertices; /** vertices of the mesh */
};

/**
 * Initialize a mesh, call this function before using
 * param mesh pointer to the mesh to initialize
 * return ec_no_error upon success
 */
enum error_code_e mesh_init(
        _IN struct mesh_st * mesh);

/**
 * Deallocates memory used by a mesh
 * param mesh pointer to the mesh to clean
 * return ec_no_error upon success
 */
enum error_code_e mesh_cleanup(
        _IN struct mesh_st * mesh);

/**
 * Adds a face to the mesh, vertices that compose the face
 * must inserted beforehand.
 * param mesh Pointer to the mesh to add the face to
 * param face Face to add the mesh
 * param index Position of the face in the array
 * return ec_no_error upon success
 */
enum error_code_e mesh_add_face(
        _IN struct mesh_st * mesh,
        _IN struct face_st face,
        _OUT size_t * index);

/**
 * Adds a vertex to the mesh.
 * param mesh Pointer to the mesh to add the vertex to
 * param v Vertex to add the mesh
 * param index Position of the vertex in the array
 * return ec_no_error upon success
 */
enum error_code_e mesh_add_vertex(
        _IN struct mesh_st * mesh,
        _IN struct vector_st v,
        _OUT size_t * index);

#endif
