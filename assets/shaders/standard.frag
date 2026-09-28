#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform bool useTexture;
uniform vec3 solidColor;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 fogColor;
uniform float fogDensity;

uniform float time;
uniform bool isWater;
uniform float emissive;

void main() {
    vec3 norm = normalize(Normal);
    vec2 uvs = TexCoords;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(lightPos - FragPos);

    if (isWater) {
        // Dual animated sine wave ripples (micro + macro)
        float wave1 = sin(FragPos.x * 2.2 + time * 2.8 + FragPos.z * 1.1) * 0.14;
        float wave2 = cos(FragPos.z * 1.8 - time * 2.2 + FragPos.x * 1.4) * 0.12;
        float wave3 = sin((FragPos.x + FragPos.z) * 3.5 + time * 3.2) * 0.08;
        norm = normalize(norm + vec3(wave1 + wave3, 0.0, wave2 + wave3));
        uvs += vec2(sin(time * 0.4 + uvs.y * 3.0) * 0.035, cos(time * 0.3 + uvs.x * 3.0) * 0.035);
    }

    // Ambient Lighting with Crevice & Contact Occlusion
    float contactAO = clamp(0.68f + 0.32f * smoothstep(0.01f, 0.40f, FragPos.y), 0.68f, 1.0f);
    float underOcclusion = clamp(norm.y * 0.22f + 0.78f, 0.58f, 1.0f);
    float ambientStrength = 0.38f * contactAO * underOcclusion;
    vec3 ambient = ambientStrength * lightColor;
  	
    // Diffuse with Half-Lambert Sunlight Wrap for organic outdoor surfaces
    float NdotL = dot(norm, lightDir);
    float wrapDiff = pow(max((NdotL + 0.35f) / 1.35f, 0.0f), 1.25f);
    float hardDiff = max(NdotL, 0.0f);
    float diff = isWater ? hardDiff : mix(hardDiff, wrapDiff, 0.72f);
    
    // Subsurface translucency backlight through foliage and leaves
    float foliageTranslucency = pow(max(dot(viewDir, -lightDir), 0.0f), 2.2f) * 0.22f;
    vec3 diffuse = (diff * underOcclusion + foliageTranslucency) * lightColor;
    
    // Specular with high-gloss caustic shine on water
    float specularStrength = isWater ? 1.4f : 0.35f;
    float shininess = isWater ? 128.0f : 28.0f;
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    vec3 specular = specularStrength * spec * lightColor;  

    // Fresnel Rim Backlight for natural outdoor silhouettes and water reflection
    float fresnel = pow(1.0f - max(dot(norm, viewDir), 0.0f), 3.0f);
    vec3 rim = fresnel * lightColor * (isWater ? 0.65f : 0.22f);
        
    vec3 baseColor;
    if (useTexture) {
        vec3 texSample = texture(texture_diffuse1, uvs).rgb;
        // Multi-scale macro modulation to break up repeating grass & ground textures
        float macroWave = sin(FragPos.x * 0.06f) * cos(FragPos.z * 0.06f) * 0.07f;
        float elevationHue = clamp(FragPos.y * 0.025f, -0.04f, 0.06f);
        baseColor = (texSample + vec3(macroWave + elevationHue, macroWave, macroWave - elevationHue * 0.6f)) * solidColor;
    } else {
        baseColor = solidColor;
        // Subtle micro-surface variation for untextured stone/wood
        if (!isWater && emissive <= 0.0f) {
            float microGrain = (sin(FragPos.x * 25.0f) * sin(FragPos.y * 25.0f + FragPos.z * 25.0f)) * 0.03f;
            baseColor += vec3(microGrain);
        }
    }

    // Water Depth Tinting & Shimmer
    if (isWater) {
        vec3 deepWater = vec3(0.08f, 0.32f, 0.58f);
        vec3 shallowWater = vec3(0.32f, 0.78f, 0.95f);
        float shimmer = sin(FragPos.x * 4.0f + FragPos.z * 4.0f + time * 3.0f) * 0.08f;
        baseColor = mix(deepWater, shallowWater, clamp(fresnel + shimmer + 0.3f, 0.0f, 1.0f));
    }
    
    vec3 result = (ambient + diffuse + specular + rim) * baseColor;
    
    // Emissive boost (for glowing lanterns, campfire, BBQ grill)
    if (emissive > 0.0f) {
        result += solidColor * emissive;
    }
    
    // Atmospheric Exponential-Squared Depth Fog
    float distance = length(viewPos - FragPos);
    float actualDensity = (fogDensity > 0.0001f) ? fogDensity : 0.008f;
    float fogFactor = exp(-pow(actualDensity * distance, 1.8f));
    fogFactor = clamp(fogFactor, 0.0f, 1.0f);
    
    result = mix(fogColor, result, fogFactor);

    FragColor = vec4(result, 1.0);
}
