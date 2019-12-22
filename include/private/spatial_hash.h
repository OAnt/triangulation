#ifndef GEOMETRY_SPATIAL_HASH_H
#define GEOMETRY_SPATIAL_HASH_H
#include <stdbool.h>
#include <stdlib.h>
#include <private/common.h>
#include <private/vector.h>

struct spatial_hash_st;

/**
 * Instantiate a spatial hash of the chosen size.
 * The spatial hash is 2D grid that is repeated all over the
 * xy plane
 * param n_x_bkts Number of buckets along x.
 * param n_y_bkts Number of buckets along y.
 * param x_cell_size Size of a cell along x.
 * param y_cell_size Size of a cell along Y.
 * param _sph The newly allocated spatial hash will be stored in _sph.
 * return ec_no_error on success or ec_memory_error if
 * an allocation failed
 */
enum error_code_e spatial_hash_new(
        _IN int32_t n_x_bkts,
        _IN int32_t n_y_bkts,
        _IN double x_cell_size,
        _IN double y_cell_size,
        _OUT struct spatial_hash_st ** _sph);

/**
 * Deletes a spatial hash, freeing any allocated memory for it.
 * param sph Pointer to the spatial hash that will be deleted.
 * return Nothing.
 */
void spatial_hash_delete(
        struct spatial_hash_st ** sph);

/**
 * Adds an objects referenced by index and bounded by min and max to the
 * spatial hash.
 * param sph Pointer to the spatial hash to which the objects will be added.
 * param min Bottom left bound of the object.
 * param max Top right bound of the object.
 * param index Reference of the object in the spatial hash.
 * param handle Pointer to the location where the handle to added object will be
 * stored. The handle references the position of the object in the spatial hash
 * it is need when removing an object from the spatial hash.
 * return ec_no_error on success or ec_memory_error if an allocation failed
 */
enum error_code_e spatial_hash_add(
        _IN struct spatial_hash_st * sph,
        _IN struct box_st box,
        _IN size_t index,
        _OUT size_t * handle);

/**
 * Removes the object referenced by handle (returned when added).
 * param sph Pointer to the spatial hash from which the object is to be removed.
 * param handle Reference of the object to remove.
 * return noting
 */
void spatial_hash_remove(
        _IN struct spatial_hash_st * sph,
        _IN size_t handle);

/**
 * Prototype of a callback function used to returned matching references.
 * param index A reference that was found to be matching.
 * param data Caller supplied data.
 * return true if the caller wishes to stop the search, false otherwise.
 */
typedef bool (*spatial_hash_get_callback_f)(size_t index, void * data);

/**
 * Iterates over the objects that where added to the grid that match the
 * supplied min and max bounds. The references are returned via a callback.
 * param sph Spatial hash to query.
 * param min Bottom left bound of the queried rectangle.
 * param max Top right bound of the queries rectangle.
 * param get_callback Iteration callback called upon finding a matching object.
 * param data Caller supplied data that will be passed to the iteration callback.
 */
void spatial_hash_get(
        _IN struct spatial_hash_st * sph,
        _IN struct box_st box,
        _IN spatial_hash_get_callback_f get_callback,
        _IN void * data);

#endif 

