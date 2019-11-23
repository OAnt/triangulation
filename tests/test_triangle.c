#include "private/plane.h"
#include <check.h>
#include <stdio.h>
#include <private/triangle.h>

static struct triangle_st tr_test = {
    {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};

START_TEST(triangle_classification_above)
{
    struct plane_st pl = {1, 1, 1, 0.1};
    ck_assert(triangle_plane_classify(&tr_test, &pl) == trcl_above);
}
END_TEST

START_TEST(triangle_classification_below)
{
    struct plane_st pl = {-1, -1, -1, 0.1};
    ck_assert(triangle_plane_classify(&tr_test, &pl) == trcl_below);
}
END_TEST

START_TEST(triangle_classification_instersect)
{
    struct plane_st pl = {-1, 1, -1, 0.1};
    ck_assert(triangle_plane_classify(&tr_test, &pl) == trcl_intersected);
}
END_TEST

START_TEST(triangle_classification_coplanar)
{
    struct plane_st pl = {1, 1, 1, 1};
    ck_assert(triangle_plane_classify(&tr_test, &pl) == trcl_coplanar);
}
END_TEST

START_TEST(triangle_compute_circumcenter_test)
{
    struct plane_st p = {1, 1, 1, 1};
    struct vector_st v = {10, 10, 10};
    ck_assert(triangle_compute_circumcircle_center(&tr_test, &v) ==
            ec_no_error);
    ck_assert_double_eq(plane_vector_classify(&p, &v), 0.0);
    struct vector_st cc_center_to_corner_0, \
        cc_center_to_corner_1, cc_center_to_corner_2;
    vector_subtraction(tr_test.t, &v, &cc_center_to_corner_0);
    double sq_dist_0 = vector_dot_product(
           &cc_center_to_corner_0,
           &cc_center_to_corner_0);
    vector_subtraction(tr_test.t + 1, &v, &cc_center_to_corner_1);
    double sq_dist_1 = vector_dot_product(
           &cc_center_to_corner_1,
           &cc_center_to_corner_1);
    vector_subtraction(tr_test.t + 2, &v, &cc_center_to_corner_2);
    double sq_dist_2 = vector_dot_product(
           &cc_center_to_corner_2,
           &cc_center_to_corner_2);
    ck_assert_double_eq(sq_dist_0, sq_dist_1);
    ck_assert_double_eq(sq_dist_2, sq_dist_1);
}
END_TEST

Suite * mk_triangle_classification_suite(void){
    Suite * s = suite_create("Triangle Classification");
    TCase * tr_classification = tcase_create(
            "Triangle Classification");
    tcase_add_test(tr_classification, triangle_classification_above);
    tcase_add_test(tr_classification, triangle_classification_below);
    tcase_add_test(tr_classification,
            triangle_classification_instersect);
    tcase_add_test(tr_classification,
            triangle_classification_coplanar);
    tcase_add_test(tr_classification,
            triangle_compute_circumcenter_test);
    suite_add_tcase(s, tr_classification);
    return s;
}

