/*  ADMesh -- process triangulated solid meshes
 *  Copyright (C) 1995, 1996  Anthony D. Martin <amartin@engr.csulb.edu>
 *  Copyright (C) 2013, 2014  several contributors, see AUTHORS
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  Questions, comments, suggestions, etc to
 *           https://github.com/admesh/admesh/issues
 */

#ifndef __admesh_stl__
#define __admesh_stl__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <boost/detail/endian.hpp>

#ifndef BOOST_LITTLE_ENDIAN
#error "admesh works correctly on little endian machines only!"
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #include "windows.h"
    #define ADMESH_CHAR wchar_t
    #define stl_fopen(file, mode) _wfopen(file, L##mode)
#else
    #define ADMESH_CHAR char
    #define stl_fopen(file, mode) fopen(file, mode)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define STL_MAX(A,B) ((A)>(B)? (A):(B))
#define STL_MIN(A,B) ((A)<(B)? (A):(B))
#define ABS(X)  ((X) < 0 ? -(X) : (X))

// Size of the binary STL header, free form.
#define LABEL_SIZE             80
// Binary STL, length of the "number of faces" counter.
#define NUM_FACET_SIZE         4
// Binary STL, sizeof header + number of faces.
#define HEADER_SIZE            84
#define STL_MIN_FILE_SIZE      284
#define ASCII_LINES_PER_FACET  7
// Comparing an edge by memcmp, 2x3x4 bytes = 24
#define SIZEOF_EDGE_SORT       24

typedef struct {
  float x;
  float y;
  float z;
} stl_vertex;

#ifdef static_assert
static_assert(sizeof(stl_vertex) == 12, "size of stl_vertex incorrect");
#endif

typedef stl_vertex stl_normal;

typedef char stl_extra[2];

typedef struct {
  stl_normal normal;
  stl_vertex vertex[3];
  stl_extra  extra;//��һλ�����ж���������������
} stl_facet;
#define SIZEOF_STL_FACET       50

#ifdef static_assert
static_assert(offsetof(stl_facet, normal) == 0, "stl_facet.normal has correct offset");
static_assert(offsetof(stl_facet, vertex) == 12, "stl_facet.vertex has correct offset");
static_assert(offsetof(stl_facet, extra ) == 48, "stl_facet.extra has correct offset");
static_assert(sizeof(stl_facet) >= SIZEOF_STL_FACET, "size of stl_facet incorrect");
#endif

typedef enum {binary, ascii, inmemory} stl_type;

typedef struct {
  stl_vertex p1;
  stl_vertex p2;
  int        facet_number1;
  int        facet_number2;
} stl_edge;

typedef struct stl_hash_edge {
  // Key of a hash edge: 2x binary copy of a floating point vertex.
  uint32_t       key[6];
  // Index of a facet owning this edge.
  int            facet_number;
  // Index of this edge inside the facet with an index of facet_number.
  // ����������Ƿ���洢 ��which_edge����3   If this edge is stored backwards, which_edge is increased by 3.
  int            which_edge;
  struct stl_hash_edge  *next;
} stl_hash_edge;

#ifdef static_assert
static_assert(offsetof(stl_hash_edge, facet_number) == SIZEOF_EDGE_SORT, "size of stl_hash_edge.key incorrect");
#endif

typedef struct {
  // Index of a neighbor facet.
  int   neighbor[3];
  // Index of an opposite�෴�� vertex at the neighbor face.
  char  which_vertex_not[3];
} stl_neighbors;

typedef struct {
  int   vertex[3];
} v_indices_struct;

//�洢����ͬһ������漯������
//��������ʾһ��ͬ���͵�������
typedef struct {
    int num;    //�������
    int* v_shared_face; //�����������
}v_face_struct;

typedef struct {
    char          header[81];
    stl_type      type;
    int           number_of_facets;
    stl_vertex    max;
    stl_vertex    min;
    stl_vertex    size;
    float         bounding_diameter;
    float         shortest_edge;
    float         volume;
    unsigned      number_of_blocks;
    int           connected_edges;
    int           connected_facets_1_edge;
    int           connected_facets_2_edge;
    int           connected_facets_3_edge;	//���������ߵ���
    int           facets_w_1_bad_edge;
    int           facets_w_2_bad_edge;
    int           facets_w_3_bad_edge;
    int           original_num_facets;
    int           edges_fixed;
    int           degenerate_facets;
    int           facets_removed;
    int           facets_added;
    int           facets_reversed;
    int           backwards_edges;
    int           normals_fixed;
    int           number_of_parts;
    int           malloced;
    int           freed;
    int           facets_malloced;
    int           collisions;
    int           shared_vertices;
    int           shared_malloced;
} stl_stats;

typedef struct {
    FILE* fp;
    stl_facet* facet_start;
    stl_edge* edge_start;
    stl_hash_edge** heads;
    stl_hash_edge* tail;
    int           M;
    stl_neighbors* neighbors_start;
    v_indices_struct* v_indices;	//�������������������
    stl_vertex* v_shared;	//���������
    v_face_struct* v_shared_faces;    //�洢�㻷���������,add
    stl_stats     stats;
    char          error;
} stl_file;


extern void stl_open(stl_file *stl, const ADMESH_CHAR *file);
extern void stl_close(stl_file *stl);
extern void stl_stats_out(stl_file *stl, FILE *file, char *input_file);
extern void stl_print_edges(stl_file *stl, FILE *file);
extern void stl_print_neighbors(stl_file *stl, ADMESH_CHAR *file);
extern void stl_put_little_int(FILE *fp, int value_in);
extern void stl_put_little_float(FILE *fp, float value_in);
extern void stl_write_ascii(stl_file *stl, const ADMESH_CHAR *file, const char *label);
extern void stl_write_binary(stl_file *stl, const ADMESH_CHAR *file, const char *label);
extern void stl_write_binary_block(stl_file *stl, FILE *fp);
extern void stl_check_facets_exact(stl_file *stl);
extern void stl_check_facets_nearby(stl_file *stl, float tolerance);
extern void stl_remove_unconnected_facets(stl_file *stl);
extern void stl_write_vertex(stl_file *stl, int facet, int vertex);
extern void stl_write_facet(stl_file *stl, char *label, int facet);
extern void stl_write_edge(stl_file *stl, char *label, stl_hash_edge edge);
extern void stl_write_neighbor(stl_file *stl, int facet);
extern void stl_write_quad_object(stl_file *stl, ADMESH_CHAR *file);
extern void stl_verify_neighbors(stl_file *stl);
extern void stl_fill_holes(stl_file *stl);
extern void stl_fix_normal_directions(stl_file *stl);
extern void stl_fix_normal_values(stl_file *stl);
extern void stl_reverse_all_facets(stl_file *stl);
extern void stl_translate(stl_file *stl, float x, float y, float z);
extern void stl_translate_relative(stl_file *stl, float x, float y, float z);
extern void stl_scale_versor(stl_file *stl, float versor[3]);
extern void stl_scale(stl_file *stl, float factor);
extern void calculate_normals(stl_file *stl);
extern void stl_rotate_x(stl_file *stl, float angle);
extern void stl_rotate_y(stl_file *stl, float angle);
extern void stl_rotate_z(stl_file *stl, float angle);
extern void stl_mirror_xy(stl_file *stl);
extern void stl_mirror_yz(stl_file *stl);
extern void stl_mirror_xz(stl_file *stl);
extern void stl_transform(stl_file *stl, float *trafo3x4);
extern void stl_open_merge(stl_file *stl, ADMESH_CHAR *file);
extern void stl_invalidate_shared_vertices(stl_file *stl);

//���ܣ��õ������͵�����Χ����������ݹ�ϵ��
//����1��stlģ���ļ���
//����2��Ҫ���صĵ�����Χ����������ݹ�ϵ��
extern void stl_generate_shared_vertices_faces(stl_file *stl);

extern void stl_write_obj(stl_file *stl, const ADMESH_CHAR *file);
extern void stl_write_off(stl_file *stl, ADMESH_CHAR *file);
extern void stl_write_dxf(stl_file *stl, ADMESH_CHAR *file, char *label);
extern void stl_write_vrml(stl_file *stl, ADMESH_CHAR *file);
extern void stl_calculate_normal(float normal[], stl_facet *facet);
extern void stl_normalize_vector(float v[]);
extern void stl_calculate_volume(stl_file *stl);

extern void stl_repair(stl_file *stl, int fixall_flag, int exact_flag, int tolerance_flag, float tolerance, int increment_flag, float increment, int nearby_flag, int iterations, int remove_unconnected_flag, int fill_holes_flag, int normal_directions_flag, int normal_values_flag, int reverse_all_flag, int verbose_flag);

extern void stl_initialize(stl_file *stl);
extern void stl_count_facets(stl_file *stl, const ADMESH_CHAR *file);
extern void stl_allocate(stl_file *stl);
extern void stl_read(stl_file *stl, int first_facet, int first);
extern void stl_facet_stats(stl_file *stl, stl_facet facet, int first);
extern void stl_reallocate(stl_file *stl);
extern void stl_add_facet(stl_file *stl, stl_facet *new_facet);
extern void stl_get_size(stl_file *stl);

extern void stl_clear_error(stl_file *stl);
extern int stl_get_error(stl_file *stl);
extern void stl_exit_on_error(stl_file *stl);

#ifdef __cplusplus
}
#endif

#endif
