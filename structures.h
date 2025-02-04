#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

// A heightFunction takes an x and z, and returns the height.
typedef GLfloat (*heightFunction) (GLfloat, GLfloat);

// Holds information about a 3D point.
typedef struct {
    GLfloat x, y, z;
} Vector3;

// A colour takes an x and y and z, and returns the colour.
typedef Vector3 (*colourFunction) (GLfloat, GLfloat, GLfloat);

// Holds information about a 2D point.
typedef struct {
    GLfloat x, y;
} Vector2;

// Returns the dot product of 2 2D vectors.
extern GLfloat dot_product_2(Vector2, Vector2);
// Returns the dot product of 2 3D vectors.
extern GLfloat dot_product_3(Vector3, Vector3);

// Return a normal vector perpendicular to the two inputs (NOT NORMALISED).
// The vector will be in the direction of (0, 1, 0) (i.e. y = 1, upwards).
// PRE: Vectors are not parallel.
extern Vector3 cross_product_3(Vector3, Vector3);

// normalises the vector 3
extern Vector3 normalise_3(Vector3);

// Allocates a 2D array of pointers initialised to NULL. Takes a pointer to the array.
extern int allocate2D(void****, int, int);

// Frees a 2D array generated by allocate2D. Takes a pointer to the array.
extern void free2D(void****, int, int);

// Stores the information for a camera.
typedef struct {
    GLfloat eyeX, eyeY, eyeZ;
    GLfloat upX, upY, upZ;

    float zoom;
    float rotationX, rotationY;

    int mode;
} Camera;

// Stores the information for a mouse.
typedef struct {
    double lastMouseX, lastMouseY;
    int isDragging;
} Mouse;

// Creates a camera with the input values passed to it.
// Returns a pointer to the camera.
Camera* createCamera(GLfloat eyeX,
                     GLfloat eyeY,
                     GLfloat eyeZ,
                     GLfloat upX,
                     GLfloat upY,
                     GLfloat upZ);

// Creates a mouse with all values 0.
// Returns a pointer to the mouse.
Mouse* createMouse(void);

#endif
