#ifndef GEOMETRY_MESH_H
#define GEOMETRY_MESH_H

#include <stdbool.h>
#include <private/vector.h>
#include <public/mesh.h>

/**
 * Structure specifying an edge between two faces. 
 */
struct edge_spec_st{
    size_t face_index_0; /** First face. */
    size_t face_index_1; /** Second face. */
    int32_t vertex_offset_0; /** First edge vertex is a vertex of face 0 at this offset */
    int32_t vertex_offset_1; /** Second edge vertex is a vertex of face 1 at this offset */
};

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
 * Prototype for an edge iteration callback
 * param edge Specification of an edge that is being iterated upon.
 * param collinear Whether edge is collinear to the input.
 * param data Caller supplied pointer (not modified).
 * return true if the caller wishes to stop the iteration false otherwise.
 */
typedef bool (*mesh_edge_iteration_callback_f)(
        _IN struct edge_spec_st edge,
        _IN bool collinear,
        _IN void * data);

/**
 * Iterates over the edges of 2D mesh that intersects an eventual edge formed by
 * vertices v_0 and v_1. This function assumes that there are no holes between
 * v_0, v_1. The boundary edges of the mesh must form a convex polygon.
 * param mesh Mesh to look for intersection into.
 * param v_0 First vertex of the eventual edge.
 * param v_1 Second vertex of the eventual edge.
 * param x First axis for projection.
 * param y Second axis for projection.
 * param callback Caller issued callback, called upon finding an intersection.
 * param data Caller issued pointer, forwarded to callback (not modified).
 * return ec_no_error upon success, it was possible to traverse the mesh from
 * v_0 to v_1 and intersecting edges were forwarded or the caller requested an early
 * stop. ec_topology_error if a hole was encountered and v_1 was not reached, not
 * all edges were forwarded to the caller.
 */
enum error_code_e mesh_iterate_over_projected_intersecting_edges(
        _IN const struct mesh_st * mesh,
        _IN size_t v_0,
        _IN size_t v_1,
        _IN int32_t x,
        _IN int32_t y,
        _IN mesh_edge_iteration_callback_f callback,
        _IN void * data);

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
        _IN struct mesh_st * mesh,
        _IN size_t n);

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
 * Determines whether a polygon is strictly convex.
 * This test is O(n) and non robust, it ignores floating point number
 * imprecision.
 * param polygon see polygon_point_position.
 * param n_vertices number of vertices in the polygon.
 * param vertices coordinates of the polygon vertices.
 * param x Index of the axis that should be considered as first.
 * param y Index of the axis that should be considered as second. System must
 * be direct.
 * returns whether the polygon is convex (true) or not (false).
 */
bool polygon_is_convex(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _IN int32_t x,
        _IN int32_t y);

/**
 * Determines whether a polygon is strictly convex and oriented
 * (vertices are turning in the counter clockwise order).
 * This test does four orientation tests to determine the polygon status.
 * It uses predicates and is less sensible to floating point number imprecision.
 * This is a special case of the gift wrapping algorithm.
 * param polygon see polygon_point_position. It is assumed to have four vertices.
 * param vertices coordinates of the polygon vertices.
 * param x Index of the axis that should be considered as first.
 * param y Index of the axis that should be considered as second. System must
 * be direct.
 * returns whether the polygon is convex and oriented (true) or not (false).
 */
bool quadrilateral_polygon_is_convex_and_oriented(
        size_t * polygon,
        struct vector_st * vertices,
        int32_t x,
        int32_t y);

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
