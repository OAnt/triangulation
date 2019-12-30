#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <check.h>
#include <private/array.h>
#include <private/debug.h>
#include <private/delaunay_triangulation.h>
#include <private/predicates.h>
#include <private/triangle.h>
#include <private/vector.h>
#include <public/mesh.h>

#define N_VERTEX 20

void validate_triangle_is_delaunay_conformant(
        struct mesh_st * mesh, 
        size_t face_index)
{
    struct face_st * face = &mesh->faces[face_index];
    struct triangle_st tr = {{
        mesh->points[face->f[0]],
        mesh->points[face->f[1]],
        mesh->points[face->f[2]],
    }};
    for(int32_t i = 0; i < FACE_SIZE; i++){
        size_t ngb_index = mesh_get_neighbors(mesh, face_index).f[i];
        if(ngb_index == INVALID_INDEX) continue;
        struct face_st * neighbor = &mesh->faces[ngb_index];
        for(int32_t j = 0; j < FACE_SIZE; j++){
            size_t v_index = neighbor->f[j];
            if(v_index != face->f[0] && v_index != face->f[1] && v_index != face->f[2]){
                /*vector_subtraction(&mesh->points[v_index], &cc_center, &cc_to_vertex);*/
                /*double sq_dist = vector_dot_product(&cc_to_vertex, &cc_to_vertex);*/
                /*ck_assert_float_gt(sq_dist, sq_cc_radius);*/
                bool is_in_circumcenter = vertex_is_in_triangle_circumcenter(
                            mesh->points[v_index], tr, pp_xy);
                if(is_in_circumcenter){
                    _vertex_is_in_triangle_circumcenter(
                            mesh->points[v_index], tr, pp_xy, true);
                }
                ck_assert(!is_in_circumcenter);
            }
        }
    }
}

void validate_mesh_is_delaunay_conformant(
        struct mesh_st * mesh)
{
    for(size_t f = 0; f < array_length(mesh->faces); f++){
        validate_triangle_is_delaunay_conformant(mesh, f);
    }
}

struct vector_st vector_distribution_uniform(struct vector_st v){
    return v;
}

struct mesh_st _generate_pointcloud_2d(double range, size_t n_vertices, unsigned int state,
        struct vector_st vector_distribution_f(struct vector_st))
{
    struct mesh_st mesh;
    mesh_init(&mesh);
    /*unsigned int state = time(NULL);*/
    for(int32_t i = 0; i < n_vertices; i++){
        struct vector_st v = VEC3(
            ((double)rand_r(&state)/(double)(RAND_MAX)) * range,
            ((double)rand_r(&state)/(double)(RAND_MAX)) * range,
            0.0
        );
        mesh_add_vertex(&mesh, vector_distribution_f(v), NULL);
    }
    return mesh;
}

struct mesh_st generate_pointcloud_2d(double range)
{
    return _generate_pointcloud_2d(range, N_VERTEX, 1575405021, vector_distribution_uniform);
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
    ck_assert_int_eq(array_length(mesh.points), N_VERTEX);
    validate_mesh_is_delaunay_conformant(&mesh);
    mesh_cleanup(&mesh);
}
END_TEST

START_TEST(test_triangulation_on_duplicated)
{
    struct mesh_st mesh = generate_pointcloud_2d(10);
    mesh.points[9] = mesh.points[4];
    enum error_code_e err = mesh_delaunay_triangulation(&mesh, pp_xy);
    export_triangulation(&mesh);
    ck_assert_int_eq(err, ec_no_error);
    for(size_t f = 0; f < array_length(mesh.faces); f++){
        for(int32_t i = 0;  i < FACE_SIZE; i++){
            ck_assert_int_ne(mesh.faces[f].f[i], 9);
        }
    }
    validate_mesh_is_delaunay_conformant(&mesh);
    mesh_cleanup(&mesh);
}
END_TEST

#define N_BOUNDARIES 4
struct vector_st boundary_vertices[N_BOUNDARIES] = {
    VEC3(0.0, 0.0, 0.0), VEC3(10.0, 0.0, 0.0),
    VEC3(0.0, 10.0, 0.0), VEC3(10.0, 10.0, 0.0)
};

START_TEST(test_triangulation_on_limits)
{
    struct mesh_st mesh = generate_pointcloud_2d(10);
    mesh.points[0].v[0] = 0.0;
    mesh.points[1].v[0] = 0.0;
    mesh.points[2].v[1] = 0.0;
    mesh.points[3].v[1] = 0.0;
    mesh.points[5].v[1] = 0.0;
    mesh.points[5].v[0] = 0.0;
    mesh.points[10].v[1] = 0.0;
    mesh.points[15].v[0] = 0.0;
    size_t boundary_vertices_indexes[N_BOUNDARIES];
    for(int32_t i = 0; i < N_BOUNDARIES; i++){
        mesh_add_vertex(&mesh, boundary_vertices[i],
                boundary_vertices_indexes + i);
    }
    struct face_st bg_0 = {{
        boundary_vertices_indexes[0],
        boundary_vertices_indexes[1],
        boundary_vertices_indexes[2]
    }};
    mesh_add_face(&mesh, bg_0, NULL);
    struct face_st bg_1 = {{
        boundary_vertices_indexes[2],
        boundary_vertices_indexes[1],
        boundary_vertices_indexes[3]
    }};
    mesh_add_face(&mesh, bg_1, NULL);
    enum error_code_e err = mesh_delaunay_triangulation_user_defined_boundaries(
            &mesh, pp_xy);
    export_triangulation(&mesh);
    ck_assert_int_eq(err, ec_no_error);
    int32_t vertex_5_found = 0;
    for(size_t f = 0; f < array_length(mesh.faces); f++){
        for(int32_t i = 0;  i < FACE_SIZE; i++){
            if(mesh.faces[f].f[i] == 5)
                vertex_5_found = 1;
        }
    }
    validate_mesh_is_delaunay_conformant(&mesh);
    mesh_cleanup(&mesh);
    ck_assert_int_eq(vertex_5_found, 0);
}
END_TEST

START_TEST(test_triangulation_with_vertex_on_edge)
{
    struct mesh_st mesh = generate_pointcloud_2d(10);
    for(size_t i = 2; i < N_VERTEX; i += 2){
        vector_addition(
                &mesh.points[i-2],
                &mesh.points[i-1],
                &mesh.points[i]);
        vector_scale_by_scalar(&mesh.points[i], 0.5,
                &mesh.points[i]);
    }
    enum error_code_e err = mesh_delaunay_triangulation(&mesh, pp_xy);
    export_triangulation(&mesh);
    ck_assert_int_eq(err, ec_no_error);
    ck_assert_int_eq(array_length(mesh.points), N_VERTEX);
    validate_mesh_is_delaunay_conformant(&mesh);
    mesh_cleanup(&mesh);
}
END_TEST

START_TEST(test_triangulation_on_invalid_mesh)
{
    struct mesh_st mesh;
    mesh_init(&mesh);
    struct vector_st v0 = VEC3(0.0, 0.0, 0.0);
    mesh_add_vertex(&mesh, v0, NULL);
    struct vector_st v1 = VEC3(1.0, 0.0, 0.0);
    mesh_add_vertex(&mesh, v1, NULL);
    ck_assert_int_eq(mesh_delaunay_triangulation(&mesh, pp_xy),
            ec_topology_error);
    struct vector_st v2 = VEC3(1.0, 1.0, 0.0);
    mesh_add_vertex(&mesh, v2, NULL);
    struct face_st f = {{0, 1, 2}};
    mesh_add_face(&mesh, f, NULL);
    ck_assert_int_eq(mesh_delaunay_triangulation(&mesh, pp_xy),
            ec_out_of_bound_error);
    validate_mesh_is_delaunay_conformant(&mesh);
    mesh_cleanup(&mesh);
}
END_TEST

struct vector_st vector_distribution_non_uniform(struct vector_st v){
    static double cst = 12.0;
    /*static double cst = 1.0;*/
    struct vector_st nv = VEC2(pow(v.v[0], cst), pow(v.v[1], cst));
    return nv;
}

START_TEST(test_delaunay_triangulation_performance)
{
    size_t _n_points = 100;
    for(int32_t i = 1; i < 12; i++){
        size_t n_points = i * _n_points;
        double size = 1.0;
        if(i == 11) n_points = 10000;
        if(i == 12) n_points = 100000;
        if(i == 13) n_points = 1000000;
        unsigned int state = time(NULL);
        struct mesh_st mesh = _generate_pointcloud_2d(size, n_points, state,
                vector_distribution_non_uniform);
        clock_t clk_start = clock();
        enum error_code_e err = mesh_delaunay_triangulation(&mesh, pp_xy);
        clock_t clk_end = clock();
        if(i >= 10){
            export_triangulation(&mesh);
        }
        validate_mesh_is_delaunay_conformant(&mesh);
        ck_assert_int_eq(err, ec_no_error);
        printf("Triangulation of %ld points done in %f seconds\n",
                n_points, ((float)(clk_end - clk_start))/CLOCKS_PER_SEC);
        mesh_cleanup(&mesh);
    }
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
    TCase * tc2 = tcase_create(
            "Delaunay Performance");
    tcase_set_timeout(tc2, 40);
    tcase_add_test(tc2, test_delaunay_triangulation_performance);
    suite_add_tcase(s, tc2);
    return s;
}

