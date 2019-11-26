#ifndef GEOMETRY_PUBLIC_COMMON_H
#define GEOMETRY_PUBLIC_COMMON_H

/**
 * Marks an input variable as input
 */
#define _IN
/**
 * Marks an input variable as output
 */
#define _OUT

/**
 * Types of errors returned by functions
 */
enum error_code_e {
    ec_no_error = 0, /** The function did not encountered any error */
    ec_error = 1, /** The function encountered an unknown error */
    ec_io_error = 2, /** The function encountered an error during io operation */
    ec_memory_error, /** An allocation failed */
    ec_out_of_bound_error, /** An index was found to be out of the required range */
    ec_div_by_zero_error, /** A division by zero was attempted */
    ec_topology_error, /** Adding face would create a non manifold mesh */
};

#endif
