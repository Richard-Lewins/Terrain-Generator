#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include "perlin.h"
#include "structures.h"
#include "testing.h"

#define XSIZE 50
#define ZSIZE 50
#define EPSILON 0.01

#define TEST_OK_OUT NULL
#define TEST_FAIL_OUT stdout

int main(void) {
    // Creates a perlin and checks it has been created successfully.
    Perlin* perlin = create_perlin(XSIZE, ZSIZE);
    assert_test(perlin != NULL, "Perlin created successfully.", TEST_OK_OUT, TEST_FAIL_OUT);

    assert_test(perlin->xSize == XSIZE, "Perlin xSize correct.", TEST_OK_OUT, TEST_FAIL_OUT);

    assert_test(perlin->zSize == ZSIZE, "Perlin zSize correct.", TEST_OK_OUT, TEST_FAIL_OUT);

    // Checking every vector is assigned a set of good values.
    for (int x = 0; x < XSIZE - 1; x++) {
        for (int z = 0; z < ZSIZE - 1; z++) {
            Vector2* v = perlin->vectorGrid[x][z];
            GLfloat mag = sqrtf((v->x * v->x) + (v->y * v->y));
            assert_test(mag >= 1 - EPSILON && mag <= 1 + EPSILON, "Perlin vector magnitudes correct.", TEST_OK_OUT, TEST_FAIL_OUT);
        }
    }
    
    // Checking for 0 at grid boundaries.
    for (int x = 0; x < XSIZE - 1; x++) {
        for (int z = 0; z < ZSIZE - 1; z++) {
            Vector2 v = {
                .x = x,
                .y = z
            };
            GLfloat val0 = get_perlin_value(perlin, v, 0);
            assert_test(val0 >= -EPSILON && val0 <= EPSILON, "Perlin grid value zero with mode 0.", TEST_OK_OUT, TEST_FAIL_OUT);
            GLfloat val1 = get_perlin_value(perlin, v, 1);
            assert_test(val1 >= -EPSILON && val1 <= EPSILON, "Perlin grid value zero with mode 1.", TEST_OK_OUT, TEST_FAIL_OUT);
            GLfloat val2 = get_perlin_value(perlin, v, 2);
            assert_test(val2 >= -EPSILON && val2 <= EPSILON, "Perlin grid value zero with mode 2.", TEST_OK_OUT, TEST_FAIL_OUT);
        }
    }

    // Checking random perlin values are between -1 and 1.
    for (int x = 0; x < XSIZE - 1; x++) {
        for (int z = 0; z < ZSIZE - 1; z++) {
            Vector2 v = {
                .x = x + ((double)rand()) / RAND_MAX,
                .y = z + ((double)rand()) / RAND_MAX
            };
            GLfloat val0 = get_perlin_value(perlin, v, 0);
            assert_test(val0 >= -1 - EPSILON && val0 <= 1 + EPSILON, "Perlin value with mode 0 bounded.", TEST_OK_OUT, TEST_FAIL_OUT);
            GLfloat val1 = get_perlin_value(perlin, v, 1);
            assert_test(val1 >= -1 - EPSILON && val1 <= 1 + EPSILON, "Perlin value with mode 1 bounded.", TEST_OK_OUT, TEST_FAIL_OUT);
            GLfloat val2 = get_perlin_value(perlin, v, 2);
            assert_test(val2 >= -1 - EPSILON && val2 <= 1 + EPSILON, "Perlin value with mode 2 bounded.", TEST_OK_OUT, TEST_FAIL_OUT);
        }
    }

    free_perlin(perlin);
    return EXIT_SUCCESS;
}

