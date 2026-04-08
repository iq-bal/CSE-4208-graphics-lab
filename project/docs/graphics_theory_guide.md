# Computer Graphics Theory and Implementation Guide

This document is a deep-reference guide for the project so you can answer theory and implementation questions quickly.

Scope covered:
- Geometry and modeling
- Texturing and blending
- Lighting sources
- Shading and illumination models
- Viewing and projection
- Runtime controls and where each effect appears in the scene

Primary code locations:
- `main.cpp`
- `vshader.glsl`
- `fshader.glsl`

---

## 1) Geometry and Modeling

## 1.1 Cubic and Quadratic Bezier Curves

Theory:
- A Bezier curve is a parametric polynomial curve controlled by control points.
- For a cubic Bezier (4 points),
  B(t) = (1-t)^3*P0 + 3(1-t)^2*t*P1 + 3(1-t)*t^2*P2 + t^3*P3, t in [0,1]
- For a quadratic Bezier (3 points),
  B(t) = (1-t)^2*P0 + 2(1-t)*t*P1 + t^2*P2
- Intuition: endpoints anchor the shape; middle control points pull curvature.

How it works in code:
- Water displacement uses multiple cubic Bezier wave components in `vshader.glsl`.
  - The shader computes four separate cubic curves (`bezA`, `bezB`, `bezC`, `bezD`) with different frequencies/phases.
  - Their sum displaces vertex Y: `localPos.y += (...) * bezierAmplitude`.
- Water color-depth behavior in `fshader.glsl` also uses cubic Bezier mapping.
  - Radial depth from island center is transformed through Bezier control points to produce non-linear shoreline-to-deep transitions.
- Swinging trap blade shape in `main.cpp` uses a quadratic Bezier curve.
  - The blade is tessellated into many vertical slices.
  - Each slice position and arc depth comes from evaluating the quadratic Bezier between `p0`, `p1`, `p2`.

Where you see it in the UI:
- Exterior ocean surface:
  - Water mesh physically undulates (vertex motion).
  - Shoreline and deep-water color transitions look smooth and natural instead of linear.
- Interior corridor trap:
  - Blade lower edge forms a smooth sweeping arc (not a straight/chamfered approximation).

Runtime controls:
- `[` / `]`: decrease/increase `bezierAmplitude` for ocean wave height.
- `H`: halt/resume trap swinging motion.

Common viva answer:
- "Bezier is used both as geometry deformation (vertex displacement and blade profile) and as a non-linear remapping function (water depth and shore transition)."

---

## 1.2 Fractal/Procedural Palm Generation

Theory:
- Fractal/procedural modeling builds complex structure from repeated local rules.
- Here, the scene uses hierarchical composition with pseudo-random variation to mimic natural tree irregularity.
- Even without strict recursive function calls at every level, the generated structure is fractal-like because repeated motifs scale and vary across layers.

How it works in code:
- `drawFractalDesertTree(...)` in `main.cpp` builds each tree using:
  - A trunk from stacked tapered cylinder segments.
  - Noise-driven lean and trunk dimension variation (`pseudoNoise01`).
  - Outer and inner frond layers generated in loops with angle/tilt/length variation.
- `fractalDepth` controls frond count:
  - Outer layer count: `fractalDepth * 6`
  - Inner layer count: `fractalDepth * 3`

Where you see it in the UI:
- Exterior island perimeter trees become sparse or dense depending on depth.
- Tree silhouettes look non-uniform and organic due to seed-based variation.

Runtime controls:
- `,` and `.` to decrease/increase `fractalDepth` (1 to 6).
- `U` teleport to inspect a tree up close.

Common viva answer:
- "The fractal concept is implemented as hierarchical procedural generation with repeated motifs and scale/angle perturbations, controlled globally by `fractalDepth`."

---

## 2) Texturing and Blending

## 2.1 Simple Texturing

Theory:
- Texture mapping assigns 2D image data to 3D surface points via UV coordinates.
- In fragment shading, sampled texels become surface albedo/base color.

How it works in code:
- Textures are loaded in `main.cpp` using `loadTexture(...)` and bound with `GL_TEXTURE_2D`.
- The shader receives `texture1` sampler and UV transform controls (`uvScale`, `uvOffset`, `rotateUV90`).
- In `fshader.glsl`, if `useTexture` is true:
  - `baseColor = texture(texture1, scaledTexCoords).rgb`

Where you see it in the UI:
- Sand, walls, pillars, doors, lanterns, pyramid blocks, water detail, bark/canopy, camel skin.

Runtime controls:
- `T` toggles `texturesEnabled` globally.

Common viva answer:
- "Texturing is sampled per fragment after UV transform, so texture detail is preserved regardless of object scale and camera movement."

---

## 2.2 Vertex Blend and Interpolation

Theory:
- During rasterization, vertex attributes are interpolated across triangle interiors.
- This gives per-fragment UVs, normals, and colors without manually computing each pixel on CPU.

How it works in code:
- Vertex shader outputs `TexCoords`, `Normal`, and `GouraudColor`.
- GPU rasterizer interpolates these values for each fragment before fragment shader execution.
- In Gouraud mode, interpolated `GouraudColor` directly contributes to final lighting.

Where you see it in the UI:
- Smooth transitions across surfaces (UV continuity, gradual light changes in Gouraud mode).
- No explicit key toggle because this is intrinsic to the graphics pipeline.

Common viva answer:
- "Vertex blending here means pipeline interpolation, not skeletal animation; it is the mechanism that turns per-vertex attributes into per-pixel inputs."

---

## 2.3 Fragment-Level Multi-Layer Blending

Theory:
- Fragment blending/mixing combines multiple color contributions to produce physically richer materials.
- Typical operations: linear interpolation (`mix`) and weighted summation.

How it works in code:
- Water shading in `fshader.glsl` blends:
  - Bezier-mapped depth palette (shallow/mid/deep).
  - Multi-octave texture samples (`uv1..uv4`) with weighted sum.
  - Shore tint, foam mask, subsurface tint, Fresnel reflection, caustics, horizon haze.
- This is done per fragment, so depth and angle effects respond smoothly to camera and position.

Where you see it in the UI:
- Exterior shoreline:
  - Color shifts from turquoise to deep navy.
  - Foam rings and glints appear dynamically.
  - Distant water fades into atmospheric haze.

Runtime observation tips:
- Use `K` to jump to shoreline.
- Change wave amplitude with `[` and `]` to see geometry and shading coupling.

Common viva answer:
- "The water look comes from layered per-fragment blending, not a single texture; depth remapping, Fresnel, foam and glints are composited in shader space."

---

## 3) Lighting Sources

## 3.1 Directional-like Sun (implemented using distant point lights)

Theory:
- True directional light has constant direction and no attenuation.
- A distant point light can approximate directional behavior if attenuation is made almost negligible.

How it works in code:
- In exterior mode, `main.cpp` configures bright light at high Y (`pointLights[0]`) and a second fill light (`pointLights[1]`).
- Linear/quadratic attenuation are tiny, producing near-directional behavior over gameplay distances.

Where you see it in the UI:
- Exterior desert/pyramid global illumination and readable texture contrast.
- When toggled off, scene becomes dramatically darker (sky remains emissive but dimmed).

Runtime controls:
- `1` toggles `directionalLightOn`.

Common viva answer:
- "Sunlight is approximated through far point lights with tiny attenuation to emulate directional coverage while reusing point-light shader logic."

---

## 3.2 Point Lights (Lantern System)

Theory:
- Point light emits omnidirectionally from a position with distance attenuation:
  attenuation = 1 / (k_c + k_l*d + k_q*d^2)

How it works in code:
- Interior lantern positions are stored in arrays and sent each frame as `pointLights[i]`.
- `CalcPointLight(...)` in `fshader.glsl` computes ambient + diffuse + specular, then multiplies by attenuation.
- Flicker effect is generated by sinusoidal modulation of diffuse intensity in `main.cpp`.

Where you see it in the UI:
- Warm, localized pools of light on corridor walls and second room burial area.
- Flicker produces subtle temporal variation in brightness.

Runtime controls:
- `2` toggles `pointLightsOn` globally.
- `L` toggles lantern flame visual state and related visual intensity logic.

Common viva answer:
- "Point-light attenuation plus animated intensity produces physically plausible torch falloff and perceptual flame flicker."

---

## 3.3 Spot Light (Camera Flashlight)

Theory:
- Spotlights are cone-restricted lights.
- Intensity commonly uses inner and outer cutoff for soft penumbra:
  intensity = clamp((theta - outerCutOff)/(cutOff - outerCutOff), 0, 1)

How it works in code:
- Spotlight position and direction are bound to camera each frame:
  - `spotLight.position = camera.Position`
  - `spotLight.direction = camera.Front`
- `CalcSpotLight(...)` in `fshader.glsl` computes diffuse/specular and applies attenuation and cone intensity.
- Cone width is controlled by `spotConeAngle` and a fixed outer offset.

Where you see it in the UI:
- Interior aiming behavior: beam follows mouse look in FPS style.
- Narrow cone feels focused; wide cone acts floodlight-like.

Runtime controls:
- `F` or `3`: toggle spotlight.
- `0`: cycle cone angle 14 -> 25 -> 40 -> 60 degrees.

Common viva answer:
- "The flashlight is a view-coupled spotlight with soft edge blending via inner/outer cutoff interpolation."

---

## 4) Shading and Illumination Models

## 4.1 Ambient, Diffuse, Specular Components

Theory:
- Ambient: constant low-level term approximating indirect light.
- Diffuse (Lambert): proportional to max(dot(N,L), 0).
- Specular (Phong): proportional to max(dot(V,R), 0)^n, where n is shininess.

How it works in code:
- In `fshader.glsl`, both `CalcPointLight` and `CalcSpotLight` compute all three components.
- Component toggles (`ambientOn`, `diffuseOn`, `specularOn`) selectively zero terms.
- `specularPower` controls highlight sharpness.

Where you see it in the UI:
- Ambient off: deep shadows become nearly black.
- Diffuse off: shapes lose directional volume cue.
- Specular off: materials become matte with no highlights.
- High shininess: tiny bright highlights; low shininess: broad glossy highlights.

Runtime controls:
- `4`: ambient toggle.
- `5`: diffuse toggle.
- `6`: specular toggle.
- `8` / `9`: decrease/increase `specularPower` (2 to 256).

Common viva answer:
- "The lighting model is decomposed into physically interpretable terms, each user-toggleable for educational isolation."

---

## 4.2 Phong vs Gouraud

Theory:
- Gouraud shading: lighting is computed per vertex and linearly interpolated across fragments.
- Phong shading: normals (or lighting vectors) are interpolated and lighting is computed per fragment.
- Phong generally gives smoother, more accurate highlights on large polygons.

How it works in code:
- `useGouraud` uniform switches path.
- In `vshader.glsl`, Gouraud lighting is computed into `GouraudColor` when enabled.
- In `fshader.glsl`:
  - Gouraud mode: `result = GouraudColor * baseColor`.
  - Phong mode: executes full per-fragment point/spot light loops.

Where you see it in the UI:
- Large surfaces/walls:
  - Gouraud can show interpolation banding.
  - Phong gives smoother gradients and specular response.

Runtime controls:
- `7` toggles Gouraud/Phong.

Important note for defense:
- The older summary says Gouraud is not implemented. In current code, Gouraud mode is implemented and switchable.

Common viva answer:
- "The project supports both Gouraud and Phong, allowing direct side-by-side perception of per-vertex versus per-fragment shading artifacts."

---

## 5) Viewing and Projection

## 5.1 Perspective Projection

Theory:
- Perspective projection preserves depth cue via foreshortening.
- Objects farther away project smaller; parallel lines may converge.

How it works in code:
- `main.cpp` uses `glm::perspective(...)` for normal gameplay view.
- Camera matrix comes from `camera.GetViewMatrix()`.

Where you see it in the UI:
- Default full-screen gameplay mode.
- Natural FPS depth perception indoors and outdoors.

---

## 5.2 Orthographic Projections (Top, Front, Isometric)

Theory:
- Orthographic projection preserves scale along axes without perspective shrink.
- Useful for technical inspection and spatial reasoning.

How it works in code:
- In multi-viewport mode, three orthographic cameras are created with `glm::ortho(...)`:
  - Top view: elevated camera looking down.
  - Front view: camera placed ahead of player looking back.
  - Isometric-like view: offset `(40,40,40)` and orthographic projection.
- Fourth quadrant remains perspective.

Where you see it in the UI:
- Four-way split screen with synchronized tracking of player movement.
- Great for demonstrating projection theory live.

Runtime controls:
- `V` toggles multi-viewport mode.

Common viva answer:
- "Orthographic views remove foreshortening and emphasize geometric layout; perspective view maintains immersive depth cues."

---

## 6) Feature-to-UI Quick Map

- Exterior ocean motion and shape: cubic Bezier displacement in vertex shader.
- Exterior water color realism: fragment-layer blending + Bezier depth remap.
- Swinging trap blade profile: quadratic Bezier in CPU-side geometry slicing.
- Palm complexity: procedural/fractal-style frond generation controlled by `fractalDepth`.
- Texture realism on surfaces: sampler-based texture mapping with UV transforms.
- Interior mood lighting: attenuated point lights with flicker and spotlight cone.
- Material response: ambient/diffuse/specular decomposition and shininess control.
- Shading model comparison: runtime switch between Gouraud and Phong.
- Projection theory demo: perspective + top/front/isometric orthographic split screen.

---

## 7) Demo Script for Q&A (Fast)

Use this sequence during presentation if asked to prove each concept:

1. Press `O`, then `K`.
2. Hold `]` and `[` to show Bezier wave amplitude control.
3. Toggle `T` to show texture contribution.
4. Press `1` to remove/add sun lighting in exterior.
5. Press `I` to enter interior.
6. Toggle `2`, `3`/`F`, and `0` to demonstrate point and spot light behavior.
7. Toggle `4`, `5`, `6` to isolate shading components.
8. Hold `8` / `9` to show specular exponent effect.
9. Press `7` to switch Gouraud/Phong.
10. Press `V` to show top/front/isometric/perspective simultaneously.
11. Press `U` and use `,` / `.` to demonstrate procedural tree density.

---

## 8) If Asked "Why This Is Good Graphics Engineering?"

- It cleanly separates CPU scene orchestration (`main.cpp`) from GPU shading logic (`vshader.glsl`, `fshader.glsl`).
- It exposes educational parameters at runtime (keys), making theory observable and testable.
- It combines geometric modeling, material response, and camera math in one coherent pipeline.
- It includes both physically inspired effects (attenuation, Fresnel-like reflection) and pedagogical toggles (component/shading-mode isolation).

This is exactly the type of implementation that supports both visual quality and oral technical defense.
