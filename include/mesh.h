#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <vector>

struct Vertex {
    float Position[3];
    float Normal[3];
    float TexCoords[2];
};

class Mesh {
public:
    GLuint VAO, VBO;
    int vertexCount;

    Mesh();
    ~Mesh();
    void setupMesh(const std::vector<Vertex>& vertices);
    void draw() const;

    // Static generators
    static float getTerrainHeight(float x, float z);
    static Mesh* createTerrain(float size, int resolution);
    static Mesh* createPlane(float size, float texTiling);
    static Mesh* createCube(float width, float height, float depth);
    static Mesh* createCylinder(float radiusBottom, float radiusTop, float height, int sectors = 12);
    static Mesh* createSphere(float radius, int sectors = 12, int stacks = 8);
    static Mesh* createCone(float radius, float height, int sectors = 12);
    static Mesh* createDisk(float radius, int sectors = 24);
    static Mesh* createSkyDome(float radius = 100.0f, int sectors = 32, int stacks = 16);
};

#endif
