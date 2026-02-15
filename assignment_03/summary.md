# Project Defense & Study Guide: Assignment 03

**Project Name**: 3D Bus Simulation with Advanced Phong Lighting
**Core Concept**: A 3D graphics application using OpenGL to simulate a bus with realistic lighting physics (Sunlight, Interior Lights, Headlights) and multiple camera perspectives.

---

## 1. Comprehensive Summary (The "Elevator Pitch")
"This project is a 3D simulation of a bus driving on a road, built using C++ and OpenGL. The core focus is on **Lighting Physics**. I implemented the **Phong Lighting Model** to simulate how light interacts with materials. The scene features three distinct types of light sources: a **Directional Light** (Sun), **Point Lights** (Interior Cabin Lights), and a **Spotlight** (Bus Headlights). I also implemented a **Multi-Viewport System** that splits the screen into four quadrants, each showing a different camera angle and lighting configuration to demonstrate the individual components of the lighting model."

---

## 2. Key Technical Concepts (What you need to know)

### A. The Lighting Model (Phong)
The shader calculates the color of every pixel using this formula:
$$ \text{Color} = (\text{Ambient} + \text{Diffuse} + \text{Specular}) \times \text{ObjectColor} $$

1.  **Ambient**: "Base brightness". Prevents the scene from being pitch black. (Controlled by Key 5).
2.  **Diffuse**: "Matte reflection". Brightest when the light hits the surface directly (90 degrees). (Controlled by Key 6).
3.  **Specular**: "Shiny reflection". The white highlight you see on the bus roof. Depends on your viewing angle. (Controlled by Key 7).
4.  **Emissive**: "Self-glowing". The light bulbs themselves glow independently of other lights. (Toggled with Key 2).

### B. The Light Sources
1.  **Directional Light (Key 1)**: Represents the **Sun**. It has no position, only a direction. It tracks the bus but rays always come from the same angle.
2.  **Point Lights (Key 2)**: Represent **Interior Bulbs**. They have a position inside the bus and shine in all directions. Light gets weaker with distance (Attenuation).
3.  **Spotlight (Key 3)**: Represents **Headlights**. It has a position (Front Bumper), a direction (Forward), and a "Cutoff" angle (the cone width).

### C. The Bus (Hierarchical Modeling)
The bus isn't a single mesh. It's a **Hierarchy**:
-   **Parent**: The Bus Body (Chassis).
-   **Children**: Wheels, Lights, Seats.
-   **Why?**: When I move the "Bus Body", all the children (wheels, headlights) move with it automatically because their transformations are multiplied by the Body's matrix.

---

## 3. Potential Exam Questions & Answers

**Q: How did you make the Headlights move with the bus?**
**A:** "I updated the Spotlight's position and direction in the main loop every frame. I calculated the position based on the bus's center (`busPos`) plus an offset to the front bumper, and the direction based on the bus's rotation (`busYaw`). This ensures the light cone physically attaches to the vehicle."

**Q: Why does the 'Inside View' look different from the others?**
**A:** "The assignment required specific lighting rules for each viewport. The Inside View (Bottom-Right) was initially 'Directional Only', but I modified it to allow the **Interior Point Lights** and **Headlights** to be visible. This makes the driving simulation realistic—you can see the seats light up when you toggle Key 2."

**Q: What is the difference between Gouraud and Phong shading?**
**A:** "Gouraud calculates lighting at the **Vertices** (corners) and blends the colors. Phong calculates lighting at every **Fragment** (pixel). I used Phong shading (in the Fragment Shader) because it produces much smoother highlights, especially for the spotlight on the road."

**Q: How do you handle the 4 Viewports?**
**A:** "I use `glViewport` to define the screen area (x, y, width, height) and `glScissor` to clip rendering to that area. In the render loop, I iterate 4 times, setting the specific camera and lighting uniforms for each quadrant before drawing the scene."

---

## 4. Code Map (Where is everything?)

-   **`main.cpp`**:
    -   `processInput()`: Handles keys (WASD for bus, 1-7 for lights).
    -   `renderScene()`: The function that actually draws the Cube/Sphere meshes.
    -   **Main Loop**: Calculating physics (bus movement), updating Light Uniforms (sending positions to shader), and looping through the 4 viewports.
-   **`shaders.glsl`**:
    -   **Vertex Shader**: Transforms 3D points to 2D screen space.
    -   **Fragment Shader**: The "Brain" of the lighting. It calculates the `Ambient + Diffuse + Specular` math for every pixel.

---

## 5. Controls Cheat Sheet
-   **Driving**: Arrows / WASD.
-   **Light Sources**:
    -   `1`: Sun (Directional).
    -   `2`: Interior Lights (Point).
    -   `3`: Headlights (Spot).
-   **Material Components**:
    -   `5`: Ambient.
    -   `6`: Diffuse.
    -   `7`: Specular.
-   **Cameras**: `Q` / `Shift+W,E,R` to cycle cameras in each viewport.
