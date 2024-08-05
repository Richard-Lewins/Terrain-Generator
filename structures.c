#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "structures.h"

// Defines a constant vector to be the up direction
static Vector3 upwards = {.x = 0, .y = 1, .z = 0};

// Returns the dot product between 2 2D vectors
GLfloat dot_product_2(Vector2 v1, Vector2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

// Returns the dot product between 2 3D vectors
GLfloat dot_product_3(Vector3 v1, Vector3 v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// Returns the cross product between 2 3D vectors
Vector3 cross_product_3(Vector3 v1, Vector3 v2) {
    Vector3 out = {
        .x = (v1.y * v2.z) - (v1.z * v2.y),
	    .y = (v2.x * v1.z) - (v2.z * v1.x),
	    .z = (v1.x * v2.y) - (v1.y * v2.x)
    };
    GLfloat magnitude = sqrtf(
            (out.x * out.x)
	        + (out.y * out.y)
	        + (out.z * out.z)
        );
    assert(magnitude != 0);
    GLfloat dot = dot_product_3(out, upwards);
    if (dot >= 0) return out;
    // Vector is pointing in the wrong direction
    out.x /= -1;
    out.y /= -1;
    out.z /= -1;
    return out;
}

// Normalises a 3D vector so that it keeps its direction but has a magnitude of 1
Vector3 normalise_3(Vector3 v) {
    GLfloat magnitude = sqrtf(
            (v.x * v.x)
            + (v.y * v.y)
            + (v.z * v.z)
    );
    assert(magnitude != 0);
    v.x /= magnitude;
    v.y /= magnitude;
    v.z /= magnitude;
    return v;
}

// Allocates a 2D array of pointers, initialised to NULL
// A pointer to the array is passed into the function, as well as its dimensions
int allocate2D(void**** at, int xSize, int zSize) {
    *at = malloc(sizeof(void*) * xSize);
    if (*at == NULL) return 0;

    // Allocating Overall Space
    (*at)[0] = malloc(sizeof(void*) * xSize * zSize);
    if ((*at)[0] == NULL) {
	free(*at);
	return 0;
    }

    // Directing Pointers Properly.
    for (int i = 1; i < xSize; i++) {
    	(*at)[i] = (*at)[i - 1] + zSize;
    }

    // Filling array with NULL's.
    for (int i = 0; i < xSize; i++) {
    	for (int j = 0; j < zSize; j++) {
            (*at)[i][j] = NULL;
	    }
    }

    return 1;
}

// Frees all the memory associated with a 2D array of pointers
void free2D(void**** at, int xSize, int zSize) {
    for (int i = 0; i < xSize; i++) {
        for (int j = 0; j < zSize; j++) {
            free((*at)[i][j]);
        }
    }

    free((*at)[0]);
    free(*at);
}

// Creates a camera and assigns all its values
// Returns a pointer to the camera
Camera* createCamera(
        GLfloat eyeX,
        GLfloat eyeY,
        GLfloat eyeZ,
        GLfloat upX,
        GLfloat upY,
        GLfloat upZ) {
    Camera* out = malloc(sizeof(Camera));
    out->eyeX = eyeX;
    out->eyeY = eyeY;
    out->eyeZ = eyeZ;
    out->upX = upX;
    out->upY = upY;
    out->upZ = upZ;

    out->zoom = 1.0f;
    out->rotationX = 0.0f;
    out->rotationY = 0.0f;

    out->mode = 1;


    return out;
}

// Creates a mouse and sets all its values to 0
// Returns a pointer to the mouse
Mouse* createMouse(void) {
    Mouse* out = malloc(sizeof(Mouse));
    out->lastMouseY = 0;
    out->lastMouseX = 0;
    out->isDragging = 0;

    return out;
}
