#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <check.h>
#include <private/vector.h>
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

#define MAX_TRI_PATH 1024

void _export_triangulation(
        mesh_st * mesh,
        const char * suffix)
{
    char name[MAX_TRI_PATH] = {'\0'};
    snprintf(name, MAX_TRI_PATH, "build/tri_%s.stl", suffix);
    mesh_export_stlb(mesh, name);
}

#define export_triangulation(mesh) _export_triangulation((mesh), __func__)

START_TEST(test_triangulation_is_clean)
{
    struct mesh_st mesh = generate_pointcloud_2d(10);
    enum error_code_e err = mesh_delaunay_triangulation(&mesh, pp_xy);
    ck_assert_int_lt(array_length(mesh.faces), 10000);
    export_triangulation(&mesh);
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
    export_triangulation(&mesh);
    ck_assert_int_eq(err, ec_no_error);
    for(size_t f = 0; f < array_length(mesh.faces); f++){
        for(int32_t i = 0;  i < FACE_SIZE; i++){
            ck_assert_int_ne(mesh.faces[f].f[i], 9);
        }
    }
    mesh_cleanup(&mesh);
}
END_TEST

START_TEST(test_triangulation_on_limits)
{
    struct mesh_st mesh = generate_pointcloud_2d(10);
    mesh.vertices[0].point.v[0] = 0.0;
    mesh.vertices[1].point.v[0] = 0.0;
    mesh.vertices[2].point.v[1] = 0.0;
    mesh.vertices[3].point.v[1] = 0.0;
    mesh.vertices[5].point.v[1] = 0.0;
    mesh.vertices[5].point.v[0] = 0.0;
    mesh.vertices[10].point.v[1] = 0.0;
    mesh.vertices[15].point.v[0] = 0.0;
    enum error_code_e err = mesh_delaunay_triangulation(&mesh, pp_xy);
    export_triangulation(&mesh);
    ck_assert_int_eq(err, ec_no_error);
    ck_assert_int_eq(array_length(mesh.vertices), N_VERTEX);
    mesh_cleanup(&mesh);
}
END_TEST

START_TEST(test_triangulation_with_vertex_on_edge)
{
    struct mesh_st mesh = generate_pointcloud_2d(10);
    for(size_t i = 2; i < N_VERTEX; i += 2){
        vector_addition(
                &mesh.vertices[i-2].point,
                &mesh.vertices[i-1].point,
                &mesh.vertices[i].point);
        vector_scale_by_scalar(&mesh.vertices[i].point, 0.5,
                &mesh.vertices[i].point);
    }
    enum error_code_e err = mesh_delaunay_triangulation(&mesh, pp_xy);
    export_triangulation(&mesh);
    ck_assert_int_eq(err, ec_no_error);
    ck_assert_int_eq(array_length(mesh.vertices), N_VERTEX);
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
    tcase_add_test(tc, test_triangulation_on_limits);
    tcase_add_test(tc, test_triangulation_with_vertex_on_edge);
    suite_add_tcase(s, tc);
    return s;
}

