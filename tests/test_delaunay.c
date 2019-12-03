#include "public/mesh.h"
#include <check.h>
#include <stdlib.h>
#include <time.h>
#include <private/delaunay_triangulation.h>
#include <private/array.h>

#define N_VERTEX 20

struct mesh_st generate_pointcloud_2d(void)
{
    struct mesh_st mesh;
    mesh_init(&mesh);
    unsigned int state = time(NULL);
    double range = 10.0;
    for(int32_t i = 0; i < N_VERTEX; i++){
        struct vector_st v = {{
            ((double)rand_r(&state)/(double)(RAND_MAX)) * range,
            ((double)rand_r(&state)/(double)(RAND_MAX)) * range,
            0.0
        }};
        mesh_add_vertex(&mesh, v, NULL);
    }
    return mesh;
}

START_TEST(test_triangulation_is_clean)
{
    struct mesh_st mesh = generate_pointcloud_2d();
    enum error_code_e err = mesh_delaunay_triangulation(&mesh, pp_xy);
    ck_assert_int_lt(array_length(mesh.faces), 10000);
    mesh_export_stlb(&mesh, "build/tri.stl");
    ck_assert_int_eq(err, ec_no_error);
    ck_assert_int_eq(array_length(mesh.vertices), N_VERTEX);
    mesh_cleanup(&mesh);
}
END_TEST

Suite * mk_delaunay_suite(void){
    Suite * s = suite_create("Delaunay");
    TCase * tc = tcase_create(
            "Delaunay");
    tcase_add_test(tc, test_triangulation_is_clean);
    suite_add_tcase(s, tc);
    return s;
}

