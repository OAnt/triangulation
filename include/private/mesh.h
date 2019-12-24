#ifndef GEOMETRY_MESH_H
#define GEOMETRY_MESH_H

#include <stdbool.h>
#include <stdlib.h>
#include <private/vector.h>
#include <public/mesh.h>

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
    struct face_st * neighbors; /** neighboring faces for a given
                                  face index */
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
        struct mesh_st * mesh,
        struct face_st face,
        size_t index);

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
 * Swaps the edge between two faces. On success this function guarantees
 * that the vertex on face_index_0 that is not part of the edge between
 * the faces is third in the resulting faces vertices list.
 * param mesh Mesh the faces belong to.
 * param face_index_0 A face to swap.
 * param face_index_1 Second face to swap, faces must be adjacent.
 * return ec_no_error on success. ec_error is returned if faces are not
 * adjacent and therefore their edges swapped. ec_out_of_bound_error is
 * returned if at least one of the face is not in part of the mesh.
 */
enum error_code_e mesh_swap_edge(
        _IN struct mesh_st * mesh,
        _IN size_t face_index_0,
        _IN size_t face_index_1);

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
 * Tells if a vertex belongs to a faces.
 * param mesh Mesh the vertex belongs to.
 * param index Index of the vertex to check.
 * return true if it belongs to a face, false otherwise.
 */
bool mesh_vertex_is_in_face(
        _IN struct mesh_st * mesh,
        _IN size_t index);

/**
 * Mark the last n vertices as free to reuse.
 * param mesh Mesh from which the vertices will be removed.
 * param n Number of vertices to remove.
 * return nothing.
 */
void mesh_vertex_forget_last_n(
        struct mesh_st * mesh,
        size_t n);

/**
 * computes the normal of a polygon assuming it is planar
 * if the polygon is degenerate (cannot compute normal), returns an error.
 * param polygon Pointer to list of vertex index.
 * param n_vertices Number of vertices in the polygon.
 * param vertices Pointer to an array of vector representing vertices positions
 * param normal Normal of the polygon.
 * return ec_no_error if the polygon is regular. ec_error if it is degenerate.
 */
enum error_code_e planar_polygon_normal(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _OUT struct vector_st * normal);

/**
 * Position of a point relative to a polygon.
 */
enum point_polygon_position_e {
    ppol_in, /** Point is inside the polygon. */
    ppol_out /** Point is outside of polygon. */
};

/**
 * Computes the position of a point relative to a polygon.
 * param polygon The polygon is defined by a list of vertices indexes
 * [polygon[i], polygon[i+1]] is an edge, the polygon is closed, its
 * last edge is [polygon[n_vertices - 1], polygon[0]]
 * The test will work on 2D polygon with a point in the same plane.
 * In case of a point outside not lying on the polygon plane,
 * the test is not guaranteed to work if it is too far. The
 * Test works by projecting the plane and point on the best 
 * xy, yz or zx plane and using a winding number check on the projected
 * point in the projected polygon. The idea is to be resilient 
 * to slight misalignment in various points.
 * param n_vertices number of vertices in the polygon.
 * param vertices coordinates of the polygon vertices.
 * param point coordinates of the point to classify.
 * returns whether the point is in the polygon or not.
 */
enum point_polygon_position_e polygon_point_position(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _IN struct vector_st * point);

/** 
 * Same as polygon_point_position but lets the user specify
 * the projection plane, avoid useless computation if it is
 * already known.
 * param polygon see polygon_point_position.
 * param n_vertices number of vertices in the polygon.
 * param vertices coordinates of the polygon vertice.s
 * param point coordinates of the point to classify.
 * param pp Projection plane to use. 
 * returns whether the point is in the polygon or not.
 */
enum point_polygon_position_e projected_polygon_point_position(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _IN struct vector_st * point,
        _IN enum projection_plane_e pp);

/**
 * Determines the position of a point relative to a face.
 * param mesh Mesh the face belongs to.
 * param face_index Index of the face in the mesh face array.
 * param point coordinates of the point to classify.
 * param x Index of the axis that should be considered as first.
 * param y Index of the axis that should be considered as second. System must
 * be direct.
 * returns whether the point is in the polygon or not.
 */
enum point_polygon_position_e projected_face_point_position(
        _IN const struct mesh_st * mesh,
        _IN size_t face_index,
        _IN struct vector_st * point,
        _IN int32_t x,
        _IN int32_t y);

/**
 * Iterates over all the faces in the mesh to find a face
 * that contains the given point. Stops when a matching faces
 * if found.
 * param mesh Mesh containing the faces.
 * param point Coordinates of the point.
 * param pp Projection plane to use.
 * param face_index If a face is found the pointed variable will be updated
 * return ec_no_error if a face is found otherwise ec_error.
 */
enum error_code_e unindexed_mesh_find_first_enclosing_triangular_face(
        _IN const struct mesh_st * mesh,
        _IN struct vector_st point,
        _IN enum projection_plane_e pp,
        _OUT size_t * face_index);

#endif
