#include "private/predicates.h"
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <public/common.h>
#include <private/debug.h>
#include <private/common.h>
#include <private/vector.h>
#include <private/array.h>
#include <private/mesh.h>

/*
 * Structure containing topological information about a face.
 */
struct face_private_st{
    struct face_st neighbors;/** Neighboring faces of a face. */
    struct face_st half_edges; /** Indices of half edges in the vertex adjacency list. */
};

/** 
 * Structure containing topological information about a vertex. 
 */
struct vertex_st {
    size_t adjacent_faces; /** Head of of the liked list of faces the vertex belongs to. */
};

/**
 * Structure representing a member of a list of faces adjacent to
 * a vertex.
 */
struct vertex_adjacent_face_st{
    size_t face; /** Face adjacent to the vertex. */
    size_t opposite_vertex; /** Second vertex of the edge. */
    size_t next_adjacent_faces; /** Index of the next in list. */
    size_t prev_adjacent_faces; /** Index of the previous in list. */
};

struct mesh_collector_st{
    /** First element of the removed adjacent faces (linked) list */
    size_t removed_adjacent_faces;
};

/** Contains private date the user should not care about */
struct mesh_private_st {
    struct vertex_st * vertices; /** vertices of the mesh */
    /** Lists of faces neighboring vertices ~ half edges*/
    struct vertex_adjacent_face_st * vertex_adjacent_faces; 
    struct face_private_st * faces;/** Private information about faces */
    struct mesh_collector_st col; /** Collects removed feature so they
                                    can be reused */
};

enum error_code_e mesh_cleanup(
        struct mesh_st * mesh)
{
    // Mesh is null not doing anything
    if(!mesh) return ec_error;
    // Deleting dynamically allocated arrays
    array_delete(&mesh->private->vertex_adjacent_faces);
    array_delete(&mesh->private->vertices);
    array_delete(&mesh->private->faces);
    array_delete(&mesh->points);
    free(mesh->private);
    array_delete(&mesh->faces);
    // Setting everything to zero for good measure
    memset(mesh, 0, sizeof(struct mesh_st));
    return ec_no_error;
}

enum error_code_e mesh_init(
        struct mesh_st * mesh)
{
    // Mesh is null not doing anything
    if(!mesh) return ec_error;
    // Setting everything to zero for good measure
    memset(mesh, 0, sizeof(struct mesh_st));
    mesh->private = calloc(1, sizeof(struct mesh_private_st));
    if(!mesh->private) goto fail_no_priv;
    mesh->private->col.removed_adjacent_faces = INVALID_INDEX;
    // Initializing arrays, in case of failure going to an
    // error handler that reverts what was done until the error.
    // This assumes that array_new does the same.
    if(array_new(struct face_st, 0, &mesh->faces) != ec_no_error)
        goto fail_no_faces;
    if(array_new(struct vector_st, 0, &mesh->points) != ec_no_error)
        goto fail_no_points;
    if(array_new(struct vertex_st, 0, &mesh->private->vertices) != ec_no_error)
        goto fail_no_vec;
    if(array_new(struct face_private_st, 0, &mesh->private->faces) != ec_no_error)
        goto fail_no_neighbors;
    if(array_new(struct vertex_adjacent_face_st,
                0, &mesh->private->vertex_adjacent_faces) != ec_no_error)
        goto fail_no_adj;
    return ec_no_error;
    // Faces, vertices and neighbors were allocated
fail_no_adj:
    array_delete(&mesh->private->vertex_adjacent_faces);
    // Faces and vertices were allocated
fail_no_neighbors:
    array_delete(&mesh->private->vertices);
fail_no_vec:
    array_delete(&mesh->points);
    // Only the faces were allocated
fail_no_points:
    array_delete(&mesh->faces);
fail_no_faces:
    free(mesh->private);
    // Nothing was allocated yet
fail_no_priv:
    return ec_memory_error;
}

#define mesh_face_private(mesh, face_index) (mesh)->private->faces[(face_index)]
#define _mesh_face_half_edges(mesh, face_index) mesh_face_private(mesh, face_index).half_edges.f
#define __mesh_face_neighbors(mesh, face_index) mesh_face_private(mesh, face_index).neighbors
#define _mesh_face_neighbors(mesh, face_index) __mesh_face_neighbors(mesh, face_index).f

struct face_st mesh_get_neighbors(
        struct mesh_st * mesh,
        size_t face_index)
{
    G_ASSERT(face_index < array_length(mesh->private->faces), 
            "Face out of bounds");
    return __mesh_face_neighbors(mesh, face_index);
}

enum error_code_e mesh_vertex_add_adjacent_face(
        struct mesh_st * mesh,
        size_t face_index,
        size_t vertex_offset)
{
    G_ASSERT(face_index < array_length(mesh->faces),
            "Face is out of bounds");
    G_ASSERT(vertex_offset < FACE_SIZE,
            "Vertex is out of bounds");
    size_t adj_index;
    struct mesh_collector_st * col = &mesh->private->col;
    struct mesh_private_st * priv = mesh->private;
    // There something in the linked list, pop it
    if(col && col->removed_adjacent_faces != INVALID_INDEX){
        adj_index = col->removed_adjacent_faces;
        col->removed_adjacent_faces =
            priv->vertex_adjacent_faces[adj_index].next_adjacent_faces;
    }else{
        size_t n_adj = array_length(priv->vertex_adjacent_faces);
        enum error_code_e err = array_resize(
                &priv->vertex_adjacent_faces, n_adj + 1);
        if(err != ec_no_error) return err;
        adj_index = n_adj;
    }
    size_t vertex_index = mesh->faces[face_index].f[vertex_offset];
    size_t opposite_vertex_index =
         mesh->faces[face_index].f[(vertex_offset + 1) % FACE_SIZE];
    priv->vertex_adjacent_faces[adj_index].face = face_index;
    priv->vertex_adjacent_faces[adj_index].opposite_vertex = 
        opposite_vertex_index;
    // This is the new head of the list
    priv->vertex_adjacent_faces[adj_index].prev_adjacent_faces = INVALID_INDEX;
    size_t last_head = priv->vertices[vertex_index].adjacent_faces;
    priv->vertex_adjacent_faces[adj_index].next_adjacent_faces = last_head;
    // The list is not empty, setting the new item as prev in what was the head
    if(last_head != INVALID_INDEX){
        priv->vertex_adjacent_faces[last_head].prev_adjacent_faces = adj_index;
    }
    priv->vertices[vertex_index].adjacent_faces = adj_index;
    // Registering adj_index as an edge of the face at face_index
    _mesh_face_half_edges(mesh, face_index)[vertex_offset] = adj_index;
    return ec_no_error;
}

enum error_code_e mesh_face_add_adajcent_face(
        struct mesh_st * mesh,
        size_t face_index,
        size_t vertex_offset)
{
    G_ASSERT(face_index < array_length(mesh->faces),
            "Face is out of bounds");
    G_ASSERT(vertex_offset < FACE_SIZE,
            "Vertex is out of bounds");
    size_t vertex_index = mesh->faces[face_index].f[vertex_offset];
    // Each faces (this one and the adjacent one) are
    // turning in counter clockwise order. Edge X to Y
    // was added to X's adjacent linked list. Now This face is
    // adjacent to the previous one and as the to should be
    // turning counter clockwise the X to Y edge will appear
    // as Y to X. 
    size_t opposite_vertex_offset = (vertex_offset + 2) % FACE_SIZE;
    size_t opposite_vertex_index = 
        mesh->faces[face_index].f[opposite_vertex_offset];
    struct mesh_private_st * priv = mesh->private;
    size_t next_adjacent_faces = 
        priv->vertices[vertex_index].adjacent_faces;
    size_t neighbor_face = INVALID_INDEX;
#ifndef NDEBUG
    size_t initial_adjacent_face = next_adjacent_faces;
#endif
    // Iterating over the list of adjacent faces until
    // we find the corresponding one
    while(next_adjacent_faces != INVALID_INDEX){
        struct vertex_adjacent_face_st * vadj = 
            priv->vertex_adjacent_faces + next_adjacent_faces;
        next_adjacent_faces = vadj->next_adjacent_faces;
        if(vadj->opposite_vertex == opposite_vertex_index){
            neighbor_face = vadj->face;
            break;
        }
        G_ASSERT(next_adjacent_faces != initial_adjacent_face,
                "Infinite loop");
    }
    // No neighbors found, the face is 1-manifold, stop here
    if(neighbor_face == INVALID_INDEX)
        return ec_no_error;
    G_ASSERT(neighbor_face < array_length(mesh->faces),
            "Neighbor is out of bounds");
    // Neighbor found, setting it accordingly
    _mesh_face_neighbors(mesh, face_index)[opposite_vertex_offset] = 
        neighbor_face;
    // Iterating over the neighbors vertices to find where
    // is face_index in the list of neighboring faces
    size_t neighbor_face_offset = INVALID_INDEX;
    for(size_t i = 0; i < FACE_SIZE; i++){
        if(mesh->faces[neighbor_face].f[i] == vertex_index){
            neighbor_face_offset = i;
        }
    }
    if(neighbor_face_offset == INVALID_INDEX)
        return ec_no_error;
    // Trying to add a neighbor to a face that already have one
    // at the same place, this is creating a 3-manifold edge,
    // this is not supported, preventing it
    if(_mesh_face_neighbors(mesh, neighbor_face)[neighbor_face_offset] !=
            INVALID_INDEX){
        return ec_topology_error;
    }else{
        _mesh_face_neighbors(mesh, neighbor_face)[neighbor_face_offset] =
            face_index;
    }
    return ec_no_error;
}

struct face_st invalid_face = {
    {INVALID_INDEX, INVALID_INDEX, INVALID_INDEX}};

static inline enum error_code_e mesh_face_check(
        struct mesh_st * mesh,
        struct face_st face)
{
    size_t n_vertices = array_length(mesh->points);
    // Ensuring we are not adding a face that references
    // unknown vertices
    for(size_t i = 0; i < FACE_SIZE; i++){
        if(face.f[i] >= n_vertices) return ec_out_of_bound_error;
    }
    return ec_no_error;
}

enum error_code_e mesh_face_add_topology(
        mesh_st * mesh,
        size_t face_index)
{
    enum error_code_e err = ec_no_error;
    for(size_t i = 0; i < FACE_SIZE; i++){
        err = mesh_face_add_adajcent_face(
                mesh, face_index, i);
        if(err != ec_no_error) return err;
        err = mesh_vertex_add_adjacent_face(
                mesh, face_index, i);
        if(err != ec_no_error) return err;
    }
    return err;
}

static inline enum error_code_e _mesh_add_or_replace_face(
        struct mesh_st * mesh,
        struct face_st face,
        size_t * index)
{
    // getting lengths of arrays until now
    size_t n_faces = array_length(mesh->faces);
    enum error_code_e err;
    if((err = mesh_face_check(mesh, face)) != ec_no_error)
        return err;
    // Resizing both the array, now they can hold the correct number of
    // features
    size_t face_index;
    // there is a collector and it contains a removed face, using it
    if((err = array_resize(&mesh->faces, n_faces + 1)) != ec_no_error)
        goto fail_no_face;
    if((err = array_resize(&mesh->private->faces, n_faces + 1)) != 
            ec_no_error)
        goto fail_no_neighbors;
    face_index = n_faces;
    // appending the face and returns its index
    mesh->faces[face_index] = face;
    __mesh_face_neighbors(mesh, face_index) = invalid_face;
    struct mesh_private_st * priv = mesh->private;
    size_t n_adj = array_length(priv->vertex_adjacent_faces);
    err = mesh_face_add_topology(mesh, face_index);
    if(err != ec_no_error) goto fail_no_adj;
    if(index) *index = face_index;
    return ec_no_error;
fail_no_adj:
    array_resize(&priv->vertex_adjacent_faces, n_adj);
    // If something fails and face was not allocated
    // (reused from collector) this does nothing
    array_resize(&mesh->private->faces, n_faces);
fail_no_neighbors:
    // If something fails and face was not allocated
    // (reused from collector) this does nothing
    array_resize(&mesh->faces, n_faces);
fail_no_face:
    return err;
}

enum error_code_e mesh_add_face(
        struct mesh_st * mesh,
        struct face_st face,
        size_t * index)
{
    return _mesh_add_or_replace_face(mesh, face, index);
}

/**
 * Tells if a given face has been removed
 * param mesh Mesh the face is supposed to have been removed from.
 * param face_index Index of the face to check.
 * return True if the face was removed (or not present) false otherwise.
 */
bool mesh_face_is_removed(
        const struct mesh_st * mesh,
        size_t face_index)
{
    // A face is considered removed if is first two vertices
    // are INVALID_INDEX, the last one is the index of the
    // next removed face.
    size_t n_faces = array_length(mesh->faces);
    return face_index >= n_faces || (
            mesh->faces[face_index].f[0] == INVALID_INDEX &&
            mesh->faces[face_index].f[1] == INVALID_INDEX);
}

/**
 * Convenience macro to determine is face can be used.
 */
#define mesh_face_is_valid(mesh, face_index) !mesh_face_is_removed(\
        (mesh), (face_index))

void mesh_face_remove_from_neigbhors(
        mesh_st * mesh,
        size_t face_index,
        size_t neighbor_index)
{
    if(neighbor_index == INVALID_INDEX) return;
    G_ASSERT(face_index < array_length(mesh->faces),
            "Face is out of bounds");
    G_ASSERT(neighbor_index < array_length(mesh->faces),
            "Neighbor is out of bounds");
    for(int32_t i = 0; i < FACE_SIZE; i++){
        if(_mesh_face_neighbors(mesh, neighbor_index)[i] == face_index){
            _mesh_face_neighbors(mesh, neighbor_index)[i] = INVALID_INDEX;
        }
    }
}

void mesh_face_remove_from_vertex_adjacent_faces(
        mesh_st * mesh,
        size_t face_index,
        int32_t vertex_offset)
{
    struct mesh_collector_st * col = &mesh->private->col;
    struct mesh_private_st * priv = mesh->private;
    G_ASSERT(face_index < array_length(mesh->faces),
            "Face is out of bounds");
    G_ASSERT(vertex_offset < FACE_SIZE,
            "Vertex is out of bounds");
    size_t next_adjacent_faces = _mesh_face_half_edges(mesh, face_index)[vertex_offset];
    if(next_adjacent_faces == INVALID_INDEX) return;
    struct vertex_adjacent_face_st * vadj = 
        priv->vertex_adjacent_faces + next_adjacent_faces;
    G_ASSERT(vadj->face == face_index, "Edge is not in face");
    // Removing the vertex adjacency from its liked list.
    if(vadj->next_adjacent_faces != INVALID_INDEX){
        priv->vertex_adjacent_faces[vadj->next_adjacent_faces].prev_adjacent_faces =
            vadj->prev_adjacent_faces;
    }
    if(vadj->prev_adjacent_faces != INVALID_INDEX){
        priv->vertex_adjacent_faces[vadj->prev_adjacent_faces].next_adjacent_faces =
            vadj->next_adjacent_faces;
    // This is the head, it is being removed, the vertex needs to point correctly
    }else{
        priv->vertices[mesh->faces[face_index].f[vertex_offset]].adjacent_faces =
            vadj->next_adjacent_faces;
    }
    // inserting the adjacent face to list of available ones
    vadj->next_adjacent_faces = col->removed_adjacent_faces;
    col->removed_adjacent_faces = next_adjacent_faces;
    // resetting the values
    vadj->face = INVALID_INDEX;
    vadj->opposite_vertex = INVALID_INDEX;
    vadj->prev_adjacent_faces = INVALID_INDEX;
    _mesh_face_half_edges(mesh, face_index)[vertex_offset] = INVALID_INDEX;
}

void mesh_face_remove_topology(
        mesh_st * mesh,
        size_t face_index)
{
    for(int32_t i = 0; i < FACE_SIZE; i++){
        // Removing face from its ith neighbors
        mesh_face_remove_from_neigbhors(
                mesh, face_index, 
                _mesh_face_neighbors(mesh, face_index)[i]);
        // Removing face from its ith vertex adjacent faces list
        mesh_face_remove_from_vertex_adjacent_faces(
                mesh, face_index, i);
    }
}

// Depending on what you wish to accomplish you may this function or
// mesh_remove_face which also marks faces and neighbors for reuse.
// When a face is popped, the face array size is reduced, the 
// index is not usable anymore, it should not be added collected.
// Use this instead.
enum error_code_e _mesh_remove_face(
        struct mesh_st * mesh,
        size_t face_index)
{
    // checking if the face is already removed (also check if it
    // is out of bounds)
    G_ASSERT(mesh_face_is_valid(mesh, face_index),
            "Face was already removed");
    mesh_face_remove_topology(mesh, face_index);
    // Removing the face's neighbors, no linked list needed,
    // neighbors index follows face index
    __mesh_face_neighbors(mesh, face_index) = invalid_face;
    return ec_no_error;
}

enum error_code_e mesh_pop_face(
        struct mesh_st * mesh,
        struct face_st * face) 
{
    size_t n_faces = array_length(mesh->faces);
    if(n_faces == 0) return ec_out_of_bound_error;
    *face = mesh->faces[n_faces - 1];
    // Only collect vertex topology which may be harder to really removed
    // Moreover there is no sense in iterating over it so I don't feel
    // entitled not to leave unused element in it
    _mesh_remove_face(mesh, n_faces - 1);
    array_resize(&mesh->faces, n_faces - 1);
    array_resize(&mesh->private->faces, n_faces - 1);
    return ec_no_error;
}

/**
 * Removes a face from a mesh. This does not free any memory.
 * instead, it swaps the last face with the face to delete and
 * forget about it.
 * param mesh Mesh from which a face will be removed.
 * param face_index Index of the face to remove.
 * return ec_no_error on success. It returns an error the face
 * can't be removed (because it is not in the mesh)
 */
enum error_code_e mesh_remove_face(
        struct mesh_st * mesh,
        size_t face_index)
{
    if(mesh_face_is_removed(mesh, face_index)) return ec_out_of_bound_error;
    size_t n_faces = array_length(mesh->faces);
    struct face_st old_face;
    // This mesh_face_id_removed already does the out of bounds check
    mesh_pop_face(mesh, &old_face);
    // The face to remove is the last one this is the same as pop
    if( face_index + 1 == n_faces) return ec_no_error;
    mesh_face_remove_topology(mesh, face_index);
    mesh->faces[face_index] = old_face;
    __mesh_face_neighbors(mesh, face_index) = invalid_face;
    enum error_code_e err = mesh_face_add_topology(
            mesh, face_index);
    G_ASSERT(err != ec_topology_error,
            "Re-adding a face after removing another one should not break topology");
    return err;
}

enum error_code_e mesh_swap_edge(
        struct mesh_st * mesh,
        size_t face_index_0,
        size_t face_index_1)
{
    size_t n_faces = array_length(mesh->faces);
    if(face_index_0 >= n_faces || face_index_1 >= n_faces)
        return ec_out_of_bound_error;
    int32_t edge_offset_0 = -1, edge_offset_1 = -1;
    for(int32_t i = 0; i < FACE_SIZE; i++){
        if(_mesh_face_neighbors(mesh, face_index_0)[i] == face_index_1){
            edge_offset_0 = i;
        }
        if(_mesh_face_neighbors(mesh, face_index_1)[i] == face_index_0){
            edge_offset_1 = i;
        }
    }
    // faces are not adjacent
    if(edge_offset_0 == -1 || edge_offset_1 == -1) return ec_error;
    mesh_face_remove_topology(mesh, face_index_0);
    mesh_face_remove_topology(mesh, face_index_1);
    int32_t new_edge_offset_0 = (edge_offset_0 + 2) % FACE_SIZE;
    int32_t new_edge_offset_1 = (edge_offset_1 + 2) % FACE_SIZE;
    struct face_st new_face_0 = {{
        mesh->faces[face_index_0].f[edge_offset_0],
        mesh->faces[face_index_1].f[new_edge_offset_1],
        mesh->faces[face_index_0].f[new_edge_offset_0],
    }};
    struct face_st new_face_1 = {{
        mesh->faces[face_index_1].f[new_edge_offset_1],
        mesh->faces[face_index_1].f[edge_offset_1],
        mesh->faces[face_index_0].f[new_edge_offset_0],
    }};
    mesh->faces[face_index_0] = new_face_0;
    __mesh_face_neighbors(mesh, face_index_0) = invalid_face;
    mesh->faces[face_index_1] = new_face_1;
    __mesh_face_neighbors(mesh, face_index_1) = invalid_face;
    enum error_code_e err = ec_no_error;
    err = mesh_face_add_topology(mesh, face_index_0);
    G_ASSERT(err != ec_topology_error,
            "Swapping the edge between to valid edges should not alter the topological soundness of the mesh");
    err = mesh_face_add_topology(mesh, face_index_1);
    G_ASSERT(err != ec_topology_error,
            "Swapping the edge between to valid edges should not alter the topological soundness of the mesh");
    return err;
}

enum error_code_e mesh_replace_face(
        struct mesh_st * mesh,
        struct face_st face,
        size_t index)
{
    enum error_code_e err;
    if((err = mesh_face_check(mesh, face)) != ec_no_error){
        return err;
    }
    struct face_st old_face = mesh->faces[index];
    err = _mesh_remove_face(mesh, index);
    /*mesh_face_remove_topology(mesh, index);*/
    mesh->faces[index] = face;
    __mesh_face_neighbors(mesh, index) = invalid_face;
    err = mesh_face_add_topology(
                    mesh, index);
    if(err != ec_no_error && err != ec_memory_error)
    {
        // Putting back the previous face if something failed to ensure there
        // are no border effects. Not doing it in case of memory error because
        // some vertex_face_adjacency objects may not have been collected
        mesh_face_remove_topology(mesh, index);
        mesh->faces[index] = old_face;
        __mesh_face_neighbors(mesh, index) = invalid_face;
        enum error_code_e err1 = mesh_face_add_topology(mesh, index);
        (void)err1;
        G_ASSERT(err1 != ec_topology_error,
            "Re-adding a face should not break topology");
        return err;
    }else{
        return err;
    }
}

enum error_code_e mesh_add_vertex(
        struct mesh_st * mesh,
        struct vector_st v,
        size_t * index)
{
    struct mesh_private_st * priv = mesh->private;
    size_t n_vertices = array_length(priv->vertices);
    G_ASSERT(n_vertices == array_length(mesh->points),
            "Not the same number of points and vertices");
    // Resizing the array, now it can holds the correct number of
    // features
    if(array_resize(&priv->vertices, n_vertices + 1) != ec_no_error)
        return ec_memory_error;
    if(array_resize(&mesh->points, n_vertices + 1) != ec_no_error)
        return ec_memory_error;
    // appending the vertex and returns its index
    mesh->points[n_vertices] = v;
    priv->vertices[n_vertices].adjacent_faces = INVALID_INDEX;
    if(index) *index = n_vertices;
    return ec_no_error;
}

bool mesh_vertex_is_in_face(
        struct mesh_st * mesh,
        size_t index)
{
    G_ASSERT(index < array_length(mesh->points), "Vertex is out of bounds");
    return mesh->private->vertices[index].adjacent_faces != INVALID_INDEX;
}

void mesh_vertex_forget_last_n(
        struct mesh_st * mesh,
        size_t n)
{
    size_t n_vertices = array_length(mesh->points);
    G_ASSERT(n_vertices == array_length(mesh->private->vertices), "Vertex count mismatch");
    array_resize(&mesh->private->vertices, n_vertices - n);
    array_resize(&mesh->points, n_vertices - n);
}


/**
 * Type of intersection between an infinite ray along the
 * horizontal axis and an edge.
 */
enum intersection_type_e{
    it_upward = 0, /** Edge is intersected and going upward. */
    it_downward = 1, /** Edge is intersected and going downward. */
    it_no = 2, /** Edge is not intersected. */
};

/** 
 * Determines the type of intersection if there is one.
 * param point Infinite horizontal ray is going through point.
 * param seg Supporting segment of the edge.
 * param y Vertical axis dimension index.
 * return type of intersection if there is one
 */
enum intersection_type_e  edge_determine_intersection_type(
        struct vector_st * point,
        struct vector_st * seg0,
        struct vector_st * seg1,
        int32_t y)
{
    //point is between seg[0] and seg[1], edge is
    //pointing upward and there is an intersection
    if(seg0->v[y] <= point->v[y] && \
            point->v[y] < seg1->v[y]){
        return it_upward;
    //point is between seg[1] and seg[0], edge is
    //pointing downward and there is an intersection
    }else if(seg1->v[y] <= point->v[y] && \
            point->v[y] < seg0->v[y]){
        return it_downward;
    //point is not between the segment vertical bounds
    //there cannot be an intersection with an horizontal
    //axis going through point
    }else{
        return it_no;
    }
}

#define point_is_left_of(p, s0, s1, x, y) \
    _vector_position_relative_to_segment(p, s0, s1, x, y) == pt_left
#define point_is_right_of(p, s0, s1, x, y) \
    _vector_position_relative_to_segment(p, s0, s1, x, y) == pt_right

/**
 * Increments or decrements the winding number
 * according to the relative position of point
 * and seg.
 * param point Point for which one wants to determine
 * the position relative to a polygon.
 * param seg Supporting segment for an edge of the
 * polygon.
 * param pp Projection plane for the computations.
 * param winding_number pointer to the winding number
 * of the point relative to the polygon
 * return nothing
 */
void winding_number_modify(
        struct vector_st * point,
        struct vector_st * seg0,
        struct vector_st * seg1,
        int32_t x,
        int32_t y,
        int32_t * winding_number)
{
    // determines what is the vertical axis 
    // for a given projection plane
    // determines if there is an intersection and what kind
    // of intersection it is
    enum intersection_type_e it = edge_determine_intersection_type(
            point, seg0, seg1, y);
    if(it == it_no) return;
    enum point_position_e pos = _vector_position_relative_to_segment(
           point, seg0, seg1, x, y); 
    // Edge is going upward, an oriented polygon turns
    // counterclockwise, if the point is left of the edge,
    // it is inside once
    if(it == it_upward && pos == pt_left){
        (*winding_number)++;
    // Edge is going downward, an oriented polygon turns
    // counterclockwise, if the point is right of the edge,
    // it is outside once
    }else if(it == it_downward && pos == pt_right){
        (*winding_number)--;
    }
    /*printf("%d, %d, %d -> [%f, %f], [[%f, %f], [%f, %f]], %d, %d, %d -> %d\n",*/
            /*x, y, pp,*/
            /*point->v[x], point->v[y],*/
            /*seg->s[0].v[x], seg->s[0].v[y],*/
            /*seg->s[1].v[x], seg->s[1].v[y], it,*/
            /*point_is_left_of(point, seg, pp),*/
            /*point_is_right_of(point, seg, pp),*/
            /**winding_number);*/
}

static struct vector_st x = VEC3(1.0, 0.0, 0.0);
static struct vector_st y = VEC3(0.0, 1.0, 0.0);
static struct vector_st z = VEC3(0.0, 0.0, 1.0);

// computes the normal of a polygon assuming it is planar
// if the polygon is degenerate (cannot compute normal),
// returns an error
enum error_code_e planar_polygon_normal(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _OUT struct vector_st * normal)
{
    bool degenerate_polygon = true;
    // Computing the normal iterating over
    // groups of three points until we
    // find a group where they are not aligned
    for(size_t i = 0; i < n_vertices; i++){
        size_t v = polygon[i];
        size_t next = polygon[(i + 1) % n_vertices];
        size_t next_over = polygon[(i + 2) % n_vertices];
        struct vector_st edge_a, edge_b;
        vector_subtraction(&vertices[next], &vertices[v], &edge_a);
        vector_subtraction(&vertices[next_over], &vertices[v], &edge_b);
        vector_cross_product(&edge_a, &edge_b, normal);
        double sq_norm = vector_dot_product(normal, normal);
        // found a normal with non zero norm, non collinear edges
        if(sq_norm > EPSILON) {
            degenerate_polygon = false;
            break;
        }
    }
    // All the points are aligned, this is a degenerate polygon (a line)
    if(degenerate_polygon) return ec_error;
    return ec_no_error;
}

enum error_code_e planar_polygon_best_projection(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _OUT enum projection_plane_e * _pp)
{
    struct vector_st normal;
    enum error_code_e err = planar_polygon_normal(
            polygon, n_vertices, vertices, &normal);
    if(err != ec_no_error) return err;
    // Finding which of xy, yz ans zx is the best plane
    // to project the polygon on. The higher the absolute
    // dot product of the normal and unit vector is the 
    // better the plane fits. I think it is impossible
    // to find a plane that is orthogonal to all three
    // xy, yz ans zx planes
    enum projection_plane_e pp = pp_xy;
    double x_dot = fabs(vector_dot_product(&normal, &x));
    double y_dot = fabs(vector_dot_product(&normal, &y));
    double z_dot = fabs(vector_dot_product(&normal, &z));
    if(x_dot > y_dot){
        if(z_dot > x_dot){
            pp = pp_xy;
        }else{
            pp = pp_yz;
        }
    }else{
        if(z_dot > y_dot){
            pp = pp_xy;
        }else{
            pp = pp_zx;
        }
    }
    *_pp = pp;
    return ec_no_error;
}

static inline enum point_polygon_position_e _polygon_point_position(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _IN struct vector_st * point,
        _IN int32_t x,
        _IN int32_t y)
{
    int32_t winding_number = 0;
    for(size_t i = 0; i < n_vertices; i++){
        size_t v = polygon[i];
        size_t next_v = polygon[(i + 1) % n_vertices];
        /*struct segment_st seg = {{*/
            /*vertices[v], vertices[next_v]*/
        /*}};*/
        winding_number_modify(
                point,
                vertices + v,
                vertices + next_v,
                x,
                y,
                &winding_number);
    }
    if(winding_number > 0){
        return ppol_in;
    }else{
        return ppol_out;
    }
}

enum point_polygon_position_e polygon_point_position(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _IN struct vector_st * point)
{
    // this functions assumes the polygon is plane
    // it computes its normal by taking the first
    // three vertices
    if(n_vertices <= 2) return ppol_out;
    enum projection_plane_e pp;
    if(planar_polygon_best_projection(
                polygon, n_vertices, vertices, &pp) != ec_no_error)
        return ppol_out;
    int32_t x, y;
    get_axis_system_from_projection_plane(pp, &x, &y);
    return _polygon_point_position(
            polygon, n_vertices, vertices, point, x, y);
}

enum point_polygon_position_e projected_polygon_point_position(
        _IN size_t * polygon,
        _IN size_t n_vertices,
        _IN struct vector_st * vertices,
        _IN struct vector_st * point,
        _IN enum projection_plane_e pp)
{
    // this functions assumes the polygon is plane
    // it computes its normal by taking the first
    // three vertices
    if(n_vertices <= 2) return ppol_out;
    int32_t x, y;
    get_axis_system_from_projection_plane(pp, &x, &y);
    return _polygon_point_position(
            polygon, n_vertices, vertices, point, x, y);
}

enum point_polygon_position_e projected_face_point_position(
        const struct mesh_st * mesh,
        size_t face_index,
        struct vector_st * point,
        _IN int32_t x,
        _IN int32_t y)
{
    struct face_st * face = mesh->faces + face_index;
    double p[2] = {point->v[x], point->v[y]};
    double pa[2] = {
        mesh->points[face->f[0]].v[x], mesh->points[face->f[0]].v[y]};
    double pb[2] = {
        mesh->points[face->f[1]].v[x], mesh->points[face->f[1]].v[y]};
    if(orient2d(pa, pb, p) < 0) return ppol_out;
    double pc[2] = {
        mesh->points[face->f[2]].v[x], mesh->points[face->f[2]].v[y]};
    if(orient2d(pb, pc, p) < 0) return ppol_out;
    if(orient2d(pc, pa, p) < 0) return ppol_out;
    return ppol_in;
    /*return _polygon_point_position(*/
            /*mesh->faces[face_index].f, FACE_SIZE, mesh->points, point, x, y);*/
}

enum error_code_e unindexed_mesh_find_first_enclosing_triangular_face(
        const struct mesh_st * mesh,
        struct vector_st point,
        enum projection_plane_e pp,
        size_t * face_index)
{
    int32_t x, y;
    get_axis_system_from_projection_plane(pp, &x, &y);
    for(size_t f = 0; f < array_length(mesh->faces); f++){
        if(projected_face_point_position(mesh, f, &point, x, y) == ppol_in){
            *face_index = f;
            return ec_no_error;
        }
    }
    return ec_error;
}

