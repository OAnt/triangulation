#include <stdio.h>
#include <private/common.h>
#include <private/array.h>
#include <private/export_stl.h>
#include <private/mesh.h>

#define STLB_HEADER_SIZE 80

struct stlb_vector_st {
    float v[3];
};

#define VECTOR_SIZE sizeof(struct stlb_vector_st)

size_t vector_write(
        struct vector_st * vec,
        FILE * file)
{
    // converting doubles to float before
    // writing the vector because stl uses floats
    struct stlb_vector_st svec;
    svec.v[0] = vec->v[0];
    svec.v[1] = vec->v[1];
    svec.v[2] = vec->v[2];
    return fwrite(&svec,
            VECTOR_SIZE, 1, file);
}

enum error_code_e _mesh_export_stlb_face(
        const struct mesh_st * mesh,
        int32_t face_index,
        FILE * file)
{
    // Writing a dummy normal I don't know them
    struct vector_st dummy_normal = {{0.0, 0.0, 0.0}};
    if(vector_write(&dummy_normal, file) != 1)
        return ec_io_error;
    // Write face vertices one by one
    struct face_st face = mesh->faces[face_index];
    for(int32_t i = 0; i < FACE_SIZE; i++){
        size_t v_index = face.f[i];
        if(vector_write(&mesh->points[v_index], file) != 1)
            return ec_io_error;
    }
    // Writing the last 16 bits
    uint16_t ctrl = 12;
    if(fwrite(&ctrl, sizeof(uint16_t), 1, file) != 1)
        return ec_io_error;
    return ec_no_error;
}

enum error_code_e _mesh_export_stlb(
        const struct mesh_st * mesh,
        FILE * file)
{
    // Leaving space for the 80 byte header
    fseek(file, STLB_HEADER_SIZE, SEEK_CUR);
    // Getting the number of face, writing it and exporting faces
    // one by one
    int32_t n_faces = array_length(mesh->faces);
    fwrite(&n_faces, sizeof(int32_t), 1, file);
    for(int32_t i = 0; i < n_faces; i++){
        if(_mesh_export_stlb_face(mesh, i, file) != ec_no_error)
            return ec_io_error;
    }
    return ec_no_error;
}

enum error_code_e mesh_export_stlb(
        const struct mesh_st * mesh,
        const char * filename)
{
    // Open the file and deal with errors, leave the actual
    // export to _mesh_export_stlb
    FILE * file = fopen(filename, "w");
    if(file){
        enum error_code_e err = _mesh_export_stlb(mesh, file);
        fclose(file);
        return err;
    }else{
        return ec_io_error;
    }
}
