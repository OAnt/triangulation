#include <check.h>
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
    suite_add_tcase(s, tr_classification);
    return s;
}

