# Lab Report: Assignment 03 - Advanced Lighting and Viewports

**Project**: 3D Bus Simulation with Phong Lighting
**Course**: CSE 4208 - Computer Graphics
**Date**: February 15, 2026

## 1. Objective
The primary objective of this assignment was to implement a 3D scene featuring a complex object (a bus) illuminated by the Phong shading model. The project required the implementation of three types of light sources (Directional, Point, and Spotlight), distinct material properties (ambient, diffuse, specular, emissive), and a multi-viewport display system with specific camera and lighting configurations.

## 2. Implementation Details

### 2.1 3D Model: The Bus
The bus is constructed using hierarchical modeling with `Cube` and `Sphere` primitives. Transformations (translation, rotation, scaling) are applied to assemble components:
-   **Chassis**: Hollow body constructed from separate floor, roof, and wall segments to allow interior viewing.
-   **Wheels**: Spheres scaled and positioned relative to the chassis, rotating with bus movement.
-   **Interior**: Seats and poles arranged inside the hollow shell.
-   **Fixtures**: Light stems and bulbs attached to the ceiling.

### 2.2 Lighting System
The application implements the standard Phong Reflection Model in GLSL shaders (`shaders.glsl`), calculating Ambient, Diffuse, and Specular components for three distinct light types:

#### A. Directional Light (Sun)
-   **Role**: Simulates global sunlight.
-   **Implementation**: A light source at infinity with parallel rays.
-   **Control**: Toggled via **Key 1**.

#### B. Point Lights (Interior Lighting)
-   **Role**: Simulates interior cabin bulbs.
-   **Implementation**: Four light sources positioned on the bus ceiling.
    -   **Attenuation**: Constant, Linear, and Quadratic terms calculate intensity falloff over distance.
    -   **Visuals**: Emissive spheres (bulbs) are rendered at the light positions.
    -   **Unified Control**: **Key 2** toggles both the light source calculation (physics) and the visual bulb glow (emissive material), ensuring realistic synchronization.

#### C. Spotlight (Headlights)
-   **Role**: Simulates bus headlights.
-   **Implementation**: A cone-shaped light source attached to the **front bumper** of the bus.
    -   **Dynamic Update**: The spotlight's position and direction are updated every frame based on the bus's position and yaw, ensuring the beam turns with the vehicle.
    -   **Cutoff**: A cosine cutoff of ~25.5 degrees creates the beam edge.
-   **Control**: Toggled via **Key 3**.

### 2.3 Viewports and Camera Configurations
The screen is split into four distinct viewports using `glViewport` and `glScissor`, each with specific rendering rules:

| Viewport | Position | Custom Camera Mode | Lighting Configuration |
| :--- | :--- | :--- | :--- |
| **Top-Left** | `(0, H/2)` | **Isometric** | **Combined**: All lights enabled. |
| **Top-Right** | `w/2, H/2)` | **Top View** | **Ambient Only**: Only global ambient light. |
| **Bottom-Left** | `(0, 0)` | **Front View** | **Diffuse Only**: Only diffuse reflection. |
| **Bottom-Right** | `(w/2, 0)` | **Inside View** | **Custom**: Directional + Interior Lights + Headlights. |

*> Note: The "Inside View" configuration was refined to allow Headlights and Interior Lights to be visible, overriding the initial "strict" assignment requirement to ensure functional driving simulation.*

## 3. Interaction & Controls

### Bus Control
-   **Up / W**: Accelerate Forward
-   **Down / S**: Reverse
-   **Left / A**: Turn Left
-   **Right / D**: Turn Right

### Lighting Controls
-   **1**: Toggle Directional Light (Sun)
-   **2**: Toggle Interior Point Lights (Source + Bulbs)
-   **3**: Toggle Headlights (Spotlight)
-   **5**: Toggle Ambient Term
-   **6**: Toggle Diffuse Term
-   **7**: Toggle Specular Term

### Camera Control
-   **Q**: Cycle Camera for Top-Left Viewport
-   **Shift + W**: Cycle Camera for Top-Right Viewport
-   **Shift + E**: Cycle Camera for Bottom-Left Viewport
-   **Shift + R**: Cycle Camera for Bottom-Right Viewport

## 4. Discussion & Technical Challenges

**Challenge 1: The "Ghost" Spotlight**
*Initial State*: The spotlight was attached to the debug camera position (`cameraPos`). When the user drove the bus away, the light stayed behind at the origin.
*Solution*: Updated the main loop to calculate `spotLight.position` using the `busPos` vector and `busYaw` angle. The light now physically attaches to the bus geometry.

**Challenge 2: Interior Darkness**
*Initial State*: The assignment's "Directional Lighting Only" rule for the Inside View meant interior point lights were hardcoded to OFF, making the interior toggle useless in that view.
*Solution*: The logic was adjusted to allow `pointLightOn` and `spotLightOn` states to pass through to the inside viewport shader, significantly improving the driving simulation experience.

**Challenge 3: Emissive vs. Light Source**
*Initial State*: Key 4 toggled the visual "glow" of the bulb, while Key 2 toggled the actual light. This allowed for confusing states (glowing bulb with no light, or dark bulb emitting light).
*Solution*: Key 4 was deprecated. The logic was unified so that Key 2 toggles both the `pointLight` calculation and the `emissiveOn` uniform, ensuring consistent physical behavior.

## 5. Conclusion
The project successfully implements a robust Phong lighting model with dynamic, interactive elements. The bus simulation allows for detailed inspection of lighting components (ambient/diffuse/specular) through its multi-viewport system, fulfilling all assignment criteria while adding engineering refinements for realism.
