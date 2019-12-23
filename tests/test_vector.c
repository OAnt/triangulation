#include <check.h>
#include <math.h>
#include <private/vector.h>

START_TEST(vector_test_orthogonal_dot_product)
{
    struct vector_st a = {{1, 0, 1}};
    struct vector_st b = {{0, 1, 0}};
    ck_assert_double_eq(vector_dot_product(&a, &b), 0.0);
}
END_TEST

START_TEST(vector_test_colinear_dot_product)
{
    struct vector_st a = {{1, 0, 0}};
    struct vector_st b = {{1, 0, 0}};
    ck_assert_double_eq(vector_dot_product(&a, &b), 1.0);
}
END_TEST

START_TEST(vector_test_subtraction)
{
    struct vector_st a = {{2, 3, 4.2}};
    struct vector_st b = {{1, -1, 8}};
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
    struct vector_st a = {{1, 2, -3.4}};
    vector_scale_by_scalar(&a, 2.0, &a);
    ck_assert_double_eq(a.v[0], 2.0);
    ck_assert_double_eq(a.v[1], 4.0);
    ck_assert_double_eq(a.v[2], -6.8);
}

START_TEST(vector_test_addition)
{
    struct vector_st a = {{2, 3, 4.2}};
    vector_addition(&a, &a, &a);
    ck_assert_double_eq(a.v[0], 4.0);
    ck_assert_double_eq(a.v[1], 6.0);
    ck_assert_double_eq(a.v[2], 8.4);
}
END_TEST

START_TEST(vector_test_position)
{
    struct segment_st seg = {{
        {{1, 0, 0}}, {{0, 1, 0}}
    }};
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

START_TEST(vector_test_cross_product)
{
    struct vector_st x = {{1.0, 0.0, 0.0}};
    struct vector_st y = {{0.0, 1.0, 0.0}};
    struct vector_st z = {{0.0, 0.0, 1.0}};
    struct vector_st _x = {{-1.0, 0.0, 0.0}};
    struct vector_st _y = {{0.0, -1.0, 0.0}};
    struct vector_st _z = {{0.0, 0.0, -1.0}};
    struct vector_st zero = {{0.0, 0.0, 0.0}};
    struct vector_st tmp = {{0.0, 0.0, 0.0}};
    vector_cross_product(&x, &y, &tmp);
    ck_assert_mem_eq(&tmp, &z, sizeof(struct vector_st));
    vector_cross_product(&y, &z, &tmp);
    ck_assert_mem_eq(&tmp, &x, sizeof(struct vector_st));
    vector_cross_product(&z, &x, &tmp);
    ck_assert_mem_eq(&tmp, &y, sizeof(struct vector_st));
    vector_cross_product(&x, &x, &tmp);
    ck_assert_mem_eq(&tmp, &zero, sizeof(struct vector_st));
    vector_cross_product(&y, &y, &tmp);
    ck_assert_mem_eq(&tmp, &zero, sizeof(struct vector_st));
    vector_cross_product(&z, &z, &tmp);
    ck_assert_mem_eq(&tmp, &zero, sizeof(struct vector_st));
    vector_cross_product(&y, &x, &tmp);
    ck_assert_mem_eq(&tmp, &_z, sizeof(struct vector_st));
    vector_cross_product(&z, &y, &tmp);
    ck_assert_mem_eq(&tmp, &_x, sizeof(struct vector_st));
    vector_cross_product(&x, &z, &tmp);
    ck_assert_mem_eq(&tmp, &_y, sizeof(struct vector_st));
}
END_TEST

START_TEST(test_axis_system)
{
    int32_t x = 3;
    int32_t y = 3;
    get_axis_system_from_projection_plane(pp_xy, &x, &y);
    ck_assert_int_eq(x, 0);
    ck_assert_int_eq(y, 1);
    get_axis_system_from_projection_plane(pp_yz, &x, &y);
    ck_assert_int_eq(x, 1);
    ck_assert_int_eq(y, 2);
    get_axis_system_from_projection_plane(pp_zx, &x, &y);
    ck_assert_int_eq(x, 0);
    ck_assert_int_eq(y, 2);
}
END_TEST

START_TEST(test_2D_boxes_intersect)
{
    struct box_st a = {{{1.0, 3.0, 0.0}}, {{3.0, 6.0, 0.0}}};
    struct box_st b = {{{1.5, 3.5, 0.0}}, {{7.0, 9.0, 0.0}}};
    ck_assert(box_intersection_2D(&a, &b));
}
END_TEST

START_TEST(test_2D_box_intersects_corner)
{
    struct box_st a = {{{1.0, 3.0, 0.0}}, {{3.0, 6.0, 0.0}}};
    struct box_st b = {{{3.0, 6.0, 0.0}}, {{3.0, 6.0, 0.0}}};
    ck_assert(box_intersection_2D(&a, &b) && box_intersection_2D(&b, &a));
}
END_TEST

START_TEST(test_2D_boxes_do_not_intersect)
{
    struct box_st a = {{{1.0, 3.0, 0.0}}, {{3.0, 6.0, 0.0}}};
    struct box_st b = {{{0.5, 0.5, 0.0}}, {{7.0, 2.0, 0.0}}};
    ck_assert(!box_intersection_2D(&a, &b));
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
    tcase_add_test(tc, vector_test_cross_product);
    tcase_add_test(tc, test_axis_system);
    tcase_add_test(tc, test_2D_boxes_intersect);
    tcase_add_test(tc, test_2D_box_intersects_corner);
    tcase_add_test(tc, test_2D_boxes_do_not_intersect);
    suite_add_tcase(s, tc);
    return s;
}

