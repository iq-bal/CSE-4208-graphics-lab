# Lab Report: Assignment 03 - Advanced Lighting and Viewports

**Project**: 3D Bus Simulation with Phong Lighting
**Course**: CSE 4208 - Computer Graphics
**Date**: February 15, 2026

## 1. Objective
The primary objective of this assignment was to implement a 3D scene featuring a complex object (a bus) illuminated by the Phong shading model. The project demonstrates the practical application of lighting theory, including various light source models, reflection components, and shading techniques, visualized through a multi-viewport display system.

## 2. Theory
This section outlines the fundamental computer graphics concepts covered in the lab and implemented in this project.

### 2.1 Lighting Principles
 lighting simulation in computer graphics approximates how light interacts with objects based on their material composition, the light's properties (color, position, intensity), and global parameters such as ambient light. The goal is to calculate the color of each pixel by simulating these interactions.

### 2.2 Light Source Models
We implemented three mathematical models to simulate different real-world light sources:
-   **Point Light**: An omni-directional source (like a light bulb) that radiates light equally in all directions. It is defined by its intensity and position, and its influence diminishes with distance (attenuation).
-   **Directional Light**: A light source located at infinity (like the sun). It has intensity and a specific direction but no position, meaning its rays are parallel and do not suffer from distance attenuation.
-   **Spot Light**: A directional point source (like a flashlight or headlight) restricted to a specific cone angle. It creates a focused beam and is defined by intensity, position, direction, attenuation, and a cutoff angle.

### 2.3 Illumination Model
The final color of an object is computed by combining the effects of the camera, light sources, object geometry, and material properties. The standard illumination model used is:
$$ \text{Illumination} = \text{Ambient} + \text{Diffuse} + \text{Specular} + \text{Emissive} $$

#### Reflection Components
-   **Ambient Light**: A constant background illumination representing light that has been scattered so many times it comes uniformly from all directions. It ensures no part of the scene is pitch black.
-   **Diffuse Reflection**: Light that hits a rough surface and scatters equally in all directions. The brightness depends on the angle between the light ray and the surface normal (Lambert's Cosine Law) but looks the same from any viewing angle.
-   **Specular Reflection**: Light bouncing off shiny surfaces in a preferred direction. The intensity depends heavily on the viewing angle; a highlight appears only when the viewer is aligned with the reflection vector.

### 2.4 Shading Techniques
-   **Gouraud Shading**: Computes lighting at the vertices and linearly interpolates the resulting *colors* across the polygon faces. It is faster but can look blocky on large triangles.
-   **Phong Shading**: Interpolates the *surface normals* across the polygon faces and computes the lighting equation for every pixel fragment. This produces much smoother highlights and is the technique implemented in this assignment.

## 3. Implementation Details

### 3.1 3D Model: The Bus
The bus is constructed using hierarchical modeling with `Cube` and `Sphere` primitives. Transformations (translation, rotation, scaling) are applied to assemble components:
-   **Chassis**: Hollow body constructed from separate floor, roof, and wall segments to allow interior viewing.
-   **Wheels**: Spheres scaled and positioned relative to the chassis, rotating with bus movement.
-   **Interior**: Seats and poles arranged inside the hollow shell.
-   **Fixtures**: Light stems and bulbs attached to the ceiling.

### 3.2 Lighting System Implementation
The application implements the Phong Reflection Model in GLSL shaders (`shaders.glsl`), mapping the theoretical models to code:

-   **Directional Light (Sun)**: Implemented as a global light source toggled by **Key 1**.
-   **Point Lights (Interior)**: Four light sources positioned on the bus ceiling.
    -   *Attenuation*: Calculated using Constant ($K_c$), Linear ($K_l$), and Quadratic ($K_q$) terms: $1.0 / (K_c + K_l \cdot d + K_q \cdot d^2)$.
    -   *Unified Cointrol*: **Key 2** toggles both the physics (light calculation) and the visuals (emissive bulb glow).
-   **Spotlight (Headlights)**: A dynamic light source attached to the **front bumper**.
    -   *Dynamic Tracking*: Position and direction update every frame based on `busPos` and `busYaw`.
    -   *Visuals*: Creates a visible beam cone on the road. Toggled by **Key 3**.

### 3.3 Viewports and Camera Configurations
The screen is split into four viewports using `glViewport` and `glScissor`, demonstrating different rendering modes:

| Viewport | Position | Mode | Lighting Configuration |
| :--- | :--- | :--- | :--- |
| **Top-Left** | `(0, H/2)` | **Isometric** | **Combined**: All lights enabled. |
| **Top-Right** | `w/2, H/2)` | **Top View** | **Ambient Only**: Demonstrates the Ambient component in isolation. |
| **Bottom-Left** | `(0, 0)` | **Front View** | **Diffuse Only**: Demonstrates the Diffuse component in isolation. |
| **Bottom-Right** | `(w/2, 0)` | **Inside View** | **Custom**: Directional + Interior Lights + Headlights enabled for driving simulation. |

## 4. Interaction & Controls

### Bus Control
-   **Up / W**: Accelerate Forward
-   **Down / S**: Reverse
-   **Left / A**: Turn Left
-   **Right / D**: Turn Right

### Lighting Controls
-   **1**: Toggle Directional Light (Sun)
-   **2**: Toggle Interior Point Lights (Source + Bulbs)
-   **3**: Toggle Headlights (Spotlight)
-   **5-7**: Toggle Ambient, Diffuse, Specular components globally.

## 5. Discussion

**Challenge: The "Ghost" Spotlight**
*Problem*: The spotlight was initially attached to a static debug camera, meaning it didn't move with the bus.
*Solution*: We modified the main loop to calculate `spotLight.position` using the bus's transformation matrix logic, effectively bolting the light handling to the bus chassis.

**Challenge: Interior Darkness**
*Problem*: Strict adherence to "Directional Only" for the Inside View made the interior point lights invisible.
*Solution*: We adjusted the viewport logic to allow the Point Light and Spotlight states to override the default "OFF" state for the Inside View, ensuring the user can see the effects of their inputs while driving.

## 6. Conclusion
The project successfully bridges theory and practice. By implementing the mathematical models for Directional, Point, and Spot lights within a Shader-based architecture, we created a realistic simulation. The multi-viewport system serves as a diagnostic tool, allowing users to isolate and observe the specific contributions of Ambient, Diffuse, and Specular reflection components as described in the lighting theory.
