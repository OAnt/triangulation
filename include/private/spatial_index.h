#ifndef GEOMETRY_SPATIAL_INDEX_H
#define  GEOMETRY_SPATIAL_INDEX_H

#include <stdbool.h>
#include <stdlib.h>
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
        _IN struct spatial_index_st * spi);

/**
 * Adds an object delimited by box and determined by index
 * to the spatial index spi.
 * param spi The spatial index an object is being added to.
 * param box Boundaries of the object.
 * param index An integer that the caller may use to
 * recognize the object (returned by a subsequent get).
 * param handle Pointer to an integer, that will contains
 * a value that may be used to remove the inserted object
 * from the index.
 * return ec_no_error on success, ec_memory_error otherwise.
 */
enum error_code_e spatial_index_add(
        _IN struct spatial_index_st * spi,
        _IN struct box_st box,
        _IN size_t index,
        _OUT size_t * handle);

/**
 * Prototype of a callback function used to returned matching references.
 * param index A reference that was found to be matching.
 * param data Caller supplied data.
 * return true if the caller wishes to stop the search, false otherwise.
 */
typedef bool (*spatial_index_get_callback_f)(
        _IN size_t index,
        _IN void * data);

/**
 * Iterates over the list of objects that may intersects box. Their indices
 * are returned by calling caller issued get_callback.
 * param spi Saptial index to query.
 * param box Query box.
 * param get_callback Function upon finding a potentially intersecting object.
 * param data Caller issued pointer, forwarded to get_callback.
 * return nothing
 */
void spatial_index_get(
        _IN struct spatial_index_st * spi,
        _IN struct box_st box,
        _IN spatial_index_get_callback_f get_callback,
        _IN void * data);

/**
 * Remove all the objects that corresponds to handle (output of the add method).
 * param spi Spatial index the objects should be removed from.
 * param handle Values describing the objects to remove.
 * return nothing.
 */
void spatial_index_remove(
        struct spatial_index_st * spi,
        size_t handle);

#endif
