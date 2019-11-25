#include <math.h>
#include <stdbool.h>
#include <private/common.h>
#include <private/vector.h>
#include <private/array.h>
#include <private/mesh.h>

#define INVALID_INDEX (size_t)-1

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
    if(array_new(struct vertex_st, 0, &mesh->vertices) != ec_no_error)
        goto fail_no_vec;
    if(array_new(struct face_st, 0, &mesh->neighbors) != ec_no_error)
        goto fail_no_neighbors;
    return ec_no_error;
    // Faces and vertices were allocated
fail_no_neighbors:
    array_delete(&mesh->vertices);
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
    // Resizing both the array, now they can hold the correct number of
    // features
    if(array_resize(&mesh->faces, n_faces + 1) != ec_no_error)
        return ec_memory_error;
    if(array_resize(&mesh->faces, n_faces + 1) != ec_no_error)
        return ec_memory_error;
    // appending the face and returns its index
    mesh->faces[n_faces] = face;
    for(size_t i = 0; i < FACE_SIZE; i++){
        mesh->vertices[i].face = n_faces;
    }
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
    mesh->vertices[n_vertices].point = v;
    mesh->vertices[n_vertices].face = INVALID_INDEX;
    if(index) *index = n_vertices;
    return ec_no_error;
}

/**
 * Type of intersection between an infinite ray along the
 * horizontal axis and an edge.
 */
enum intersection_type_e{
    it_upward = 0, /** Edge is intersected and going upward. */
    it_downward = 1, /** Edge is intersected and going downward. */
    it_no = 2, /** Edge is not intersected. */
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
    int32_t x, y;
    get_axis_system_from_projection_plane(pp, &x, &y);
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
    /*printf("%d, %d, %d -> [%f, %f], [[%f, %f], [%f, %f]], %d, %d, %d -> %d\n",*/
            /*x, y, pp,*/
            /*point->v[x], point->v[y],*/
            /*seg->s[0].v[x], seg->s[0].v[y],*/
            /*seg->s[1].v[x], seg->s[1].v[y], it,*/
            /*point_is_left_of(point, seg, pp),*/
            /*point_is_right_of(point, seg, pp),*/
            /**winding_number);*/
}

static struct vector_st x = {1.0, 0.0, 0.0};
static struct vector_st y = {0.0, 1.0, 0.0};
static struct vector_st z = {0.0, 0.0, 1.0};

enum point_polygon_position_e polygon_point_position(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _IN struct vector_st * point)
{
    // this functions assumes the polygon is plane
    // it computes its normal by taking the first
    // three vertices
    if(n_vertices <= 2) return ppol_out;
    bool degenerate_polygon = true;
    // Computing the normal iterating over
    // groups of three points until we
    // find a group where they are not aligned
    struct vector_st normal;
    for(size_t i = 0; i < n_vertices; i++){
        size_t next = (i + 1) % n_vertices;
        size_t next_over = (i + 2) % n_vertices;
        struct vector_st edge_a, edge_b;
        vector_subtraction(&vertices[next], &vertices[i], &edge_a);
        vector_subtraction(&vertices[next_over], &vertices[i], &edge_b);
        vector_cross_product(&edge_a, &edge_b, &normal);
        double sq_norm = vector_dot_product(&normal, &normal);
        // found a normal with non zero norm, non collinear edges
        if(sq_norm > EPSILON) {
            degenerate_polygon = false;
            break;
        }
    }
    // All the points are aligned, this is a degenerate polygon (a line)
    if(degenerate_polygon) return ppol_out;
    // Finding which of xy, yz ans zx is the best plane
    // to project the polygon on. The higher the absolute
    // dot product of the normal and unit vector is the 
    // better the plane fits. I think it is impossible
    // to find a plane that is orthogonal to all three
    // xy, yz ans zx planes
    enum projection_plane_e pp = pp_xy;
    double x_dot = fabs(vector_dot_product(&normal, &x));
    double y_dot = fabs(vector_dot_product(&normal, &y));
    double z_dot = fabs(vector_dot_product(&normal, &z));
    if(x_dot > y_dot){
        if(z_dot > x_dot){
            pp = pp_xy;
        }else{
            pp = pp_yz;
        }
    }else{
        if(z_dot > y_dot){
            pp = pp_xy;
        }else{
            pp = pp_zx;
        }
    }
    /*printf("%f, %f, %f, %d\n", x_dot, y_dot, z_dot, pp);*/
    int32_t winding_number = 0;
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

