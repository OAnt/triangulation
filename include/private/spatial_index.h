#ifndef GEOMETRY_SPATIAL_INDEX_H
#define  GEOMETRY_SPATIAL_INDEX_H

#include <private/common.h>
#include <private/vector.h>

struct spatial_index_st;

/**
 * Allocates a new spatial index for the region defined
 * by soft_boundaries. 
 * param soft_boundaries Boundaries of the spatial index,
 * represents the largest objects that be efficiently
 * stored.
 * param spi Storage for the pointer to the newly allocated spatial index.
 * return ec_no_error on success or ec_memory_error if
 * an allocation failed.
 */
enum error_code_e spatial_index_new(
        _IN struct box_st soft_boundaries,
        _OUT struct spatial_index_st ** spi);

/**
 * Deallocates a spatial index
 * param spi Spatial index to deallocate
 * return nothing
 */
void spatial_index_delete(
        struct spatial_index_st * spi);

#endif
