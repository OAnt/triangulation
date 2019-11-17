#ifndef GEOMETRY_PUBLIC_MESH_H
#define GEOMETRY_PUBLIC_MESH_H

#include <public/common.h>

typedef struct mesh_st mesh_st;

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
