#ifndef _MYLIB_H_

#define _MYLIB_H_

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct {
    vec4 x;
    vec4 y;
    vec4 z;
    vec4 w;
} mat4;


// Insert function signatures after this line

void v4_print(vec4 v);

vec4 v4_mul(float s, vec4 v);

vec4 v4_add(vec4 v1, vec4 v2);

vec4 v4_sub(vec4 v1, vec4 v2);

double v4_mag(vec4 v);

float v4_dot(vec4 v1, vec4 v2);

vec4 v4_cross(vec4 v1, vec4 v2);

void mat_print(mat4 m);

mat4 mat_scalar_mul(float s, mat4 m);

mat4 mat_add(mat4 m1, mat4 m2);

mat4 mat_sub(mat4 m1, mat4 m2);

mat4 mat_mul(mat4 m1, mat4 m2);

mat4 mat_inv(mat4 m);

mat4 mat_trans(mat4 m);

vec4 mat_v4_mul(mat4 m, vec4 v);

mat4 mat_translate(float x, float y, float z);

mat4 mat_scale(float x, float y, float z);

mat4 mat_rot_z(float th);

mat4 mat_rot_x(float th);

mat4 mat_rot_y(float th);
// Do not put anything after this line

#endif
