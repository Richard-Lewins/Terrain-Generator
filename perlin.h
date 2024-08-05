#ifndef PERLIN_H
#define PERLIN_H

#include "structures.h"

// Holds the grid of 2D gradient vectors used to generate perlin noise
typedef struct {
    int xSize, zSize;
    Vector2*** vectorGrid;
} Perlin;

// Creates a perlin and its grid of random 2D gradient vectors
// All gradient vectors are normalised to magnitude 1
extern Perlin* create_perlin(int, int);

// Frees the memory associated with perlin and its grid of vectors
extern void free_perlin(Perlin* perlin);

// Compute Perlin noise at certain coordinates
extern GLfloat get_perlin_value(Perlin*, Vector2, int);

#endif
