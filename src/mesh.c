#include <stdbool.h>
#include <stdio.h>
#include <private/common.h>
#include <private/vector.h>
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

/**
 * Type of intersection between an infinite ray along the
 * horizontal axis and an edge.
 */
enum intersection_type_e{
    it_upward, /** Edge is intersected and going upward. */
    it_downward, /** Edge is intersected and going downward. */
    it_no, /** Edge is not intersected. */
};

/** 
 * Determines the type of intersection if there is one.
 * param point Infinite horizontal ray is going through point.
 * param seg Supporting segment of the edge.
 * param y Vertical axis dimension index.
 * return type of intersection if there is one
 */
enum intersection_type_e  edge_determine_intersection_type(
        struct vector_st * point,
        struct segment_st * seg,
        int32_t y)
{
    //point is between seg[0] and seg[1], edge is
    //pointing upward and there is an intersection
    if(seg->s[0].v[y] <= point->v[y] && \
            point->v[y] <= seg->s[1].v[y]){
        return it_upward;
    //point is between seg[1] and seg[0], edge is
    //pointing downward and there is an intersection
    }else if(seg->s[1].v[y] <= point->v[y] && \
            point->v[y] <= seg->s[0].v[y]){
        return it_downward;
    //point is not between the segment vertical bounds
    //there cannot be an intersection with an horizontal
    //axis going through point
    }else{
        return it_no;
    }
}

#define point_is_left_of(p, s, pp) \
    vector_position_relative_to_segment(p, s, pp) == pt_left
#define point_is_right_of(p, s, pp) \
    vector_position_relative_to_segment(p, s, pp) == pt_right

/**
 * Increments or decrements the winding number
 * according to the relative position of point
 * and seg.
 * param point Point for which one wants to determine
 * the position relative to a polygon.
 * param seg Supporting segment for an edge of the
 * polygon.
 * param pp Projection plane for the computations.
 * param winding_number pointer to the winding number
 * of the point relative to the polygon
 * return nothing
 */
void winding_number_modify(
        struct vector_st * point,
        struct segment_st * seg,
        enum projection_plane_e pp,
        int32_t * winding_number)
{
    // determines what is the vertical axis 
    // for a given projection plane
    int32_t y = (pp + 1) % 3;
    // determines if there is an intersection and what kind
    // of intersection it is
    enum intersection_type_e it = edge_determine_intersection_type(
            point, seg, y);
    // Edge is going upward, an oriented polygon turns
    // counterclockwise, if the point is left of the edge,
    // it is inside once
    if(it == it_upward && point_is_left_of(point, seg, pp)){
        (*winding_number)++;
    // Edge is going downward, an oriented polygon turns
    // counterclockwise, if the point is right of the edge,
    // it is outside once
    }else if(it == it_downward && point_is_right_of(point, seg, pp)){
        (*winding_number)--;
    }
}

enum point_polygon_position_e polygon_point_position(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _IN struct vector_st * point)
{
    int32_t winding_number = 0;
    enum projection_plane_e pp = pp_xy;
    for(size_t i = 0; i < n_vertices; i++){
        struct segment_st seg = {
            vertices[i], vertices[(i+1) % n_vertices]};
        winding_number_modify(point, &seg, pp, &winding_number);
    }
    if(winding_number > 0){
        return ppol_in;
    }else{
        return ppol_out;
    }
}

