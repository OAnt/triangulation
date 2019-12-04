#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <check.h>
#include <public/mesh.h>
#include <private/delaunay_triangulation.h>
#include <private/array.h>

#define N_VERTEX 20

struct mesh_st generate_pointcloud_2d(double range)
{
    struct mesh_st mesh;
    mesh_init(&mesh);
    /*unsigned int state = time(NULL);*/
    /*printf("%d\n", state);*/
    unsigned int state = 1575405021;
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
    struct mesh_st mesh = generate_pointcloud_2d(10);
    enum error_code_e err = mesh_delaunay_triangulation(&mesh, pp_xy);
    ck_assert_int_lt(array_length(mesh.faces), 10000);
    mesh_export_stlb(&mesh, "build/tri.stl");
    ck_assert_int_eq(err, ec_no_error);
    ck_assert_int_eq(array_length(mesh.vertices), N_VERTEX);
    mesh_cleanup(&mesh);
}
END_TEST

START_TEST(test_triangulation_on_duplicated)
{
    struct mesh_st mesh = generate_pointcloud_2d(10);
    mesh.vertices[9] = mesh.vertices[4];
    enum error_code_e err = mesh_delaunay_triangulation(&mesh, pp_xy);
    mesh_export_stlb(&mesh, "build/tri1.stl");
    ck_assert_int_eq(err, ec_no_error);
    for(size_t f = 0; f < array_length(mesh.faces); f++){
        for(int32_t i = 0;  i < FACE_SIZE; i++){
            ck_assert_int_ne(mesh.faces[f].f[i], 9);
        }
    }
    mesh_cleanup(&mesh);
}
END_TEST

START_TEST(test_triangulation_on_invalid_mesh)
{
    struct mesh_st mesh;
    mesh_init(&mesh);
    struct vector_st v0 = {{0.0, 0.0, 0.0}};
    mesh_add_vertex(&mesh, v0, NULL);
    struct vector_st v1 = {{1.0, 0.0, 0.0}};
    mesh_add_vertex(&mesh, v1, NULL);
    ck_assert_int_eq(mesh_delaunay_triangulation(&mesh, pp_xy),
            ec_topology_error);
    struct vector_st v2 = {{1.0, 1.0, 0.0}};
    mesh_add_vertex(&mesh, v2, NULL);
    struct face_st f = {{0, 1, 2}};
    mesh_add_face(&mesh, f, NULL);
    ck_assert_int_eq(mesh_delaunay_triangulation(&mesh, pp_xy),
            ec_out_of_bound_error);
}
END_TEST

Suite * mk_delaunay_suite(void){
    Suite * s = suite_create("Delaunay");
    TCase * tc = tcase_create(
            "Delaunay");
    tcase_add_test(tc, test_triangulation_is_clean);
    tcase_add_test(tc, test_triangulation_on_duplicated);
    tcase_add_test(tc, test_triangulation_on_invalid_mesh);
    suite_add_tcase(s, tc);
    return s;
}

