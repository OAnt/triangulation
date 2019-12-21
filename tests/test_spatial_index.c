#include <check.h>
#include <string.h>
#include <private/debug.h>
#include <private/spatial_index.h>
#include <private/spatial_index_declarations.h>

extern uint32_t spatial_index_compute_level(
        struct box_st,
        struct box_st);

static struct box_st bounds = {{{1.0, 2.0, 0.0}}, {{11, 10, 0.0}}};
static struct box_st bigger = {{{0.0, 0.0, 0.0}}, {{20.0, 20.0, 0.0}}};
static struct box_st smaller = {{{1.1, 2.6, 0.0}}, {{3.4, 4.5, 0.0}}};
struct box_st bounds2 = {{{0.0, 0.0, 0.0}}, {{8.0, 8.0, 0.0}}};
struct box_st smaller2 = {{{0.0, 0.0, 0.0}}, {{1.0, 1.0, 0.0}}};

START_TEST(test_compute_level_bigger)
{
    ck_assert_uint_eq(spatial_index_compute_level(
                bounds, bigger), 0);
}
END_TEST

START_TEST(test_compute_level_smaller)
{
    // Note that the level does not care if you are overlapping several
    // cells
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
    ck_assert_uint_eq(spatial_index_compute_level(
                bounds2, smaller2), 3);
}
END_TEST

START_TEST(test_new_spatial_index)
{
    struct spatial_index_st * spi;
    ck_assert_int_eq(spatial_index_new(bounds, &spi), ec_no_error);
    spatial_index_delete(spi);
}
END_TEST

extern enum error_code_e spatial_index_locate_on_grid(
        struct box_st soft_boundaries,
        struct box_st _box,
        uint32_t level,
        spatial_index_key_computation_callback_f callback,
        void * data);

struct key_array_st{
    struct spatial_index_key_st * keys;
    uint32_t n_keys;
};

enum error_code_e spatial_index_cb(
        struct spatial_index_key_st key,
        void * data)
{
    struct key_array_st * array = (struct key_array_st*)data;
    array->keys[array->n_keys++] = key;
    return ec_no_error;
}

static struct spatial_index_key_st keys[15];

void reset_keys(void){
    memset(keys, 345, 10*sizeof(struct spatial_index_key_st));
}

START_TEST(test_locate_match)
{
    reset_keys();
    struct key_array_st array = {keys, 0};
    ck_assert_int_eq(spatial_index_locate_on_grid(
                bounds2, smaller2, 3, spatial_index_cb, &array),
            ec_no_error);
    ck_assert_uint_eq(array.n_keys, 1);
    ck_assert_uint_eq(keys[0].level, 3);
    ck_assert_int_eq(keys[0].cell.x, 0);
    ck_assert_int_eq(keys[0].cell.y, 0);
    reset_keys();
    array.n_keys = 0;
    ck_assert_int_eq(spatial_index_locate_on_grid(
                bounds2, smaller2, 4, spatial_index_cb, &array),
            ec_no_error);
    ck_assert_uint_eq(array.n_keys, 4);
    ck_assert_uint_eq(keys[0].level, 4);
    ck_assert_int_eq(keys[0].cell.x, 0);
    ck_assert_int_eq(keys[0].cell.y, 0);
    ck_assert_int_eq(keys[1].cell.x, 0);
    ck_assert_int_eq(keys[1].cell.y, 1);
    ck_assert_int_eq(keys[2].cell.x, 1);
    ck_assert_int_eq(keys[2].cell.y, 0);
    ck_assert_int_eq(keys[3].cell.x, 1);
    ck_assert_int_eq(keys[3].cell.y, 1);
}
END_TEST

START_TEST(test_locate_equal)
{
    reset_keys();
    struct key_array_st array = {keys, 0};
    ck_assert_int_eq(spatial_index_locate_on_grid(
                bounds, bounds, 0, spatial_index_cb, &array),
            ec_no_error);
    ck_assert_uint_eq(array.n_keys, 1);
    ck_assert_uint_eq(keys[0].level, 0);
    ck_assert_int_eq(keys[0].cell.x, 0);
    ck_assert_int_eq(keys[0].cell.y, 0);
}
END_TEST

START_TEST(test_locate_smaller)
{
    reset_keys();
    struct key_array_st array = {keys, 0};
    ck_assert_int_eq(spatial_index_locate_on_grid(
                bounds, smaller, 2, spatial_index_cb, &array),
            ec_no_error);
    ck_assert_uint_eq(array.n_keys, 2);
    ck_assert_uint_eq(keys[0].level, 2);
    ck_assert_int_eq(keys[0].cell.x, 0);
    ck_assert_int_eq(keys[0].cell.y, 0);
    ck_assert_int_eq(keys[1].cell.x, 0);
    ck_assert_int_eq(keys[1].cell.y, 1);
}
END_TEST

START_TEST(test_locate_outside)
{
    reset_keys();
    struct key_array_st array = {keys, 0};
    ck_assert_int_eq(spatial_index_locate_on_grid(
                bounds, smaller2, 2, spatial_index_cb, &array),
            ec_no_error);
    ck_assert_uint_eq(array.n_keys, 1);
    ck_assert_int_eq(keys[0].cell.x, -1);
    ck_assert_int_eq(keys[0].cell.y, -1);
}
END_TEST

START_TEST(test_locate_bigger)
{
    reset_keys();
    struct key_array_st array = {keys, 0};
    ck_assert_int_eq(spatial_index_locate_on_grid(
                bounds, bigger, 0, spatial_index_cb, &array),
            ec_no_error);
    ck_assert_uint_eq(array.n_keys, 12);
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
    tcase_add_test(sph_tc, test_new_spatial_index);
    tcase_add_test(sph_tc, test_locate_match);
    tcase_add_test(sph_tc, test_locate_equal);
    tcase_add_test(sph_tc, test_locate_smaller);
    tcase_add_test(sph_tc, test_locate_outside);
    tcase_add_test(sph_tc, test_locate_bigger);
    suite_add_tcase(s, sph_tc);
    return s;
}
