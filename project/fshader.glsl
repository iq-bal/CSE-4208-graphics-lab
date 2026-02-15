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
uniform PointLight pointLights[10]; // Max 10 lights
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

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color);

void main()
{
    // Emissive objects bypass lighting entirely
    if (useEmissive) {
        FragColor = vec4(emissiveColor, 1.0);
        return;
    }

    vec3 norm = normalize(Normal);
    if (useNormalMap) {
        norm = texture(normalMap, TexCoords).rgb;
        norm = norm * 2.0 - 1.0;   
        norm = normalize(TBN * norm);
    }
    
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 baseColor = objectColor;
    if (useTexture) {
        baseColor = texture(texture1, TexCoords).rgb;
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