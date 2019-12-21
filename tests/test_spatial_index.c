#include <check.h>
#include <private/spatial_index.h>

extern uint32_t spatial_index_compute_level(
        struct box_st,
        struct box_st);

static struct box_st bounds = {{{1.0, 2.0, 0.0}}, {{11, 10, 0.0}}};
static struct box_st bigger = {{{0.0, 0.0, 0.0}}, {{20.0, 20.0, 0.0}}};
static struct box_st smaller = {{{1.1, 2.6, 0.0}}, {{3.4, 4.5, 0.0}}};

START_TEST(test_compute_level_bigger)
{
    ck_assert_uint_eq(spatial_index_compute_level(
                bounds, bigger), 0);
}
END_TEST

START_TEST(test_compute_level_smaller)
{
    ck_assert_uint_eq(spatial_index_compute_level(
                bounds, smaller), 2);
}
END_TEST

START_TEST(test_compute_level_equal)
{
    ck_assert_uint_eq(spatial_index_compute_level(
                bounds, bounds), 0);
}
END_TEST

START_TEST(test_compute_level_match)
{
    struct box_st bounds = {{{0.0, 0.0, 0.0}}, {{8.0, 8.0, 0.0}}};
    struct box_st smaller = {{{0.0, 0.0, 0.0}}, {{1.0, 1.0, 0.0}}};
    ck_assert_uint_eq(spatial_index_compute_level(
                bounds, smaller), 3);
}
END_TEST

Suite * mk_spatial_index_suite(void){
    Suite * s = suite_create("Spatial Index");
    TCase * sph_tc = tcase_create(
            "Spatial Index");
    tcase_add_test(sph_tc, test_compute_level_bigger);
    tcase_add_test(sph_tc, test_compute_level_smaller);
    tcase_add_test(sph_tc, test_compute_level_equal);
    tcase_add_test(sph_tc, test_compute_level_match);
    suite_add_tcase(s, sph_tc);
    return s;
}
