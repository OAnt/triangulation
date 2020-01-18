#include <math.h>
#include <stdio.h>
#include <time.h>
#include <public/delaunay_triangulation.h>

struct vector_st vector_distribution_non_uniform(struct vector_st v){
    static double cst = 12.0;
    /*static double cst = 1.0;*/
    struct vector_st nv = VEC2(pow(v.v[0], cst), pow(v.v[1], cst));
    return nv;
}

struct mesh_st _generate_pointcloud_2d(
        double range,
        size_t n_vertices,
        unsigned int state,
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

int main(int argc, char ** argv){
    double size = 1.0;
    size_t n_points = 1000;
    unsigned int state = time(NULL);
    if(argc >= 2) n_points = atoi(argv[1]);
    if(argc >= 4) state = atoi(argv[3]);
    struct mesh_st mesh = _generate_pointcloud_2d(size, n_points, state,
            vector_distribution_non_uniform);
    clock_t clk_start = clock();
    mesh_delaunay_triangulation(&mesh, pp_xy);
    clock_t clk_end = clock();
    printf("Triangulation of %ld points done in %f seconds\n",
            n_points, ((float)(clk_end - clk_start))/CLOCKS_PER_SEC);
    if(argc >= 3){
        mesh_export_stlb(&mesh, argv[2]);
    }
    mesh_cleanup(&mesh);
    return 0;
}
