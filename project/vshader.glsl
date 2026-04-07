#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool useWaterSurface;
uniform float waterTime;

void main()
{
    vec3 localPos = aPos;
    if (useWaterSurface) {
        // --- Cubic Bezier wave displacement ---
        // B(t) = (1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3
        // Control points define the wave envelope shape:
        //   P0 = trough, P1 = rising slope, P2 = crest overshoot, P3 = crest

        // Wave A: large swell (Bezier control points)
        float P0 = -0.04;  float P1 = 0.01;  float P2 = 0.06;  float P3 = 0.04;
        float tA = sin(aPos.x * 0.08 + waterTime * 0.70) * 0.5 + 0.5;
        float uA = 1.0 - tA;
        float bezA = uA*uA*uA*P0 + 3.0*uA*uA*tA*P1 + 3.0*uA*tA*tA*P2 + tA*tA*tA*P3;

        // Wave B: cross-wave
        float tB = cos(aPos.z * 0.10 + waterTime * 0.55) * 0.5 + 0.5;
        float uB = 1.0 - tB;
        float bezB = uB*uB*uB*P0 + 3.0*uB*uB*tB*P1 + 3.0*uB*tB*tB*P2 + tB*tB*tB*P3;

        // Wave C: diagonal ripple (tighter Bezier control points)
        float P0c = -0.02; float P1c = 0.00; float P2c = 0.035; float P3c = 0.025;
        float tC = sin((aPos.x + aPos.z) * 0.06 + waterTime * 0.38) * 0.5 + 0.5;
        float uC = 1.0 - tC;
        float bezC = uC*uC*uC*P0c + 3.0*uC*uC*tC*P1c + 3.0*uC*tC*tC*P2c + tC*tC*tC*P3c;

        // Wave D: fine detail chop
        float P0d = -0.01; float P1d = 0.005; float P2d = 0.02; float P3d = 0.015;
        float tD = sin(aPos.x * 0.22 - waterTime * 0.90) * cos(aPos.z * 0.18 + waterTime * 0.60) * 0.5 + 0.5;
        float uD = 1.0 - tD;
        float bezD = uD*uD*uD*P0d + 3.0*uD*uD*tD*P1d + 3.0*uD*tD*tD*P2d + tD*tD*tD*P3d;

        localPos.y += bezA + bezB + bezC + bezD;
    }

    FragPos = vec3(model * vec4(localPos, 1.0));
    TexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * aNormal;
    
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    TBN = mat3(T, B, N);
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}