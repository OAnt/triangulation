#include <stdio.h>
#include <check.h>
#include <private/array.h>

extern void array_debug_header(void * ptr, 
        size_t * n,
        size_t * m,
        size_t * size,
        size_t * offset);

void print_array(void * array){
    size_t n, m, size, offset;
    array_debug_header(array, &n, &m, &size, &offset);
    printf("%ld %ld %ld %ld\n", n, m, size, offset);
}

START_TEST(test_manipulation)
{
    int * array;
    ck_assert(array_new(int, 3, &array) == ec_no_error);
    ck_assert(array_length(array) == 3);
    ck_assert(array_resize(&array, 10) == ec_no_error);
    ck_assert(array_length(array) == 10);
    ck_assert(array_resize(&array, 1) == ec_no_error);
    ck_assert(array_shrink(&array) == ec_no_error);
    ck_assert(array_resize(&array, 3) == ec_no_error);
    ck_assert(array_delete(&array) == ec_no_error);
}
END_TEST

Suite * mk_array_classification_suite(void){
    Suite * s = suite_create("Array Classification");
    TCase * array_tc = tcase_create(
            "Array Classification");
    tcase_add_test(array_tc, test_manipulation);
    suite_add_tcase(s, array_tc);
    return s;
}

