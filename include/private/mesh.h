#ifndef GEOMETRY_MESH_H
#define GEOMETRY_MESH_H

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

/** 
 * Structure representing a vertex. It is defined and one
 * of the face it belongs to.
 */
struct vertex_st {
    struct vector_st point; /** Supporting point. */
    size_t adjacent_faces; /** Face the vertex belongs to. */
};

/**
 * Structure representing a member of a list of faces adjacent to
 * a vertex.
 */
struct vertex_adjacent_face_st{
    size_t face; /** Face adjacent to the vertex. */
    size_t opposite_vertex; /** Second vertex of the edge. */
    size_t next_adjacent_faces; /** Index of the next in list. */
};

struct mesh_private_st;

/**
 * Structure representing a 3D mesh.
 */
struct mesh_st {
    struct face_st * faces; /** faces of the mesh */
    struct vertex_st * vertices; /** vertices of the mesh */
    struct face_st * neighbors; /** neighboring faces for a given
                                  face index */
    /** Lists of faces neighboring vertices */
    struct vertex_adjacent_face_st * vertex_adjacent_faces; 
    struct mesh_private_st * private;
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
 * must be inserted beforehand.
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
 * Replaces the face at index by the given one. Vertices must
 * be inserted beforehand.
 * param mesh Pointer to the mesh containing the replaced face.
 * param face Replacement face.
 * param index Position of the face to replace
 * return ec_no_error upon success. In case of memory error
 * the mesh vertex face adjacency will be incomplete. Repeating
 * the operation after freeing some enough may fix the issue.
 */
enum error_code_e mesh_replace_face(
        struct mesh_st * mesh,
        struct face_st face,
        size_t index);

/**
 * Removes the last face from the list and returns it.
 * param mesh Pointer to the mesh containing from which the face will be
 * removed.
 * return The list of vertices of the removed face
 */
struct face_st mesh_pop_face(
        _IN struct mesh_st * mesh);

/**
 * Adds a vertex to the mesh.
 * param mesh Pointer to the mesh to add the vertex to.
 * param v Vertex to add the mesh.
 * param index Position of the vertex in the array.
 * return ec_no_error upon success.
 */
enum error_code_e mesh_add_vertex(
        _IN struct mesh_st * mesh,
        _IN struct vector_st v,
        _OUT size_t * index);

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
