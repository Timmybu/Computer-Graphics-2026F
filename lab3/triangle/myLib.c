#include "myLib.h"
#include <stdio.h>
#include <math.h>


/*

---- Vectors Operations----

*/

void v4_print(vec4 v) {
    printf("[ %7.3f %7.3f %7.3f %7.3f ]\n", v.x, v.y, v.z, v.w);
}

vec4 v4_mul(float s, vec4 v) {
    vec4 product = {
        s * v.x, 
        s * v.y, 
        s * v.z, 
        s * v.w
    };
    return product;
}

vec4 v4_add(vec4 v1, vec4 v2) {
    vec4 sum = {
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z,
        v1.w + v2.w
    };
    return sum;
}

vec4 v4_sub(vec4 v1, vec4 v2) {
    vec4 diff = {
        v1.x - v2.x,
        v1.y - v2.y,
        v1.z - v2.z,
        v1.w - v2.w
    };
    return diff;
}

double v4_mag(vec4 v) {
    double mag = sqrt(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z + 
        v.w * v.w
    );

    return fabs(mag);
}

vec4 v4_norm(vec4 v) {
    double mag = v4_mag(v);
    vec4 norm = {
        (1/mag) * v.x,
        (1/mag) * v.y,
        (1/mag) * v.z,
        (1/mag) * v.w  
    };

    return norm;
}

float v4_dot(vec4 v1, vec4 v2) {
    float dot_prod = 
        v1.x * v2.x +
        v1.y * v2.y +
        v1.z * v2.z +
        v1.w * v2.w;

    return dot_prod;
}

// "Dont forget about right-hand rule"
vec4 v4_cross(vec4 v1, vec4 v2) {
    vec4 cross_prod = {
        (v1.y * v2.z) - (v1.z * v2.y),
        (v1.z * v2.x) - (v1.x * v2.z),
        (v1.x * v2.y) - (v1.y * v2.x),
        0.0
    };
    
    return cross_prod;
}

/*

---- Matrix Operations ----

*/

// ugg
void mat_print(mat4 m) { 
    printf("%7.3f %7.3f %7.3f %7.3f \n%7.3f %7.3f %7.3f %7.3f \n%7.3f %7.3f %7.3f %7.3f \n%7.3f %7.3f %7.3f %7.3f \n", m.x.x, m.y.x, m.z.x, m.w.x, m.x.y, m.y.y, m.z.y, m.w.y, m.x.z, m.y.z, m.z.z, m.w.z, m.x.w, m.y.w, m.z.w, m.w.w);
}

mat4 mat_scalar_mul(float s, mat4 m) {
    mat4 product = {
        v4_mul(s, m.x),
        v4_mul(s, m.y),
        v4_mul(s, m.z),
        v4_mul(s, m.w)
    };
    
    return product;
}

mat4 mat_add(mat4 m1, mat4 m2) {
    mat4 sum = {
        v4_add(m1.x, m2.x),
        v4_add(m1.y, m2.y),
        v4_add(m1.z, m2.z),
        v4_add(m1.w, m2.w)
    };

    return sum;
}

mat4 mat_sub(mat4 m1, mat4 m2) {
    mat4 diff = {
        v4_sub(m1.x, m2.x),
        v4_sub(m1.y, m2.y),
        v4_sub(m1.z, m2.z),
        v4_sub(m1.w, m2.w)
    };

    return diff;
}

mat4 mat_mul(mat4 m1, mat4 m2) {
    mat4 product;
    product.x = mat_v4_mul(m1, m2.x);
    product.y = mat_v4_mul(m1, m2.y);
    product.z = mat_v4_mul(m1, m2.z);
    product.w = mat_v4_mul(m1, m2.w);

    return product;
}

static float det3(  float a, float b, float c, float d, float e, float f, float g, float h, float i) {
    return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
    }

mat4 mat_inv(mat4 m) {
    mat4 mat_minor(mat4 m) {
    mat4 minor;
 
    /* drop m.x */
    minor.x.x = det3(m.y.y, m.y.z, m.y.w,
                     m.z.y, m.z.z, m.z.w,
                     m.w.y, m.w.z, m.w.w);
    minor.x.y = det3(m.y.x, m.y.z, m.y.w,
                     m.z.x, m.z.z, m.z.w,
                     m.w.x, m.w.z, m.w.w);
    minor.x.z = det3(m.y.x, m.y.y, m.y.w,
                     m.z.x, m.z.y, m.z.w,
                     m.w.x, m.w.y, m.w.w);
    minor.x.w = det3(m.y.x, m.y.y, m.y.z,
                     m.z.x, m.z.y, m.z.z,
                     m.w.x, m.w.y, m.w.z);
 
    /* drop m.y */
    minor.y.x = det3(m.x.y, m.x.z, m.x.w,
                     m.z.y, m.z.z, m.z.w,
                     m.w.y, m.w.z, m.w.w);
    minor.y.y = det3(m.x.x, m.x.z, m.x.w,
                     m.z.x, m.z.z, m.z.w,
                     m.w.x, m.w.z, m.w.w);
    minor.y.z = det3(m.x.x, m.x.y, m.x.w,
                     m.z.x, m.z.y, m.z.w,
                     m.w.x, m.w.y, m.w.w);
    minor.y.w = det3(m.x.x, m.x.y, m.x.z,
                     m.z.x, m.z.y, m.z.z,
                     m.w.x, m.w.y, m.w.z);
 
    /* drop m.z */
    minor.z.x = det3(m.x.y, m.x.z, m.x.w,
                     m.y.y, m.y.z, m.y.w,
                     m.w.y, m.w.z, m.w.w);
    minor.z.y = det3(m.x.x, m.x.z, m.x.w,
                     m.y.x, m.y.z, m.y.w,
                     m.w.x, m.w.z, m.w.w);
    minor.z.z = det3(m.x.x, m.x.y, m.x.w,
                     m.y.x, m.y.y, m.y.w,
                     m.w.x, m.w.y, m.w.w);
    minor.z.w = det3(m.x.x, m.x.y, m.x.z,
                     m.y.x, m.y.y, m.y.z,
                     m.w.x, m.w.y, m.w.z);
 
    /* drop m.w */
    minor.w.x = det3(m.x.y, m.x.z, m.x.w,
                     m.y.y, m.y.z, m.y.w,
                     m.z.y, m.z.z, m.z.w);
    minor.w.y = det3(m.x.x, m.x.z, m.x.w,
                     m.y.x, m.y.z, m.y.w,
                     m.z.x, m.z.z, m.z.w);
    minor.w.z = det3(m.x.x, m.x.y, m.x.w,
                     m.y.x, m.y.y, m.y.w,
                     m.z.x, m.z.y, m.z.w);
    minor.w.w = det3(m.x.x, m.x.y, m.x.z,
                     m.y.x, m.y.y, m.y.z,
                     m.z.x, m.z.y, m.z.z);
 
    return minor;
}


    mat4 mat_cofactor(mat4 m) {
        m.x.y = -m.x.y;
        m.x.w = -m.x.w;
        m.y.x = -m.y.x;
        m.y.z = -m.y.z;
        m.z.y = -m.z.y;
        m.z.w = -m.z.w;
        m.w.x = -m.w.x;
        m.w.z = -m.w.z;
        return m;
    }

    float mat_determinant(mat4 m, mat4 minor) {
        float det = 
            m.x.x * minor.x.x - 
            m.x.y * minor.x.y +
            m.x.z * minor.x.z - 
            m.x.w * minor.x.w;
        return det;
    }
    
    mat4 minor = mat_minor(m);
    mat4 cofactor = mat_cofactor(minor);
    mat4 transpose = mat_trans(cofactor);
    float determinant = mat_determinant(m, minor);
    mat4 m_inv = mat_scalar_mul(1/determinant, transpose);

    return m_inv;
}

mat4 mat_trans(mat4 m) {
    mat4 r;
    
    r.x.x = m.x.x;  r.x.y = m.y.x;  r.x.z = m.z.x;  r.x.w = m.w.x;
    r.y.x = m.x.y;  r.y.y = m.y.y;  r.y.z = m.z.y;  r.y.w = m.w.y;
    r.z.x = m.x.z;  r.z.y = m.y.z;  r.z.z = m.z.z;  r.z.w = m.w.z;
    r.w.x = m.x.w;  r.w.y = m.y.w;  r.w.z = m.z.w;  r.w.w = m.w.w;

    return r;
}

vec4 mat_v4_mul(mat4 m, vec4 v) {
    vec4 product;
    
    product.x = m.x.x * v.x + m.y.x * v.y + m.z.x * v.z + m.w.x * v.w;
    product.y = m.x.y * v.x + m.y.y * v.y + m.z.y * v.z + m.w.y * v.w;
    product.z = m.x.z * v.x + m.y.z * v.y + m.z.z * v.z + m.w.z * v.w;
    product.w = m.x.w * v.x + m.y.w * v.y + m.z.w * v.z + m.w.w * v.w;

    return product;
}