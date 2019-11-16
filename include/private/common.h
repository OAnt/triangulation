#ifndef GEOMETRY_SIGN_H
#define GEOMETRY_SIGN_H

enum error_code_e {
    ec_no_error = 0,
    ec_error = 1,
    ec_io_error = 2,
    ec_memory_error,
};

#define SIGN(x) ((x) > 0) - ((x) < 0)
#define MAX(x, y) (x) > (y) ? x : y
#define MIN(x, y) (x) < (y) ? x : y

#endif
