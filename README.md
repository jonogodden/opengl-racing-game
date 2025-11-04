# OpenGL Racing Game

A simple 3D racing game prototype built with **C++**, **OpenGL**, **GLAD**, and **GLFW**.

---

## Overview

This project features a basic 3D "car" (currently represented as a cuboid) that can drive around a generated bumpy terrain.

Key features:

- **Car movement**:
  - Use the **arrow keys** to drive the car.
  - The car moves up and down hills and valleys, staying aligned with the terrain.
  - Movement is currently at a constant speed.
- **Camera control**:
  - Rotate the camera around the car using the **mouse**.
  - Zoom in and out using the **W, A, S, D** keys.
- **Terrain**:
  - Simple generated bumpy terrain to navigate.

---

## Dependencies

The project uses the following libraries (all included in the `include/` folder):

- [GLAD](https://glad.dav1d.de/) – OpenGL function loader  
- [GLFW](https://www.glfw.org/) – Windowing and input  
- [GLM](https://glm.g-truc.net/) – Mathematics library for OpenGL  

---

## Building

This project uses **CMake** to manage builds.

1. Create a build directory:
```bash
   mkdir build
   cd build
```

2. Generate build files with CMake:
```bash
   cmake ..
```

3. Build the project:
```bash
   cmake --build .
```

The executable will be placed in the build folder (e.g., `build/Debug` or `build/Release`).

**Note:** Ensure that `glfw3.dll` is in the same folder as the executable or in your system PATH.

---

## Controls

- **Arrow keys**: Move the car forward, backward, left, and right.
- **Mouse**: Rotate the camera around the car.
- **W / A / S / D**: Zoom the camera in/out and move slightly around.

---

## Project Structure
```
opengl-racing-game/
│
├─ include/         # Header files for GLAD, GLFW, GLM, KHR
├─ lib/             # GLFW static library (libglfw3dll.a)
├─ src/             # Source files (main.cpp, glad.c)
├─ glfw3.dll        # GLFW dynamic library
├─ CMakeLists.txt   # CMake build configuration
└─ README.md
```

---

## Future Work

- Replace the cuboid with a proper 3D car model.
- Implement physics-based acceleration and movement.
- Improve terrain generation for more realistic hills and valleys.
- Add obstacles, tracks, and gameplay elements.

---

## License

This project is open-source and available under the MIT License.