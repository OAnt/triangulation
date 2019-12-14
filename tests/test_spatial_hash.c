#include <check.h>
#include <private/spatial_hash.h>

static double cell_size = 10.0;
static int32_t n_cells = 10;

struct object_st {
    struct vector_st min;
    struct vector_st max;
    size_t index;
};

#define N_OBJECTS 3
static struct object_st objects[N_OBJECTS] = {
    {{{5.0, 5.0, 0.0}}, {{15.0, 6.0, 0.0}}, 0},
    {{{45.0, 45.0, 0.0}}, {{46.0, 46.0, 0.0}}, 1},
    {{{7.0, 7.0, 0.0}}, {{8.0, 16.0, 0.0}}, 2}
};

bool increment(void * data){
    int32_t * count = (int32_t *)data;
    *count += 1;
    return false;
}

bool get_object_0(size_t index, void * data){
    ck_assert_int_eq(index, 0);
    return increment(data);
}

bool get_object_0_or_2(size_t index, void * data){
    ck_assert(index == 0 || index == 2);
    return increment(data);
}

bool get_object_2(size_t index, void * data){
    ck_assert_int_eq(index, 2);
    return increment(data);
}

bool get_object_1(size_t index, void * data){
    ck_assert_int_eq(index, 1);
    return increment(data);
}

struct query_st{
    struct vector_st min;
    struct vector_st max;
    int32_t expected_count;
    spatial_hash_get_callback_f get_callback;
};

#define N_QUERIES 5
struct query_st queries[N_QUERIES] = {
    {{{4.0, 3.0, 0.0}}, {{5.0, 5.0, 0.0}}, 2, get_object_0_or_2},
    {{{11.0, 2.0, 0.0}}, {{13.0, 8.0, 0.0}}, 1, get_object_0},
    {{{6.0, 11.0, 0.0}}, {{14.0, 12.0, 0.0}}, 1, get_object_2},
    {{{40.0, 40.0, 0.0}}, {{60.0, 60.0, 0.0}}, 1, get_object_1},
    {{{0.0, 0.0, 0.0}}, {{30.0, 30.0, 0.0}}, 4, get_object_0_or_2},
};

void spatial_hash_fill(
        struct spatial_hash_st * sph,
        size_t handles[N_OBJECTS])
{
    for(int32_t i = 0; i < N_OBJECTS; i++){
        ck_assert(spatial_hash_add(
                    sph,
                    objects[i].min,
                    objects[i].max,
                    objects[i].index,
                    handles + i) == ec_no_error);
    }
}

struct spatial_hash_st * _spatial_hash_create(void){
    struct spatial_hash_st * sph;
    ck_assert(spatial_hash_new(
                n_cells, n_cells, cell_size, cell_size, &sph) == ec_no_error);
    return sph;
}

struct spatial_hash_st * spatial_hash_create(size_t handles[N_OBJECTS]){
    struct spatial_hash_st * sph = _spatial_hash_create();
    spatial_hash_fill(sph, handles);
    return sph;
}

START_TEST(test_spatial_hash_get)
{
    size_t handles[N_OBJECTS];
    struct spatial_hash_st * sph = spatial_hash_create(handles);
    for(int32_t i = 0; i < N_QUERIES; i++){
        int32_t count = 0;
        spatial_hash_get(sph, queries[i].min, queries[i].max,
                queries[i].get_callback, &count);
        ck_assert_int_eq(count, queries[i].expected_count);
    }
    spatial_hash_delete(&sph);
}
END_TEST

START_TEST(test_spatial_hash_remove)
{
    size_t handles[N_OBJECTS];
    struct spatial_hash_st * sph = spatial_hash_create(handles);
    spatial_hash_remove(sph, handles[0]);
    int32_t count = 0;
    spatial_hash_get(sph,
            queries[0].min, queries[0].max, get_object_2, &count);
    ck_assert_int_eq(count, 1);
    count = 0;
    spatial_hash_get(sph,
            queries[1].min, queries[1].max, get_object_0, &count);
    ck_assert_int_eq(count, 0);
    count = 0;
    spatial_hash_get(sph,
            queries[4].min, queries[4].max, get_object_2, &count);
    ck_assert_int_eq(count, 2);
    spatial_hash_remove(sph, handles[1]);
    count = 0;
    spatial_hash_get(sph,
            queries[3].min, queries[3].max, get_object_1, &count);
    ck_assert_int_eq(count, 0);
    spatial_hash_remove(sph, handles[2]);
    count = 0;
    spatial_hash_get(sph,
            queries[2].min, queries[2].max, get_object_2, &count);
    ck_assert_int_eq(count, 0);
    spatial_hash_delete(&sph);
}
END_TEST

START_TEST(test_spatial_hash_thrash)
{
    struct spatial_hash_st * sph = _spatial_hash_create();
    for(int32_t i = 0; i < 10; i++){
        size_t handles[N_OBJECTS];
        spatial_hash_fill(sph, handles);
        for(int32_t j = 0; j < N_OBJECTS; j++){
            spatial_hash_remove(sph, handles[j]);
        }
    }
    
    spatial_hash_delete(&sph);
}
END_TEST

Suite * mk_spatial_hash_suite(void){
    Suite * s = suite_create("Spatial Hash");
    TCase * sph_tc = tcase_create(
            "Spatial Hash");
    tcase_add_test(sph_tc, test_spatial_hash_get);
    tcase_add_test(sph_tc, test_spatial_hash_remove);
    tcase_add_test(sph_tc, test_spatial_hash_thrash);
    suite_add_tcase(s, sph_tc);
    return s;
}
