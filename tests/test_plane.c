#include <stdlib.h>
#include <check.h>
#include <private/plane.h>

START_TEST(vector_classification)
{
    struct plane_st pl_xy = {0, 0, 1, 0};
    struct vector_st v_above = {0, 0, 0.5};
    double v_classification = plane_vector_classify(
            &pl_xy,
            &v_above);
    ck_assert_double_gt(v_classification, 0.0);
}
END_TEST

Suite * vector_classification_suite(void){
    Suite * s = suite_create("Vector Classification");
    TCase * v_above = tcase_create("Point Above Success");
    tcase_add_test(v_above, vector_classification);
    suite_add_tcase(s, v_above);
    return s;
}

int main(void){
    Suite * s = vector_classification_suite();
    SRunner * sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    int n_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return n_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

