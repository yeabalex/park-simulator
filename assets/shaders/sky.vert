#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 WorldPos;
out vec3 RayDir;

uniform mat4 view;
uniform mat4 projection;

void main() {
    WorldPos = aPos;
    // Strip translation from view matrix so sky dome stays centered on camera
    mat4 viewNoTrans = mat4(mat3(view));
    vec4 pos = projection * viewNoTrans * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // Forces depth = 1.0 (far plane)
    RayDir = aPos;
}
