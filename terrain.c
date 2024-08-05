#include "terrain.h"
#include "structures.h"
#include "perlin.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    Vector3 n1;
    Vector3 n2;
} vertexNormals;

// Creates a terrain with empty heights and normals
Terrain* createTerrain(int xSize, int zSize, int height) {
    // Allocates memory for terrain
    Terrain* terrain = malloc(sizeof(Terrain));
    // Assigns values
    terrain->xSize = xSize;
    terrain->zSize = zSize;
    terrain->height = height;

    terrain->spinning = false;
    terrain->morphing = false;

    //Allocates memory for normals
    terrain->normals = malloc(sizeof(Vector3*) * xSize);
    assert(terrain->normals != NULL);
    for (int x = 0; x < xSize; x++) {
        terrain->normals[x] = malloc(sizeof(Vector3) * zSize);
        assert(terrain->normals[x] != NULL);
    }

    // Allocates memory for heights
    terrain->heights = malloc(sizeof(float*) * xSize);
    assert(terrain->heights != NULL);
    for (int x = 0; x < xSize; x++) {
        terrain->heights[x] = malloc(sizeof(float) * zSize);
        assert(terrain->heights[x] != NULL);
    }

    return terrain;
}

static Vector3 *getFaceNormalInBounds(Terrain* terrain, vertexNormals*** faceNormals, int x, int z, int firstOrSecond) {
    if (x >= 0 && z >= 0 && x < terrain->xSize - 1 && z < terrain->zSize - 1) {
        return (firstOrSecond == 0) ? (&(faceNormals[x][z]->n1)) : (&(faceNormals[x][z]->n2));
    }
    return NULL;
}

// Calculates the heights and normals
void populateTerrain(Terrain* terrain, heightFunction hf) {
    for (int x = 0; x < terrain->xSize; x++) {
        for (int z = 0; z < terrain->zSize; z++) {
            terrain->heights[x][z] = hf(x, z) * terrain->height;
        }
    }

    vertexNormals ***faceNormals;
    allocate2D((void****)&faceNormals, terrain->xSize-1, terrain->zSize-1);
    // generate face normals
    for (int z = 0; z < terrain->zSize - 1; z++) {
        for (int x = 0; x < terrain->zSize - 1; x++) {
            // Get the four vertices of the quad
            Vector3 v0 = {x, terrain->heights[x][z], z};
            Vector3 v1 = {x + 1, terrain->heights[x + 1][z], z};
            Vector3 v2 = {x, terrain->heights[x][z + 1], z + 1};
            Vector3 v3 = {x + 1, terrain->heights[x + 1][z + 1], z + 1};

            // Calculate the normal for the first triangle (v0, v1, v2)
            Vector3 edge1 = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
            Vector3 edge2 = {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
            Vector3 normal1 = cross_product_3(edge1, edge2);
            normal1 = normalise_3(normal1);

            vertexNormals *vn = malloc(sizeof(vertexNormals));
            // Store the normal
            vn->n1 = normal1;

            // Calculate the normal for the second triangle (v1, v3, v2)
            edge1 = (Vector3){v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};
            edge2 = (Vector3){v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
            Vector3 normal2 = cross_product_3(edge1, edge2);
            normal2 = normalise_3(normal2);

            // Store the normal
            vn->n2 = normal2;

            faceNormals[x][z] = vn;
        }
    }
    // now average out nearby face normals for each vertex
    for (int z = 0; z < terrain->xSize; z++) {
        for (int x = 0; x < terrain->xSize; x++) {
            Vector3* normals[6];

            // up to 6 associated triangles per vertex
            normals[0] = getFaceNormalInBounds(terrain, faceNormals, x-1, z-1, 1); // top left adjacent
            normals[1] = getFaceNormalInBounds(terrain, faceNormals, x, z-1, 0); // top right adj 1
            normals[2] = getFaceNormalInBounds(terrain, faceNormals, x, z-1, 1); // top right adj 2
            normals[3] = getFaceNormalInBounds(terrain, faceNormals, x-1, z, 0); // bottom left adj 1
            normals[4] = getFaceNormalInBounds(terrain, faceNormals, x-1, z, 1); // bottom left adj 2
            normals[5] = getFaceNormalInBounds(terrain, faceNormals, x, z, 0); // bottom right adj

            Vector3 normal = {0.0f, 0.0f, 0.0f};
            int faces = 0; // how many successful faces around
            for (int i = 0; i < 6; i++) {
                if (normals[i] != NULL) {
                    normal.x += normals[i]->x;
                    normal.y += normals[i]->y;
                    normal.z += normals[i]->z;
                    faces++;
                }
            }
            terrain->normals[x][z] = normalise_3(normal);
        }
    }
    free2D((void****)&faceNormals, terrain->xSize - 1, terrain->zSize - 1);
}

// Frees the heights, then the normals, and finally the terrain itself
void freeTerrain(Terrain* terrain) {
    for (int x = 0; x < terrain->xSize; x++) {
        free(terrain->heights[x]);
    }
    free(terrain->heights);

    for (int x = 0; x < terrain->xSize; x++) {
        free(terrain->normals[x]);
    }
    free(terrain->normals);

    free(terrain);
}

