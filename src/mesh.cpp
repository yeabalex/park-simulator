#include "mesh.h"
#include <glm/glm.hpp>
#include <cmath>

Mesh::Mesh() : VAO(0), VBO(0), vertexCount(0) {}

Mesh::~Mesh() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

void Mesh::setupMesh(const std::vector<Vertex>& vertices) {
    vertexCount = vertices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh::draw() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

float Mesh::getTerrainHeight(float x, float z) {
    float dist = glm::length(glm::vec2(x, z));

    // Natural rolling landscape: gentle harmonic waves without sharp creasing
    float h1 = (std::sin(x * 0.007f) * std::cos(z * 0.007f) + 1.0f) * 0.5f * 4.2f;
    float h2 = (std::sin(x * 0.0035f + 1.2f) * std::sin(z * 0.004f + 0.8f) + 1.0f) * 0.5f * 3.2f;
    float baseHeight = h1 + h2; // Gentle 0 to 7.4m natural rolling meadows
    
    float attenuation = 1.0f;

    // Main road axes flattening (clearance 10m path + 10m buffer)
    if (std::abs(x) < 22.0f) {
        float factor = (std::abs(x) - 8.0f) / 14.0f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= (factor * factor * (3.0f - 2.0f * factor));
    }
    if (std::abs(z) < 22.0f) {
        float factor = (std::abs(z) - 8.0f) / 14.0f; 
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= (factor * factor * (3.0f - 2.0f * factor));
    }

    // Center Plaza Plateau (radius 0 to 52m flat)
    if (dist < 72.0f) {
        float factor = (dist - 48.0f) / 24.0f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= (factor * factor * (3.0f - 2.0f * factor));
    }

    // Outer Ring Road (radius 250m)
    if (dist > 215.0f && dist < 285.0f) {
        float ringDist = std::abs(dist - 250.0f);
        if (ringDist < 22.0f) {
            float factor = (ringDist - 8.0f) / 14.0f;
            if (factor < 0.0f) factor = 0.0f;
            attenuation *= factor;
        }
    }

    // Helper lambda for distance from point (px, pz) to line segment (ax, az)-(bx, bz)
    auto distToSegment = [](float px, float pz, float ax, float az, float bx, float bz) {
        float dx = bx - ax, dz = bz - az;
        float l2 = dx * dx + dz * dz;
        if (l2 < 0.001f) return std::hypot(px - ax, pz - az);
        float t = std::max(0.0f, std::min(1.0f, ((px - ax) * dx + (pz - az) * dz) / l2));
        return std::hypot(px - (ax + t * dx), pz - (az + t * dz));
    };

    // Leveled Architectural Plateau: Victorian Gazebo at (-40, -40)
    float distToGazebo = std::hypot(x + 40.0f, z + 40.0f);
    if (distToGazebo < 22.0f) {
        float factor = (distToGazebo - 13.0f) / 9.0f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= (factor * factor * (3.0f - 2.0f * factor));
    }

    // Branch path to Gazebo: from (-20, -20) to (-40, -40)
    float dPathGazebo = distToSegment(x, z, -20.0f, -20.0f, -40.0f, -40.0f);
    if (dPathGazebo < 8.0f) {
        float factor = (dPathGazebo - 3.5f) / 4.5f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= factor;
    }

    // Leveled Architectural Plateau: Picnic & BBQ Area at (40, -40)
    float distToPicnic = std::hypot(x - 40.0f, z + 40.0f);
    if (distToPicnic < 22.0f) {
        float factor = (distToPicnic - 13.0f) / 9.0f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= (factor * factor * (3.0f - 2.0f * factor));
    }

    // Branch path to Picnic: from (20, -20) to (40, -40)
    float dPathPicnic = distToSegment(x, z, 20.0f, -20.0f, 40.0f, -40.0f);
    if (dPathPicnic < 8.0f) {
        float factor = (dPathPicnic - 3.5f) / 4.5f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= factor;
    }

    // Leveled Architectural Plateau: Kids Playground at (-60, 60)
    float distToPlayground = std::hypot(x + 60.0f, z - 60.0f);
    if (distToPlayground < 28.0f) {
        float factor = (distToPlayground - 18.0f) / 10.0f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= (factor * factor * (3.0f - 2.0f * factor));
    }

    // Branch path to Playground: from (-28, 28) to (-60, 60)
    float dPathPlay = distToSegment(x, z, -28.0f, 28.0f, -60.0f, 60.0f);
    if (dPathPlay < 8.0f) {
        float factor = (dPathPlay - 3.5f) / 4.5f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= factor;
    }

    // Branch path to Lake Pier: from (0, -100) to (-76, -100)
    float dPathPier = distToSegment(x, z, 0.0f, -100.0f, -76.0f, -100.0f);
    if (dPathPier < 8.0f) {
        float factor = (dPathPier - 3.5f) / 4.5f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= factor;
    }
    
    // Attenuate near branch path to the sign (x from 0 to 90, z around -80)
    if (x > 0.0f && x < 90.0f && std::abs(z + 80.0f) < 15.0f) {
        float factor = (std::abs(z + 80.0f) - 5.0f) / 10.0f; 
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= factor;
    }
    
    // Attenuate around the Sign Plaza itself
    float distToSign = glm::length(glm::vec2(x - 80.0f, z + 80.0f));
    if (distToSign < 25.0f) {
        float factor = (distToSign - 12.0f) / 13.0f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= factor;
    }
    
    // Carve a smooth parabolic bowl for the Lake at (-100, -100)
    float distToPond = glm::length(glm::vec2(x + 100.0f, z + 100.0f));
    if (distToPond < 40.0f) {
        float factor = (distToPond - 21.0f) / 19.0f;
        if (factor < 0.0f) factor = 0.0f;
        
        float currentH = baseHeight * attenuation * (factor * factor * (3.0f - 2.0f * factor));
        
        // Deep parabolic bowl inside the lake
        if (distToPond < 21.0f) {
            float depthFactor = distToPond / 21.0f; // 0 at center, 1 at edge
            currentH = -3.5f * (1.0f - depthFactor * depthFactor); // Smooth parabolic bowl
        }
        
        return currentH;
    }
    
    // Attenuate for East Restaurant (250, 150)
    float distToEastRest = glm::length(glm::vec2(x - 250.0f, z - 150.0f));
    if (distToEastRest < 35.0f) {
        float factor = (distToEastRest - 24.0f) / 11.0f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= factor;
    }
    
    // Attenuate for West Restaurant (-100, -30)
    float distToWestRest = glm::length(glm::vec2(x + 100.0f, z + 30.0f));
    if (distToWestRest < 35.0f) {
        float factor = (distToWestRest - 24.0f) / 11.0f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= factor;
    }

    // Attenuate for Shop (50, 50)
    float distToShop = glm::length(glm::vec2(x - 50.0f, z - 50.0f));
    if (distToShop < 22.0f) {
        float factor = (distToShop - 14.0f) / 8.0f;
        if (factor < 0.0f) factor = 0.0f;
        attenuation *= factor;
    }

    return baseHeight * attenuation;
}

Mesh* Mesh::createTerrain(float size, int resolution) {
    std::vector<Vertex> vertices;
    float step = size / resolution;
    float start = -size / 2.0f;
    
    auto getNormal = [&](float x, float z) {
        float hL = getTerrainHeight(x - 0.1f, z);
        float hR = getTerrainHeight(x + 0.1f, z);
        float hD = getTerrainHeight(x, z - 0.1f);
        float hU = getTerrainHeight(x, z + 0.1f);
        glm::vec3 normal(hL - hR, 0.2f, hD - hU);
        return glm::normalize(normal);
    };

    for (int i = 0; i < resolution; ++i) {
        for (int j = 0; j < resolution; ++j) {
            float x0 = start + j * step;
            float z0 = start + i * step;
            float x1 = x0 + step;
            float z1 = z0 + step;
            
            float y00 = getTerrainHeight(x0, z0);
            float y10 = getTerrainHeight(x1, z0);
            float y01 = getTerrainHeight(x0, z1);
            float y11 = getTerrainHeight(x1, z1);
            
            glm::vec3 n00 = getNormal(x0, z0);
            glm::vec3 n10 = getNormal(x1, z0);
            glm::vec3 n01 = getNormal(x0, z1);
            glm::vec3 n11 = getNormal(x1, z1);
            
            vertices.push_back({{x0, y00, z0}, {n00.x, n00.y, n00.z}, {(x0-start)/4.0f, (z0-start)/4.0f}});
            vertices.push_back({{x0, y01, z1}, {n01.x, n01.y, n01.z}, {(x0-start)/4.0f, (z1-start)/4.0f}});
            vertices.push_back({{x1, y11, z1}, {n11.x, n11.y, n11.z}, {(x1-start)/4.0f, (z1-start)/4.0f}});
            
            vertices.push_back({{x1, y11, z1}, {n11.x, n11.y, n11.z}, {(x1-start)/4.0f, (z1-start)/4.0f}});
            vertices.push_back({{x1, y10, z0}, {n10.x, n10.y, n10.z}, {(x1-start)/4.0f, (z0-start)/4.0f}});
            vertices.push_back({{x0, y00, z0}, {n00.x, n00.y, n00.z}, {(x0-start)/4.0f, (z0-start)/4.0f}});
        }
    }
    
    Mesh* mesh = new Mesh();
    mesh->setupMesh(vertices);
    return mesh;
}

Mesh* Mesh::createPlane(float size, float texTiling) {
    std::vector<Vertex> vertices = {
        // positions            // normals           // texcoords
        {{-size, 0.0f, -size}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-size, 0.0f,  size}, {0.0f, 1.0f, 0.0f}, {0.0f, texTiling}},
        {{ size, 0.0f,  size}, {0.0f, 1.0f, 0.0f}, {texTiling, texTiling}},

        {{ size, 0.0f,  size}, {0.0f, 1.0f, 0.0f}, {texTiling, texTiling}},
        {{ size, 0.0f, -size}, {0.0f, 1.0f, 0.0f}, {texTiling, 0.0f}},
        {{-size, 0.0f, -size}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}
    };
    Mesh* mesh = new Mesh();
    mesh->setupMesh(vertices);
    return mesh;
}
Mesh* Mesh::createCube(float w, float h, float d) {
    w/=2; h/=2; d/=2;
    std::vector<Vertex> vertices = {
        // Back face
        {{-w, -h, -d},  {0.0f,  0.0f, -1.0f},  {0.0f, 0.0f}}, // Bottom-left
        {{ w,  h, -d},  {0.0f,  0.0f, -1.0f},  {1.0f, 1.0f}}, // top-right
        {{ w, -h, -d},  {0.0f,  0.0f, -1.0f},  {1.0f, 0.0f}}, // bottom-right         
        {{ w,  h, -d},  {0.0f,  0.0f, -1.0f},  {1.0f, 1.0f}}, // top-right
        {{-w, -h, -d},  {0.0f,  0.0f, -1.0f},  {0.0f, 0.0f}}, // bottom-left
        {{-w,  h, -d},  {0.0f,  0.0f, -1.0f},  {0.0f, 1.0f}}, // top-left
        // Front face
        {{-w, -h,  d},  {0.0f,  0.0f,  1.0f},  {0.0f, 0.0f}}, // bottom-left
        {{ w, -h,  d},  {0.0f,  0.0f,  1.0f},  {1.0f, 0.0f}}, // bottom-right
        {{ w,  h,  d},  {0.0f,  0.0f,  1.0f},  {1.0f, 1.0f}}, // top-right
        {{ w,  h,  d},  {0.0f,  0.0f,  1.0f},  {1.0f, 1.0f}}, // top-right
        {{-w,  h,  d},  {0.0f,  0.0f,  1.0f},  {0.0f, 1.0f}}, // top-left
        {{-w, -h,  d},  {0.0f,  0.0f,  1.0f},  {0.0f, 0.0f}}, // bottom-left
        // Left face
        {{-w,  h,  d},  {-1.0f,  0.0f,  0.0f},  {1.0f, 0.0f}}, // top-right
        {{-w,  h, -d},  {-1.0f,  0.0f,  0.0f},  {1.0f, 1.0f}}, // top-left
        {{-w, -h, -d},  {-1.0f,  0.0f,  0.0f},  {0.0f, 1.0f}}, // bottom-left
        {{-w, -h, -d},  {-1.0f,  0.0f,  0.0f},  {0.0f, 1.0f}}, // bottom-left
        {{-w, -h,  d},  {-1.0f,  0.0f,  0.0f},  {0.0f, 0.0f}}, // bottom-right
        {{-w,  h,  d},  {-1.0f,  0.0f,  0.0f},  {1.0f, 0.0f}}, // top-right
        // Right face
        {{ w,  h,  d},  {1.0f,  0.0f,  0.0f},  {1.0f, 0.0f}}, // top-left
        {{ w, -h, -d},  {1.0f,  0.0f,  0.0f},  {0.0f, 1.0f}}, // bottom-right
        {{ w,  h, -d},  {1.0f,  0.0f,  0.0f},  {1.0f, 1.0f}}, // top-right         
        {{ w, -h, -d},  {1.0f,  0.0f,  0.0f},  {0.0f, 1.0f}}, // bottom-right
        {{ w,  h,  d},  {1.0f,  0.0f,  0.0f},  {1.0f, 0.0f}}, // top-left
        {{ w, -h,  d},  {1.0f,  0.0f,  0.0f},  {0.0f, 0.0f}}, // bottom-left     
        // Bottom face
        {{-w, -h, -d},  {0.0f, -1.0f,  0.0f},  {0.0f, 1.0f}}, // top-right
        {{ w, -h, -d},  {0.0f, -1.0f,  0.0f},  {1.0f, 1.0f}}, // top-left
        {{ w, -h,  d},  {0.0f, -1.0f,  0.0f},  {1.0f, 0.0f}}, // bottom-left
        {{ w, -h,  d},  {0.0f, -1.0f,  0.0f},  {1.0f, 0.0f}}, // bottom-left
        {{-w, -h,  d},  {0.0f, -1.0f,  0.0f},  {0.0f, 0.0f}}, // bottom-right
        {{-w, -h, -d},  {0.0f, -1.0f,  0.0f},  {0.0f, 1.0f}}, // top-right
        // Top face
        {{-w,  h, -d},  {0.0f,  1.0f,  0.0f},  {0.0f, 1.0f}}, // top-left
        {{ w,  h,  d},  {0.0f,  1.0f,  0.0f},  {1.0f, 0.0f}}, // bottom-right
        {{ w,  h, -d},  {0.0f,  1.0f,  0.0f},  {1.0f, 1.0f}}, // top-right     
        {{ w,  h,  d},  {0.0f,  1.0f,  0.0f},  {1.0f, 0.0f}}, // bottom-right
        {{-w,  h, -d},  {0.0f,  1.0f,  0.0f},  {0.0f, 1.0f}}, // top-left
        {{-w,  h,  d},  {0.0f,  1.0f,  0.0f},  {0.0f, 0.0f}}  // bottom-left        
    };
    Mesh* mesh = new Mesh();
    mesh->setupMesh(vertices);
    return mesh;
}

Mesh* Mesh::createCylinder(float radiusBottom, float radiusTop, float height, int sectors) {
    std::vector<Vertex> vertices;
    const float PI = 3.14159265359f;
    float dr = radiusBottom - radiusTop;
    float slantLen = std::sqrt(dr * dr + height * height);
    float ny = (slantLen > 0.0001f) ? (dr / slantLen) : 0.0f;
    float nr = (slantLen > 0.0001f) ? (height / slantLen) : 1.0f;

    for (int i = 0; i < sectors; ++i) {
        float a1 = (float)i * 2.0f * PI / (float)sectors;
        float a2 = (float)(i + 1) * 2.0f * PI / (float)sectors;

        float u1 = (float)i / (float)sectors;
        float u2 = (float)(i + 1) / (float)sectors;

        float cos1 = std::cos(a1), sin1 = std::sin(a1);
        float cos2 = std::cos(a2), sin2 = std::sin(a2);

        glm::vec3 b1(cos1 * radiusBottom, 0.0f, sin1 * radiusBottom);
        glm::vec3 b2(cos2 * radiusBottom, 0.0f, sin2 * radiusBottom);
        glm::vec3 t1(cos1 * radiusTop, height, sin1 * radiusTop);
        glm::vec3 t2(cos2 * radiusTop, height, sin2 * radiusTop);

        glm::vec3 n1(cos1 * nr, ny, sin1 * nr);
        glm::vec3 n2(cos2 * nr, ny, sin2 * nr);

        // Side wall: Triangle 1 (b1, t1, t2)
        vertices.push_back({{b1.x, b1.y, b1.z}, {n1.x, n1.y, n1.z}, {u1, 0.0f}});
        vertices.push_back({{t1.x, t1.y, t1.z}, {n1.x, n1.y, n1.z}, {u1, 1.0f}});
        vertices.push_back({{t2.x, t2.y, t2.z}, {n2.x, n2.y, n2.z}, {u2, 1.0f}});

        // Side wall: Triangle 2 (b1, t2, b2)
        vertices.push_back({{b1.x, b1.y, b1.z}, {n1.x, n1.y, n1.z}, {u1, 0.0f}});
        vertices.push_back({{t2.x, t2.y, t2.z}, {n2.x, n2.y, n2.z}, {u2, 1.0f}});
        vertices.push_back({{b2.x, b2.y, b2.z}, {n2.x, n2.y, n2.z}, {u2, 0.0f}});

        // Bottom cap
        if (radiusBottom > 0.0001f) {
            vertices.push_back({{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}});
            vertices.push_back({{b2.x, b2.y, b2.z}, {0.0f, -1.0f, 0.0f}, {0.5f + 0.5f * cos2, 0.5f + 0.5f * sin2}});
            vertices.push_back({{b1.x, b1.y, b1.z}, {0.0f, -1.0f, 0.0f}, {0.5f + 0.5f * cos1, 0.5f + 0.5f * sin1}});
        }

        // Top cap
        if (radiusTop > 0.0001f) {
            vertices.push_back({{0.0f, height, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f}});
            vertices.push_back({{t1.x, t1.y, t1.z}, {0.0f, 1.0f, 0.0f}, {0.5f + 0.5f * cos1, 0.5f + 0.5f * sin1}});
            vertices.push_back({{t2.x, t2.y, t2.z}, {0.0f, 1.0f, 0.0f}, {0.5f + 0.5f * cos2, 0.5f + 0.5f * sin2}});
        }
    }

    Mesh* mesh = new Mesh();
    mesh->setupMesh(vertices);
    return mesh;
}

Mesh* Mesh::createSphere(float radius, int sectors, int stacks) {
    std::vector<Vertex> vertices;
    const float PI = 3.14159265359f;

    for (int i = 0; i < stacks; ++i) {
        float phi1 = (float)i * PI / (float)stacks;
        float phi2 = (float)(i + 1) * PI / (float)stacks;

        float y1 = radius * std::cos(phi1);
        float r1 = radius * std::sin(phi1);
        float y2 = radius * std::cos(phi2);
        float r2 = radius * std::sin(phi2);

        float v1 = (float)i / (float)stacks;
        float v2 = (float)(i + 1) / (float)stacks;

        for (int j = 0; j < sectors; ++j) {
            float theta1 = (float)j * 2.0f * PI / (float)sectors;
            float theta2 = (float)(j + 1) * 2.0f * PI / (float)sectors;

            float u1 = (float)j / (float)sectors;
            float u2 = (float)(j + 1) / (float)sectors;

            glm::vec3 p1(r1 * std::sin(theta1), y1, r1 * std::cos(theta1));
            glm::vec3 p2(r1 * std::sin(theta2), y1, r1 * std::cos(theta2));
            glm::vec3 p3(r2 * std::sin(theta1), y2, r2 * std::cos(theta1));
            glm::vec3 p4(r2 * std::sin(theta2), y2, r2 * std::cos(theta2));

            glm::vec3 n1 = (radius > 0.0f) ? glm::normalize(p1) : glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 n2 = (radius > 0.0f) ? glm::normalize(p2) : glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 n3 = (radius > 0.0f) ? glm::normalize(p3) : glm::vec3(0.0f, -1.0f, 0.0f);
            glm::vec3 n4 = (radius > 0.0f) ? glm::normalize(p4) : glm::vec3(0.0f, -1.0f, 0.0f);

            if (i == 0) {
                // Top cap triangle
                vertices.push_back({{p1.x, p1.y, p1.z}, {n1.x, n1.y, n1.z}, {(u1 + u2) * 0.5f, v1}});
                vertices.push_back({{p3.x, p3.y, p3.z}, {n3.x, n3.y, n3.z}, {u1, v2}});
                vertices.push_back({{p4.x, p4.y, p4.z}, {n4.x, n4.y, n4.z}, {u2, v2}});
            } else if (i == stacks - 1) {
                // Bottom cap triangle
                vertices.push_back({{p1.x, p1.y, p1.z}, {n1.x, n1.y, n1.z}, {u1, v1}});
                vertices.push_back({{p3.x, p3.y, p3.z}, {n3.x, n3.y, n3.z}, {(u1 + u2) * 0.5f, v2}});
                vertices.push_back({{p2.x, p2.y, p2.z}, {n2.x, n2.y, n2.z}, {u2, v1}});
            } else {
                // Quad - 2 triangles
                vertices.push_back({{p1.x, p1.y, p1.z}, {n1.x, n1.y, n1.z}, {u1, v1}});
                vertices.push_back({{p3.x, p3.y, p3.z}, {n3.x, n3.y, n3.z}, {u1, v2}});
                vertices.push_back({{p4.x, p4.y, p4.z}, {n4.x, n4.y, n4.z}, {u2, v2}});

                vertices.push_back({{p1.x, p1.y, p1.z}, {n1.x, n1.y, n1.z}, {u1, v1}});
                vertices.push_back({{p4.x, p4.y, p4.z}, {n4.x, n4.y, n4.z}, {u2, v2}});
                vertices.push_back({{p2.x, p2.y, p2.z}, {n2.x, n2.y, n2.z}, {u2, v1}});
            }
        }
    }

    Mesh* mesh = new Mesh();
    mesh->setupMesh(vertices);
    return mesh;
}

Mesh* Mesh::createCone(float radius, float height, int sectors) {
    std::vector<Vertex> vertices;
    const float PI = 3.14159265359f;

    float slantLen = std::sqrt(radius * radius + height * height);
    float ny = (slantLen > 0.0001f) ? (radius / slantLen) : 0.0f;
    float nr = (slantLen > 0.0001f) ? (height / slantLen) : 1.0f;

    glm::vec3 apex(0.0f, height, 0.0f);

    for (int i = 0; i < sectors; ++i) {
        float a1 = (float)i * 2.0f * PI / (float)sectors;
        float a2 = (float)(i + 1) * 2.0f * PI / (float)sectors;

        float u1 = (float)i / (float)sectors;
        float u2 = (float)(i + 1) / (float)sectors;

        float cos1 = std::cos(a1), sin1 = std::sin(a1);
        float cos2 = std::cos(a2), sin2 = std::sin(a2);

        glm::vec3 b1(cos1 * radius, 0.0f, sin1 * radius);
        glm::vec3 b2(cos2 * radius, 0.0f, sin2 * radius);

        glm::vec3 n1(cos1 * nr, ny, sin1 * nr);
        glm::vec3 n2(cos2 * nr, ny, sin2 * nr);
        float aMid = (a1 + a2) * 0.5f;
        glm::vec3 nApex(std::cos(aMid) * nr, ny, std::sin(aMid) * nr);

        // Side triangle: apex -> b1 -> b2
        vertices.push_back({{apex.x, apex.y, apex.z}, {nApex.x, nApex.y, nApex.z}, {(u1 + u2) * 0.5f, 1.0f}});
        vertices.push_back({{b1.x, b1.y, b1.z}, {n1.x, n1.y, n1.z}, {u1, 0.0f}});
        vertices.push_back({{b2.x, b2.y, b2.z}, {n2.x, n2.y, n2.z}, {u2, 0.0f}});

        // Base cap: center -> b2 -> b1
        vertices.push_back({{0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}});
        vertices.push_back({{b2.x, b2.y, b2.z}, {0.0f, -1.0f, 0.0f}, {0.5f + 0.5f * cos2, 0.5f + 0.5f * sin2}});
        vertices.push_back({{b1.x, b1.y, b1.z}, {0.0f, -1.0f, 0.0f}, {0.5f + 0.5f * cos1, 0.5f + 0.5f * sin1}});
    }

    Mesh* mesh = new Mesh();
    mesh->setupMesh(vertices);
    return mesh;
}

Mesh* Mesh::createDisk(float radius, int sectors) {
    std::vector<Vertex> vertices;
    const float PI = 3.14159265359f;
    for (int i = 0; i < sectors; ++i) {
        float a1 = (float)i * 2.0f * PI / (float)sectors;
        float a2 = (float)(i + 1) * 2.0f * PI / (float)sectors;

        float cos1 = std::cos(a1), sin1 = std::sin(a1);
        float cos2 = std::cos(a2), sin2 = std::sin(a2);

        glm::vec3 b1(cos1 * radius, 0.0f, sin1 * radius);
        glm::vec3 b2(cos2 * radius, 0.0f, sin2 * radius);

        // Center -> b1 -> b2 with Normal (0, 1, 0)
        vertices.push_back({{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f}});
        vertices.push_back({{b1.x, b1.y, b1.z}, {0.0f, 1.0f, 0.0f}, {0.5f + 0.5f * cos1, 0.5f + 0.5f * sin1}});
        vertices.push_back({{b2.x, b2.y, b2.z}, {0.0f, 1.0f, 0.0f}, {0.5f + 0.5f * cos2, 0.5f + 0.5f * sin2}});
    }

    Mesh* mesh = new Mesh();
    mesh->setupMesh(vertices);
    return mesh;
}

Mesh* Mesh::createSkyDome(float radius, int sectors, int stacks) {
    std::vector<Vertex> vertices;
    const float PI = 3.14159265359f;

    for (int i = 0; i < stacks; ++i) {
        float phi1 = (float)i * (PI * 0.55f) / (float)stacks;
        float phi2 = (float)(i + 1) * (PI * 0.55f) / (float)stacks;

        float y1 = radius * std::cos(phi1);
        float r1 = radius * std::sin(phi1);
        float y2 = radius * std::cos(phi2);
        float r2 = radius * std::sin(phi2);

        float v1 = (float)i / (float)stacks;
        float v2 = (float)(i + 1) / (float)stacks;

        for (int j = 0; j < sectors; ++j) {
            float theta1 = (float)j * 2.0f * PI / (float)sectors;
            float theta2 = (float)(j + 1) * 2.0f * PI / (float)sectors;

            float u1 = (float)j / (float)sectors;
            float u2 = (float)(j + 1) / (float)sectors;

            glm::vec3 p1(r1 * std::sin(theta1), y1, r1 * std::cos(theta1));
            glm::vec3 p2(r1 * std::sin(theta2), y1, r1 * std::cos(theta2));
            glm::vec3 p3(r2 * std::sin(theta1), y2, r2 * std::cos(theta1));
            glm::vec3 p4(r2 * std::sin(theta2), y2, r2 * std::cos(theta2));

            // Inward-pointing normals for inside viewing
            glm::vec3 n1 = -glm::normalize(p1);
            glm::vec3 n2 = -glm::normalize(p2);
            glm::vec3 n3 = -glm::normalize(p3);
            glm::vec3 n4 = -glm::normalize(p4);

            // Winding order inverted for inside viewing
            vertices.push_back({{p1.x, p1.y, p1.z}, {n1.x, n1.y, n1.z}, {u1, v1}});
            vertices.push_back({{p4.x, p4.y, p4.z}, {n4.x, n4.y, n4.z}, {u2, v2}});
            vertices.push_back({{p3.x, p3.y, p3.z}, {n3.x, n3.y, n3.z}, {u1, v2}});

            vertices.push_back({{p1.x, p1.y, p1.z}, {n1.x, n1.y, n1.z}, {u1, v1}});
            vertices.push_back({{p2.x, p2.y, p2.z}, {n2.x, n2.y, n2.z}, {u2, v1}});
            vertices.push_back({{p4.x, p4.y, p4.z}, {n4.x, n4.y, n4.z}, {u2, v2}});
        }
    }

    Mesh* mesh = new Mesh();
    mesh->setupMesh(vertices);
    return mesh;
}

