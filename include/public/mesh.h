#ifndef GEOMETRY_PUBLIC_MESH_H
#define GEOMETRY_PUBLIC_MESH_H

#include <stdlib.h>
#include <public/common.h>

#define FACE_SIZE 3

/**
 * Structure representing a triangular face. Note that this
 * use vertex index in order to preserve topology.
 */
struct face_st {
    size_t f[FACE_SIZE];/** vertices composing the face. */
};

struct mesh_private_st;

/**
 * Structure representing a 3D mesh.
 */
struct mesh_st {
    struct face_st * faces; /** faces of the mesh */
    struct vector_st * points; /** Points supporting the vertices of the mesh */
    struct mesh_private_st * private; /** mesh private member. Contains
                                        information about topology that must
                                        not be tempered with */
};

/**
 * Initialize a mesh, call this function before using
 * param mesh pointer to the mesh to initialize
 * return ec_no_error upon success. ec_memory_error if an error
 * was encountered during an allocation (a NULL pointer was
 * returned by an allocator).
 */
enum error_code_e mesh_init(
        _IN struct mesh_st * mesh);

/**
 * Deallocates memory used by a mesh
 * param mesh pointer to the mesh to clean
 * return ec_no_error upon success or ec_error if a NULL pointer
 * was issued.
 */
enum error_code_e mesh_cleanup(
        _IN struct mesh_st * mesh);

/**
 * Adds a face to the mesh, vertices that compose the face
 * must be inserted beforehand.
 * param mesh Pointer to the mesh to add the face to
 * param face Face to add the mesh
 * param index Position of the face in the array
 * return ec_no_error upon success. Returns ec_memory_error if it is
 * impossible to allocates the memory to store the face or
 * ec_topology_error if adding the face would create non manifold edges.
 * 1 - manifold edges are allowed, 3 or more manifold edges are forbidden.
 * Returns ec_out_of_bound_error if the target vertices are not in the mesh.
 */
enum error_code_e mesh_add_face(
        _IN struct mesh_st * mesh,
        _IN struct face_st face,
        _OUT size_t * index);

/**
 * Replaces the face at index by the given one. Vertices must
 * be inserted beforehand.
 * param mesh Pointer to the mesh containing the replaced face.
 * param face Replacement face.
 * param index Position of the face to replace
 * return ec_no_error upon success. Returns ec_memory_error if it is
 * impossible to allocates the memory to store the face or
 * ec_topology_error if adding the face would create non manifold edges.
 * 1 - manifold edges are allowed, 3 or more manifold edges are forbidden.
 * In case of memory error the mesh vertex face adjacency will be incomplete.
 * Repeating the operation after freeing enough memory may fix the issue.
 * Returns ec_out_of_bound_error if the target vertices are not in the
 * mesh.
 */
enum error_code_e mesh_replace_face(
        _IN struct mesh_st * mesh,
        _IN struct face_st face,
        _IN size_t index);

/**
 * Removes the last face from the list and returns it.
 * param mesh Pointer to the mesh containing from which the face will be
 * removed.
 * param face Pointer to the storage for the popped face.
 * return ec_no_erro upon success. It will return ec_out_of_bounds
 * if the mesh is empty.
 */
enum error_code_e mesh_pop_face(
        _IN struct mesh_st * mesh,
        _OUT struct face_st * face);

/**
 * Removes a face from a mesh. This does not free any memory.
 * instead, it swaps the last face with the face to delete and
 * forget about it.
 * param mesh Mesh from which a face will be removed.
 * param face_index Index of the face to remove.
 * return ec_no_error on success. It returns an ec_out_of_bound_error
 * if the face can't be removed (because it is not in the mesh).
 */
enum error_code_e mesh_remove_face(
        _IN struct mesh_st * mesh,
        _IN size_t face_index);

/**
 * Get the list of faces adjacent to face_index, assumes the mesh is 2-manifold and that
 * face_index is in the mesh.
 * param mesh Mesh the face belongs to.
 * param face_index Index of the face for which to retrieve neighbors.
 * return The list of neighboring faces.
 */
struct face_st mesh_get_neighbors(
        _IN const struct mesh_st * mesh,
        _IN size_t face_index);

/**
 * Adds a vertex to the mesh.
 * param mesh Pointer to the mesh to add the vertex to.
 * param v Vertex to add the mesh.
 * param index Position of the vertex in the array.
 * return ec_no_error upon success. Returns ec_memory_error if it is
 * impossible to allocates the memory to store the vertex.
 */
enum error_code_e mesh_add_vertex(
        _IN struct mesh_st * mesh,
        _IN struct vector_st v,
        _OUT size_t * index);

/**
 * Exports a mesh to a binary file.
 * param mesh Mesh to export.
 * param filename File to write to.
 * return ec_no_error id successful
 */
enum error_code_e mesh_export_stlb(
        _IN const struct mesh_st * mesh,
        _IN const char * filename);

#endif
