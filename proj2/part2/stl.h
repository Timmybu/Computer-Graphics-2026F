#ifndef _STL_H_
#define _STL_H_

#include "myLib.h"

/*
 * Reads an STL file (ASCII or binary, detected automatically).
 *
 * Returns a malloc'd array of vertex positions (w = 1.0), three per triangle:
 *   [0], [1], [2] = triangle 0
 *   [3], [4], [5] = triangle 1 ...
 * The number of vertices (3 * number of triangles) is stored in *num_vertices.
 *
 * The largest absolute coordinate value (the biggest |x|, |y|, or |z| of any
 * vertex) is stored in *max_value. Multiplying every position by
 * 1.0f / *max_value fits the model inside the -1 to 1 cube. Pass NULL for
 * max_value if you don't need it.
 *
 * Returns NULL (and sets *num_vertices and *max_value to 0) on failure.
 * The caller owns the returned array and must free() it.
 */
vec4 *stl_read(const char *filename, int *num_vertices, float *max_value);

/*
 * Generates a random RGBA color for each triangle. Each color is repeated for
 * all three of that triangle's vertices, so the result lines up 1:1 with the
 * array returned by stl_read (ready for glDrawArrays(GL_TRIANGLES, ...)).
 *
 * RGB components are in [0, 1], alpha is 1.0.
 * num_vertices must be a positive multiple of 3.
 * Call srand() once in your program beforehand to get different colors per run.
 *
 * Returns NULL on failure. The caller must free() the returned array.
 */
vec4 *stl_random_colors(int num_vertices);

#endif