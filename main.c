#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <errno.h>
#include "perlin.h"
#include "structures.h"
#include "terrain.h"

//For text overlay
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"


#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define MAX_HEIGHT 30

static int height_mode = 0;
static int size = 100;

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

heightFunction height_function;
GLfloat simple_perlin(GLfloat x, GLfloat z);
GLfloat double_perlin(GLfloat x, GLfloat z);
GLfloat simple_perlin_blocky(GLfloat x, GLfloat z);
GLfloat double_perlin_blocky(GLfloat x, GLfloat z);



Camera* camera;
Terrain* terrain;
Mouse* mouse;
Perlin* perlin;

int main(int argc, char** argv) {
    if (argc > 3) {
        fprintf(stderr, "Proper usage: ./main -m=[Height Mode] -s=[Size]\n");
        return EXIT_FAILURE;
    }
    if (argc > 1 && (sscanf(argv[1], "-m=%d", &height_mode) != 1 && sscanf(argv[1], "-s=%d", &size) != 1)) {
        fprintf(stderr, "First argument must be either '-m=[Height Mode]' or '-s=[Size]'.\n");
        return EXIT_FAILURE;
    }
    if (argc > 2 && (sscanf(argv[2], "-m=%d", &height_mode) != 1 && sscanf(argv[2], "-s=%d", &size) != 1)) {
        fprintf(stderr, "Second argument must be either '-m=[Height Mode]' or '-s=[Size]'.\n");
        return EXIT_FAILURE;
    }
    if (height_mode < 0 || height_mode > 3) {
        fprintf(stderr, "Mode must be either of the following:\n"
                             "\t0.\tSimple Perlin Noise\n"
                             "\t1.\tOverlayed Perlin Noise\n"
                             "\t2.\tMode 0 but Blocky Terrain\n"
                             "\t3.\tMode 1 but Blocky Terrain\n");
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
        default:
            fprintf(stderr, "Impossible mode.\n");
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
    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Terrain Generation", NULL, NULL);
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

    // Wait until pressed the close button or other action
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // Execute any events e.g. resizes.
	    display(window);
    }

    // Terminate glfw and destroy window
    glfwDestroyWindow(window);
    glfwTerminate();

    //Free everything
    free(mouse);
    free(camera);
    freeTerrain(terrain);
    free_perlin(perlin);
    return EXIT_SUCCESS;
}

void setupOpenGL(void) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)WIN_WIDTH / (double)WIN_HEIGHT, 1.0, 1000.0);

}

void drawTerrain(void) {
    for (int z = 0; z < terrain->zSize - 2; ++z) {

        for (int x = 0; x < terrain->xSize - 2; ++x) {
            glBegin(GL_TRIANGLE_STRIP);
            glNormal3f(terrain->normals[x][z].x, terrain->normals[x][z].y, terrain->normals[x][z].z);
            glColor3f(0.0f, terrain->heights[x][z] / MAX_HEIGHT, 1.0f - terrain->heights[x][z] / MAX_HEIGHT);
            glVertex3f(x, terrain->heights[x][z], z);

            glNormal3f(terrain->normals[x+1][z].x, terrain->normals[x+1][z].y, terrain->normals[x+1][z].z);
            glColor3f(0.0f, terrain->heights[x + 1][z] / MAX_HEIGHT, 1.0f - terrain->heights[x + 1][z] / MAX_HEIGHT);
            glVertex3f(x + 1, terrain->heights[x + 1][z], z);

            glNormal3f(terrain->normals[x][z+1].x, terrain->normals[x][z+1].y, terrain->normals[x][z+1].z);
            glColor3f(0.0f, terrain->heights[x][z + 1] / MAX_HEIGHT, 1.0f - terrain->heights[x][z + 1] / MAX_HEIGHT);
            glVertex3f(x, terrain->heights[x][z + 1], z + 1);

            glNormal3f(terrain->normals[x+1][z+1].x, terrain->normals[x+1][z+1].y, terrain->normals[x+1][z+1].z);
            glColor3f(0.0f, terrain->heights[x + 1][z + 1] / MAX_HEIGHT, 1.0f - terrain->heights[x + 1][z + 1] / MAX_HEIGHT);
            glVertex3f(x + 1, terrain->heights[x + 1][z + 1], z + 1);
            glEnd();
        }

    }
}

void drawText(float x, float y, const char *text) {
    char buffer[99999];
    int num_quads;

    num_quads = stb_easy_font_print(x, y, (char *)text, NULL, buffer, sizeof(buffer));
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}


void display(GLFWwindow* window) {
    //Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (camera->mode == 1) {
        float centerX = camera->eyeX + sin(camera->rotationX * M_PI / 180.0);
        float centerY = camera->eyeY - tan(camera->rotationY * M_PI / 180.0);
        float centerZ = camera->eyeZ - cos(camera->rotationX * M_PI / 180.0);

        gluLookAt(camera->eyeX, camera->eyeY, camera->eyeZ, centerX, centerY, centerZ,
                  camera->upX, camera->upY, camera->upZ);
    }


    if (camera->mode == 2) {
        float centerX = terrain->xSize / 2.0;
        float centerY = 0.0;
        float centerZ = terrain->xSize / 2.0;

        gluLookAt(camera->eyeX, camera->eyeY * camera->zoom, camera->eyeZ * camera->zoom, centerX, centerY, centerZ,
                  camera->upX, camera->upY, camera->upZ);

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
    gluOrtho2D(0, WIN_WIDTH, WIN_HEIGHT, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Change colour of text
    glColor3f(0.0f, 0.0f, 0.0f);


    // Render keybinds text
    drawText(10, 20, "WASD: Move");
    drawText(10, 40, "Q/E: Move Down/Up");
    drawText(10, 60, "1: First-Person View");
    drawText(10, 80, "2: Overhead View");
    drawText(10, 100, "Space: Morph Terrain");

    // Restore the previous projection and modelview matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glfwSwapBuffers(window);

}

GLfloat simple_perlin(GLfloat x, GLfloat z) {
    Vector2 vect = { .x = x * 0.05 * (100.0 / (float) size), .y = z * 0.05 * (100.0 / (float) size)};
    return get_perlin_value(perlin, vect, 2) * (float) size / 100.0;
}

GLfloat double_perlin(GLfloat x, GLfloat z) {
    Vector2 vect1 = { .x = (x * 0.02) + 1, .y = (z * 0.02) + 1};
    GLfloat perlin1 = get_perlin_value(perlin, vect1, 2) * 1.2;

    Vector2 vect2 = { .x = x * 0.05, .y = z * 0.05 };
    GLfloat perlin2 = get_perlin_value(perlin, vect2, 2) * 0.4;

    return perlin1 + perlin2;
}

GLfloat simple_perlin_blocky(GLfloat x, GLfloat z) {
    return floor(simple_perlin(x, z) * MAX_HEIGHT) / (float) MAX_HEIGHT;
}

GLfloat double_perlin_blocky(GLfloat x, GLfloat z) {
    return floor(double_perlin(x, z) * MAX_HEIGHT) / (float) MAX_HEIGHT;
}

void morph(GLFWwindow* window) {
    Terrain* oldTerrain = createTerrain(terrain->xSize, terrain->zSize, terrain->height);
    populateTerrain(oldTerrain, height_function);

    free_perlin(perlin);
    perlin = create_perlin(terrain->xSize, terrain->zSize);

    Terrain* newTerrain = createTerrain(terrain->xSize, terrain->zSize, terrain->height);
    populateTerrain(newTerrain, height_function);

    int num_steps = 30;
    for (int step = 0; !glfwWindowShouldClose(window) && step < num_steps; step++) {
        //Change current terrain normals based on step
        for (int x = 0; x < terrain->xSize - 1; x++) {
            for (int z = 0; z < terrain->zSize - 1; z++) {
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

    freeTerrain(terrain);
    terrain = newTerrain;
    freeTerrain(oldTerrain);

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

            camera->eyeX = terrain->xSize/2;
            camera->eyeY = 150.0f;
            camera->eyeZ = (terrain->zSize / 2.0 + 150.0);
            camera->zoom = 1.0f;

            camera->rotationX = 0.0f;
            camera->rotationY = 0.0f;
        } else if (key == GLFW_KEY_SPACE) {
            morph(window);
        }
    }
    printf("X: %f, Y: %f, Z: %f\n",camera->eyeX,camera->eyeY,camera->eyeZ);
}

