#ifndef GEOMETRY_DELAUNAY_TRIANGULATION_H
#define GEOMETRY_DELAUNAY_TRIANGULATION_H

#include <private/mesh.h>

/**
 * Computes the Delaunay triangulation for a set of points in
 * mesh. The mesh must not contain any faces.
 * param mesh Mesh containing only vertices that will be triangulated
 * param pp Projection place to use to compute the triangulation
 * return ec_no_error on success. If an error is returned, the may
 * have been modified. This function have border effects
 */
enum error_code_e mesh_delaunay_triangulation(
        struct mesh_st * mesh,
        enum projection_plane_e pp);

/**
 * Computes the Delaunay triangulation for a set of points in
 * mesh. The mesh must contain faces, they represent the boundaries
 * of the triangulation.
 * param mesh Mesh containing only vertices that will be triangulated
 * param pp Projection place to use to compute the triangulation
 * return ec_no_error on success. If an error is returned, the may
 * have been modified. This function have border effects
 */
enum error_code_e mesh_delaunay_triangulation_user_defined_boundaries(
        struct mesh_st * mesh,
        enum projection_plane_e pp);

#endif
