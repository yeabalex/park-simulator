#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 RayDir;

uniform vec3 lightDir;     // Normalized sun/moon direction
uniform vec3 sunColor;     // Golden/white for noon, amber/rose for sunset, soft cyan-white for moon
uniform vec3 zenithColor;  // Top of sky
uniform vec3 horizonColor; // Horizon haze
uniform float time;
uniform int timeOfDay;     // 0=NOON, 1=SUNSET, 2=NIGHT

// Procedural hash for stars / clouds
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main() {
    vec3 dir = normalize(RayDir);
    float h = clamp(dir.y, 0.0f, 1.0f);
    
    // Atmospheric gradient from horizon to zenith
    float grad = pow(1.0f - h, 2.2f);
    vec3 sky = mix(zenithColor, horizonColor, grad);
    
    // Sun / Moon disc
    float sunDot = dot(dir, lightDir);
    if (timeOfDay != 2) {
        // Daytime / Sunset Sun
        float sunDisc = smoothstep(0.9965f, 0.9985f, sunDot);
        float sunGlow = pow(max(sunDot, 0.0f), 28.0f) * 0.55f;
        float sunHalo = pow(max(sunDot, 0.0f), 5.0f) * 0.35f;
        sky += sunColor * (sunDisc * 3.0f + sunGlow + sunHalo);
    } else {
        // Moon disc at night
        float moonDisc = smoothstep(0.995f, 0.9975f, sunDot);
        float moonGlow = pow(max(sunDot, 0.0f), 20.0f) * 0.4f;
        sky += vec3(0.92f, 0.96f, 1.0f) * (moonDisc * 2.5f + moonGlow);
        
        // Starfield
        if (dir.y > 0.04f) {
            vec2 starCoord = dir.xz / (dir.y + 0.35f) * 140.0f;
            float n = hash(floor(starCoord));
            if (n > 0.982f) {
                float twinkle = sin(time * 3.5f + n * 45.0f) * 0.35f + 0.65f;
                sky += vec3(0.9f, 0.95f, 1.0f) * twinkle * (n - 0.982f) * 60.0f;
            }
        }
    }
    
    // Soft procedural drifting clouds
    if (dir.y > 0.03f) {
        vec2 cloudUV = dir.xz / (dir.y + 0.18f) * 0.7f + vec2(time * 0.012f, time * 0.006f);
        float c1 = noise(cloudUV * 3.5f);
        float c2 = noise(cloudUV * 7.0f) * 0.5f;
        float cloudDensity = smoothstep(0.52f, 0.82f, c1 + c2);
        
        vec3 cloudColor = (timeOfDay == 1) ? vec3(0.95f, 0.55f, 0.38f) : // Sunset warm clouds
                          (timeOfDay == 2) ? vec3(0.06f, 0.08f, 0.14f) :  // Night dark clouds
                                             vec3(1.0f, 1.0f, 1.0f);     // Day fluffy white clouds
        sky = mix(sky, cloudColor, cloudDensity * 0.6f * smoothstep(0.03f, 0.22f, dir.y));
    }
    
    FragColor = vec4(sky, 1.0f);
}
