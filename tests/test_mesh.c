#include <check.h>
#include <private/mesh.h>

struct vector_st vertices[] = {
    {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};

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
    struct face_st face = {0, 1, 2};
    ck_assert(mesh_add_face(&mesh, face, &index) == ec_no_error);
    ck_assert(index == 0);
    ck_assert(mesh_cleanup(&mesh) == ec_no_error);
}
END_TEST

START_TEST(test_mesh_add_face_fails){
    struct mesh_st mesh;
    ck_assert(mesh_init(&mesh) == ec_no_error);
    size_t index;
    struct face_st face = {0, 1, 2};
    ck_assert(mesh_add_face(
                &mesh, face, &index) == ec_out_of_bound_error);
    ck_assert(mesh_cleanup(&mesh) == ec_no_error);
}
END_TEST

struct vector_st cube_vertices[] = {
    {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
    {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}};
struct face_st cube_faces[] = {
    {0, 1, 2}, {2, 3, 0}, {0, 5, 1}, {0, 4, 5},
    {2, 1, 5}, {5, 6, 2}, {3, 2, 7}, {2, 6, 7},
    {6, 5, 4}, {7, 6, 4}, {3, 4, 0}, {3, 7, 4}};

START_TEST(test_stl_export_stlb)
{
    struct mesh_st mesh;
    ck_assert(mesh_init(&mesh) == ec_no_error);
    for(size_t i = 0; i < 8; i++){
        mesh_add_vertex(&mesh, cube_vertices[i], NULL);
    }
    for(size_t i = 0; i < 12; i++){
        mesh_add_face(&mesh, cube_faces[i], NULL);
    }
    ck_assert(mesh_export_stlb(&mesh, "build/cube.stl") == ec_no_error);
    mesh_cleanup(&mesh);
}
END_TEST

Suite * mk_mesh_suite(void){
    Suite * s = suite_create("Mesh");
    TCase * tc = tcase_create(
            "Mesh");
    tcase_add_test(tc, test_mesh_add_features);
    tcase_add_test(tc, test_mesh_add_face_fails);
    tcase_add_test(tc, test_stl_export_stlb);
    suite_add_tcase(s, tc);
    return s;
}

