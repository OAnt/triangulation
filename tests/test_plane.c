#include <check.h>
#include <private/plane.h>

static struct plane_st pl_test = {{1, 1, 1, 1}};

START_TEST(vector_classification_above)
{
    struct vector_st v_above = VEC3(0.5, 0.5, 0.5);
    double v_classification = plane_vector_classify(
            &pl_test,
            &v_above);
    ck_assert_double_gt(v_classification, 0.0);
}
END_TEST

START_TEST(vector_classification_below)
{
    struct vector_st v_above = VEC3(0.2, 0.2, 0.2);
    double v_classification = plane_vector_classify(
            &pl_test,
            &v_above);
    ck_assert_double_lt(v_classification, 0.0);
}
END_TEST

START_TEST(vector_classification_on_plane)
{
    struct vector_st v_above = VEC3(1.0/3.0, 1.0/3.0, 1.0/3.0);
    double v_classification = plane_vector_classify(
            &pl_test,
            &v_above);
    ck_assert_double_eq(v_classification, 0.0);
}
END_TEST

Suite * mk_vector_classification_suite(void){
    Suite * s = suite_create("Vector Classification");
    TCase * v_classification = tcase_create("Vector Classification");
    tcase_add_test(v_classification, vector_classification_above);
    tcase_add_test(v_classification, vector_classification_below);
    tcase_add_test(v_classification, vector_classification_on_plane);
    suite_add_tcase(s, v_classification);
    return s;
}

