#include <check.h>
#include <private/debug.h>
#include <private/mesh.h>
#include <private/array.h>

struct vector_st vertices[] = {
    VEC3(1.0, 0.0, 0.0), VEC3(0.0, 1.0, 0.0), VEC3(0.0, 0.0, 1.0)};

START_TEST(test_mesh_add_features){
    struct mesh_st mesh;
    ck_assert(mesh_init(&mesh) == ec_no_error);
    for(size_t i = 0; i < FACE_SIZE; i++){
        size_t index;
        ck_assert(mesh_add_vertex(
                    &mesh, vertices[i], &index) == ec_no_error);
        ck_assert(index == i);
    }
    size_t index;
    struct face_st face = {{0, 1, 2}};
    ck_assert(mesh_add_face(&mesh, face, &index) == ec_no_error);
    ck_assert(index == 0);
    ck_assert(mesh_cleanup(&mesh) == ec_no_error);
}
END_TEST

START_TEST(test_mesh_add_face_fails){
    struct mesh_st mesh;
    ck_assert(mesh_init(&mesh) == ec_no_error);
    size_t index;
    struct face_st face = {{0, 1, 2}};
    ck_assert(mesh_add_face(
                &mesh, face, &index) == ec_out_of_bound_error);
    for(size_t i = 0; i < FACE_SIZE; i++){
        size_t index;
        ck_assert(mesh_add_vertex(
                    &mesh, vertices[i], &index) == ec_no_error);
        ck_assert(index == i);
    }
    ck_assert(mesh_add_face(&mesh, face, &index) == ec_no_error);
    ck_assert(index == 0);
    struct vector_st another_vertex = VEC3(1.0, 1.0, 1.0);
    ck_assert(mesh_add_vertex(
                &mesh, another_vertex, &index) == ec_no_error);
    struct vector_st yet_another_vertex = VEC3(-1.0, -1.0, -1.0);
    ck_assert(mesh_add_vertex(
                &mesh, yet_another_vertex, &index) == ec_no_error);
    struct face_st another_face = {{1, 3, 2}};
    ck_assert(mesh_add_face(
                &mesh, another_face, &index) == ec_no_error);
    ck_assert(index == 1);
    struct face_st yet_another_face = {{1, 4, 2}};
    ck_assert(mesh_add_face(
                &mesh, yet_another_face, &index) == ec_topology_error);
    struct face_st still_another_face = {{0, 3, 2}};
    ck_assert(mesh_add_face(
                &mesh, still_another_face, &index) == ec_no_error);
    ck_assert(index == 2);
    ck_assert(array_length(mesh.faces) == 3);
    /*ck_assert(array_length(mesh.neighbors) == 3);*/
    ck_assert(mesh_cleanup(&mesh) == ec_no_error);
}
END_TEST

struct vector_st cube_vertices[] = {
    VEC3(0, 0, 0), VEC3(1, 0, 0), VEC3(1, 1, 0), VEC3(0, 1, 0),
    VEC3(0, 0, 1), VEC3(1, 0, 1), VEC3(1, 1, 1), VEC3(0, 1, 1)
};
struct face_st cube_faces[] = {
    {{0, 1, 2}}, {{2, 3, 0}}, {{0, 5, 1}}, {{0, 4, 5}},
    {{2, 1, 5}}, {{5, 6, 2}}, {{3, 2, 7}}, {{2, 6, 7}},
    {{6, 5, 4}}, {{7, 6, 4}}, {{3, 4, 0}}, {{3, 7, 4}}
};
struct face_st neighbors[] = {
    {{2, 4, 1}}, {{6, 10, 0}}, {{3, 4, 0}}, {{10, 8, 2}}
};

struct mesh_st create_mesh(
        struct vector_st * vecs,
        size_t n_vecs,
        struct face_st * faces,
        size_t n_faces)
{
    struct mesh_st mesh;
    ck_assert(mesh_init(&mesh) == ec_no_error);
    for(size_t i = 0; i < n_vecs; i++){
        ck_assert(
                mesh_add_vertex(&mesh, vecs[i], NULL) ==
                ec_no_error);
    }
    for(size_t i = 0; i < n_faces; i++){
        ck_assert(
                mesh_add_face(&mesh, faces[i], NULL) ==
                ec_no_error);
    }
    return mesh;
}

struct mesh_st create_cube_mesh(void)
{
    return create_mesh(cube_vertices, 8, cube_faces, 12);
}

START_TEST(test_stl_export_stlb)
{
    struct mesh_st mesh = create_cube_mesh();
    int s = sizeof(neighbors) / sizeof(struct face_st);
    for(int i = 0; i < s; i++){
        struct face_st neighbor = mesh_get_neighbors(&mesh, i);
        ck_assert_mem_eq(&neighbor, neighbors + i,
                sizeof(struct face_st));
    }
    ck_assert(mesh_export_stlb(&mesh, "build/cube.stl") == ec_no_error);
    mesh_cleanup(&mesh);
}
END_TEST

START_TEST(test_mesh_replace_face)
{
    struct mesh_st mesh;
    ck_assert(mesh_init(&mesh) == ec_no_error);
    for(size_t i = 0; i < 5; i++){
        ck_assert(
                mesh_add_vertex(&mesh, cube_vertices[i], NULL) ==
                ec_no_error);
    }
    struct face_st face = {{0, 1, 2}};
    ck_assert(mesh_add_face(
                &mesh, face, NULL) == ec_no_error);
    struct face_st other_face = {{0, 4, 1}};
    ck_assert(mesh_add_face(
                &mesh, other_face, NULL) == ec_no_error);
    struct face_st replacement_face = {{0, 1, 3}};
    ck_assert(mesh_replace_face(
                &mesh, replacement_face, 0) == ec_no_error);
    struct face_st other_neighbors = {{INVALID_INDEX, INVALID_INDEX, 0}};
    struct face_st neighbor = mesh_get_neighbors(&mesh, 1);
    ck_assert_mem_eq(&neighbor, &other_neighbors, sizeof(struct face_st));
    struct face_st replacement_neighbors = {{1, INVALID_INDEX, INVALID_INDEX}};
    struct face_st neighbor2 = mesh_get_neighbors(&mesh, 0);
    ck_assert_mem_eq(&neighbor2, &replacement_neighbors, sizeof(struct face_st));
    ck_assert_mem_eq(mesh.faces, &replacement_face, sizeof(struct face_st));
    mesh_cleanup(&mesh);
}
END_TEST

void validate_mesh(struct mesh_st mesh){
    struct mesh_st unchanged_mesh = create_cube_mesh();
    ck_assert_mem_eq(mesh.faces, unchanged_mesh.faces ,
            array_length(unchanged_mesh.faces) * sizeof(struct face_st)); 
    for(size_t f = 0; f < array_length(mesh.faces); f++){
        struct face_st neighbor = mesh_get_neighbors(&mesh, f);
        struct face_st uneighbor = mesh_get_neighbors(&unchanged_mesh, f);
        ck_assert_mem_eq(&neighbor, &uneighbor,
                sizeof(struct face_st)); 
    }
    mesh_cleanup(&unchanged_mesh);
}

START_TEST(test_mesh_replace_no_border_effects)
{
    struct mesh_st mesh = create_cube_mesh();
    ck_assert_int_eq(mesh_replace_face(&mesh, cube_faces[0], 3), ec_topology_error);
    validate_mesh(mesh);
    mesh_cleanup(&mesh);
}
END_TEST

START_TEST(test_mesh_replace_no_border_effects2)
{
    struct mesh_st mesh = create_cube_mesh();
    struct face_st invalid_face = {{100, 101, 102}};
    ck_assert_int_eq(mesh_replace_face(&mesh, invalid_face, 2), ec_out_of_bound_error);
    validate_mesh(mesh);
    mesh_cleanup(&mesh);
}
END_TEST

START_TEST(test_mesh_enclosing_triangular_face)
{
    struct mesh_st mesh = create_cube_mesh();
    size_t face_index;
    struct vector_st point = VEC3(0.1, 0.9, 0.0);
    ck_assert_int_eq(unindexed_mesh_find_first_enclosing_triangular_face(
                &mesh, point, pp_xy, &face_index), ec_no_error);
    ck_assert_uint_eq(face_index, 1);
    struct vector_st another_point = VEC3(0.9, 0.9, 0.0);
    ck_assert_int_eq(unindexed_mesh_find_first_enclosing_triangular_face(
                &mesh, another_point, pp_xy, &face_index), ec_no_error);
    ck_assert_uint_eq(face_index, 0);
    struct vector_st yet_another_point = VEC3(100.0, 40.0, 0.0);
    ck_assert_int_eq(unindexed_mesh_find_first_enclosing_triangular_face(
                &mesh, yet_another_point, pp_xy, &face_index), ec_error);
    mesh_cleanup(&mesh);
}
END_TEST

struct vector_st ladder_vertices[] = {
    VEC2(0, 0), VEC2(1, 0), VEC2(0, 1), VEC2(1, 1),
    VEC2(0, 2), VEC2(1, 2), VEC2(0, 3), VEC2(1, 3),
    VEC2(2, 0), VEC2(1, -1), VEC2(2, -1),
};

struct face_st ladder_faces[] = {
    {{0, 1, 2}}, {{1, 3, 2}}, {{2, 3, 5}},
    {{2, 5, 4}}, {{4, 5, 6}}, {{5, 7, 6}},
    {{1, 8, 3}}, {{9, 8, 1}}, {{9, 10, 8}},
};

struct mesh_st create_ladder_mesh(void){
    return create_mesh(ladder_vertices, 11, ladder_faces, 9);
}

bool edge_intersection_callback(struct edge_spec_st edge, bool collinear, void * data){
    struct edge_spec_st ** edges = (struct edge_spec_st **)data;
    size_t n_edges = array_length(*edges);
    array_resize(edges, n_edges + 1);
    (*edges)[n_edges] = edge;
    return false;
}

bool edge_intersection_callback_collinear(
        struct edge_spec_st edge, bool collinear, void * data)
{
    ck_assert(collinear);
    return edge_intersection_callback(edge, collinear, data);
}

bool edge_intersection_callback_cross(
        struct edge_spec_st edge, bool collinear, void * data)
{
    ck_assert(!collinear);
    return edge_intersection_callback(edge, collinear, data);
}


START_TEST(test_mesh_edge_intersection_generic)
{
    struct mesh_st ladder = create_ladder_mesh();
    struct edge_spec_st * edges;
    array_new(struct edge_spec_st, 0, &edges);
    ck_assert(mesh_iterate_over_projected_intersecting_edges(
            &ladder, 0, 7, 0, 1, edge_intersection_callback_cross, &edges) == ec_no_error);
    ck_assert_int_eq(array_length(edges), 5);
    array_resize(&edges, 0);
    ck_assert(mesh_iterate_over_projected_intersecting_edges(
            &ladder, 1, 6, 0, 1, edge_intersection_callback_cross, &edges) == ec_no_error);
    ck_assert_int_eq(array_length(edges), 3);
    array_resize(&edges, 0);
    ck_assert(mesh_iterate_over_projected_intersecting_edges(
            &ladder, 4, 1, 0, 1, edge_intersection_callback_cross, &edges) == ec_no_error);
    ck_assert_int_eq(array_length(edges), 2);
    array_resize(&edges, 0);
    ck_assert(mesh_iterate_over_projected_intersecting_edges(
            &ladder, 4, 3, 0, 1, edge_intersection_callback_cross, &edges) == ec_no_error);
    ck_assert_int_eq(array_length(edges), 1);
    array_delete(&edges);
    mesh_cleanup(&ladder);
}
END_TEST

START_TEST(test_mesh_edge_intersection_collinear)
{
    struct mesh_st ladder = create_ladder_mesh();
    struct edge_spec_st * edges;
    array_new(struct edge_spec_st, 0, &edges);
    ck_assert(mesh_iterate_over_projected_intersecting_edges(
            &ladder, 1, 2, 0, 1, edge_intersection_callback_collinear, &edges) == ec_no_error);
    ck_assert_int_eq(array_length(edges), 1);
    array_resize(&edges, 0);
    ck_assert(mesh_iterate_over_projected_intersecting_edges(
            &ladder, 0, 6, 0, 1, edge_intersection_callback_collinear, &edges) == ec_no_error);
    ck_assert_int_eq(array_length(edges), 3);
    array_delete(&edges);
    mesh_cleanup(&ladder);
}
END_TEST

START_TEST(test_mesh_edge_intersection_mixed)
{
    struct mesh_st ladder = create_ladder_mesh();
    struct edge_spec_st * edges;
    array_new(struct edge_spec_st, 0, &edges);
    ck_assert(mesh_iterate_over_projected_intersecting_edges(
            &ladder, 2, 10, 0, 1, edge_intersection_callback, &edges) == ec_no_error);
    ck_assert_int_eq(array_length(edges), 2);
    array_delete(&edges);
    mesh_cleanup(&ladder);
}
END_TEST

START_TEST(test_mesh_edge_intersection_fails)
{
    struct mesh_st ladder = create_ladder_mesh();
    struct edge_spec_st * edges;
    array_new(struct edge_spec_st, 0, &edges);
    ck_assert(mesh_iterate_over_projected_intersecting_edges(
            &ladder, 0, 10, 0, 1, edge_intersection_callback_cross, &edges) == 
            ec_topology_error);
    ck_assert_int_eq(array_length(edges), 0);
    array_resize(&edges, 0);
    ck_assert(mesh_iterate_over_projected_intersecting_edges(
            &ladder, 10, 7, 0, 1, edge_intersection_callback, &edges) == ec_topology_error);
    ck_assert_int_eq(array_length(edges), 3);
    array_delete(&edges);
    mesh_cleanup(&ladder);
}
END_TEST

START_TEST(test_pop_face)
{
    struct mesh_st mesh = create_cube_mesh();
    struct face_st popped;
    mesh_pop_face(&mesh, &popped);
    ck_assert_mem_eq(&popped, cube_faces + 11, sizeof(struct face_st));
    ck_assert_int_eq(array_length(mesh.faces), 11);
    /*ck_assert_int_eq(array_length(mesh.neighbors), 11);*/
    for(int32_t i = 0; i < 11; i++){
        ck_assert_int_eq(mesh_pop_face(&mesh, &popped), ec_no_error);
    }
    ck_assert_int_eq(mesh_pop_face(&mesh, &popped), ec_out_of_bound_error);
    mesh_cleanup(&mesh);
}
END_TEST

START_TEST(test_remove_face)
{
    struct mesh_st mesh = create_cube_mesh();
    mesh_remove_face(&mesh, 0);
    ck_assert_int_eq(array_length(mesh.faces), 11);
    /*ck_assert_int_eq(array_length(mesh.neighbors), 11);*/
    for(int32_t i = 0; i < 11; i++){
        ck_assert_int_eq(mesh_remove_face(&mesh, 0), ec_no_error);
    }
    ck_assert_int_eq(mesh_remove_face(&mesh, 0), ec_out_of_bound_error);
    mesh_cleanup(&mesh);
}

START_TEST(test_point_in_polygon)
{
    size_t polygon[] = {0, 1, 2, 3};
    struct vector_st vertices[] = {
        VEC3(0.0, 0.0, 0.0), VEC3(1.0, 0.0, 0.0),
        VEC3(1.0, 1.0, 0.0), VEC3(0.0, 1.0, 0.0)
    };
    struct vector_st in_point = VEC3(0.5, 0.5, 0.0);
    struct vector_st out_point = VEC3(1.5, 0.5, 0.0);
    ck_assert(polygon_point_position(polygon, 4, vertices, &in_point) == ppol_in);
    ck_assert(polygon_point_position(polygon, 4, vertices, &out_point) == ppol_out);
}
END_TEST

START_TEST(test_point_in_non_convex_polygon)
{
    size_t polygon[] = {0, 1, 2, 3, 4, 5};
    struct vector_st vertices[] = {
        VEC3(0.0, 0.0, 0.0), VEC3(1.0, 0.0, 0.0),
        VEC3(1.5, 0.0, 0.9), VEC3(2.0, 0.0, 0.0),
        VEC3(2.5, 0.0, 1.0), VEC3(0.0, 0.0, 1.0)
    };
    struct vector_st in_point = VEC3(0.5, 0.0, 0.5);
    struct vector_st out_point = VEC3(3.5, 0.0, 0.5);
    ck_assert(polygon_point_position(polygon, 6, vertices, &in_point) == ppol_in);
    ck_assert(projected_polygon_point_position(polygon, 6, vertices, &in_point, pp_zx) == ppol_in);
    ck_assert(polygon_point_position(polygon, 6, vertices, &out_point) == ppol_out);
    ck_assert(projected_polygon_point_position(polygon, 6, vertices, &out_point, pp_zx) == ppol_out);
}
END_TEST

START_TEST(test_point_in_polygon_2)
{
    size_t polygon[] = {0, 1, 2};
    struct vector_st vertices[] = {
        VEC3(-209.20614087917198, -209.17138216978469, 0.0), VEC3(209.23109276184397, 0.047234650723279759, 0.0),
        VEC3(9.6638643926306926, 0.56884694405312042, 0.0)
    };
    struct vector_st out_point = VEC3(1.4261942782561268, 0.047234650723279759, 0.0);
    ck_assert(projected_polygon_point_position(polygon, 3, vertices, &out_point, pp_xy) == ppol_out);
}
END_TEST

Suite * mk_mesh_suite(void){
    Suite * s = suite_create("Mesh");
    TCase * tc = tcase_create(
            "Mesh");
    tcase_add_test(tc, test_mesh_add_features);
    tcase_add_test(tc, test_mesh_add_face_fails);
    tcase_add_test(tc, test_mesh_replace_face);
    tcase_add_test(tc, test_pop_face);
    tcase_add_test(tc, test_remove_face);
    tcase_add_test(tc, test_mesh_replace_no_border_effects);
    tcase_add_test(tc, test_mesh_replace_no_border_effects2);
    tcase_add_test(tc, test_stl_export_stlb);
    suite_add_tcase(s, tc);
    TCase * tp = tcase_create(
            "Polygon");
    tcase_add_test(tp, test_point_in_polygon);
    tcase_add_test(tp, test_point_in_polygon_2);
    tcase_add_test(tp, test_point_in_non_convex_polygon);
    suite_add_tcase(s, tp);
    TCase * tq = tcase_create("Queries");
    tcase_add_test(tq, test_mesh_enclosing_triangular_face);
    tcase_add_test(tq, test_mesh_edge_intersection_generic);
    tcase_add_test(tq, test_mesh_edge_intersection_collinear);
    tcase_add_test(tq, test_mesh_edge_intersection_mixed);
    tcase_add_test(tq, test_mesh_edge_intersection_fails);
    suite_add_tcase(s, tq);
    return s;
}

