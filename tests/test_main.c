#include <stdlib.h>
#include <check.h>

typedef Suite * (*mk_geo_test_suite_t)(void);

extern Suite * mk_vector_classification_suite(void);
extern Suite * mk_triangle_classification_suite(void);

mk_geo_test_suite_t all_suites[] = {
    mk_vector_classification_suite,
    mk_triangle_classification_suite,
};

int main(void){
    int n_suites = sizeof(all_suites) / sizeof(all_suites[0]);
    SRunner * sr;
    for(int i = 0; i < n_suites; i++){
        Suite * s = all_suites[i]();
        if(i == 0){
            sr = srunner_create(s);
        }else{
            srunner_add_suite(sr, s);
        }
    }
    srunner_run_all(sr, CK_NORMAL);
    int n_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return n_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

