#include <stdio.h>
#include <check.h>
#include <private/array.h>

int arr_1[] = {1, 2, 3};
size_t n_arr_1 = sizeof(arr_1) / sizeof(arr_1[0]);
int arr_2[] = {4, 5, 6, 7};
size_t n_arr_2 = sizeof(arr_2) / sizeof(arr_2[0]);
int arr_3[] = {9, 10};
size_t n_arr_3 = sizeof(arr_3) / sizeof(arr_3[0]);

START_TEST(test_array_extension)
{
    struct array_st int_ar;
    array_init(&int_ar, int);
    ck_assert(array_extend(&int_ar, arr_1, n_arr_1) == ec_no_error);
    ck_assert(array_size(&int_ar) == n_arr_1);
    ck_assert(array_extend(&int_ar, arr_2, n_arr_2) == ec_no_error);
    ck_assert(array_size(&int_ar) == n_arr_1 + n_arr_2);
    ck_assert(array_extend(&int_ar, arr_3, n_arr_3) == ec_no_error);
    ck_assert(array_size(&int_ar) == n_arr_1 + n_arr_2 + n_arr_3);
    ck_assert_mem_eq(array_ptr(&int_ar), arr_1, n_arr_1*sizeof(int));
    ck_assert_mem_eq(
            ((int*)array_ptr(&int_ar)) + n_arr_1,
            arr_2, n_arr_2*sizeof(int));
    array_cleanup(&int_ar);
}
END_TEST

START_TEST(test_array_retraction)
{
    struct array_st int_ar;
    array_init(&int_ar, int);
    ck_assert(array_extend(&int_ar, arr_2, n_arr_2) == ec_no_error);
    void * arr;
    size_t n_rem = 0;
    ck_assert(array_retract(
                &int_ar, 2, &arr,  &n_rem) == ec_no_error);
    ck_assert(n_rem == 2);
    ck_assert(array_size(&int_ar) == 2);
    ck_assert_mem_eq(arr, arr_2 + 2, 2*sizeof(int));
    ck_assert(array_retract(
                &int_ar, 10, &arr,  &n_rem) == ec_no_error);
    ck_assert(n_rem == 2);
    ck_assert(array_size(&int_ar) == 0);
    ck_assert_mem_eq(arr, arr_2, 2*sizeof(int));
    array_cleanup(&int_ar);
}
END_TEST

START_TEST(test_array_set)
{
    struct array_st int_ar;
    array_init(&int_ar, int);
    ck_assert(array_set(&int_ar, arr_1, 0, n_arr_1) == ec_no_error);
    ck_assert(array_size(&int_ar) == n_arr_1);
    ck_assert_mem_eq(array_ptr(&int_ar), arr_1,
            n_arr_1 * sizeof(int));
    ck_assert(array_set(&int_ar, arr_3, 1, n_arr_3) == ec_no_error);
    ck_assert(array_size(&int_ar) == n_arr_1);
    ck_assert_mem_eq((int*)array_ptr(&int_ar) + 1, arr_3,
            n_arr_3 * sizeof(int));
    ck_assert(array_set(&int_ar, arr_2, 0, n_arr_2) == ec_no_error);
    ck_assert(array_size(&int_ar) == n_arr_2);
    ck_assert_mem_eq(array_ptr(&int_ar), arr_2,
            n_arr_2 * sizeof(int));

    array_cleanup(&int_ar);
}
END_TEST

Suite * mk_array_classification_suite(void){
    Suite * s = suite_create("Array Classification");
    TCase * array_tc = tcase_create(
            "Array Classification");
    tcase_add_test(array_tc, test_array_extension);
    tcase_add_test(array_tc, test_array_retraction);
    tcase_add_test(array_tc, test_array_set);
    suite_add_tcase(s, array_tc);
    return s;
}

