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
        // Multi-octave organic waves for realistic water surface.
        float waveA = sin(aPos.x * 0.08 + waterTime * 0.70) * 0.045;
        float waveB = cos(aPos.z * 0.10 + waterTime * 0.55) * 0.035;
        float waveC = sin((aPos.x + aPos.z) * 0.06 + waterTime * 0.38) * 0.025;
        float waveD = sin(aPos.x * 0.22 - waterTime * 0.90) * cos(aPos.z * 0.18 + waterTime * 0.60) * 0.018;
        float waveE = cos(aPos.x * 0.35 + aPos.z * 0.30 + waterTime * 1.20) * 0.010;
        localPos.y += (waveA + waveB + waveC + waveD + waveE);
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