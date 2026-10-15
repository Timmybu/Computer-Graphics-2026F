/* test.c -- tests for myLib.h
 *
 * Build:
 *     gcc -std=c11 -Wall -Wextra -o test test.c myLib.c -lm
 * Run:
 *     ./test          (exit status 0 = all passed, 1 = at least one failure)
 *
 * CONVENTIONS ASSUMED BY THESE TESTS
 * ----------------------------------
 * 1. mat4 is COLUMN MAJOR: m.x is the first column, m.y the second,
 *    m.z the third, m.w the fourth. Every expected matrix below is written
 *    out column by column, and failures are dumped column by column.
 *
 *        m.x.x  m.y.x  m.z.x  m.w.x
 *        m.x.y  m.y.y  m.z.y  m.w.y
 *        m.x.z  m.y.z  m.z.z  m.w.z
 *        m.x.w  m.y.w  m.z.w  m.w.w
 *
 * 2. v4_dot uses all four components (x*x + y*y + z*z + w*w).
 * 3. v4_cross is the 3D cross product of the xyz parts, with w set to 0.
 * 4. Transform matrices act on COLUMN vectors (M * v), so translation
 *    lives in the last column, and T * R * S applies S first, then R,
 *    then T. Points have w = 1, directions have w = 0.
 * 5. mat_rot_x/y/z take angles in DEGREES and rotate counter-clockwise
 *    (right-handed) when looking down the axis toward the origin:
 *    rot_z takes x toward y, rot_x takes y toward z, rot_y takes z toward x.
 *
 * If your library disagrees on 2, 3, or 5, the affected checks are marked
 * [CONVENTION] and you can adjust or delete them.
 */



/* Prompt: 
    Can you write me a simple test file that uses these funcs and tests 
    its expected output. the header file is "myLib.h" and test file is "test.c". 
    Prints should be in column major order.
*/

#include <stdio.h>
#include <math.h>

#include "myLib.h"

/* ------------------------------------------------------------------ */
/* tiny test harness                                                   */
/* ------------------------------------------------------------------ */

#define EPS 1e-4

static int g_checks = 0;
static int g_fails  = 0;

static int near_d(double a, double b) {
    double m = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    if (m < 1.0) m = 1.0;
    return fabs(a - b) <= EPS * m;
}

static vec4 v4(float x, float y, float z, float w) {
    vec4 v;
    v.x = x; v.y = y; v.z = z; v.w = w;
    return v;
}

/* columns, in order */
static mat4 m4(vec4 c0, vec4 c1, vec4 c2, vec4 c3) {
    mat4 m;
    m.x = c0; m.y = c1; m.z = c2; m.w = c3;
    return m;
}

static void pass(const char *name) {
    g_checks++;
    printf("  ok    %s\n", name);
}

static void fail(const char *name) {
    g_checks++;
    g_fails++;
    printf("  FAIL  %s\n", name);
}

static void dump_v4(const char *label, vec4 v) {
    printf("        %-6s (%g, %g, %g, %g)\n", label, v.x, v.y, v.z, v.w);
}

/* prints the four columns one per line */
static void dump_mat(const char *label, mat4 m) {
    printf("        %s\n", label);
    printf("          col0 (%g, %g, %g, %g)\n", m.x.x, m.x.y, m.x.z, m.x.w);
    printf("          col1 (%g, %g, %g, %g)\n", m.y.x, m.y.y, m.y.z, m.y.w);
    printf("          col2 (%g, %g, %g, %g)\n", m.z.x, m.z.y, m.z.z, m.z.w);
    printf("          col3 (%g, %g, %g, %g)\n", m.w.x, m.w.y, m.w.z, m.w.w);
}

static void check_f(const char *name, double got, double want) {
    if (near_d(got, want)) {
        pass(name);
    } else {
        fail(name);
        printf("        got  %g\n        want %g\n", got, want);
    }
}

static void check_v4(const char *name, vec4 got, vec4 want) {
    if (near_d(got.x, want.x) && near_d(got.y, want.y) &&
        near_d(got.z, want.z) && near_d(got.w, want.w)) {
        pass(name);
    } else {
        fail(name);
        dump_v4("got", got);
        dump_v4("want", want);
    }
}

static void check_mat(const char *name, mat4 got, mat4 want) {
    const vec4 g[4] = { got.x,  got.y,  got.z,  got.w  };
    const vec4 w[4] = { want.x, want.y, want.z, want.w };
    for (int i = 0; i < 4; i++) {
        if (!near_d(g[i].x, w[i].x) || !near_d(g[i].y, w[i].y) ||
            !near_d(g[i].z, w[i].z) || !near_d(g[i].w, w[i].w)) {
            fail(name);
            dump_mat("got (by column)",  got);
            dump_mat("want (by column)", want);
            return;
        }
    }
    pass(name);
}

/* ------------------------------------------------------------------ */
/* shared fixtures                                                     */
/* ------------------------------------------------------------------ */

/* I = identity */
static mat4 ident(void) {
    return m4(v4(1, 0, 0, 0),
              v4(0, 1, 0, 0),
              v4(0, 0, 1, 0),
              v4(0, 0, 0, 1));
}

/* A: columns are 1..4, 5..8, 9..12, 13..16
 *      1  5   9  13
 *      2  6  10  14
 *      3  7  11  15
 *      4  8  12  16
 * (singular on purpose -- never fed to mat_inv) */
static mat4 mat_A(void) {
    return m4(v4( 1,  2,  3,  4),
              v4( 5,  6,  7,  8),
              v4( 9, 10, 11, 12),
              v4(13, 14, 15, 16));
}

/* D = diag(1, 2, 3, 4) */
static mat4 mat_D(void) {
    return m4(v4(1, 0, 0, 0),
              v4(0, 2, 0, 0),
              v4(0, 0, 3, 0),
              v4(0, 0, 0, 4));
}

/* C: invertible, det = 25
 *      2  1  0  1
 *      0  3  1  2
 *      1  0  4  3
 *      0  0  0  1   */
static mat4 mat_C(void) {
    return m4(v4(2, 0, 1, 0),
              v4(1, 3, 0, 0),
              v4(0, 1, 4, 0),
              v4(1, 2, 3, 1));
}

/* ------------------------------------------------------------------ */
/* tests                                                              */
/* ------------------------------------------------------------------ */

static void test_v4_arith(void) {
    printf("vec4 arithmetic\n");

    vec4 a = v4(1, 2, 3, 0);
    vec4 b = v4(4, 5, 6, 0);
    vec4 p = v4(1, -2, 0.5f, 3);

    check_v4("v4_add(a, b)",        v4_add(a, b),      v4(5, 7, 9, 0));
    check_v4("v4_add is commutative", v4_add(b, a),    v4_add(a, b));
    check_v4("v4_sub(a, b)",        v4_sub(a, b),      v4(-3, -3, -3, 0));
    check_v4("v4_sub(a, a) == 0",   v4_sub(a, a),      v4(0, 0, 0, 0));
    check_v4("v4_mul(2, a)",        v4_mul(2.0f, a),   v4(2, 4, 6, 0));
    check_v4("v4_mul(-1, p)",       v4_mul(-1.0f, p),  v4(-1, 2, -0.5f, -3));
    check_v4("v4_mul(0, p) == 0",   v4_mul(0.0f, p),   v4(0, 0, 0, 0));
    check_v4("v4_mul(0.5, p)",      v4_mul(0.5f, p),   v4(0.5f, -1, 0.25f, 1.5f));
}

static void test_v4_products(void) {
    printf("vec4 magnitude / dot / cross\n");

    vec4 a  = v4(1, 2, 3, 0);
    vec4 b  = v4(4, 5, 6, 0);
    vec4 ex = v4(1, 0, 0, 0);
    vec4 ey = v4(0, 1, 0, 0);
    vec4 ez = v4(0, 0, 1, 0);

    check_f("v4_mag(3,4,0,0) == 5",      v4_mag(v4(3, 4, 0, 0)), 5.0);
    check_f("v4_mag(0,0,0,0) == 0",      v4_mag(v4(0, 0, 0, 0)), 0.0);
    check_f("v4_mag(1,1,1,1) == 2",      v4_mag(v4(1, 1, 1, 1)), 2.0);
    check_f("v4_mag(a) == sqrt(14)",     v4_mag(a), sqrt(14.0));
    check_f("v4_mag(-a) == v4_mag(a)",   v4_mag(v4_mul(-1.0f, a)), v4_mag(a));

    check_f("v4_dot(a, b) == 32",        v4_dot(a, b), 32.0);
    check_f("v4_dot is symmetric",       v4_dot(b, a), v4_dot(a, b));
    check_f("v4_dot(ex, ey) == 0",       v4_dot(ex, ey), 0.0);
    check_f("v4_dot(a, a) == mag^2",     v4_dot(a, a), v4_mag(a) * v4_mag(a));
    /* [CONVENTION] fails if dot only uses xyz (would give 0) */
    check_f("v4_dot uses w component",
            v4_dot(v4(1, 0, 0, 2), v4(0, 1, 0, 3)), 6.0);

    check_v4("v4_cross(ex, ey) == ez",   v4_cross(ex, ey), ez);
    check_v4("v4_cross(ey, ez) == ex",   v4_cross(ey, ez), ex);
    check_v4("v4_cross(ez, ex) == ey",   v4_cross(ez, ex), ey);
    check_v4("v4_cross(ey, ex) == -ez",  v4_cross(ey, ex), v4(0, 0, -1, 0));
    check_v4("v4_cross(a, a) == 0",      v4_cross(a, a), v4(0, 0, 0, 0));
    /* [CONVENTION] assumes w of the result is 0 */
    check_v4("v4_cross(a, b)",           v4_cross(a, b), v4(-3, 6, -3, 0));
    check_f("cross(a,b) is perp to a",   v4_dot(v4_cross(a, b), a), 0.0);
    check_f("cross(a,b) is perp to b",   v4_dot(v4_cross(a, b), b), 0.0);
}

static void test_mat_arith(void) {
    printf("mat4 arithmetic\n");

    mat4 A = mat_A();
    mat4 D = mat_D();
    mat4 I = ident();

    /* 2*D = diag(2,4,6,8) */
    check_mat("mat_scalar_mul(2, D)", mat_scalar_mul(2.0f, D),
              m4(v4(2, 0, 0, 0),
                 v4(0, 4, 0, 0),
                 v4(0, 0, 6, 0),
                 v4(0, 0, 0, 8)));

    check_mat("mat_scalar_mul(1, A) == A", mat_scalar_mul(1.0f, A), A);

    /*  A + D =    2  5   9  13
     *             2  8  10  14
     *             3  7  14  15
     *             4  8  12  20 */
    check_mat("mat_add(A, D)", mat_add(A, D),
              m4(v4(2, 2, 3, 4),
                 v4(5, 8, 7, 8),
                 v4(9, 10, 14, 12),
                 v4(13, 14, 15, 20)));

    check_mat("mat_add is commutative", mat_add(D, A), mat_add(A, D));

    /*  A - D =    0  5   9  13
     *             2  4  10  14
     *             3  7   8  15
     *             4  8  12  12 */
    check_mat("mat_sub(A, D)", mat_sub(A, D),
              m4(v4(0, 2, 3, 4),
                 v4(5, 4, 7, 8),
                 v4(9, 10, 8, 12),
                 v4(13, 14, 15, 12)));

    check_mat("mat_sub(A, A) == 0", mat_sub(A, A),
              m4(v4(0, 0, 0, 0), v4(0, 0, 0, 0),
                 v4(0, 0, 0, 0), v4(0, 0, 0, 0)));

    check_mat("A + 0 == A", mat_add(A, mat_sub(A, A)), A);
    check_mat("I - I == 0", mat_sub(I, I), mat_scalar_mul(0.0f, I));
}

static void test_mat_mul(void) {
    printf("mat4 multiplication\n");

    mat4 A = mat_A();
    mat4 D = mat_D();
    mat4 I = ident();

    check_mat("A * I == A", mat_mul(A, I), A);
    check_mat("I * A == A", mat_mul(I, A), A);
    check_mat("I * I == I", mat_mul(I, I), I);

    /* A * D scales column j of A by D[j][j] */
    check_mat("mat_mul(A, D)", mat_mul(A, D),
              m4(v4( 1,  2,  3,  4),
                 v4(10, 12, 14, 16),
                 v4(27, 30, 33, 36),
                 v4(52, 56, 60, 64)));

    /* D * A scales row i of A by D[i][i] -- different result, so this
     * also catches an operand/order mix-up inside mat_mul */
    check_mat("mat_mul(D, A)", mat_mul(D, A),
              m4(v4( 1,  4,  9, 16),
                 v4( 5, 12, 21, 32),
                 v4( 9, 20, 33, 48),
                 v4(13, 28, 45, 64)));

    /* (A*D)^T == D^T * A^T */
    check_mat("transpose of a product",
              mat_trans(mat_mul(A, D)),
              mat_mul(mat_trans(D), mat_trans(A)));
}

static void test_mat_trans(void) {
    printf("mat4 transpose\n");

    mat4 A = mat_A();
    mat4 I = ident();

    /* columns of A^T are the rows of A */
    check_mat("mat_trans(A)", mat_trans(A),
              m4(v4(1, 5,  9, 13),
                 v4(2, 6, 10, 14),
                 v4(3, 7, 11, 15),
                 v4(4, 8, 12, 16)));

    check_mat("mat_trans(mat_trans(A)) == A", mat_trans(mat_trans(A)), A);
    check_mat("mat_trans(I) == I", mat_trans(I), I);

    /* symmetric matrix is its own transpose */
    mat4 S = m4(v4(1, 2, 3, 4),
                v4(2, 5, 6, 7),
                v4(3, 6, 8, 9),
                v4(4, 7, 9, 10));
    check_mat("symmetric matrix unchanged", mat_trans(S), S);
}

static void test_mat_inv(void) {
    printf("mat4 inverse\n");

    mat4 I = ident();
    mat4 D = mat_D();
    mat4 C = mat_C();

    check_mat("mat_inv(I) == I", mat_inv(I), I);

    /* inverse of a diagonal matrix is the reciprocal diagonal */
    check_mat("mat_inv(diag(1,2,3,4))", mat_inv(D),
              m4(v4(1, 0, 0, 0),
                 v4(0, 0.5f, 0, 0),
                 v4(0, 0, 1.0f / 3.0f, 0),
                 v4(0, 0, 0, 0.25f)));

    check_mat("C * inv(C) == I", mat_mul(C, mat_inv(C)), I);
    check_mat("inv(C) * C == I", mat_mul(mat_inv(C), C), I);
    check_mat("inv(inv(C)) == C", mat_inv(mat_inv(C)), C);
    check_mat("inv(C^T) == inv(C)^T", mat_inv(mat_trans(C)),
              mat_trans(mat_inv(C)));

    /* note: mat_A() is singular, so mat_inv(mat_A()) is not tested --
     * decide separately what your library should do with a singular
     * matrix (return identity? all zeros? set errno?) and test that. */
}

static void test_mat_v4_mul(void) {
    printf("mat4 * vec4\n");

    mat4 A = mat_A();
    mat4 D = mat_D();
    mat4 I = ident();
    vec4 v = v4(1, 2, 3, 4);
    vec4 ones = v4(1, 1, 1, 1);

    check_v4("I * v == v", mat_v4_mul(I, v), v);
    check_v4("D * (1,2,3,4)", mat_v4_mul(D, v), v4(1, 4, 9, 16));

    /* A * (1,1,1,1) sums the columns of A */
    check_v4("A * (1,1,1,1)", mat_v4_mul(A, ones), v4(28, 32, 36, 40));

    /* A * (1,0,0,0) picks out column 0 */
    check_v4("A * e0 == column 0", mat_v4_mul(A, v4(1, 0, 0, 0)), A.x);
    check_v4("A * e3 == column 3", mat_v4_mul(A, v4(0, 0, 0, 1)), A.w);

    /* A * (1,2,3,4) = 1*col0 + 2*col1 + 3*col2 + 4*col3 */
    check_v4("A * (1,2,3,4)", mat_v4_mul(A, v),
             v4(1 + 10 + 27 + 52,
                2 + 12 + 30 + 56,
                3 + 14 + 33 + 60,
                4 + 16 + 36 + 64));

    /* (A*D)*v == A*(D*v) */
    check_v4("associativity with mat_mul",
             mat_v4_mul(mat_mul(A, D), v),
             mat_v4_mul(A, mat_v4_mul(D, v)));

    check_v4("A * 0 == 0", mat_v4_mul(A, v4(0, 0, 0, 0)), v4(0, 0, 0, 0));
}

/* ------------------------------------------------------------------ */
/* transform builders: translate / scale / rotate                      */
/* ------------------------------------------------------------------ */

#define PI 3.14159265358979323846

static vec4 pt(float x, float y, float z)  { return v4(x, y, z, 1); } /* point     */
static vec4 dir(float x, float y, float z) { return v4(x, y, z, 0); } /* direction */

static void test_mat_translate(void) {
    printf("mat_translate\n");

    mat4 I = ident();
    mat4 T = mat_translate(2, -3, 5);

    check_mat("mat_translate(0,0,0) == I", mat_translate(0, 0, 0), I);

    /* translation lives in the last column (column major) */
    check_mat("mat_translate(2,-3,5)", T,
              m4(v4(1, 0, 0, 0),
                 v4(0, 1, 0, 0),
                 v4(0, 0, 1, 0),
                 v4(2, -3, 5, 1)));

    check_v4("T * point moves it",      mat_v4_mul(T, pt(1, 1, 1)),  pt(3, -2, 6));
    check_v4("T * origin == offset",    mat_v4_mul(T, pt(0, 0, 0)),  pt(2, -3, 5));
    /* w = 0 means "direction", which translation must not affect */
    check_v4("T * direction unchanged", mat_v4_mul(T, dir(1, 2, 3)), dir(1, 2, 3));

    check_mat("T(a) * T(b) == T(a+b)",
              mat_mul(mat_translate(1, 2, 3), mat_translate(4, 5, 6)),
              mat_translate(5, 7, 9));
    check_mat("translations commute",
              mat_mul(mat_translate(1, 2, 3), mat_translate(4, 5, 6)),
              mat_mul(mat_translate(4, 5, 6), mat_translate(1, 2, 3)));
    check_mat("T(v) * T(-v) == I", mat_mul(T, mat_translate(-2, 3, -5)), I);
    check_mat("mat_inv(T(v)) == T(-v)", mat_inv(T), mat_translate(-2, 3, -5));
}

static void test_mat_scale(void) {
    printf("mat_scale\n");

    mat4 I = ident();
    mat4 S = mat_scale(2, 3, 4);

    check_mat("mat_scale(1,1,1) == I", mat_scale(1, 1, 1), I);

    /* w stays 1 -- scale must not touch the homogeneous coordinate */
    check_mat("mat_scale(2,3,4)", S,
              m4(v4(2, 0, 0, 0),
                 v4(0, 3, 0, 0),
                 v4(0, 0, 4, 0),
                 v4(0, 0, 0, 1)));

    check_v4("S * point",            mat_v4_mul(S, pt(1, 1, 1)),      pt(2, 3, 4));
    check_v4("S * direction",        mat_v4_mul(S, dir(1, -1, 0.5f)), dir(2, -3, 2));
    check_v4("S * origin == origin", mat_v4_mul(S, pt(0, 0, 0)),      pt(0, 0, 0));

    check_mat("mat_scale(-1,1,1) mirrors x", mat_scale(-1, 1, 1),
              m4(v4(-1, 0, 0, 0),
                 v4( 0, 1, 0, 0),
                 v4( 0, 0, 1, 0),
                 v4( 0, 0, 0, 1)));
    check_v4("zero scale flattens an axis",
             mat_v4_mul(mat_scale(1, 0, 1), pt(5, 6, 7)), pt(5, 0, 7));

    check_mat("S(a) * S(b) == S(a*b)",
              mat_mul(mat_scale(2, 3, 4), mat_scale(0.5f, 2, -1)),
              mat_scale(1, 6, -4));
    check_mat("mat_inv(S) == S(1/x,1/y,1/z)", mat_inv(S),
              mat_scale(0.5f, 1.0f / 3.0f, 0.25f));
}

/* checks shared by every rotation builder */
static void check_rot_common(const char *axis, mat4 (*rot)(float)) {
    char name[96];
    mat4 I = ident();
    float th = 37.0f;   /* degrees; arbitrary, not a "nice" angle */
    mat4 R = rot(th);
    vec4 v = dir(1, -2, 3);

    snprintf(name, sizeof name, "%s(0) == I", axis);
    check_mat(name, rot(0), I);

    snprintf(name, sizeof name, "%s(360) == I", axis);
    check_mat(name, rot(360.0f), I);

    snprintf(name, sizeof name, "%s(a) * %s(b) == %s(a+b)", axis, axis, axis);
    check_mat(name, mat_mul(rot(20.0f), rot(45.0f)), rot(65.0f));

    snprintf(name, sizeof name, "%s(th) * %s(-th) == I", axis, axis);
    check_mat(name, mat_mul(R, rot(-th)), I);

    snprintf(name, sizeof name, "%s: R^T * R == I (orthonormal)", axis);
    check_mat(name, mat_mul(mat_trans(R), R), I);

    snprintf(name, sizeof name, "%s: inv(R) == R^T", axis);
    check_mat(name, mat_inv(R), mat_trans(R));

    snprintf(name, sizeof name, "%s: preserves length", axis);
    check_f(name, v4_mag(mat_v4_mul(R, v)), v4_mag(v));

    snprintf(name, sizeof name, "%s: origin stays put", axis);
    check_v4(name, mat_v4_mul(R, pt(0, 0, 0)), pt(0, 0, 0));

    snprintf(name, sizeof name, "%s: col3 == (0,0,0,1)", axis);
    check_v4(name, R.w, v4(0, 0, 0, 1));
}

/* [CONVENTION] the direction-specific checks below assume DEGREES and
 * right-handed counter-clockwise rotation (see convention 5 at the top).
 * With a left-handed / clockwise library, every rotation matrix comes out
 * transposed, so the "* ex == ey" style checks and the 30-degree matrices
 * fail while the check_rot_common properties still pass. If the function
 * took radians instead, almost everything here would fail. */
static void test_mat_rot(void) {
    printf("mat_rot_x / mat_rot_y / mat_rot_z\n");

    const float q = 90.0f;                /* degrees */
    const float c = (float)cos(PI / 6.0); /* cos 30 deg, computed in radians */
    const float s = (float)sin(PI / 6.0); /* sin 30 deg */
    vec4 ex = dir(1, 0, 0), ey = dir(0, 1, 0), ez = dir(0, 0, 1);

    /* ---- rot_z ---- */
    check_v4("rot_z(90) * ex == ey",  mat_v4_mul(mat_rot_z(q), ex), ey);
    check_v4("rot_z(90) * ey == -ex", mat_v4_mul(mat_rot_z(q), ey), dir(-1, 0, 0));
    check_v4("rot_z(90) * ez == ez",  mat_v4_mul(mat_rot_z(q), ez), ez);
    check_mat("mat_rot_z(30 deg)", mat_rot_z(30.0f),
              m4(v4( c, s, 0, 0),
                 v4(-s, c, 0, 0),
                 v4( 0, 0, 1, 0),
                 v4( 0, 0, 0, 1)));

    /* ---- rot_x ---- */
    check_v4("rot_x(90) * ey == ez",  mat_v4_mul(mat_rot_x(q), ey), ez);
    check_v4("rot_x(90) * ez == -ey", mat_v4_mul(mat_rot_x(q), ez), dir(0, -1, 0));
    check_v4("rot_x(90) * ex == ex",  mat_v4_mul(mat_rot_x(q), ex), ex);
    check_mat("mat_rot_x(30 deg)", mat_rot_x(30.0f),
              m4(v4(1,  0, 0, 0),
                 v4(0,  c, s, 0),
                 v4(0, -s, c, 0),
                 v4(0,  0, 0, 1)));

    /* ---- rot_y ---- */
    check_v4("rot_y(90) * ez == ex",  mat_v4_mul(mat_rot_y(q), ez), ex);
    check_v4("rot_y(90) * ex == -ez", mat_v4_mul(mat_rot_y(q), ex), dir(0, 0, -1));
    check_v4("rot_y(90) * ey == ey",  mat_v4_mul(mat_rot_y(q), ey), ey);
    check_mat("mat_rot_y(30 deg)", mat_rot_y(30.0f),
              m4(v4(c, 0, -s, 0),
                 v4(0, 1,  0, 0),
                 v4(s, 0,  c, 0),
                 v4(0, 0,  0, 1)));

    /* half turn about z negates x and y, leaves z and w alone */
    check_v4("rot_z(180) * (1,2,3,1)",
             mat_v4_mul(mat_rot_z(180.0f), pt(1, 2, 3)), pt(-1, -2, 3));

    /* ---- properties every rotation must have, whatever the convention ---- */
    check_rot_common("rot_x", mat_rot_x);
    check_rot_common("rot_y", mat_rot_y);
    check_rot_common("rot_z", mat_rot_z);

    /* rotations about different axes do NOT commute; also catches two
     * builders accidentally returning the same matrix */
    check_v4("rot_x(90)*rot_y(90) * ez",
             mat_v4_mul(mat_mul(mat_rot_x(q), mat_rot_y(q)), ez), dir(1, 0, 0));
    check_v4("rot_y(90)*rot_x(90) * ez",
             mat_v4_mul(mat_mul(mat_rot_y(q), mat_rot_x(q)), ez), dir(0, -1, 0));
}

static void test_transform_compose(void) {
    printf("composed transforms (T * R * S)\n");

    mat4 T = mat_translate(1, 2, 3);
    mat4 R = mat_rot_z(90.0f);
    mat4 S = mat_scale(2, 2, 2);
    mat4 M = mat_mul(T, mat_mul(R, S));
    vec4 p = pt(1, 0, 0);

    /* M = T*R*S applies S first, then R, then T:
     *   (1,0,0) -S-> (2,0,0) -R-> (0,2,0) -T-> (1,4,3) */
    check_v4("T*R*S * (1,0,0,1)", mat_v4_mul(M, p), pt(1, 4, 3));

    check_v4("T*R*S * p == T*(R*(S*p))", mat_v4_mul(M, p),
             mat_v4_mul(T, mat_v4_mul(R, mat_v4_mul(S, p))));

    /* reverse order gives a different answer:
     *   (1,0,0) -T-> (2,2,3) -R-> (-2,2,3) -S-> (-4,4,6) */
    check_v4("S*R*T * (1,0,0,1)",
             mat_v4_mul(mat_mul(S, mat_mul(R, T)), p), pt(-4, 4, 6));

    /* T*R keeps the translation untouched in col3; R*T rotates it */
    check_v4("col3 of T*R", mat_mul(T, R).w, pt(1, 2, 3));
    check_v4("col3 of R*T", mat_mul(R, T).w, pt(-2, 1, 3));

    check_mat("inv(T*R*S) * (T*R*S) == I", mat_mul(mat_inv(M), M), ident());
    check_mat("inv(T*R*S) == S^-1 * R^-1 * T^-1", mat_inv(M),
              mat_mul(mat_scale(0.5f, 0.5f, 0.5f),
                      mat_mul(mat_rot_z(-90.0f),
                              mat_translate(-1, -2, -3))));
}

/* The print functions return void, so these cannot be asserted --
 * eyeball them against the expected values printed alongside. */
static void test_prints(void) {
    printf("print functions (visual check)\n\n");

    printf("  v4_print(v4(1, 2, 3, 4)) -- expect 1, 2, 3, 4:\n");
    v4_print(v4(1, 2, 3, 4));

    printf("\n  v4_print(v4(-0.5, 0, 2.25, 100)) -- expect -0.5, 0, 2.25, 100:\n");
    v4_print(v4(-0.5f, 0.0f, 2.25f, 100.0f));

    printf("\n  mat_print(identity) -- expect 1s down the diagonal:\n");
    mat_print(ident());

    printf("\n  mat_print(A), columns in order:\n"
           "      col0 = 1,  2,  3,  4\n"
           "      col1 = 5,  6,  7,  8\n"
           "      col2 = 9,  10, 11, 12\n"
           "      col3 = 13, 14, 15, 16\n");
    mat_print(mat_A());

    printf("\n  mat_print(mat_trans(A)), columns in order:\n"
           "      col0 = 1, 5, 9,  13\n"
           "      col1 = 2, 6, 10, 14\n"
           "      col2 = 3, 7, 11, 15\n"
           "      col3 = 4, 8, 12, 16\n");
    mat_print(mat_trans(mat_A()));

    printf("\n  mat_print(mat_translate(2, -3, 5)), columns in order:\n"
           "      col0 = 1, 0, 0, 0\n"
           "      col1 = 0, 1, 0, 0\n"
           "      col2 = 0, 0, 1, 0\n"
           "      col3 = 2, -3, 5, 1\n");
    mat_print(mat_translate(2, -3, 5));

    printf("\n");
}

int main(void) {
    test_v4_arith();
    test_v4_products();
    test_mat_arith();
    test_mat_mul();
    test_mat_trans();
    test_mat_inv();
    test_mat_v4_mul();
    test_mat_translate();
    test_mat_scale();
    test_mat_rot();
    test_transform_compose();
    printf("\n");
    test_prints();

    printf("--------------------------------\n");
    printf("%d checks, %d failed\n", g_checks, g_fails);
    return g_fails == 0 ? 0 : 1;
}