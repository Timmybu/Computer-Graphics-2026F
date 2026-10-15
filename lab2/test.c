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
 *
 * If your library disagrees on 2 or 3, the affected checks are marked
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
    printf("\n");
    test_prints();

    printf("--------------------------------\n");
    printf("%d checks, %d failed\n", g_checks, g_fails);
    return g_fails == 0 ? 0 : 1;
}