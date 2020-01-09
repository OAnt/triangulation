#include <math.h>
#include <stdbool.h>
#include <private/vector.h>
#include <private/mesh.h>

// This file contains parts of code, adapted and / or taken from other sources
// on the internet, it was not written by me and are not licensed under
// same terms as the rest of the code 

// #################
// taken from 
// https://stackoverflow.com/questions/471962/how-do-i-efficiently-determine-if-a-polygon-is-convex-non-convex-or-complex 
// response by Rory Daulton (https://stackoverflow.com/users/6246044/rory-daulton) edited by
// cs95 (https://stackoverflow.com/users/4909087/cs95)

#define DIRECTION(vertices, vertex, previous_vertex, x, y) \
    atan2((vertices)[(vertex)].v[(y)] - vertices[(previous_vertex)].v[(y)], \
            vertices[(vertex)].v[(x)] - vertices[(previous_vertex)].v[(x)])

bool polygon_is_convex(
        size_t * polygon,
        size_t n_vertices,
        struct vector_st * vertices,
        int32_t x,
        int32_t y)
{
    if(n_vertices < 3) return false;
    size_t previous_vertex = polygon[n_vertices - 2];
    size_t vertex = polygon[n_vertices - 1];
    double direction = DIRECTION(vertices, vertex, previous_vertex, x, y);
    double angle_sum = 0.0;
    int32_t orientation = 0;
    for(size_t i = 0; i < n_vertices; i++){
        previous_vertex = vertex;
        vertex = polygon[i];
        if(vertices[vertex].v[x] == vertices[previous_vertex].v[x] &&
                vertices[vertex].v[y] == vertices[previous_vertex].v[y])
            return false;
        double previous_direction = direction;
        direction = DIRECTION(vertices, vertex, previous_vertex, x, y);
        double angle = direction - previous_direction;
        if(angle <= - M_PI)
            angle += 2*M_PI;
        else if(angle > M_PI)
            angle -= 2*M_PI;
        if(i == 0){
            if(angle == 0.0) return false;
            orientation = SIGN(angle);
        }else{
            if(orientation != SIGN(angle)) return false;
        }
        angle_sum += angle;
    }
    return fabs(round(angle_sum / (2*M_PI))) == 1;
}
// #################

// #################
// winding number algorithm adapted from
// http://geomalgorithms.com/a03-_inclusion.html
// Copyright 2000 softSurfer, 2012 Dan Sunday
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

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
        struct vector_st * seg0,
        struct vector_st * seg1,
        int32_t y)
{
    //point is between seg[0] and seg[1], edge is
    //pointing upward and there is an intersection
    if(seg0->v[y] <= point->v[y] && \
            point->v[y] < seg1->v[y]){
        return it_upward;
    //point is between seg[1] and seg[0], edge is
    //pointing downward and there is an intersection
    }else if(seg1->v[y] <= point->v[y] && \
            point->v[y] < seg0->v[y]){
        return it_downward;
    //point is not between the segment vertical bounds
    //there cannot be an intersection with an horizontal
    //axis going through point
    }else{
        return it_no;
    }
}

#define point_is_left_of(p, s0, s1, x, y) \
    _vector_position_relative_to_segment(p, s0, s1, x, y) == pt_left
#define point_is_right_of(p, s0, s1, x, y) \
    _vector_position_relative_to_segment(p, s0, s1, x, y) == pt_right

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
        struct vector_st * seg0,
        struct vector_st * seg1,
        int32_t x,
        int32_t y,
        int32_t * winding_number)
{
    // determines what is the vertical axis 
    // for a given projection plane
    // determines if there is an intersection and what kind
    // of intersection it is
    enum intersection_type_e it = edge_determine_intersection_type(
            point, seg0, seg1, y);
    if(it == it_no) return;
    enum point_position_e pos = _vector_position_relative_to_segment(
           point, seg0, seg1, x, y); 
    // Edge is going upward, an oriented polygon turns
    // counterclockwise, if the point is left of the edge,
    // it is inside once
    if(it == it_upward && pos == pt_left){
        (*winding_number)++;
    // Edge is going downward, an oriented polygon turns
    // counterclockwise, if the point is right of the edge,
    // it is outside once
    }else if(it == it_downward && pos == pt_right){
        (*winding_number)--;
    }
}

enum point_polygon_position_e _polygon_point_position(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _IN struct vector_st * point,
        _IN int32_t x,
        _IN int32_t y)
{
    int32_t winding_number = 0;
    for(size_t i = 0; i < n_vertices; i++){
        size_t v = polygon[i];
        size_t next_v = polygon[(i + 1) % n_vertices];
        winding_number_modify(
                point,
                vertices + v,
                vertices + next_v,
                x,
                y,
                &winding_number);
    }
    if(winding_number > 0){
        return ppol_in;
    }else{
        return ppol_out;
    }
}
// #################

