#ifndef GEOMETRY_PREDICATES_H
#define GEOMETRY_PREDICATES_H

#include <stdbool.h>
#include <private/debug.h>
#include <private/triangle.h>

double incircle(double * pa, double * pb, double * pc, double * p);
double orient2d(double * pa, double * pb, double * pc);

static inline bool triangle_is_regular(
        struct vector_st v[3],
        enum projection_plane_e pp)
{
    int32_t x, y;
    get_axis_system_from_projection_plane(
            pp, &x, &y);
    double pa[2] = {v[0].v[x], v[0].v[y]};
    double pb[2] = {v[1].v[x], v[1].v[y]};
    double pc[2] = {v[2].v[x], v[2].v[y]};
    return orient2d(pa, pb, pc) > 0;
}

static inline bool _vertex_is_in_triangle_circumcenter(
        struct vector_st v,
        struct triangle_st tr,
        enum projection_plane_e pp,
        bool debug)
{
    int32_t x, y;
    get_axis_system_from_projection_plane(
            pp, &x, &y);
    double pa[2] = {tr.t[0].v[x], tr.t[0].v[y]};
    double pb[2] = {tr.t[1].v[x], tr.t[1].v[y]};
    double pc[2] = {tr.t[2].v[x], tr.t[2].v[y]};
    double p[2] = {v.v[x], v.v[y]};
    if(debug){
        debug_print("p: {%f, %f}, tr: {{%f, %f}, {%f, %f}, {%f, %f}} => %f\n",
                p[0], p[1], pa[0], pa[1], pb[0], pb[1], pc[0], pc[1], incircle(pa, pb, pc, p));
    }
    return incircle(pa, pb, pc, p) >= 0;
}

static inline bool vertex_is_in_triangle_circumcenter(
        struct vector_st v,
        struct triangle_st tr,
        enum projection_plane_e pp)
{
    return _vertex_is_in_triangle_circumcenter(v, tr, pp, false);
}

#endif
