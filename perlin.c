#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include "perlin.h"
#include "structures.h"

// Returns a Vector2 at a random angle with magnitude 1
static Vector2* generate_random_vector2() {
    Vector2* out = malloc(sizeof(Vector2));
    if (out == NULL) {
        fprintf(stderr, "Could not allocate memory for vector.\n");
        return NULL;
    }
    GLfloat a = (((GLfloat)rand())/((GLfloat)RAND_MAX)) * 2 * M_PI;
    out->x = cosf(a);
    out->y = sinf(a);
    return out;
}

// Frees the vectorGrid of the perlin then frees the perlin itself
void free_perlin(Perlin* perlin) {
    free2D((void****) &(perlin->vectorGrid), perlin->xSize, perlin->zSize);
    free(perlin);
}

// Returns a perlin with its grid of random 2D vectors initialised and generated
Perlin* create_perlin(int xSize, int zSize) {
    // Creating perlin
    Perlin* perlin = malloc(sizeof(Perlin));
    if (perlin == NULL) {
        fprintf(stderr, "Allocation of Perlin failed.\n");
        return NULL;
    }
    // Creating the vector grid
    Vector2*** grid;
    if (allocate2D((void ****) &grid, xSize, zSize) == 0) {
        free(perlin);
        return NULL;
    }
    // Initialising and generating the random vectors
    for (int i = 0; i < xSize; i++) {
        for (int j = 0; j < zSize; j++) {
            grid[i][j] = generate_random_vector2();
            if (grid[i][j] == NULL) {
                fprintf(stderr, "Allocation of vector (x, z) = (%d, %d) failed.\n", i, j);
                free2D((void ****) &grid, xSize, zSize);
                free_perlin(perlin);
                return NULL;
            }
        }
    }
    perlin->vectorGrid = grid;
    perlin->xSize = xSize;
    perlin->zSize = zSize;
    return perlin;
}

// Returns the dot product of the vector to v from the point (xGrid, yGrid) with
// the gradient vector at the point (xGrid, yGrid)
// This is used to get the perlin value at a point, once it has been interpolated
static GLfloat get_gradient_dot(Perlin* perlin, int xGrid, int yGrid, Vector2 v) {
    // (dx, dy) is the vector to v from the point (xGrid, yGrid)
    GLfloat dx = v.x - (GLfloat) xGrid;
    GLfloat dy = v.y - (GLfloat) yGrid;
    Vector2* gradient = perlin->vectorGrid[xGrid][yGrid];

    // Returns the dot product
    return (dx * gradient->x + dy * gradient->y);
}

// 'weight' should be between 0 and 1 but will work with other values thanks to clamping
// Returns a value which is 'weight' of the way towards 'val2' from 'val1'
// along a linear function
static GLfloat interpolate_linear(GLfloat val1, GLfloat val2, GLfloat weight) {
    // Clamping
    if (weight < 0) return val1;
    if (weight > 1) return val2;

    // Linear function
    return (val2 - val1) * weight + val1;
}

// 'weight' should be between 0 and 1 but will work with other values thanks to clamping
// Returns a value which is 'weight' of the way towards 'val2' from 'val1'
// along the smoothstep function
static GLfloat interpolate_smoothstep(GLfloat val1, GLfloat val2, GLfloat weight) {
    // Clamping
    if (weight < 0) return val1;
    if (weight > 1) return val2;

    // Smoothstep function
    return (val2 - val1) * (3 - 2 * weight) * weight * weight + val1;
}

// 'weight' should be between 0 and 1 but will work with other values thanks to clamping
// Returns a value which is 'weight' of the way towards 'val2' from 'val1'
// along the smootherstep function
static GLfloat interpolate_smootherstep(GLfloat val1, GLfloat val2, GLfloat weight) {
    // Clamping
    if (weight < 0) return val1;
    if (weight > 1) return val2;

    // Smootherstep function
    return (val2 - val1) * ((6 * weight - 15) * weight + 10) * weight * weight * weight + val1;
}

// General interpolation function which calls one of the specific interpolation functions
// based on the selected mode or returns 0 for an invalid mode
static GLfloat interpolate_general(GLfloat val1, GLfloat val2, GLfloat weight, int mode) {
    switch (mode){
        case 0:
            return interpolate_linear(val1, val2, weight);
            break;
        case 1:
            return interpolate_smoothstep(val1, val2, weight);
            break;
        case 2:
            return interpolate_smootherstep(val1, val2, weight);
            break;
        default:
            fprintf(stderr, "Not a valid interpolation mode - returned 0\n");
            return 0;
    }
}

// Returns the perlin value (between -1 and 1) at the point specified by the vector v
// The interpolation mode specifies which specific interpolation function should be used
// for the interpolation, giving the resulting terrain a different look
GLfloat get_perlin_value(Perlin* perlin, Vector2 v, int interpolation_mode){
    assert(v.x >= 0 && v.x < perlin->xSize);
    assert(v.y >= 0 && v.y < perlin->zSize);
    // Finds the coordinates of points surrounding the grid cell containing vector v.
    int x0 = (int) floor(v.x);
    int x1 = x0 + 1;
    int y0 = (int) floor(v.y);
    int y1 = y0 + 1;

    // Obtain dot products to corners
    GLfloat dotx0y0 = get_gradient_dot(perlin, x0, y0, v);
    GLfloat dotx1y0 = get_gradient_dot(perlin, x1, y0, v);
    GLfloat dotx0y1 = get_gradient_dot(perlin, x0, y1, v);
    GLfloat dotx1y1 = get_gradient_dot(perlin, x1, y1, v);

    // Get interpolation weights
    GLfloat wx = v.x - x0;
    GLfloat wy = v.y - y0;

    // Interpolate the value between the dot products to corners
    // Interpolate in the x direction
    GLfloat ix0 = interpolate_general(dotx0y0, dotx1y0, wx, interpolation_mode);
    GLfloat ix1 = interpolate_general(dotx0y1, dotx1y1, wx, interpolation_mode);
    // Interpolate in the y direction
    GLfloat out = interpolate_general(ix0, ix1, wy, interpolation_mode);

    return out;
}

