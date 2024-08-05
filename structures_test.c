#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include "structures.h"
#include "testing.h"

#define EPSILON 0.01

#define TEST_OK_OUT NULL
#define TEST_FAIL_OUT stdout

static bool eps_equals(GLfloat expected, GLfloat actual) {
    return expected <= actual + EPSILON && expected >= actual - EPSILON;
}

int main(void) {
    // Testing functions for 2D vectors
    printf("Testing Vector2: \n");
    // Defining some 2D vectors to test with
    Vector2 v2UnitX = { .x = 1, .y = 0 };
    Vector2 v2UnitY = { .x = 0, .y = 1 };
    Vector2 v2Arb1  = { .x = 1, .y = 1 };
    Vector2 v2Arb2  = { .x = 2, .y = 2 };

    // Testing dot product for 2D vectors
    assert_test(dot_product_2(v2UnitX, v2UnitY) == 0, "Dot product test 1.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(dot_product_2(v2UnitX, v2Arb1) == 1, "Dot product test 2.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(dot_product_2(v2UnitX, v2Arb2) == 2, "Dot product test 3.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(dot_product_2(v2UnitY, v2Arb2) == 2, "Dot product test 4.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(dot_product_2(v2Arb1, v2Arb2) == 4, "Dot product test 5.", TEST_OK_OUT, TEST_FAIL_OUT);

    // Testing functions for 3D vectors
    printf("\nTesting Vector3: \n");
    // Defining some 3D vectors to test with
    Vector3 v3UnitX = { .x = 1, .y = 0, .z = 0 };
    Vector3 v3UnitY = { .x = 0, .y = 1, .z = 0 };
    Vector3 v3UnitZ = { .x = 0, .y = 0, .z = 1 };
    Vector3 v3Arb1  = { .x = 1, .y = 1, .z = 0 };
    Vector3 v3Arb2  = { .x = 1, .y = 2, .z = 1 };

    // Testing dot product for 3D vectors
    assert_test(dot_product_3(v3UnitX, v3UnitY) == 0, "Dot product test 1.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(dot_product_3(v3UnitX, v3Arb1) == 1, "Dot product test 2.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(dot_product_3(v3UnitX, v3Arb2) == 1, "Dot product test 3.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(dot_product_3(v3UnitZ, v3Arb2) == 1, "Dot product test 4.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(dot_product_3(v3Arb1, v3Arb2) == 3, "Dot product test 5.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(dot_product_3(v3UnitY, v3Arb2) == 2, "Dot product test 5.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(dot_product_3(v3UnitZ, v3Arb2) == 1, "Dot product test 5.", TEST_OK_OUT, TEST_FAIL_OUT);

    // Testing cross product for 3D vectors
    Vector3 crossXY = cross_product_3(v3UnitX, v3UnitY);
    Vector3 crossXZ = cross_product_3(v3UnitX, v3UnitZ);
    Vector3 crossArb = cross_product_3(v3Arb1, v3Arb2);
    assert_test(crossXY.x == 0 && crossXY.y == 0 && crossXY.z == 1, "Cross product test 1.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(crossXZ.x == 0 && crossXZ.y == 1 && crossXZ.z == 0, "Cross product test 2.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(crossArb.x == -1 && crossArb.y == 1 && crossArb.z == -1, "Cross product test 3.", TEST_OK_OUT, TEST_FAIL_OUT);

    // Testing normalise function for 3D vectors
    Vector3 normX = normalise_3(v3UnitX);
    Vector3 normArb1 = normalise_3(v3Arb1);
    Vector3 normArb2 = normalise_3(v3Arb2);
    assert_test(normX.x == 1 && normX.y == 0 && normX.z == 0, "Normalisation test 1.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(eps_equals(normArb1.x, 1/sqrtf(2)) && eps_equals(normArb1.y, 1/sqrtf(2)) && normArb1.z == 0, "Normalisation test 2.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(eps_equals(normArb2.x, 1/sqrtf(6)) && eps_equals(normArb2.y, 2/sqrtf(6)) && eps_equals(normArb2.z, 1/sqrtf(6)), "Normalisation test 3.", TEST_OK_OUT, TEST_FAIL_OUT);

    // Testing createCamera
    printf("\nTesting Create Camera: \n");
    Camera* cam = createCamera(1, 2, 3, 4, 5, 6);
    assert_test(cam->eyeX == 1, "eyeX correct.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(cam->eyeY == 2, "eyeY correct.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(cam->eyeZ == 3, "eyeZ correct.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(cam->upX == 4, "upX correct.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(cam->upY == 5, "upY correct.", TEST_OK_OUT, TEST_FAIL_OUT);
    assert_test(cam->upZ == 6, "upZ correct.", TEST_OK_OUT, TEST_FAIL_OUT);

    free(cam);
    return EXIT_SUCCESS;
}
