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

#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "portable_endian.h"
#include "stl.h"

#ifndef SEEK_SET
#error "SEEK_SET not defined"
#endif

void
stl_open(stl_file *stl, const ADMESH_CHAR *file) {
  stl_initialize(stl);
  stl_count_facets(stl, file);
  stl_allocate(stl);
  stl_read(stl, 0, 1);
  if (!stl->error) fclose(stl->fp);
}


void
stl_initialize(stl_file *stl) {
  stl->error = 0;
  stl->stats.degenerate_facets = 0;
  stl->stats.edges_fixed  = 0;
  stl->stats.facets_added = 0;
  stl->stats.facets_removed = 0;
  stl->stats.facets_reversed = 0;
  stl->stats.normals_fixed = 0;
  stl->stats.number_of_parts = 0;
  stl->stats.original_num_facets = 0;
  stl->stats.number_of_facets = 0;
  stl->stats.bounding_diameter = 0;
  stl->stats.shortest_edge = FLT_MAX;
  stl->stats.facets_malloced = 0;
  stl->stats.volume = -1.0;

  stl->neighbors_start = NULL;
  stl->facet_start = NULL;
  stl->v_indices = NULL;
  stl->v_shared = NULL;
  stl->v_shared_faces = NULL;
}

void
stl_count_facets(stl_file *stl, const ADMESH_CHAR *file) {
  long           file_size;
  uint32_t       header_num_facets;
  uint32_t       num_facets;
  int            i;
  size_t         s;
  unsigned char  chtest[128];
  int            num_lines = 1;
  //char           *error_msg;

  if (stl->error) return;

  /* Open the file in binary mode first */
  stl->fp = stl_fopen(file, "rb");
  if(stl->fp == NULL) {
    perror("stl_initialize: Couldn't open file for reading");
    stl->error = 1;
    return;
  }
  /* Find size of file */
  fseek(stl->fp, 0, SEEK_END);
  file_size = ftell(stl->fp);

  /* Check for binary or ASCII file */
  fseek(stl->fp, HEADER_SIZE, SEEK_SET);
  if (!fread(chtest, sizeof(chtest), 1, stl->fp)) {
    perror("The input is an empty file");
    stl->error = 1;
    return;
  }
  stl->stats.type = ascii;
  for(s = 0; s < sizeof(chtest); s++) {
    if(chtest[s] > 127) {
      stl->stats.type = binary;
      break;
    }
  }
  rewind(stl->fp);

  /* Get the header and the number of facets in the .STL file */
  /* If the .STL file is binary, then do the following */
  if(stl->stats.type == binary) {
    /* Test if the STL file has the right size  */
    if(((file_size - HEADER_SIZE) % SIZEOF_STL_FACET != 0)
        || (file_size < STL_MIN_FILE_SIZE)) {
      fprintf(stderr, "The file %ws has the wrong size.\n", file);
      stl->error = 1;
      return;
    }
    num_facets = (file_size - HEADER_SIZE) / SIZEOF_STL_FACET;

    /* Read the header */
    if (fread(stl->stats.header, LABEL_SIZE, 1, stl->fp) > 79) {
      stl->stats.header[80] = '\0';
    }

    /* Read the int following the header.  This should contain # of facets */
    if((!fread(&header_num_facets, sizeof(uint32_t), 1, stl->fp)) || num_facets != le32toh(header_num_facets)) {
      fprintf(stderr,
              "Warning: File size doesn't match number of facets in the header\n");

      if(num_facets > header_num_facets) {
          // this file is garbage.
          stl->error = 1; 
          return;
      }
    }
  }
  /* Otherwise, if the .STL file is ASCII, then do the following */
  else {
    /* Reopen the file in text mode (for getting correct newlines on Windows) */
    // fix to silence a warning about unused return value.
    // obviously if it fails we have problems....
    fclose(stl->fp);
    stl->fp = stl_fopen(file, "r");

    // do another null check to be safe
    if(stl->fp == NULL) {
      perror("stl_initialize: Couldn't open file for reading");
      stl->error = 1;
      return;
    }
    
    /* Find the number of facets */
    char linebuf[100];
    while (fgets(linebuf, 100, stl->fp) != NULL) {
        /* don't count short lines */
        if (strlen(linebuf) <= 4) continue;
        
        /* skip solid/endsolid lines as broken STL file generators may put several of them */
        if (strncmp(linebuf, "solid", 5) == 0 || strncmp(linebuf, "endsolid", 8) == 0) continue;
        
        ++num_lines;
    }
    
    rewind(stl->fp);
    
    /* Get the header */
    for(i = 0;
        (i < 80) && (stl->stats.header[i] = getc(stl->fp)) != '\n'; i++);
    stl->stats.header[i] = '\0'; /* Lose the '\n' */
    stl->stats.header[80] = '\0';

    num_facets = num_lines / ASCII_LINES_PER_FACET;
  }
  stl->stats.number_of_facets += num_facets;
  stl->stats.original_num_facets = stl->stats.number_of_facets;
}

void
stl_allocate(stl_file *stl) {
  if (stl->error) return;

  /*  Allocate memory for the entire .STL file */
  stl->facet_start = (stl_facet*)calloc(stl->stats.number_of_facets,
                                        sizeof(stl_facet));
  if(stl->facet_start == NULL) perror("stl_initialize");
  stl->stats.facets_malloced = stl->stats.number_of_facets;

  /* Allocate memory for the neighbors list */
  stl->neighbors_start = (stl_neighbors*)
                         calloc(stl->stats.number_of_facets, sizeof(stl_neighbors));
  if(stl->facet_start == NULL) perror("stl_initialize");
}

void
stl_open_merge(stl_file *stl, ADMESH_CHAR *file_to_merge) {
  int num_facets_so_far;
  stl_type origStlType;
  FILE *origFp;
  stl_file stl_to_merge;

  if (stl->error) return;

  /* Record how many facets we have so far from the first file.  We will start putting
     facets in the next position.  Since we're 0-indexed, it'l be the same position. */
  num_facets_so_far = stl->stats.number_of_facets;

  /* Record the file type we started with: */
  origStlType=stl->stats.type;
  /* Record the file pointer too: */
  origFp=stl->fp;

  /* Initialize the sturucture with zero stats, header info and sizes: */
  stl_initialize(&stl_to_merge);
  stl_count_facets(&stl_to_merge, file_to_merge);

  /* Copy what we need to into stl so that we can read the file_to_merge directly into it
     using stl_read:  Save the rest of the valuable info: */
  stl->stats.type=stl_to_merge.stats.type;
  stl->fp=stl_to_merge.fp;

  /* Add the number of facets we already have in stl with what we we found in stl_to_merge but
     haven't read yet. */
  stl->stats.number_of_facets=num_facets_so_far+stl_to_merge.stats.number_of_facets;

  /* Allocate enough room for stl->stats.number_of_facets facets and neighbors: */
  stl_reallocate(stl);

  /* Read the file to merge directly into stl, adding it to what we have already.
     Start at num_facets_so_far, the index to the first unused facet.  Also say
     that this isn't our first time so we should augment stats like min and max
     instead of erasing them. */
  stl_read(stl, num_facets_so_far, 0);

  /* Restore the stl information we overwrote (for stl_read) so that it still accurately
     reflects the subject part: */
  stl->stats.type=origStlType;
  stl->fp=origFp;
}

extern void
stl_reallocate(stl_file *stl) {
  if (stl->error) return;
  /*  Reallocate more memory for the .STL file(s) */
  stl->facet_start = (stl_facet*)realloc(stl->facet_start, stl->stats.number_of_facets *
                                         sizeof(stl_facet));
  if(stl->facet_start == NULL) perror("stl_initialize");
  stl->stats.facets_malloced = stl->stats.number_of_facets;

  /* Reallocate more memory for the neighbors list */
  stl->neighbors_start = (stl_neighbors*)
                         realloc(stl->neighbors_start, stl->stats.number_of_facets *
                                 sizeof(stl_neighbors));
  if(stl->facet_start == NULL) perror("stl_initialize");
}


/* Reads the contents of the file pointed to by stl->fp into the stl structure,
   starting at facet first_facet.  The second argument says if it's our first
   time running this for the stl and therefore we should reset our max and min stats. */
void
stl_read(stl_file *stl, int first_facet, int first) {
  stl_facet facet;
  int   i, j;
  const int facet_float_length = 12;
  float *facet_floats[12];
  char facet_buffer[12 * sizeof(float)];
  uint32_t endianswap_buffer;  /* for byteswapping operations */

  facet_floats[0] = &facet.normal.x;
  facet_floats[1] = &facet.normal.y;
  facet_floats[2] = &facet.normal.z;
  facet_floats[3] = &facet.vertex[0].x;
  facet_floats[4] = &facet.vertex[0].y;
  facet_floats[5] = &facet.vertex[0].z;
  facet_floats[6] = &facet.vertex[1].x;
  facet_floats[7] = &facet.vertex[1].y;
  facet_floats[8] = &facet.vertex[1].z;
  facet_floats[9] = &facet.vertex[2].x;
  facet_floats[10] = &facet.vertex[2].y;
  facet_floats[11] = &facet.vertex[2].z;

  if (stl->error) return;

  if(stl->stats.type == binary) {
    fseek(stl->fp, HEADER_SIZE, SEEK_SET);
  } else {
    rewind(stl->fp);
  }

  for(i = first_facet; i < stl->stats.number_of_facets; i++) {
    if(stl->stats.type == binary)
      /* Read a single facet from a binary .STL file */
    {
      if(fread(facet_buffer, sizeof(facet_buffer), 1, stl->fp)
         + fread(&facet.extra, sizeof(char), 2, stl->fp) != 3) {
        perror("Cannot read facet");
        stl->error = 1;
        return;
      }

      for(j = 0; j < facet_float_length; j++) {
        /* convert LE float to host byte order */
        memcpy(&endianswap_buffer, facet_buffer + j * sizeof(float), 4);
        endianswap_buffer = le32toh(endianswap_buffer);
        memcpy(facet_floats[j], &endianswap_buffer, 4);
      }
    } else
      /* Read a single facet from an ASCII .STL file */
    {
      // skip solid/endsolid
      // (in this order, otherwise it won't work when they are paired in the middle of a file)
      fscanf(stl->fp, "endsolid\n");
      fscanf(stl->fp, "solid%*[^\n]\n");  // name might contain spaces so %*s doesn't work and it also can be empty (just "solid")
      
      if((fscanf(stl->fp, " facet normal %f %f %f\n", &facet.normal.x, &facet.normal.y, &facet.normal.z) + \
          fscanf(stl->fp, " outer loop\n") + \
          fscanf(stl->fp, " vertex %f %f %f\n", &facet.vertex[0].x, &facet.vertex[0].y,  &facet.vertex[0].z) + \
          fscanf(stl->fp, " vertex %f %f %f\n", &facet.vertex[1].x, &facet.vertex[1].y,  &facet.vertex[1].z) + \
          fscanf(stl->fp, " vertex %f %f %f\n", &facet.vertex[2].x, &facet.vertex[2].y,  &facet.vertex[2].z) + \
          fscanf(stl->fp, " endloop\n") + \
          fscanf(stl->fp, " endfacet\n")) != 12) {
       // perror("Something is syntactically very wrong with this ASCII STL!");
       // stl->error = 1;
       // return;
      }
    }

#if 0
      // Report close to zero vertex coordinates. Due to the nature of the floating point numbers,
      // close to zero values may be represented with singificantly higher precision than the rest of the vertices.
      // It may be worth to round these numbers to zero during loading to reduce the number of errors reported
      // during the STL import.
      for (size_t j = 0; j < 3; ++ j) {
        if (facet.vertex[j].x > -1e-12f && facet.vertex[j].x < 1e-12f)
            printf("stl_read: facet %d.x = %e\r\n", j, facet.vertex[j].x);
        if (facet.vertex[j].y > -1e-12f && facet.vertex[j].y < 1e-12f)
            printf("stl_read: facet %d.y = %e\r\n", j, facet.vertex[j].y);
        if (facet.vertex[j].z > -1e-12f && facet.vertex[j].z < 1e-12f)
            printf("stl_read: facet %d.z = %e\r\n", j, facet.vertex[j].z);
      }
#endif

#if 1
    {
      // Positive and negative zeros are possible in the floats, which are considered equal by the FP unit.
      // When using a memcmp on raw floats, those numbers report to be different.
      // Unify all +0 and -0 to +0 to make the floats equal under memcmp.
      uint32_t *f = (uint32_t*)&facet;
      for (int j = 0; j < 12; ++ j, ++ f) // 3x vertex + normal: 4x3 = 12 floats
        if (*f == 0x80000000)
          // Negative zero, switch to positive zero.
          *f = 0;
    }
#else
    {
      // Due to the nature of the floating point numbers, close to zero values may be represented with singificantly higher precision 
      // than the rest of the vertices. Round them to zero.
      float *f = (float*)&facet;
      for (int j = 0; j < 12; ++ j, ++ f) // 3x vertex + normal: 4x3 = 12 floats
        if (*f > -1e-12f && *f < 1e-12f)
          // Negative zero, switch to positive zero.
          *f = 0;
    }
#endif
    /* Write the facet into memory. */
    memcpy(stl->facet_start+i, &facet, SIZEOF_STL_FACET);
    stl_facet_stats(stl, facet, first);
    first = 0;
  }
  stl->stats.size.x = stl->stats.max.x - stl->stats.min.x;
  stl->stats.size.y = stl->stats.max.y - stl->stats.min.y;
  stl->stats.size.z = stl->stats.max.z - stl->stats.min.z;
  stl->stats.bounding_diameter = sqrt(
                                   stl->stats.size.x * stl->stats.size.x +
                                   stl->stats.size.y * stl->stats.size.y +
                                   stl->stats.size.z * stl->stats.size.z
                                 );
}

void
stl_facet_stats(stl_file *stl, stl_facet facet, int first) {
  float diff_x;
  float diff_y;
  float diff_z;
  float max_diff;

  if (stl->error) return;

  /* while we are going through all of the facets, let's find the  */
  /* maximum and minimum values for x, y, and z  */

  /* Initialize the max and min values the first time through*/
  if (first) {
    stl->stats.max.x = facet.vertex[0].x;
    stl->stats.min.x = facet.vertex[0].x;
    stl->stats.max.y = facet.vertex[0].y;
    stl->stats.min.y = facet.vertex[0].y;
    stl->stats.max.z = facet.vertex[0].z;
    stl->stats.min.z = facet.vertex[0].z;

    diff_x = ABS(facet.vertex[0].x - facet.vertex[1].x);
    diff_y = ABS(facet.vertex[0].y - facet.vertex[1].y);
    diff_z = ABS(facet.vertex[0].z - facet.vertex[1].z);
    max_diff = STL_MAX(diff_x, diff_y);
    max_diff = STL_MAX(diff_z, max_diff);
    stl->stats.shortest_edge = max_diff;

    first = 0;
  }

  /* now find the max and min values */
  stl->stats.max.x = STL_MAX(stl->stats.max.x, facet.vertex[0].x);
  stl->stats.min.x = STL_MIN(stl->stats.min.x, facet.vertex[0].x);
  stl->stats.max.y = STL_MAX(stl->stats.max.y, facet.vertex[0].y);
  stl->stats.min.y = STL_MIN(stl->stats.min.y, facet.vertex[0].y);
  stl->stats.max.z = STL_MAX(stl->stats.max.z, facet.vertex[0].z);
  stl->stats.min.z = STL_MIN(stl->stats.min.z, facet.vertex[0].z);

  stl->stats.max.x = STL_MAX(stl->stats.max.x, facet.vertex[1].x);
  stl->stats.min.x = STL_MIN(stl->stats.min.x, facet.vertex[1].x);
  stl->stats.max.y = STL_MAX(stl->stats.max.y, facet.vertex[1].y);
  stl->stats.min.y = STL_MIN(stl->stats.min.y, facet.vertex[1].y);
  stl->stats.max.z = STL_MAX(stl->stats.max.z, facet.vertex[1].z);
  stl->stats.min.z = STL_MIN(stl->stats.min.z, facet.vertex[1].z);

  stl->stats.max.x = STL_MAX(stl->stats.max.x, facet.vertex[2].x);
  stl->stats.min.x = STL_MIN(stl->stats.min.x, facet.vertex[2].x);
  stl->stats.max.y = STL_MAX(stl->stats.max.y, facet.vertex[2].y);
  stl->stats.min.y = STL_MIN(stl->stats.min.y, facet.vertex[2].y);
  stl->stats.max.z = STL_MAX(stl->stats.max.z, facet.vertex[2].z);
  stl->stats.min.z = STL_MIN(stl->stats.min.z, facet.vertex[2].z);
}

void
stl_close(stl_file *stl) {
  if (stl->error) return;

  if(stl->neighbors_start != NULL)
    free(stl->neighbors_start);
  if(stl->facet_start != NULL)
    free(stl->facet_start);
  if(stl->v_indices != NULL)
    free(stl->v_indices);
  if(stl->v_shared != NULL)
    free(stl->v_shared);
  if (stl->v_shared_faces != NULL)
      free(stl->v_shared_faces);
}

