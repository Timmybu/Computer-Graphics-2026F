#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "stl.h"

#define STL_HEADER_BYTES   80
#define STL_PREAMBLE_BYTES 84   /* 80-byte header + 4-byte triangle count */
#define STL_TRI_BYTES      50   /* normal(12) + 3 vertices(36) + attribute(2) */

/* Binary STL is little-endian; decode byte by byte so this works on any host. */
static uint32_t read_u32_le(const unsigned char *b)
{
    return (uint32_t)b[0]
         | ((uint32_t)b[1] << 8)
         | ((uint32_t)b[2] << 16)
         | ((uint32_t)b[3] << 24);
}

static float read_f32_le(const unsigned char *b)
{
    uint32_t bits = read_u32_le(b);
    float f;
    memcpy(&f, &bits, sizeof f);
    return f;
}

static long get_file_size(FILE *fp)
{
    long size;
    if (fseek(fp, 0, SEEK_END) != 0) return -1;
    size = ftell(fp);
    rewind(fp);
    return size;
}

/*
 * Some binary STL files begin with the word "solid" just like ASCII ones, so
 * the header text can't be trusted. Instead, check whether the file size
 * matches exactly what a binary file with the stated triangle count would be.
 */
static int is_binary_stl(FILE *fp, long size)
{
    unsigned char pre[STL_PREAMBLE_BYTES];
    double expected;   /* double avoids 32-bit overflow and is exact for any real file size */

    if (size < STL_PREAMBLE_BYTES) return 0;
    if (fread(pre, 1, STL_PREAMBLE_BYTES, fp) != STL_PREAMBLE_BYTES) {
        rewind(fp);
        return 0;
    }
    rewind(fp);

    expected = STL_PREAMBLE_BYTES
             + (double)read_u32_le(pre + STL_HEADER_BYTES) * STL_TRI_BYTES;
    return expected == (double)size;
}

static vec4 *read_binary(FILE *fp, int *num_vertices)
{
    unsigned char pre[STL_PREAMBLE_BYTES];
    unsigned char tri[STL_TRI_BYTES];
    uint32_t num_tris, i;
    int j;
    vec4 *verts;

    if (fread(pre, 1, STL_PREAMBLE_BYTES, fp) != STL_PREAMBLE_BYTES) {
        fprintf(stderr, "stl_read: failed to read binary header\n");
        return NULL;
    }

    num_tris = read_u32_le(pre + STL_HEADER_BYTES);
    if (num_tris == 0 || num_tris > (uint32_t)(INT_MAX / 3)) {
        fprintf(stderr, "stl_read: invalid triangle count %lu\n", (unsigned long)num_tris);
        return NULL;
    }

    verts = malloc(sizeof(vec4) * 3 * (size_t)num_tris);
    if (!verts) {
        fprintf(stderr, "stl_read: out of memory\n");
        return NULL;
    }

    for (i = 0; i < num_tris; i++) {
        if (fread(tri, 1, STL_TRI_BYTES, fp) != STL_TRI_BYTES) {
            fprintf(stderr, "stl_read: unexpected end of file at triangle %lu\n", (unsigned long)i);
            free(verts);
            return NULL;
        }
        /* Skip the 12-byte normal; vertices follow. */
        for (j = 0; j < 3; j++) {
            const unsigned char *p = tri + 12 + 12 * j;
            vec4 *v = &verts[3 * i + j];
            v->x = read_f32_le(p);
            v->y = read_f32_le(p + 4);
            v->z = read_f32_le(p + 8);
            v->w = 1.0f;
        }
    }

    *num_vertices = (int)(3 * num_tris);
    return verts;
}

static vec4 *read_ascii(FILE *fp, int *num_vertices)
{
    char tok[64];
    int count = 0, cap = 0, c;
    vec4 *verts = NULL;

    /* Skip the "solid <name>" line so a model named e.g. "vertex" can't confuse us. */
    while ((c = fgetc(fp)) != EOF && c != '\n')
        ;

    while (fscanf(fp, "%63s", tok) == 1) {
        float x, y, z;

        if (strcmp(tok, "endsolid") == 0) break;
        if (strcmp(tok, "vertex") != 0) continue;

        if (fscanf(fp, "%f %f %f", &x, &y, &z) != 3) {
            fprintf(stderr, "stl_read: malformed vertex after %d vertices\n", count);
            free(verts);
            return NULL;
        }

        if (count == cap) {
            int new_cap = cap ? cap * 2 : 768;
            vec4 *tmp = realloc(verts, sizeof(vec4) * (size_t)new_cap);
            if (!tmp) {
                fprintf(stderr, "stl_read: out of memory\n");
                free(verts);
                return NULL;
            }
            verts = tmp;
            cap = new_cap;
        }

        verts[count].x = x;
        verts[count].y = y;
        verts[count].z = z;
        verts[count].w = 1.0f;
        count++;
    }

    if (count == 0 || count % 3 != 0) {
        fprintf(stderr, "stl_read: found %d vertices (need a positive multiple of 3)\n", count);
        free(verts);
        return NULL;
    }

    *num_vertices = count;
    return verts;
}

static float abs_f(float v)
{
    return v < 0.0f ? -v : v;
}

vec4 *stl_read(const char *filename, int *num_vertices, float *max_value)
{
    FILE *fp;
    long size;
    vec4 *verts;
    int i;

    if (num_vertices) *num_vertices = 0;
    if (max_value) *max_value = 0.0f;
    if (!filename || !num_vertices) return NULL;

    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "stl_read: cannot open '%s'\n", filename);
        return NULL;
    }

    size = get_file_size(fp);
    if (is_binary_stl(fp, size))
        verts = read_binary(fp, num_vertices);
    else
        verts = read_ascii(fp, num_vertices);

    fclose(fp);

    /* Find the largest absolute coordinate so the caller can scale the model. */
    if (verts && max_value) {
        float max = 0.0f;
        for (i = 0; i < *num_vertices; i++) {
            if (abs_f(verts[i].x) > max) max = abs_f(verts[i].x);
            if (abs_f(verts[i].y) > max) max = abs_f(verts[i].y);
            if (abs_f(verts[i].z) > max) max = abs_f(verts[i].z);
        }
        *max_value = max;
    }

    return verts;
}

vec4 *stl_random_colors(int num_vertices)
{
    vec4 *colors;
    int i;

    if (num_vertices <= 0 || num_vertices % 3 != 0) {
        fprintf(stderr, "stl_random_colors: num_vertices must be a positive multiple of 3\n");
        return NULL;
    }

    colors = malloc(sizeof(vec4) * (size_t)num_vertices);
    if (!colors) {
        fprintf(stderr, "stl_random_colors: out of memory\n");
        return NULL;
    }

    for (i = 0; i < num_vertices; i += 3) {
        vec4 col;
        col.x = (float)rand() / (float)RAND_MAX;
        col.y = (float)rand() / (float)RAND_MAX;
        col.z = (float)rand() / (float)RAND_MAX;
        col.w = 1.0f;
        colors[i] = colors[i + 1] = colors[i + 2] = col;
    }

    return colors;
}