#include <check.h>
#include <math.h>
#include <private/vector.h>

START_TEST(vector_test_orthogonal_dot_product)
{
    struct vector_st a = {1, 0, 1};
    struct vector_st b = {0, 1, 0};
    ck_assert_double_eq(vector_dot_product(&a, &b), 0.0);
}
END_TEST

START_TEST(vector_test_colinear_dot_product)
{
    struct vector_st a = {1, 0, 0};
    struct vector_st b = {1, 0, 0};
    ck_assert_double_eq(vector_dot_product(&a, &b), 1.0);
}
END_TEST

START_TEST(vector_test_subtraction)
{
    struct vector_st a = {2, 3, 4.2};
    struct vector_st b = {1, -1, 8};
    vector_subtraction(&a, &b, &a);
    ck_assert_double_eq(a.v[0], 1.0);
    ck_assert_double_eq(a.v[1], 4.0);
    ck_assert_double_eq(a.v[2], -3.8);
    vector_subtraction(&a, &a, &a);
    ck_assert_double_eq(a.v[0], 0.0);
    ck_assert_double_eq(a.v[1], 0.0);
    ck_assert_double_eq(a.v[2], 0.0);
}
END_TEST

START_TEST(vector_test_scaling)
{
    struct vector_st a = {1, 2, -3.4};
    vector_scale_by_scalar(&a, 2.0, &a);
    ck_assert_double_eq(a.v[0], 2.0);
    ck_assert_double_eq(a.v[1], 4.0);
    ck_assert_double_eq(a.v[2], -6.8);
}

START_TEST(vector_test_addition)
{
    struct vector_st a = {2, 3, 4.2};
    vector_addition(&a, &a, &a);
    ck_assert_double_eq(a.v[0], 4.0);
    ck_assert_double_eq(a.v[1], 6.0);
    ck_assert_double_eq(a.v[2], 8.4);
}
END_TEST

START_TEST(vector_test_position)
{
    struct segment_st seg = {{{1, 0, 0}, {0, 1, 0}}};
    struct vector_st vec = {{0.1, 0.1, 0.0}};
    enum point_position_e pp = vector_position_relative_to_segment(&vec, &seg, pp_xy); 
    ck_assert(pp == pt_left);
    struct vector_st vec_ = {{1.1, 1.1, 0.0}};
    pp = vector_position_relative_to_segment(&vec_, &seg, pp_xy); 
    ck_assert(pp == pt_right);
    struct vector_st vec_on = {{0.5, 0.5, 0.0}};
    pp = vector_position_relative_to_segment(&vec_on, &seg, pp_xy); 
    ck_assert(pp == pt_on);
}
END_TEST

Suite * mk_vector_suite(void){
    Suite * s = suite_create("Vector");
    TCase * tc = tcase_create(
            "Vector");
    tcase_add_test(tc, vector_test_orthogonal_dot_product);
    tcase_add_test(tc, vector_test_colinear_dot_product);
    tcase_add_test(tc, vector_test_subtraction);
    tcase_add_test(tc, vector_test_scaling);
    tcase_add_test(tc, vector_test_addition);
    tcase_add_test(tc, vector_test_position);
    suite_add_tcase(s, tc);
    return s;
}

