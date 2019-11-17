#ifndef GEOMETRY_SIGN_H
#define GEOMETRY_SIGN_H

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
};

#define SIGN(x) ((x) > 0) - ((x) < 0)
#define MAX(x, y) (x) > (y) ? x : y
#define MIN(x, y) (x) < (y) ? x : y

#endif
