# First-Year C-Project Extension

## Jonathan Cunnane, Joe Reynolds, Richard Lewins, William Butler

### June 21, 2024

---

## Overview

This document provides an overview of the extension to our ARM project. The extension involves creating a terrain generation program using Perlin noise and rendering the terrain using OpenGL and GLFW. This README will cover the following sections:
1. Introduction to the extension and its purpose
2. High-level design and challenges faced
3. Testing and validation of the extension
4. Usage instructions

---

## 1. Introduction

### Terrain Generation with Perlin Noise

Our extension focuses on generating and rendering 3D terrain using the Perlin noise algorithm. This algorithm, developed by Ken Perlin in 1983, is renowned for producing natural-looking, smooth terrain. Our goal was to create an interactive program to explore the applications of Perlin noise in terrain generation, useful in game design and educational contexts.

---

## 2. Design and Implementation

### High-Level Design

The extension is divided into two primary components:
1. **Perlin Noise Algorithm Implementation**: Generates a 2D height map representing the terrain.
2. **Rendering and Interaction**: Uses OpenGL and GLFW to render the terrain and handle user inputs.

### Perlin Noise Algorithm

The algorithm follows these steps:
1. **Gradient Vector Grid**: A 2D grid of random normalized gradient vectors is generated.
2. **Dot Product Calculation**: For any point, the dot product of the gradient vectors at the grid cell corners and the offset vectors from the point to these corners is computed.
3. **Interpolation**: Interpolates these dot products to produce smooth transitions across the terrain.

### Rendering and Input Processing

Rendering and input are managed using OpenGL and GLFW:
- **Initialization**: Set up OpenGL features such as lighting and depth testing.
- **Input Handling**: Configure keyboard and mouse inputs for interaction.
- **Rendering Loop**: Continuously render the terrain and update based on user inputs.

### Input Mappings

- `1` and `2`: Switch between first-person and overhead views.
- `WASD` and `EQ`: Move the camera in first-person view.
- `Scroll-Wheel`: Zoom in and out in overhead view.
- `Left-click and Drag`: Rotate the camera/terrain.
- `Space-bar`: Morph the terrain using different Perlin noise configurations.
- `M` and `R`: Toggle morphing and rotating, respectively.

### Terrain Color and Lighting

Different color functions provide various terrain appearances. For example, one function mimics volcanic terrain by layering Perlin noise to represent lava regions. Lighting is rendered using vertex normals for smooth shading.

---

## 3. Testing and Validation

Testing involved a combination of visual inspection and unit tests:
- **Visual Testing**: Adjusting terrain parameters and observing the rendered output.
- **Unit Tests**: Ensuring correctness of vector operations, Perlin noise algorithm, and data structure initialization.

### Key Testing Points

- Verified Perlin noise values are within expected ranges.
- Ensured gradient vectors have a magnitude of 1.
- Confirmed proper initialization and functionality of data structures and algorithms.

---

## 4. Usage Instructions

### Prerequisites

Before running the program, ensure you have the following installed:

1. **C Compiler**: GCC or Clang
2. **OpenGL Libraries**: Install the necessary OpenGL development libraries
3. **GLFW**: A library for creating windows and handling inputs

On Debian-based systems, you can install the required libraries using:
```sh
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install libgl1-mesa-dev
sudo apt-get install libglfw3-dev
```

### Building the program
To build the program, follow these steps:
1. **Navigate the the program directory**
2. **Run make** to compile the program
```sh
make all
```

### Running the program
After building the program, you can run it using the following command:
```sh
./main -m=[Height mode] -s=[Size] -c=[Colour mode]
```
#### Optional Command-Line arguments
- **`-m=[Height mode]`**: Specifies the mode of terrain height generation. Available Modes: 0-4
- **`-c=[Colour mode]`**: Specifies the colour mode of the terrain. Available Modes: 0-3
- **`-s=[Size]`**: Specifies the size of the terrain.  Example: **`-s=500`**

