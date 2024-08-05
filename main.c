#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "perlin.h"
#include "structures.h"
#include "terrain.h"

//For text overlay
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"


#define MAX_HEIGHT 30

static int win_width = 800;
static int win_height = 600;

static int height_mode = 0;
static int size = 250;
static int colour_mode = 0;

static void display(GLFWwindow* window);
static void setupOpenGL(void);
static void drawTerrain(void);
void drawText(float x, float y, const char *text);
static void morph(GLFWwindow* window);

//All the callback functions for controls:
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

bool checkModeArgs(int argc, int argNumber, char **argv);

heightFunction height_function;
GLfloat simple_perlin(GLfloat x, GLfloat z);
GLfloat double_perlin(GLfloat x, GLfloat z);
GLfloat simple_perlin_blocky(GLfloat x, GLfloat z);
GLfloat double_perlin_blocky(GLfloat x, GLfloat z);
GLfloat mountain_perlin(GLfloat x, GLfloat z);

void populateColourMap(void);
colourFunction colour_function;
Vector3 classic_colour(GLfloat x, GLfloat y, GLfloat z);
Vector3 heightmap_colour(GLfloat x, GLfloat y, GLfloat z);
Vector3 grey_colour(GLfloat x, GLfloat y, GLfloat z);
Vector3 biomes_colour(GLfloat x, GLfloat y, GLfloat z);

Camera* camera;
Terrain* terrain;
Mouse* mouse;
Perlin* perlin;
GLfloat** colour_map;

int main(int argc, char** argv) {
    if (argc > 4) {
        fprintf(stderr, "Proper usage: ./main -m=[Height Mode] -s=[Size] -c=[Colour Mode]\n");
        return EXIT_FAILURE;
    }
    if (checkModeArgs(argc, 1, argv)) {
        fprintf(stderr, "First argument must be either '-m=[Height Mode]' or '-s=[Size]' or '-c=[Colour Mode]'.\n");
        return EXIT_FAILURE;
    }
    if (checkModeArgs(argc, 2, argv)) {
        fprintf(stderr, "Second argument must be either '-m=[Height Mode]' or '-s=[Size]' or '-c=[Colour Mode]'.\n");
        return EXIT_FAILURE;
    }
    if (checkModeArgs(argc, 3, argv)) {
        fprintf(stderr, "Third argument must be either '-m=[Height Mode]' or '-s=[Size]' or '-c=[Colour Mode]'.\n");
        return EXIT_FAILURE;
    }
    if (height_mode < 0 || height_mode > 4) {
        fprintf(stderr, "Mode must be either of the following:\n"
                             "\t0.\tSimple Perlin Noise\n"
                             "\t1.\tOverlayed Perlin Noise\n"
                             "\t2.\tMode 0 but Blocky Terrain\n"
                             "\t3.\tMode 1 but Blocky Terrain\n"
                             "\t4.\tMountain Perlin\n");
        return EXIT_FAILURE;
    }
    if (colour_mode < 0 || colour_mode > 4) {
        fprintf(stderr, "Colour mode must be either of the following:\n"
                        "\t0.\tClassic green colour with snowy peak mountains, with water\n"
                        "\t1.\tHeightmap with red = high, blue = low\n"
                        "\t2.\tAll a grey colour\n"
                        "\t3.\tVolcano biomes (best with mountain mode), with water\n");
        return EXIT_FAILURE;
    }
    // Select height function.
    switch (height_mode) {
        case 0:
            height_function = &simple_perlin;
            break;
        case 1:
            height_function = &double_perlin;
            break;
        case 2:
            height_function = &simple_perlin_blocky;
            break;
        case 3:
            height_function = &double_perlin_blocky;
            break;
        case 4:
            height_function = &mountain_perlin;
            break;
        default:
            fprintf(stderr, "Impossible height mode.\n");
            return EXIT_FAILURE;
    }

    // Select colour function.
    switch (colour_mode) {
        case 0:
            colour_function = &classic_colour;
            break;
        case 1:
            colour_function = &heightmap_colour;
            break;
        case 2:
            colour_function = &grey_colour;
            break;
        case 3:
            colour_function = &biomes_colour;
            break;
        default:
            fprintf(stderr, "Impossible colour mode.\n");
            return EXIT_FAILURE;
    }

    // Ensure size is > 2 and <= 1000;
    if (size <= 2 || size > 1000) {
        fprintf(stderr, "Size must be > 2 and <= 10000.\n");
    }

    // Initialise glfw
    if (glfwInit() == GLFW_FALSE) {
    	fprintf(stderr, "Failed to initialise GLFW\n");
	    return EXIT_FAILURE;
    }

    // Create window
    GLFWwindow* window = glfwCreateWindow(win_width, win_height, "Terrain Generation", NULL, NULL);
    if (window == NULL) {
    	fprintf(stderr, "Failed to open GLFW window\n");
	    glfwTerminate();
	    return EXIT_FAILURE;
    }

    // Put window into current context
    glfwMakeContextCurrent(window);

    //Initialise callbacks for controls
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);


    //Setup terrain and camera.
    terrain = createTerrain(size, size,MAX_HEIGHT);
    camera = createCamera(terrain->xSize/2, 30.0f,terrain->zSize/2 + 30.f, 0, 1, 0);
    mouse = createMouse();
    perlin = create_perlin(terrain->xSize, terrain->zSize);


    // Setup Open GL.
    setupOpenGL();
    // Choose height function.
    populateTerrain(terrain, height_function);
    if (colour_mode == 3) {
        populateColourMap();
    }

    // Wait until pressed the close button or other action
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // Execute any events e.g. resizes.
        if (terrain->morphing) {
            morph(window);
        }
        else {
            display(window);
        }
    }

    // Terminate glfw and destroy window
    glfwDestroyWindow(window);
    glfwTerminate();

    //Free everything
    free(mouse);
    free(camera);
    if (colour_mode == 3) {
        for (int x = 0; x < terrain->xSize; x++) {
            free(colour_map[x]);
        }
        free(colour_map);
    }
    freeTerrain(terrain);
    free_perlin(perlin);
    return EXIT_SUCCESS;
}

bool checkModeArgs(int argc, int argNumber, char **argv) {
    return (
            argc > argNumber &&
            (sscanf(argv[argNumber], "-m=%d", &height_mode) != 1 &&
            sscanf(argv[argNumber], "-s=%d", &size) != 1 &&
            sscanf(argv[argNumber], "-c=%d", &colour_mode) != 1)
            );
}


void setupOpenGL(void) {
    // Enable features we'll use (Depth/Lighting and Color)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    // Set a place for the light to come from.
    GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // Set background.
    glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)win_width / (double)win_height, 1.0, 1000.0);

}

// old = old colour to interpolate from (r,g,b handled separately)
// new = new colour to interpolate to
// height = the height it should interpolate to, after this height it is new colour
static GLfloat interpolate_colour(GLfloat old_col, GLfloat old_height, GLfloat new_col, GLfloat new_height, GLfloat y) {
    GLfloat adjustedHeight = new_height - old_height; // make sure not negative

    if (fabsf(adjustedHeight) < 1e-6) { // do not want to divide by 0
        adjustedHeight = (adjustedHeight < 0) ? -1e-6f : 1e-6f;
    }

    GLfloat interpolationFactor = (y - old_height) / adjustedHeight;

    // Ensure the interpolation factor is clamped between 0 and 1
    GLfloat clampedFactor = (interpolationFactor < 0.0f) ?
            0.0f : (interpolationFactor > 1.0f ? 1.0f : interpolationFactor);

    GLfloat out = old_col + (new_col - old_col) * fabsf(clampedFactor);

    // Clamp the output to be within the range [0.0, 1.0]
    if (out < 0.0f) return 0.0f;
    if (out > 1.0f) return 1.0f;

    return out;
}

void drawTerrain(void) {
    // Allocate memory for vertices, normals, colors, and indices
    int numVertices = terrain->xSize * terrain->zSize;
    int numTriangles = (terrain->xSize - 1) * (terrain->zSize - 1) * 2;

    //Instead of 2d arrays for vertices, create a 1d array
    Vector3* vertices = (Vector3*)malloc(numVertices * sizeof(Vector3));
    Vector3* normals = (Vector3*)malloc(numVertices * sizeof(Vector3));
    Vector3* colors = (Vector3*)malloc(numVertices * sizeof(Vector3));
    GLuint* indices = (GLuint*)malloc(numTriangles * 3 * sizeof(GLuint));

    // Fill the vertex, normal, and color arrays
    int index = 0;
    for (int z = 0; z < terrain->zSize; ++z) {
        for (int x = 0; x < terrain->xSize; ++x) {
            vertices[index] = (Vector3){x, terrain->heights[x][z], z};
            normals[index] = terrain->normals[x][z];
            colors[index] = colour_function(x, vertices[index].y,z);
            ++index;
        }
    }

    // Fill the index array, used to index triangles
    index = 0;
    for (int z = 0; z < terrain->zSize - 1; ++z) {
        for (int x = 0; x < terrain->xSize - 1; ++x) {
            int topLeft = z * terrain->xSize + x;
            int topRight = topLeft + 1;
            int bottomLeft = topLeft + terrain->xSize;
            int bottomRight = bottomLeft + 1;

            indices[index++] = topLeft;
            indices[index++] = bottomLeft;
            indices[index++] = topRight;

            indices[index++] = topRight;
            indices[index++] = bottomLeft;
            indices[index++] = bottomRight;
        }
    }

    // Enable arrays and set pointers
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Vector3), vertices);
    glNormalPointer(GL_FLOAT, sizeof(Vector3), normals);
    glColorPointer(3, GL_FLOAT, sizeof(Vector3), colors);

    // Draw the terrain
    // This is more efficient than just drawing each of the points individually,
    // As opengl keeps track fo the normals and colours and vertices for you.
    glDrawElements(GL_TRIANGLES, numTriangles * 3, GL_UNSIGNED_INT, indices);

    // Disable arrays
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // Free allocated memory
    free(vertices);
    free(normals);
    free(colors);
    free(indices);

    // Draw water
    if (colour_mode == 0 || colour_mode == 3) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 1.0f, 0.25f);
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0, 0.0f, 0.0f);
        glVertex3f(terrain->xSize, 0.0f, 0.0f);
        glVertex3f(terrain->xSize, 0.0f, terrain->zSize);
        glVertex3f(0.0f, 0.5f, terrain->zSize);
        glEnd();
        glDisable(GL_BLEND);
    }
}

void populateColourMap(void) {
    colour_map = malloc(sizeof(GLfloat*) * terrain->xSize);
    for (int x = 0; x < terrain->xSize; x++) {
        colour_map[x] = malloc(sizeof(GLfloat) * terrain->zSize);
        for (int z = 0; z < terrain->zSize; z++) {
            colour_map[x][z] = double_perlin(x, z);
        }
    }
}

void drawText(float x, float y, const char *text) {
    char buffer[99999];
    int num_quads;

    //Uses the stb_font function to create points which can be drawn to the screen.
    num_quads = stb_easy_font_print(x, y, (char *)text, NULL, buffer, sizeof(buffer));
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void display(GLFWwindow* window) {
    //Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // First-person view
    if (camera->mode == 1) {
        float centerX = camera->eyeX + sin(camera->rotationX * M_PI / 180.0);
        float centerY = camera->eyeY - tan(camera->rotationY * M_PI / 180.0);
        float centerZ = camera->eyeZ - cos(camera->rotationX * M_PI / 180.0);

        gluLookAt(camera->eyeX, camera->eyeY, camera->eyeZ, centerX, centerY, centerZ,
                  camera->upX, camera->upY, camera->upZ);
    }

    // Overhead view
    else if (camera->mode == 2) {
        float centerX = terrain->xSize / 2.0;
        float centerY = 0.0;
        float centerZ = terrain->xSize / 2.0;

        gluLookAt(camera->eyeX, camera->eyeY * camera->zoom, camera->eyeZ * camera->zoom, centerX, centerY, centerZ,
                  camera->upX, camera->upY, camera->upZ);

        //Set rotation based on rotationX and rotationY
        glTranslatef(terrain->xSize/2,0.0f,terrain->zSize/2); //Move to center
        glRotatef(camera->rotationX, 0.0f, 1.0f, 0.0f); //Rotate about x
        glRotatef(camera->rotationY, 1.0f, 0.0f, 0.0f); //Rotate about y
        glTranslatef(-(terrain->xSize/2), 0.0f, -(terrain->zSize/2)); //Move back from center
    }

    if (terrain->spinning) {
        camera->rotationX += 0.1f;
        camera->rotationX += 0.1f;

        glTranslatef(terrain->xSize/2,0.0f,terrain->zSize/2); //Move to center
        glRotatef(camera->rotationX, 0.0f, 1.0f, 0.0f); //Rotate about x
        glRotatef(camera->rotationY, 1.0f, 0.0f, 0.0f); //Rotate about y
        glTranslatef(-(terrain->xSize/2), 0.0f, -(terrain->zSize/2)); //Move back from center
    }

    drawTerrain();

    // Switch to orthographic projection for 2D text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // Adjust orthographic projection to flip y-coordinate
    gluOrtho2D(0, win_width, win_height, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Change colour of text
    glColor3f(0.0f, 0.0f, 0.0f);


    // Render keybinds text, select modes/etc with colour
    // The if statements are there so we can select the colour based on the mode/settings selected
    drawText(10, 20, "WASD: Move");
    drawText(10, 40, "Q/E: Move Down/Up");
    drawText(10, 60, "Mouse: Move/Zoom");
    drawText(10, 80, "Space: Morph Terrain");
    if (camera->mode == 1) glColor3f(1.0f, 0.0f, 0.0f);
    else glColor3f(0.0f, 0.0f, 0.0f);
    drawText(10, 100, "1: First-Person View");

    if (camera->mode == 2) glColor3f(1.0f, 0.0f, 0.0f);
    else glColor3f(0.0f, 0.0f, 0.0f);
    drawText(10, 120, "2: Overhead View");

    if (terrain->morphing) glColor3f(1.0f, 0.0f, 0.0f);
    else glColor3f(0.0f, 0.0f, 0.0f);
    drawText(10, 140, "M: Constant Morphing (Toggle)");

    if (terrain->spinning) glColor3f(1.0f, 0.0f, 0.0f);
    else glColor3f(0.0f, 0.0f, 0.0f);
    drawText(10, 160, "R: Constant Rotating (Toggle)");

    //Draw information
    glColor3f(0.0f, 0.0f, 0.0f);
    drawText((float) win_width-200, 20, "Command Line Arguments:");
    drawText((float) win_width-200, 40, "-c=[0-3]: Changes Colour Mode.");
    drawText((float) win_width-200, 60, "-m=[0-4]: Changes Height Mode.");
    drawText((float) win_width-200, 80, "-s=[SIZE]: Changes size of terrain.");


    // Restore the previous projection and modelview matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glfwSwapBuffers(window);

}

GLfloat simple_perlin(GLfloat x, GLfloat z) {
    Vector2 vect = { .x = x * 0.05 , .y = z * 0.05 };
    return get_perlin_value(perlin, vect, 2);
}

GLfloat double_perlin(GLfloat x, GLfloat z) {
    Vector2 vect1 = { .x = (x * 0.02) + 1, .y = (z * 0.02) + 1};
    GLfloat perlin1 = get_perlin_value(perlin, vect1, 2) * 1.2;

    Vector2 vect2 = { .x = x * 0.05, .y = z * 0.05 };
    GLfloat perlin2 = get_perlin_value(perlin, vect2, 2) * 0.4;

    return (perlin1 + perlin2)/1.6;
}

GLfloat simple_perlin_blocky(GLfloat x, GLfloat z) {
    return floor(simple_perlin(x, z) * MAX_HEIGHT) / (float) MAX_HEIGHT;
}

GLfloat double_perlin_blocky(GLfloat x, GLfloat z) {
    return floor(double_perlin(x, z) * MAX_HEIGHT) / (float) MAX_HEIGHT;
}

GLfloat mountain_perlin(GLfloat x, GLfloat z) {
    Vector2 vect1 = { .x = (x * 0.02) + 1, .y = (z * 0.02) + 1};
    GLfloat perlin1 = get_perlin_value(perlin, vect1, 2) * 1.2;

    Vector2 vect2 = { .x = x * 0.05, .y = z * 0.05 };
    GLfloat perlin2 = get_perlin_value(perlin, vect2, 2) * 0.4;

    Vector2 vect3 = { .x = x * 0.005, .y = z * 0.005 };
    GLfloat perlin3 = get_perlin_value(perlin, vect3, 1) * 10.0;
    if (perlin3 < 0.0f) perlin3 = 0.0f;

    return (perlin1 + perlin2 + perlin3)/1.6;
}

Vector3 classic_colour(GLfloat x, GLfloat y, GLfloat z) {
    GLfloat r; GLfloat g; GLfloat b;
    if (y < 0.0f) {
        // Low altitude: dark green
        r = 0.0f;
        g = 0.2f;
        b = 0.0f;
    } else if (y < (terrain->height / 3)) {
        // Medium altitude: green
        r = interpolate_colour(0.0f, 0.0f, 0.1f, terrain->height / 10, y);
        g = interpolate_colour(0.2f, 0.0f, 0.7f, terrain->height / 10, y);
        b = 0.0f;
    } else {
        // High altitude: white (snow)
        r = interpolate_colour(0.1f, 0.0f, 1.0f, terrain->height / 2, y);
        g = interpolate_colour(0.7f, 0.0f, 1.0f, terrain->height / 2, y);
        b = interpolate_colour(0.0f, 0.0f, 1.0f, terrain->height / 2, y);
    }
    return (Vector3){r, g, b};
}

Vector3 heightmap_colour(GLfloat x, GLfloat y, GLfloat z) {
    GLfloat r; GLfloat g; GLfloat b;
    if (y < 0.0f) {
        r = interpolate_colour(0.0f, -terrain->height / 2, 0.5f, 0.0f, y);
        g = 0.0f;
        b = interpolate_colour(1.0f, -terrain->height / 2,0.5f, 0.0f, y);
    } else {
        // Medium altitude: green
        r = interpolate_colour(0.5f, 0.0f, 1.0f, terrain->height / 2, y);
        g = 0.0f;
        b = interpolate_colour(1.0f, 0.0f, 0.5f, terrain->height / 2, y);
    }
    return (Vector3){r, g, b};
}

Vector3 grey_colour(GLfloat x, GLfloat y, GLfloat z) {
    return (Vector3){0.5f, 0.5f, 0.5f};
}

Vector3 biomes_colour(GLfloat x, GLfloat y, GLfloat z) {
    GLfloat r; GLfloat g; GLfloat b;
    if (y < 0.0f) {
        // Low altitude: dark green
        r = 0.0f;
        g = 0.2f;
        b = 0.0f;
    } else if (y < (terrain->height / 2)) {
        // Medium altitude: green
        r = interpolate_colour(0.0f, 0.0f, 0.1f, terrain->height / 10, y);
        g = interpolate_colour(0.2f, 0.0f, 0.7f, terrain->height / 10, y);
        b = 0.0f;
    } else {
        if (colour_map[(int)x][(int)z] > 0.0f) {
            if (y < terrain->height * 1.5){
                // High altitude in biome: dark grey mountain
                r = interpolate_colour(0.1f, terrain->height / 2, 0.3f, terrain->height, y);
                g = interpolate_colour(0.7f, terrain->height / 2, 0.3f, terrain->height, y);
                b = interpolate_colour(0.0f, terrain->height / 2, 0.3f, terrain->height, y);
            } else {
                // V High altitude in biome - lava (volcano)
                r = interpolate_colour(0.3f, terrain->height *0.7, 1.0f, terrain->height * 2, y);
                g = interpolate_colour(0.3f, terrain->height *0.7, 0.0f, terrain->height * 2, y);
                b = interpolate_colour(0.3f, terrain->height *0.7, 0.0f, terrain->height * 2, y);
            }
        } else {
            // High altitude not in biome: grey mountain
            r = interpolate_colour(0.1f, terrain->height / 2, 0.5f, terrain->height, y);
            g = interpolate_colour(0.7f, terrain->height / 2, 0.5f, terrain->height, y);
            b = interpolate_colour(0.0f, terrain->height / 2, 0.5f, terrain->height, y);
        }
    }
    return (Vector3){r, g, b};
}

void morph(GLFWwindow* window) {
    //Create a copy of the terrain we're morphing from.
    Terrain* oldTerrain = createTerrain(terrain->xSize, terrain->zSize, terrain->height);
    populateTerrain(oldTerrain, height_function);

    //Create a new perlin, and new terrain to morph to.
    free_perlin(perlin);
    perlin = create_perlin(terrain->xSize, terrain->zSize);

    Terrain* newTerrain = createTerrain(terrain->xSize, terrain->zSize, terrain->height);
    populateTerrain(newTerrain, height_function);

    //Morph by changing size from one terrain to another based heights of the old terrain and new terrain.
    int num_steps = 30;
    for (int step = 0; !glfwWindowShouldClose(window) && step < num_steps; step++) {
        //Change current terrain normals based on step
        for (int x = 0; x < terrain->xSize; x++) {
            for (int z = 0; z < terrain->zSize; z++) {
                terrain->heights[x][z] = (oldTerrain->heights[x][z] * (num_steps - step) + newTerrain->heights[x][z] * step)/num_steps ;
            }
        }

        for (int x = 0; x < terrain->xSize - 1; x++) {
            for (int z = 0; z < terrain->zSize - 1; z++) {
                terrain->normals[x][z].x =
                        (oldTerrain->normals[x][z].x * (num_steps - step) + newTerrain->normals[x][z].x * step) /
                        num_steps;
                terrain->normals[x][z].y =
                        (oldTerrain->normals[x][z].y * (num_steps - step) + newTerrain->normals[x][z].y * step) /
                        num_steps;
                terrain->normals[x][z].z =
                        (oldTerrain->normals[x][z].x * (num_steps - step) + newTerrain->normals[x][z].z * step) /
                        num_steps;
            }
        }

        glfwPollEvents(); // Execute any events e.g. resizes.
        display(window);
    }

    //Keep settings between the terrains, and free the previous terrains.
    newTerrain->morphing = terrain->morphing;
    newTerrain->spinning = terrain->spinning;
    freeTerrain(terrain);
    terrain = newTerrain;
    freeTerrain(oldTerrain);
}

//If the screen size changes, we need to change the gluPerspective to match this.
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);

    win_height = height;
    win_width = width;
}

//Callback functions for controls
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouse->isDragging = 1;
            glfwGetCursorPos(window, &mouse->lastMouseX, &mouse->lastMouseY);
        } else if (action == GLFW_RELEASE) {
            mouse->isDragging = 0;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (mouse->isDragging) {
        double dx = xpos - mouse->lastMouseX;
        double dy = ypos - mouse->lastMouseY;
        camera->rotationX += dx * 0.5f;
        camera->rotationY += dy * 0.5f;

        //Restrict view from -90 to 90 degrees when in mode 1
        if (camera->rotationY > 90.0f && camera->mode == 1) {
            camera->rotationY = 90.0f;
        } else if (camera->rotationY < -90.0f && camera->mode == 1) {
            camera->rotationY = -90.0f;
        }

        mouse->lastMouseX = xpos;
        mouse->lastMouseY = ypos;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    //Zoom is only used in mode 2
    if (camera->mode == 2) {
        camera->zoom -= yoffset * 0.1f;

        //Setting upper and lower bounds for zoom
        if (camera->zoom < 0.1f) {
            camera->zoom = 0.1f;
        }
        if (camera->zoom > 10.0f) {
            camera->zoom = 10.0f;
        }
        printf("New zoom: %f\n", camera->zoom);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    const float moveSpeed = 5.0f;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_W && camera->mode == 1) {
            // Move forward (into the screen)
            float forwardX = sin(camera->rotationX * M_PI / 180.0) * cos(camera->rotationY * M_PI / 180.0);
            float forwardY = -sin(camera->rotationY * M_PI / 180.0);
            float forwardZ = -cos(camera->rotationX * M_PI / 180.0) * cos(camera->rotationY * M_PI / 180.0);
            camera->eyeX += moveSpeed * forwardX;
            camera->eyeY += moveSpeed * forwardY;
            camera->eyeZ += moveSpeed * forwardZ;
        } else if (key == GLFW_KEY_S && camera->mode == 1) {
            // Move backward (away from the screen)
            float backwardX = -sin(camera->rotationX * M_PI / 180.0) * cos(camera->rotationY * M_PI / 180.0);
            float backwardY = sin(camera->rotationY * M_PI / 180.0);
            float backwardZ = cos(camera->rotationX * M_PI / 180.0) * cos(camera->rotationY * M_PI / 180.0);
            camera->eyeX += moveSpeed * backwardX;
            camera->eyeY += moveSpeed * backwardY;
            camera->eyeZ += moveSpeed * backwardZ;
        } else if (key == GLFW_KEY_E && camera->mode == 1) {
            //Strafe up (relative to camera)
            float upX = sin(camera->rotationX * M_PI / 180.0) * sin(camera->rotationY * M_PI / 180.0);
            float upY = cos(camera->rotationY * M_PI / 180);
            float upZ = -cos(camera->rotationX * M_PI / 180.0) * sin(camera->rotationY * M_PI / 180.0);
            camera->eyeX += moveSpeed * upX;
            camera->eyeY += moveSpeed * upY;
            camera->eyeZ += moveSpeed * upZ;
        } else if (key == GLFW_KEY_Q && camera->mode == 1) {
            //Strafe down (relative to camera)
            float downX = -sin(camera->rotationX * M_PI / 180.0) * sin(camera->rotationY * M_PI / 180.0);
            float downY = -cos(camera->rotationY * M_PI / 180);
            float downZ = cos(camera->rotationX * M_PI / 180.0) * sin(camera->rotationY * M_PI / 180.0);
            camera->eyeX += moveSpeed * downX;
            camera->eyeY += moveSpeed * downY;
            camera->eyeZ += moveSpeed * downZ;
        } else if (key == GLFW_KEY_A && camera->mode == 1) {
            //Strafe left (relative to camera)
            camera->eyeX -= moveSpeed * cos(camera->rotationX * M_PI / 180.0);
            camera->eyeZ -= moveSpeed * sin(camera->rotationX * M_PI / 180.0);
        } else if (key == GLFW_KEY_D && camera->mode == 1) {
            //Strafe right (relative to camera)
            camera->eyeX += moveSpeed * cos(camera->rotationX * M_PI / 180.0);
            camera->eyeZ += moveSpeed * sin(camera->rotationX * M_PI / 180.0);
        } else if (key == GLFW_KEY_1) {
            //Reset camera position and rotation
            camera->mode = 1;

            camera->eyeX = terrain->xSize/2;
            camera->eyeY = 30.0f;
            camera->eyeZ = terrain->zSize/2 + 30.f;
            camera->zoom = 1.0f;

            camera->rotationX = 0.0f;
            camera->rotationY = 0.0f;
        } else if (key == GLFW_KEY_2) {
            camera->mode = 2;

            camera->eyeX = terrain->xSize / 2;
            camera->eyeY = 150.0f;
            camera->eyeZ = (terrain->zSize / 2.0 + 150.0);
            camera->zoom = 1.0f;

            camera->rotationX = 0.0f;
            camera->rotationY = 0.0f;
        } else if (key == GLFW_KEY_M) {
            terrain->morphing = !terrain->morphing;
        } else if (key == GLFW_KEY_R) {
            terrain->spinning = !terrain->spinning;
        } else if (key == GLFW_KEY_SPACE) {
            morph(window);
        }
    }
    printf("X: %f, Y: %f, Z: %f\n",camera->eyeX,camera->eyeY,camera->eyeZ);
}

