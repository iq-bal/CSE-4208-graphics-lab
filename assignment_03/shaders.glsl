// Vertex Shader
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

// <!-- split -->

// Fragment Shader
#version 330 core
out vec4 FragColor;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

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
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 4

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform vec3 objectColor; // Material color

// Toggles
uniform bool dirLightOn;
uniform bool pointLightOn;
uniform bool spotLightOn;
uniform bool ambientOn;
uniform bool diffuseOn;
uniform bool specularOn;
uniform bool emissiveOn; // For emissive light (Assignment requirement 2)

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);

    // Phase 1: Directional Light
    if(dirLightOn)
        result += CalcDirLight(dirLight, norm, viewDir);

    // Phase 2: Point Lights
    if(pointLightOn) {
        for(int i = 0; i < NR_POINT_LIGHTS; i++)
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }

    // Phase 3: Spot Light
    if(spotLightOn)
        result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
        
    // Final Color = (Lighting * Material) + Emissive
    vec3 finalColor = result * objectColor;

    // Emissive component (Assignment requirement: "emissive light")
    // Adds a glow effect independent of external lighting
    if(emissiveOn) {
        finalColor += objectColor * 0.4; 
    }

    FragColor = vec4(finalColor, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Shininess 32
    // Combine results
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    
    if(!ambientOn) ambient = vec3(0.0);
    if(!diffuseOn) diffuse = vec3(0.0);
    if(!specularOn) specular = vec3(0.0);
    
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // Combine results
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    
    if(!ambientOn) ambient = vec3(0.0);
    if(!diffuseOn) diffuse = vec3(0.0);
    if(!specularOn) specular = vec3(0.0);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    // Spotlight (Single Cut-off)
    float theta = dot(lightDir, normalize(-light.direction)); 
    
    if(theta > light.cutOff) { // Inside the cone
        // Combine results
        vec3 ambient = light.ambient;
        vec3 diffuse = light.diffuse * diff;
        vec3 specular = light.specular * spec;
        
        if(!ambientOn) ambient = vec3(0.0);
        if(!diffuseOn) diffuse = vec3(0.0);
        if(!specularOn) specular = vec3(0.0);
        
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        return (ambient + diffuse + specular);
    } else {
        return vec3(0.0);
    }
}
