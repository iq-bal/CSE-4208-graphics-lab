#version 330 core
out vec4 FragColor;

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;
in mat3 TBN;

uniform int numPointLights;
uniform PointLight pointLights[16]; // Increased limit to 16 lights
uniform SpotLight spotLight;
uniform bool spotLightOn;

uniform vec3 viewPos;
uniform vec3 objectColor;
uniform bool useTexture;
uniform sampler2D texture1;
uniform sampler2D normalMap;
uniform bool useNormalMap;

// Emissive support for self-lit objects (lantern flames)
uniform bool useEmissive;
uniform vec3 emissiveColor;

uniform vec2 uvScale;
uniform vec2 uvOffset;
uniform bool rotateUV90;
uniform bool useWaterSurface;
uniform float waterTime;
uniform float waterNearZ;
uniform float waterFarZ;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color);

void main()
{
    // Emissive objects bypass lighting entirely
    vec2 emTexCoords = TexCoords * uvScale + uvOffset;
    if (rotateUV90) {
        emTexCoords = vec2(emTexCoords.y, 1.0 - emTexCoords.x);
    }
    if (useEmissive) {
        if (useTexture) {
            vec3 texColor = texture(texture1, emTexCoords).rgb;
            FragColor = vec4(texColor, 1.0);
        } else {
            FragColor = vec4(emissiveColor, 1.0);
        }
        return;
    }

    vec3 norm = normalize(Normal);
    vec2 scaledTexCoords = TexCoords * uvScale + uvOffset;
    if (rotateUV90) {
        scaledTexCoords = vec2(scaledTexCoords.y, 1.0 - scaledTexCoords.x);
    }
    if (useNormalMap) {
        norm = texture(normalMap, scaledTexCoords).rgb;
        norm = norm * 2.0 - 1.0;   
        norm = normalize(TBN * norm);
    }
    
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 baseColor = objectColor;
    if (useTexture) {
        baseColor = texture(texture1, scaledTexCoords).rgb;
    }

    if (useWaterSurface) {
        // ---- Cubic Bezier depth curve (radial, water surrounds island) ----
        // B(t) = (1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3
        // Maps radial distance to a non-linear depth factor for natural colour.
        float distFromIsland = length(FragPos.xz);
        float rawDepth = clamp((distFromIsland - waterNearZ) / (waterFarZ - waterNearZ), 0.0, 1.0);

        // Bezier control points shape the depth curve:
        //   P0=0 (shore), P1=0.15 (slow start), P2=0.7 (accelerate), P3=1.0 (deep)
        float bP0 = 0.0;  float bP1 = 0.15;  float bP2 = 0.70;  float bP3 = 1.0;
        float bu = 1.0 - rawDepth;
        float depthLerp = bu*bu*bu*bP0 + 3.0*bu*bu*rawDepth*bP1
                        + 3.0*bu*rawDepth*rawDepth*bP2 + rawDepth*rawDepth*rawDepth*bP3;

        // Shore factor via Bezier curve for smooth sandy transition
        float rawShore = 1.0 - clamp((distFromIsland - (waterNearZ - 30.0)) / 50.0, 0.0, 1.0);
        float sP0 = 1.0; float sP1 = 0.9; float sP2 = 0.2; float sP3 = 0.0;
        float su = 1.0 - rawShore;
        float shoreFactor = su*su*su*sP0 + 3.0*su*su*rawShore*sP1
                          + 3.0*su*rawShore*rawShore*sP2 + rawShore*rawShore*rawShore*sP3;
        shoreFactor = 1.0 - shoreFactor;

        // Richer colour palette: sandy turquoise shallows → deep navy
        vec3 shallowColor = vec3(0.08, 0.42, 0.48);
        vec3 midColor     = vec3(0.04, 0.22, 0.34);
        vec3 deepColor    = vec3(0.01, 0.06, 0.14);
        // Bezier-driven blend: non-linear colour transition
        vec3 depthColor   = mix(mix(shallowColor, midColor, smoothstep(0.0, 0.4, depthLerp)),
                                deepColor, smoothstep(0.4, 1.0, depthLerp));

        // ---- Multi-octave texture waves ----
        float t = waterTime;
        vec2 uv1 = scaledTexCoords * 0.6  + vec2( t * 0.0018,  t * 0.0009);
        vec2 uv2 = scaledTexCoords * 1.8  + vec2(-t * 0.0024,  t * 0.0018);
        vec2 uv3 = scaledTexCoords * 4.5  + vec2( t * 0.0036, -t * 0.0028);
        vec2 uv4 = scaledTexCoords * 9.0  + vec2(-t * 0.0015,  t * 0.0042);
        vec3 tex1 = texture(texture1, uv1).rgb;
        vec3 tex2 = texture(texture1, uv2).rgb;
        vec3 tex3 = texture(texture1, uv3).rgb;
        vec3 tex4 = texture(texture1, uv4).rgb;
        vec3 textured = tex1 * 0.35 + tex2 * 0.30 + tex3 * 0.22 + tex4 * 0.13;

        // Blend texture detail into depth colour — stronger near shore
        float texStrength = mix(0.55, 0.25, depthLerp);
        baseColor = mix(depthColor, textured * depthColor * 1.6, texStrength);

        // ---- Procedural wave normals for specular variation ----
        float nx = sin(FragPos.x * 0.12 + t * 1.1) * 0.3
                 + sin(FragPos.x * 0.28 - t * 0.7 + FragPos.z * 0.06) * 0.15
                 + sin(FragPos.x * 0.55 + t * 1.8) * 0.08;
        float nz = cos(FragPos.z * 0.10 + t * 0.9) * 0.3
                 + cos(FragPos.z * 0.24 + t * 1.4 + FragPos.x * 0.05) * 0.15
                 + cos(FragPos.z * 0.48 - t * 1.6) * 0.08;
        norm = normalize(vec3(nx, 1.0, nz));

        // ---- Shoreline sand tint (radial) ----
        vec3 submergedSand = vec3(0.62, 0.56, 0.40);
        vec3 shoreWater    = vec3(0.14, 0.48, 0.50);
        vec3 shoreTint     = mix(shoreWater, submergedSand, 0.40);
        baseColor = mix(baseColor, shoreTint, shoreFactor * 0.35);

        // ---- Shore foam (radial — rings around island) ----
        float foamWave = sin(distFromIsland * 0.08 + t * 0.6) * 0.4
                       + sin(FragPos.x * 0.15 - t * 1.1) * 0.3
                       + sin(FragPos.z * 0.12 + t * 0.8) * 0.3;
        float foamMask = smoothstep(0.55, 0.85, shoreFactor) * smoothstep(0.25, 0.65, foamWave);
        vec3 foamColor = vec3(0.85, 0.90, 0.92);
        baseColor = mix(baseColor, foamColor, foamMask * 0.55);

        // ---- Subsurface scattering approximation ----
        float sss = pow(max(dot(viewDir, vec3(0.0, -1.0, 0.0)), 0.0), 3.0) * shoreFactor;
        baseColor += vec3(0.04, 0.15, 0.12) * sss * 0.6;

        // ---- Fresnel sky reflection ----
        float fresnel = pow(1.0 - max(dot(normalize(norm), viewDir), 0.0), 4.0);
        vec3 skyReflection = mix(vec3(0.58, 0.66, 0.74), vec3(0.72, 0.78, 0.85), depthLerp);
        baseColor = mix(baseColor, skyReflection, fresnel * 0.50);

        // ---- Caustic shimmer on shallow areas ----
        float caustic1 = sin(FragPos.x * 0.35 + t * 1.5) * cos(FragPos.z * 0.28 - t * 1.2);
        float caustic2 = sin(FragPos.x * 0.22 - t * 0.9) * cos(FragPos.z * 0.40 + t * 0.7);
        float causticPattern = max(caustic1 + caustic2, 0.0) * 0.5;
        baseColor += vec3(0.06, 0.10, 0.09) * causticPattern * (1.0 - depthLerp) * 0.4;
    }
    
    vec3 result = vec3(0.0);
    
    // Point Lights
    for(int i = 0; i < numPointLights; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, baseColor);
        
    // Spot Light (Flashlight/Headlight)
    if (spotLightOn)
        result += CalcSpotLight(spotLight, norm, FragPos, viewDir, baseColor);
     
    // Ambient fallback if no lights
    if (numPointLights == 0 && !spotLightOn)
        result = baseColor * 0.1;
        
    if (useWaterSurface) {
        // ---- Sun specular highlight ----
        vec3 sunDir = normalize(vec3(50.0, 200.0, 40.0) - FragPos);
        vec3 halfVec = normalize(sunDir + viewDir);
        float sunSpec = pow(max(dot(norm, halfVec), 0.0), 128.0);
        result += vec3(1.0, 0.92, 0.80) * sunSpec * 0.55;

        // ---- Layered glints ----
        float glint1 = sin(FragPos.x * 0.05 + waterTime * 2.2)
                     * cos(FragPos.z * 0.06 - waterTime * 1.8);
        float glint2 = sin(FragPos.x * 0.13 - waterTime * 1.5)
                     * cos(FragPos.z * 0.11 + waterTime * 2.4);
        float glintCombined = max(glint1, 0.0) * 0.6 + max(glint2, 0.0) * 0.4;
        result += vec3(0.06, 0.09, 0.10) * glintCombined;

        // ---- Horizon atmospheric fog (radial) ----
        float horizonDist = clamp((length(FragPos.xz) - waterNearZ) / (waterFarZ - waterNearZ), 0.0, 1.0);
        vec3 horizonHaze = vec3(0.62, 0.66, 0.72);
        result = mix(result, horizonHaze, smoothstep(0.6, 1.0, horizonDist) * 0.45);
    }

    FragColor = vec4(result, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    // Combine
    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec; // Assuming white specularity
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    // Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // Combine
    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec;
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return (ambient + diffuse + specular);
}