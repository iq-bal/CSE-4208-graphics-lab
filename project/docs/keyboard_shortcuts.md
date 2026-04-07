# Keyboard Shortcuts — Graphics Concept Tweaks

This document lists all keyboard shortcuts available for interactively tweaking each implemented computer graphics concept at runtime. Press the key while the application is running to see the effect in real-time.

---

## 🎮 General Controls (Pre-existing)

| Key | Action |
| :--- | :--- |
| `W` / `A` / `S` / `D` | Move Forward / Left / Backward / Right |
| `Mouse` | Look around (FPS camera) |
| `Space` / `Left Shift` | Fly Up / Down (exterior only) |
| `E` | Interact (enter pyramid / open sarcophagus) |
| `Esc` | Quit application |
| `R` | Reset camera position |
| `M` | Toggle background music |

---

## 🧱 Category 1: Geometry & Mathematical Modeling

| Shortcut | Concept | Action | Where to Observe |
| :--- | :--- | :--- | :--- |
| `Ctrl + ↑` | **Bezier Curve** | **Increase** wave amplitude | **Exterior → Ocean surface.** Press `O` to teleport outside, then `K` to jump to the shoreline. Hold `Ctrl+↑` — the ocean waves will grow dramatically taller, visibly deforming the water mesh via Cubic Bezier displacement. |
| `Ctrl + ↓` | **Bezier Curve** | **Decrease** wave amplitude (down to flat) | **Exterior → Ocean surface.** Hold `Ctrl+↓` — the waves will flatten until the ocean becomes a perfectly still mirror. At `0.0`, all Bezier displacement is removed. |
| `Ctrl + →` | **Fractal** | **Increase** fractal tree density (more fronds) | **Exterior → Palm trees around island.** Press `O` to go outside. Each press adds 6 more fractal fronds to every palm tree crown (max depth = 6, yielding 36 fronds). Trees become visibly fuller and denser. |
| `Ctrl + ←` | **Fractal** | **Decrease** fractal tree density (fewer fronds) | **Exterior → Palm trees around island.** Each press removes 6 fronds from the trees (min depth = 1, yielding only 6 sparse fronds). Trees become bare skeletons. |

---

## 🖼️ Category 2: Surface Detail & Texturing

| Shortcut | Concept | Action | Where to Observe |
| :--- | :--- | :--- | :--- |
| `T` | **Simple Texture** | Toggle all textures ON/OFF | **Everywhere.** Disabling textures reveals the raw `objectColor` on all surfaces — walls become flat brown, sand becomes beige, water loses its pattern. Re-enable to restore all mapped images. |

> **Note:** *Vertex Blend* is intrinsic to the GPU rasterizer (interpolating UVs across triangles) and cannot be toggled. *Fragment Blend* is always active on the water surface (the `mix()` function blending 4 depth-dependent color layers); it can be observed by teleporting to the shoreline (`K`) and watching how the water color transitions from shallow turquoise to deep blue.

---

## 💡 Category 3: Lighting (Light Sources)

| Shortcut | Concept | Action | Where to Observe |
| :--- | :--- | :--- | :--- |
| `1` | **Directional Light** | Toggle Sun ON/OFF | **Exterior.** Press `O` to go outside. Press `1` — the entire desert and pyramid lose their primary illumination source. The sky dome remains but the ground becomes pitch dark. Press `1` again to restore sunlight. |
| `2` | **Point Light** | Toggle all lantern point lights ON/OFF | **Interior.** Press `I` to enter the crypt hallway. Press `2` — all wall-mounted lantern lights instantly turn off, plunging the corridor into darkness (only the flashlight remains if active). Press `2` again to restore the warm firelight. |
| `3` | **Spot Light** | Toggle flashlight (spotlight) ON/OFF | **Interior.** Press `I` to enter the crypt. Press `3` — your handheld flashlight beam disappears. In a dark corridor with lanterns off (`L`), this leaves you in complete blackness. Also available via `F`. |
| `0` | **Spot Light Cone** | Cycle spotlight beam width (14° → 25° → 40° → 60° → 14°) | **Interior.** Press `I`, make sure flashlight is on (`F`). Press `0` repeatedly — the flashlight beam widens from a tight focused circle to a broad floodlight, then snaps back to tight. |

---

## 🌗 Category 4: Shading & Reflection Models

| Shortcut | Concept | Action | Where to Observe |
| :--- | :--- | :--- | :--- |
| `4` | **Ambient** | Toggle ambient lighting component ON/OFF | **Interior.** Press `I` to teleport inside. Press `4` — the faint base brightness that fills shadowed areas disappears. Dark corners become truly black. The effect is subtle but visible on surfaces not directly facing a light. |
| `5` | **Diffuse** | Toggle diffuse lighting component ON/OFF | **Everywhere.** Press `5` — surfaces lose their primary brightness response to light direction. Only ambient glow and specular highlights remain, making everything look either flat or shiny-metallic. Best seen on the interior walls near lanterns. |
| `6` | **Specular** | Toggle specular highlights ON/OFF | **Interior near lanterns.** Press `I`, then `6` — the bright white/orange hot-spots that appear on surfaces facing the light vanish. Surfaces become purely matte. Re-enable to see the glossy reflections return on the stone walls. |
| `7` | **Gouraud vs Phong** | Toggle between Gouraud (per-vertex) and Phong (per-fragment) shading | **Everywhere.** Press `7` — lighting switches to Gouraud mode. You will see visible "banding" and color stepping on large flat surfaces because light is only calculated at vertices and interpolated across faces. Low-polygon surfaces like the cube walls will show obvious flat-shaded triangular artifacts compared to the smooth Phong default. |
| `8` (hold) | **Phong Shininess** | **Decrease** specular exponent (broader, softer highlights) | **Interior near lanterns.** Hold `8` — the specular exponent drops, making highlights spread wider and appear more diffuse/plastic. At very low values (~2), surfaces look wet or waxy. |
| `9` (hold) | **Phong Shininess** | **Increase** specular exponent (sharper, tighter highlights) | **Interior near lanterns.** Hold `9` — the specular exponent rises, making highlights shrink into tiny, intense pinpoints. At high values (~256), only surfaces at the exact reflection angle show a bright dot. |

---

## 📐 Category 5: Viewing & Projection

| Shortcut | Concept | Action | Where to Observe |
| :--- | :--- | :--- | :--- |
| `V` | **Multi-Viewport** | Toggle 4-way split screen (Top / Front / Isometric / Perspective) | **Everywhere.** Press `V` — the screen splits into 4 quadrants showing simultaneous Orthographic Top, Front, Isometric, and Perspective views. All views track the player in real-time. Press `V` again to return to full-screen Perspective. |

---

## 🔧 Developer Teleport Shortcuts

| Key | Destination |
| :--- | :--- |
| `I` | Interior (first room entrance) |
| `J` | Second room (DHARAGOL chamber, auto-unlocked) |
| `O` | Outside (pyramid entrance) |
| `K` | Shoreline (facing ocean — best for Bezier wave demo) |
| `P` | Pyramid peak (aerial view) |
| `H` | Halt/resume swinging trap animation |
| `L` | Toggle lantern flames ON/OFF (visual only, lights stay) |
