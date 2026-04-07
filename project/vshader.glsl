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
        // Low-amplitude layered waves for large distant water planes.
        float waveA = sin((aPos.x * 0.16) + waterTime * 0.95) * 0.035;
        float waveB = cos((aPos.z * 0.18) + waterTime * 0.72) * 0.025;
        float waveC = sin((aPos.x + aPos.z) * 0.10 + waterTime * 0.45) * 0.018;
        localPos.y += (waveA + waveB + waveC);
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