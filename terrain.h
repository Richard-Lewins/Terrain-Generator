#ifndef TERRAIN_H
#define TERRAIN_H

#include <math.h>
#include "structures.h"

// A terrain stores the heights of all the points in the grid, along with their normal
// vectors to indicate which direction every face faces (for lighting)
typedef struct {
    int xSize, zSize, height;
    GLfloat** heights; // 2D Array of GLfloat
    Vector3** normals; // 2D Array of Vector3
} Terrain;
// For the normals array above, normals[0] refers to the triangle between
// heights[0][0], heights[1][0], and heights[0][1]. normals[1] then refers
// to the triangle between heights[1][0], heights[0][1], heights[1][1]. And
// so on and so forth.

// Creates a terrain with empty heights and normals
extern Terrain* createTerrain(int xSize, int zSize, int height);
// Calculates the heights and normals
extern void populateTerrain(Terrain*, heightFunction);

// Frees the memory associated with the terrain, its heights and normals
extern void freeTerrain(Terrain*);

#endif
