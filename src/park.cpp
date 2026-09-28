#include "park.h"
#include "texture.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>

Park::Park() {
    texGrass = 0; texBark = 0; texLeaves = 0; texAsphalt = 0;
    texSign = 0; texWater = 0; texRestaurant = 0; texShop = 0;
    standardShader = nullptr;
    uiShader = nullptr;
    skyShader = nullptr;
    groundMesh = nullptr;
    trunkMesh = nullptr;
    leafMesh = nullptr;
    boxMesh = nullptr;
    waterMesh = nullptr;
    uiMesh = nullptr;
    arrowMesh = nullptr;
    cylinderMesh = nullptr;
    columnMesh = nullptr;
    sphereMesh = nullptr;
    coneMesh = nullptr;
    diskMesh = nullptr;
    lakeMesh = nullptr;
    skyDomeMesh = nullptr;

    currentTimeOfDay = TIME_NOON;
    bikePos = glm::vec3(-12.0f, 0.0f, 15.0f);
    bikePos.y = Mesh::getTerrainHeight(bikePos.x, bikePos.z);
    bikeYaw = 45.0f;
    currentPrompt = "";

    // Generate massive amount of trees over a huge area
    generateTrees(1500, 750.0f);
}

void Park::nextTimeOfDay() {
    currentTimeOfDay = static_cast<TimeOfDay>((currentTimeOfDay + 1) % 3);
}

bool Park::checkNearBench(const glm::vec3& playerPos, glm::vec3& outSitPos, float& outSitYaw) {
    for (const auto& b : benchList) {
        if (glm::distance(playerPos, b.first) < 2.8f) {
            outSitPos = b.first;
            outSitYaw = 90.0f - glm::degrees(b.second);
            return true;
        }
    }
    return false;
}

bool Park::checkNearBicycle(const glm::vec3& playerPos) {
    return glm::distance(playerPos, bikePos) < 3.2f;
}

Park::~Park() {
    delete standardShader;
    delete uiShader;
    delete skyShader;
    delete groundMesh;
    delete trunkMesh;
    delete leafMesh;
    delete boxMesh;
    delete waterMesh;
    delete uiMesh;
    delete arrowMesh;
    delete cylinderMesh;
    delete columnMesh;
    delete sphereMesh;
    delete coneMesh;
    delete diskMesh;
    delete lakeMesh;
    delete skyDomeMesh;
}

void Park::init() {
    texGrass = loadTexture("assets/grass.jpg");
    texBark = loadTexture("assets/bark.jpg");
    texLeaves = loadTexture("assets/leaves.jpg");
    texAsphalt = loadTexture("assets/asphalt.jpg");
    texSign = loadTexture("assets/sign.jpg");
    texWater = loadTexture("assets/water.jpg");
    texRestaurant = loadTexture("assets/restaurant.jpg");
    texShop = loadTexture("assets/shop.jpg");

    standardShader = new Shader("assets/shaders/standard.vert", "assets/shaders/standard.frag");
    uiShader = new Shader("assets/shaders/ui.vert", "assets/shaders/ui.frag");
    skyShader = new Shader("assets/shaders/sky.vert", "assets/shaders/sky.frag");

    // 800x800 world size Terrain
    groundMesh = Mesh::createTerrain(800.0f, 200); 
    trunkMesh = Mesh::createCube(0.6f, 2.0f, 0.6f);
    leafMesh = Mesh::createCube(2.4f, 2.4f, 2.4f);
    boxMesh = Mesh::createCube(1.0f, 1.0f, 1.0f);
    waterMesh = Mesh::createPlane(15.0f, 6.0f); // 30x30m plane, tile 6x

    // Procedural organic meshes for trees & monuments:
    cylinderMesh = Mesh::createCylinder(0.5f, 0.35f, 1.0f, 24);
    columnMesh = Mesh::createCylinder(0.5f, 0.5f, 1.0f, 24);
    sphereMesh = Mesh::createSphere(1.0f, 16, 12);
    coneMesh = Mesh::createCone(1.0f, 1.0f, 24);
    diskMesh = Mesh::createDisk(1.0f, 32);
    lakeMesh = Mesh::createDisk(1.0f, 64);
    skyDomeMesh = Mesh::createSkyDome(250.0f, 32, 16);

    // Create UI Quad (from 0 to 1)
    std::vector<Vertex> uiVerts = {
        {{0.0f, 0.0f, 0.0f}, {0,0,1}, {0,0}},
        {{0.0f, 1.0f, 0.0f}, {0,0,1}, {0,1}},
        {{1.0f, 1.0f, 0.0f}, {0,0,1}, {1,1}},
        {{1.0f, 1.0f, 0.0f}, {0,0,1}, {1,1}},
        {{1.0f, 0.0f, 0.0f}, {0,0,1}, {1,0}},
        {{0.0f, 0.0f, 0.0f}, {0,0,1}, {0,0}}
    };
    uiMesh = new Mesh();
    uiMesh->setupMesh(uiVerts);

    // Create Arrow Triangle (pointing UP)
    std::vector<Vertex> arrowVerts = {
        {{0.0f, -0.5f, 0.0f}, {0,0,1}, {0.5f, 0.0f}}, 
        {{-0.5f, 0.5f, 0.0f}, {0,0,1}, {0.0f, 1.0f}}, 
        {{0.5f, 0.5f, 0.0f}, {0,0,1}, {1.0f, 1.0f}}   
    };
    arrowMesh = new Mesh();
    arrowMesh->setupMesh(arrowVerts);
}

void Park::generateTrees(int count, float areaSize) {
    trees.clear();
    int spawned = 0;
    while (spawned < count) {
        glm::vec3 pos;
        pos.x = ((float)rand() / (float)RAND_MAX) * areaSize - (areaSize / 2.0f);
        pos.z = ((float)rand() / (float)RAND_MAX) * areaSize - (areaSize / 2.0f);
        
        bool onPath = false;
        
        // Main Axes Path Check (width = 10m, clearance = 8m)
        if (std::abs(pos.x) < 8.0f) onPath = true; // Vertical Axis
        if (std::abs(pos.z) < 8.0f) onPath = true; // Horizontal Axis
        
        float dist = glm::length(glm::vec2(pos.x, pos.z));
        
        // Center Plaza (radius 40m + width 6m + clearance = 49m)
        if (dist < 50.0f && dist > 30.0f) onPath = true; 
        
        // Outer Ring (radius 250m)
        if (dist > 242.0f && dist < 258.0f) onPath = true;
        
        // Branch Path to Sign
        if (pos.x > 0.0f && pos.x < 90.0f && std::abs(pos.z + 80.0f) < 8.0f) onPath = true;
        // Sign Plaza itself
        if (glm::length(glm::vec2(pos.x - 80.0f, pos.z + 80.0f)) < 20.0f) onPath = true;
        
        // Pond Area
        if (glm::length(glm::vec2(pos.x + 100.0f, pos.z + 100.0f)) < 25.0f) onPath = true;

        // Restaurants
        if (glm::length(glm::vec2(pos.x - 250.0f, pos.z - 150.0f)) < 30.0f) onPath = true;
        if (glm::length(glm::vec2(pos.x + 100.0f, pos.z + 30.0f)) < 25.0f) onPath = true;
        
        // Shop
        if (glm::length(glm::vec2(pos.x - 50.0f, pos.z - 50.0f)) < 15.0f) onPath = true;

        // Kids Playground Area clearance (around -60, 60)
        if (glm::length(glm::vec2(pos.x - (-60.0f), pos.z - 60.0f)) < 32.0f) onPath = true;
        // Branch path to playground
        if (pos.x < 0.0f && pos.x > -65.0f && std::abs((pos.z - 60.0f) - (pos.x - (-60.0f))) < 8.0f) onPath = true;

        // Skip spawning if on a path or feature
        if (!onPath) {
            TreeInstance inst;
            inst.pos = pos;
            inst.pos.y = Mesh::getTerrainHeight(pos.x, pos.z);
            inst.scale = 0.85f + ((float)rand() / (float)RAND_MAX) * 0.4f; // 0.85x to 1.25x scale
            inst.rotationY = ((float)rand() / (float)RAND_MAX) * 6.283185f;

            int rType = rand() % 100;
            if (rType < 32) {
                // Classic Green Oak
                inst.type = TREE_OAK;
                inst.foliageColor = glm::vec3(0.95f + ((float)rand() / (float)RAND_MAX) * 0.1f,
                                              1.05f + ((float)rand() / (float)RAND_MAX) * 0.15f,
                                              0.9f);
            } else if (rType < 58) {
                // Conical Pine / Evergreen
                inst.type = TREE_PINE;
                inst.foliageColor = glm::vec3(0.5f, 0.82f, 0.62f);
            } else if (rType < 74) {
                // Autumn Maple
                inst.type = TREE_AUTUMN;
                if (rand() % 2 == 0) {
                    inst.foliageColor = glm::vec3(1.6f, 0.72f, 0.2f); // Golden amber
                } else {
                    inst.foliageColor = glm::vec3(1.5f, 0.35f, 0.25f); // Crimson red
                }
            } else if (rType < 88) {
                // Cherry Blossom
                inst.type = TREE_CHERRY;
                inst.foliageColor = glm::vec3(1.5f, 0.85f, 1.15f); // Soft blossom pink
            } else {
                // White Birch with spring lime leaves
                inst.type = TREE_BIRCH;
                inst.foliageColor = glm::vec3(0.85f, 1.35f, 0.55f);
            }

            trees.push_back(inst);
            spawned++;
        }
    }
}

void Park::drawSky(const glm::mat4& view, const glm::mat4& projection, float time) {
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    skyShader->use();
    skyShader->setMat4("view", view);
    skyShader->setMat4("projection", projection);
    skyShader->setFloat("time", time);
    skyShader->setInt("timeOfDay", static_cast<int>(currentTimeOfDay));

    glm::vec3 lightDir;
    glm::vec3 sunColor;
    glm::vec3 zenithColor;
    glm::vec3 horizonColor;

    if (currentTimeOfDay == TIME_NOON) {
        lightDir = glm::normalize(glm::vec3(50.0f, 120.0f, 50.0f));
        sunColor = glm::vec3(1.15f, 1.05f, 0.9f);
        zenithColor = glm::vec3(0.16f, 0.42f, 0.82f);   // Rich deep azure
        horizonColor = glm::vec3(0.68f, 0.84f, 0.98f);  // Soft horizon haze
    } else if (currentTimeOfDay == TIME_SUNSET) {
        lightDir = glm::normalize(glm::vec3(150.0f, 25.0f, 100.0f));
        sunColor = glm::vec3(1.3f, 0.58f, 0.22f);
        zenithColor = glm::vec3(0.24f, 0.16f, 0.46f);   // Twilight violet/indigo
        horizonColor = glm::vec3(0.98f, 0.46f, 0.18f);  // Fiery golden hour
    } else { // TIME_NIGHT
        lightDir = glm::normalize(glm::vec3(10.0f, 60.0f, 10.0f));
        sunColor = glm::vec3(0.85f, 0.92f, 1.0f);       // Silvery moon
        zenithColor = glm::vec3(0.015f, 0.02f, 0.05f);  // Midnight navy
        horizonColor = glm::vec3(0.05f, 0.07f, 0.14f);  // Low night sky
    }

    skyShader->setVec3("lightDir", lightDir);
    skyShader->setVec3("sunColor", sunColor);
    skyShader->setVec3("zenithColor", zenithColor);
    skyShader->setVec3("horizonColor", horizonColor);

    skyDomeMesh->draw();

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void Park::drawCurbStraight(glm::vec3 start, glm::vec3 end, float width) {
    glm::vec3 dir = end - start;
    float length = glm::length(dir);
    if (length < 0.001f) return;
    float angle = atan2(dir.x, dir.z);
    glm::vec3 mid = (start + end) / 2.0f;
    float halfW = width * 0.5f;

    standardShader->use();
    standardShader->setBool("useTexture", false);
    standardShader->setVec3("solidColor", glm::vec3(0.72f, 0.70f, 0.67f)); // Light granite curb
    standardShader->setFloat("emissive", 0.0f);

    glm::vec3 perp(cos(angle), 0.0f, -sin(angle));

    // Left curb
    glm::vec3 leftPos = mid - perp * halfW;
    glm::mat4 m1 = glm::translate(glm::mat4(1.0f), glm::vec3(leftPos.x, 0.05f, leftPos.z));
    m1 = glm::rotate(m1, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    m1 = glm::scale(m1, glm::vec3(0.24f, 0.10f, length));
    standardShader->setMat4("model", m1);
    boxMesh->draw();

    // Right curb
    glm::vec3 rightPos = mid + perp * halfW;
    glm::mat4 m2 = glm::translate(glm::mat4(1.0f), glm::vec3(rightPos.x, 0.05f, rightPos.z));
    m2 = glm::rotate(m2, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    m2 = glm::scale(m2, glm::vec3(0.24f, 0.10f, length));
    standardShader->setMat4("model", m2);
    boxMesh->draw();
}

void Park::drawCurbCurve(glm::vec3 center, float radius, float startAngle, float endAngle, float width) {
    int segments = 48;
    float angleStep = (endAngle - startAngle) / (float)segments;
    float halfW = width * 0.5f;

    standardShader->use();
    standardShader->setBool("useTexture", false);
    standardShader->setVec3("solidColor", glm::vec3(0.72f, 0.70f, 0.67f));
    standardShader->setFloat("emissive", 0.0f);

    for (int i = 0; i < segments; i++) {
        float midA = startAngle + (i + 0.5f) * angleStep;
        float segLen = radius * angleStep;

        // Inner curb
        float rInner = radius - halfW;
        glm::vec3 posIn = center + glm::vec3(sin(midA) * rInner, 0.05f, cos(midA) * rInner);
        glm::mat4 m1 = glm::translate(glm::mat4(1.0f), posIn);
        m1 = glm::rotate(m1, midA + glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m1 = glm::scale(m1, glm::vec3(segLen * 1.06f, 0.10f, 0.24f));
        standardShader->setMat4("model", m1);
        boxMesh->draw();

        // Outer curb
        float rOuter = radius + halfW;
        glm::vec3 posOut = center + glm::vec3(sin(midA) * rOuter, 0.05f, cos(midA) * rOuter);
        glm::mat4 m2 = glm::translate(glm::mat4(1.0f), posOut);
        m2 = glm::rotate(m2, midA + glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        m2 = glm::scale(m2, glm::vec3(segLen * 1.06f, 0.10f, 0.24f));
        standardShader->setMat4("model", m2);
        boxMesh->draw();
    }
}

void Park::drawPlazaTerrace() {
    standardShader->use();
    standardShader->setFloat("emissive", 0.0f);

    // Granite foundation terrace disc (raised 0.12m)
    standardShader->setBool("useTexture", false);
    standardShader->setVec3("solidColor", glm::vec3(0.76f, 0.74f, 0.70f)); // French limestone / light granite
    glm::mat4 tm = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.06f, 0.0f));
    tm = glm::scale(tm, glm::vec3(52.0f, 0.12f, 52.0f)); // Diameter 52m (R = 26m)
    standardShader->setMat4("model", tm);
    cylinderMesh->draw();

    // Cobblestone / Paver infield floor
    standardShader->setBool("useTexture", true);
    standardShader->setVec2("texScale", glm::vec2(12.0f, 12.0f));
    standardShader->setVec3("solidColor", glm::vec3(0.88f, 0.86f, 0.82f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texAsphalt);
    glm::mat4 pm = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.13f, 0.0f));
    pm = glm::scale(pm, glm::vec3(24.8f, 1.0f, 24.8f));
    standardShader->setMat4("model", pm);
    diskMesh->draw();

    standardShader->setBool("useTexture", false);
    standardShader->setVec2("texScale", glm::vec2(1.0f, 1.0f));

    // Outer decorative stone ring curb
    standardShader->setVec3("solidColor", glm::vec3(0.68f, 0.66f, 0.62f));
    glm::mat4 cm = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.14f, 0.0f));
    cm = glm::scale(cm, glm::vec3(52.4f, 0.16f, 52.4f));
    standardShader->setMat4("model", cm);
    cylinderMesh->draw();
}

void Park::drawPathStraight(glm::vec3 start, glm::vec3 end, float width) {
    glm::vec3 dir = end - start;
    float length = glm::length(dir);
    if (length < 0.001f) return;
    
    float angle = atan2(dir.x, dir.z);
    glm::vec3 mid = (start + end) / 2.0f;
    
    // Base Asphalt
    standardShader->use();
    standardShader->setBool("useTexture", true);
    standardShader->setVec2("texScale", glm::vec2(1.0f, length / 4.0f));
    standardShader->setVec3("solidColor", glm::vec3(1.0f, 1.0f, 1.0f));
    glBindTexture(GL_TEXTURE_2D, texAsphalt);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(mid.x, 0.02f, mid.z));
    model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(width, 0.05f, length));
    standardShader->setMat4("model", model);
    boxMesh->draw();

    standardShader->setVec2("texScale", glm::vec2(1.0f, 1.0f));
    standardShader->setBool("useTexture", false);

    // Stone Curbs along path edges
    drawCurbStraight(start, end, width);
}

void Park::drawPathCurve(glm::vec3 center, float radius, float startAngle, float endAngle, float width) {
    int segments = 64; 
    float angleStep = (endAngle - startAngle) / (float)segments;

    standardShader->use();
    standardShader->setBool("useTexture", true);
    glBindTexture(GL_TEXTURE_2D, texAsphalt);
    standardShader->setVec3("solidColor", glm::vec3(1.0f, 1.0f, 1.0f));

    for (int i = 0; i < segments; i++) {
        float a1 = startAngle + i * angleStep;
        float a2 = a1 + angleStep;
        
        float midA = (a1 + a2) / 2.0f;
        glm::vec3 pos = center + glm::vec3(sin(midA) * radius, 0.02f, cos(midA) * radius);
        
        float segLength = radius * angleStep; 

        // Draw Asphalt segment
        standardShader->setBool("useTexture", true);
        standardShader->setVec2("texScale", glm::vec2(1.0f, 1.0f));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, midA + glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(segLength * 1.05f, 0.05f, width)); 
        standardShader->setMat4("model", model);
        boxMesh->draw();
    }

    standardShader->setBool("useTexture", false);

    // Curved Stone Curbs along path edges
    drawCurbCurve(center, radius, startAngle, endAngle, width);
}

void Park::drawBuilding(glm::vec3 pos) {
    standardShader->setBool("useTexture", false);
    
    // Main walls (Warm Beige / Stone)
    standardShader->setVec3("solidColor", glm::vec3(0.85f, 0.8f, 0.7f));
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 2.0f, 0.0f));
    m = glm::scale(m, glm::vec3(8.0f, 4.0f, 6.0f));
    standardShader->setMat4("model", m); boxMesh->draw();

    // Dark slate roof
    standardShader->setVec3("solidColor", glm::vec3(0.2f, 0.2f, 0.25f));
    m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 4.5f, 0.0f));
    m = glm::scale(m, glm::vec3(9.0f, 1.0f, 7.0f));
    standardShader->setMat4("model", m); boxMesh->draw();

    // Wooden Door
    standardShader->setVec3("solidColor", glm::vec3(0.4f, 0.25f, 0.1f));
    m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(0.0f, 1.25f, -3.01f));
    m = glm::scale(m, glm::vec3(1.5f, 2.5f, 0.1f));
    standardShader->setMat4("model", m); boxMesh->draw();
    
    // Window (Glass Blue)
    standardShader->setVec3("solidColor", glm::vec3(0.3f, 0.6f, 0.9f));
    float faceX = (pos.x > 0) ? -4.01f : 4.01f; 
    m = glm::translate(glm::mat4(1.0f), pos + glm::vec3(faceX, 2.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.1f, 1.5f, 3.0f));
    standardShader->setMat4("model", m); boxMesh->draw();
}

void Park::drawSign(glm::vec3 pos, float rotationY) {
    standardShader->setBool("useTexture", false);
    
    auto drawBox = [&](float dx, float dy, float sx, float sy, glm::vec3 color) {
        standardShader->setVec3("solidColor", color);
        glm::mat4 m = glm::translate(glm::mat4(1.0f), pos);
        m = glm::rotate(m, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::translate(m, glm::vec3(dx - 18.0f, dy, 0.0f)); // Center the 36m wide sign
        m = glm::scale(m, glm::vec3(sx, sy, 1.0f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    };

    glm::vec3 blk = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 red = glm::vec3(0.9f, 0.15f, 0.15f);

    float px = 0.0f; 
    
    // I
    drawBox(px, 5.0f, 3.0f, 1.0f, blk);
    drawBox(px, 3.0f, 1.0f, 3.0f, blk);
    drawBox(px, 1.0f, 3.0f, 1.0f, blk);
    px += 4.0f;

    // Heart (Red)
    drawBox(px - 1.0f, 4.0f, 1.5f, 1.5f, red);
    drawBox(px + 1.0f, 4.0f, 1.5f, 1.5f, red);
    drawBox(px, 2.5f, 3.5f, 2.5f, red);
    drawBox(px, 1.0f, 1.5f, 1.5f, red);
    px += 5.0f;

    // E
    drawBox(px - 1.0f, 3.0f, 1.0f, 5.0f, blk);
    drawBox(px, 5.0f, 2.0f, 1.0f, blk);
    drawBox(px, 3.0f, 2.0f, 1.0f, blk);
    drawBox(px, 1.0f, 2.0f, 1.0f, blk);
    px += 4.0f;

    // T
    drawBox(px, 5.0f, 3.0f, 1.0f, blk);
    drawBox(px, 2.5f, 1.0f, 4.0f, blk);
    px += 4.0f;

    // H
    drawBox(px - 1.0f, 3.0f, 1.0f, 5.0f, blk);
    drawBox(px + 1.0f, 3.0f, 1.0f, 5.0f, blk);
    drawBox(px, 3.0f, 1.0f, 1.0f, blk);
    px += 4.0f;

    // I
    drawBox(px, 5.0f, 3.0f, 1.0f, blk);
    drawBox(px, 3.0f, 1.0f, 3.0f, blk);
    drawBox(px, 1.0f, 3.0f, 1.0f, blk);
    px += 4.0f;

    // O
    drawBox(px - 1.0f, 3.0f, 1.0f, 5.0f, blk);
    drawBox(px + 1.0f, 3.0f, 1.0f, 5.0f, blk);
    drawBox(px, 5.0f, 1.0f, 1.0f, blk);
    drawBox(px, 1.0f, 1.0f, 1.0f, blk);
    px += 4.0f;

    // P
    drawBox(px - 1.0f, 3.0f, 1.0f, 5.0f, blk);
    drawBox(px + 0.5f, 5.0f, 2.0f, 1.0f, blk);
    drawBox(px + 0.5f, 3.0f, 2.0f, 1.0f, blk);
    drawBox(px + 1.0f, 4.0f, 1.0f, 1.0f, blk);
    px += 4.0f;

    // I
    drawBox(px, 5.0f, 3.0f, 1.0f, blk);
    drawBox(px, 3.0f, 1.0f, 3.0f, blk);
    drawBox(px, 1.0f, 3.0f, 1.0f, blk);
    px += 4.0f;

    // A
    drawBox(px - 1.0f, 2.5f, 1.0f, 4.0f, blk);
    drawBox(px + 1.0f, 2.5f, 1.0f, 4.0f, blk);
    drawBox(px, 5.0f, 3.0f, 1.0f, blk);
    drawBox(px, 3.0f, 1.0f, 1.0f, blk);
}

void Park::drawFence(float size) {
    standardShader->setBool("useTexture", false);
    standardShader->setVec3("solidColor", glm::vec3(0.3f, 0.3f, 0.35f));
    
    float poleSpacing = 4.0f;
    float height = 2.5f;
    glm::mat4 m;
    
    for (float z = -size; z <= size; z += poleSpacing) {
        m = glm::translate(glm::mat4(1.0f), glm::vec3(-size, height/2.0f, z));
        m = glm::scale(m, glm::vec3(0.15f, height, 0.15f));
        standardShader->setMat4("model", m); boxMesh->draw();
        
        m = glm::translate(glm::mat4(1.0f), glm::vec3(size, height/2.0f, z));
        m = glm::scale(m, glm::vec3(0.15f, height, 0.15f));
        standardShader->setMat4("model", m); boxMesh->draw();
    }

    for (float x = -size; x <= size; x += poleSpacing) {
        if (std::abs(x) > 10.0f) { // Leave a 20m gap in the center for entrances
            m = glm::translate(glm::mat4(1.0f), glm::vec3(x, height/2.0f, -size));
            m = glm::scale(m, glm::vec3(0.15f, height, 0.15f));
            standardShader->setMat4("model", m); boxMesh->draw();
            
            m = glm::translate(glm::mat4(1.0f), glm::vec3(x, height/2.0f, size));
            m = glm::scale(m, glm::vec3(0.15f, height, 0.15f));
            standardShader->setMat4("model", m); boxMesh->draw();
        }
    }
    
    float railThickness = 0.1f;
    m = glm::translate(glm::mat4(1.0f), glm::vec3(-size, height - 0.2f, 0.0f));
    m = glm::scale(m, glm::vec3(railThickness, railThickness, size * 2.0f));
    standardShader->setMat4("model", m); boxMesh->draw();
    
    m = glm::translate(glm::mat4(1.0f), glm::vec3(-size, height / 2.0f, 0.0f));
    m = glm::scale(m, glm::vec3(railThickness, railThickness, size * 2.0f));
    standardShader->setMat4("model", m); boxMesh->draw();
    
    m = glm::translate(glm::mat4(1.0f), glm::vec3(size, height - 0.2f, 0.0f));
    m = glm::scale(m, glm::vec3(railThickness, railThickness, size * 2.0f));
    standardShader->setMat4("model", m); boxMesh->draw();
    
    m = glm::translate(glm::mat4(1.0f), glm::vec3(size, height / 2.0f, 0.0f));
    m = glm::scale(m, glm::vec3(railThickness, railThickness, size * 2.0f));
    standardShader->setMat4("model", m); boxMesh->draw();

    // Front edge rails (z = -size) - Split into two
    float segLen = size - 10.0f;
    float segL = -size + segLen/2.0f;
    float segR = 10.0f + segLen/2.0f;

    m = glm::translate(glm::mat4(1.0f), glm::vec3(segL, height - 0.2f, -size));
    m = glm::scale(m, glm::vec3(segLen, railThickness, railThickness));
    standardShader->setMat4("model", m); boxMesh->draw();
    m = glm::translate(glm::mat4(1.0f), glm::vec3(segL, height / 2.0f, -size));
    m = glm::scale(m, glm::vec3(segLen, railThickness, railThickness));
    standardShader->setMat4("model", m); boxMesh->draw();
    
    m = glm::translate(glm::mat4(1.0f), glm::vec3(segR, height - 0.2f, -size));
    m = glm::scale(m, glm::vec3(segLen, railThickness, railThickness));
    standardShader->setMat4("model", m); boxMesh->draw();
    m = glm::translate(glm::mat4(1.0f), glm::vec3(segR, height / 2.0f, -size));
    m = glm::scale(m, glm::vec3(segLen, railThickness, railThickness));
    standardShader->setMat4("model", m); boxMesh->draw();
    
    // Back edge rails (z = size) - Split into two
    m = glm::translate(glm::mat4(1.0f), glm::vec3(segL, height - 0.2f, size));
    m = glm::scale(m, glm::vec3(segLen, railThickness, railThickness));
    standardShader->setMat4("model", m); boxMesh->draw();
    m = glm::translate(glm::mat4(1.0f), glm::vec3(segL, height / 2.0f, size));
    m = glm::scale(m, glm::vec3(segLen, railThickness, railThickness));
    standardShader->setMat4("model", m); boxMesh->draw();
    
    m = glm::translate(glm::mat4(1.0f), glm::vec3(segR, height - 0.2f, size));
    m = glm::scale(m, glm::vec3(segLen, railThickness, railThickness));
    standardShader->setMat4("model", m); boxMesh->draw();
    m = glm::translate(glm::mat4(1.0f), glm::vec3(segR, height / 2.0f, size));
    m = glm::scale(m, glm::vec3(segLen, railThickness, railThickness));
    standardShader->setMat4("model", m); boxMesh->draw();
}

void Park::drawTree(const TreeInstance& tree) {
    standardShader->use();
    standardShader->setBool("useTexture", true);
    glActiveTexture(GL_TEXTURE0);

    glm::mat4 base = glm::mat4(1.0f);
    base = glm::translate(base, tree.pos);
    base = glm::rotate(base, tree.rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
    base = glm::scale(base, glm::vec3(tree.scale));

    // Common Root Buttress Wedges grounding the tree naturally into terrain
    auto drawRootFlare = [&](float trunkRad, float barkScaleY, glm::vec3 barkCol) {
        glBindTexture(GL_TEXTURE_2D, texBark);
        standardShader->setVec2("texScale", glm::vec2(1.0f, barkScaleY));
        standardShader->setVec3("solidColor", barkCol);
        for (int r = 0; r < 4; ++r) {
            float rot = glm::radians(r * 90.0f + 18.0f);
            glm::mat4 rm = glm::rotate(base, rot, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 m = glm::translate(rm, glm::vec3(trunkRad * 0.7f, 0.12f, 0.0f));
            m = glm::rotate(m, glm::radians(32.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            m = glm::scale(m, glm::vec3(trunkRad * 0.55f, 0.55f, trunkRad * 0.45f));
            standardShader->setMat4("model", m);
            cylinderMesh->draw();
        }
    };

    if (tree.type == TREE_PINE) {
        // Pine Trunk (dark timber, tapered round cylinder with root flare)
        drawRootFlare(0.55f, 2.0f, glm::vec3(0.55f, 0.42f, 0.32f));
        glBindTexture(GL_TEXTURE_2D, texBark);
        standardShader->setVec2("texScale", glm::vec2(1.0f, 2.5f));
        standardShader->setVec3("solidColor", glm::vec3(0.55f, 0.42f, 0.32f));
        glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(0.52f, 4.2f, 0.52f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();

        // 4 Conical downward-sloping tiers of Pine foliage
        glBindTexture(GL_TEXTURE_2D, texLeaves);
        standardShader->setVec2("texScale", glm::vec2(2.5f, 1.5f));

        // Tier 1 (Lowest & Broadest)
        standardShader->setVec3("solidColor", tree.foliageColor * 0.85f);
        m = glm::translate(base, glm::vec3(0.0f, 1.5f, 0.0f));
        m = glm::scale(m, glm::vec3(2.6f, 1.8f, 2.6f));
        standardShader->setMat4("model", m);
        coneMesh->draw();

        // Tier 2
        standardShader->setVec3("solidColor", tree.foliageColor * 0.95f);
        m = glm::translate(base, glm::vec3(0.0f, 2.6f, 0.0f));
        m = glm::scale(m, glm::vec3(2.1f, 1.8f, 2.1f));
        standardShader->setMat4("model", m);
        coneMesh->draw();

        // Tier 3
        standardShader->setVec3("solidColor", tree.foliageColor * 1.05f);
        m = glm::translate(base, glm::vec3(0.0f, 3.7f, 0.0f));
        m = glm::scale(m, glm::vec3(1.5f, 1.7f, 1.5f));
        standardShader->setMat4("model", m);
        coneMesh->draw();

        // Tier 4 (Spire apex tip)
        standardShader->setVec3("solidColor", tree.foliageColor * 1.15f);
        m = glm::translate(base, glm::vec3(0.0f, 4.7f, 0.0f));
        m = glm::scale(m, glm::vec3(0.95f, 1.9f, 0.95f));
        standardShader->setMat4("model", m);
        coneMesh->draw();
    } else if (tree.type == TREE_BIRCH) {
        // Pale birch trunk with root buttresses
        drawRootFlare(0.42f, 3.0f, glm::vec3(1.5f, 1.48f, 1.45f));
        glBindTexture(GL_TEXTURE_2D, texBark);
        standardShader->setVec2("texScale", glm::vec2(1.0f, 3.5f));
        standardShader->setVec3("solidColor", glm::vec3(1.6f, 1.55f, 1.5f)); // White birch bark
        glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(0.38f, 4.4f, 0.38f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();

        // Slender birch canopy with volume layers
        glBindTexture(GL_TEXTURE_2D, texLeaves);
        standardShader->setVec2("texScale", glm::vec2(2.0f, 2.0f));

        standardShader->setVec3("solidColor", tree.foliageColor * 0.88f);
        m = glm::translate(base, glm::vec3(0.0f, 3.2f, 0.0f));
        m = glm::scale(m, glm::vec3(1.4f, 1.6f, 1.3f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();

        standardShader->setVec3("solidColor", tree.foliageColor * 1.0f);
        m = glm::translate(base, glm::vec3(0.15f, 4.2f, -0.1f));
        m = glm::scale(m, glm::vec3(1.1f, 1.5f, 1.1f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();

        standardShader->setVec3("solidColor", tree.foliageColor * 1.12f);
        m = glm::translate(base, glm::vec3(-0.15f, 5.0f, 0.1f));
        m = glm::scale(m, glm::vec3(0.85f, 1.3f, 0.85f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();
    } else if (tree.type == TREE_CHERRY) {
        // Cherry Blossom with warm trunk, root buttresses, and spreading branches
        drawRootFlare(0.58f, 1.8f, glm::vec3(0.55f, 0.42f, 0.38f));
        glBindTexture(GL_TEXTURE_2D, texBark);
        standardShader->setVec2("texScale", glm::vec2(1.0f, 1.8f));
        standardShader->setVec3("solidColor", glm::vec3(0.55f, 0.42f, 0.38f));

        glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(0.6f, 2.4f, 0.6f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();

        // 2 Branch boughs
        m = glm::translate(base, glm::vec3(0.35f, 2.0f, 0.2f));
        m = glm::rotate(m, glm::radians(35.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m = glm::scale(m, glm::vec3(0.22f, 1.2f, 0.22f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();

        m = glm::translate(base, glm::vec3(-0.35f, 2.1f, -0.2f));
        m = glm::rotate(m, glm::radians(-32.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m = glm::scale(m, glm::vec3(0.22f, 1.1f, 0.22f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();

        // Horizontal pillowy blossom canopy
        glBindTexture(GL_TEXTURE_2D, texLeaves);
        standardShader->setVec2("texScale", glm::vec2(2.0f, 2.0f));

        // Core dome
        standardShader->setVec3("solidColor", tree.foliageColor * 0.92f);
        m = glm::translate(base, glm::vec3(0.0f, 2.8f, 0.0f));
        m = glm::scale(m, glm::vec3(2.2f, 1.15f, 2.1f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();

        // Left bough
        standardShader->setVec3("solidColor", tree.foliageColor * 1.02f);
        m = glm::translate(base, glm::vec3(-1.1f, 2.7f, 0.5f));
        m = glm::scale(m, glm::vec3(1.5f, 0.95f, 1.4f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();

        // Right bough
        standardShader->setVec3("solidColor", tree.foliageColor * 1.05f);
        m = glm::translate(base, glm::vec3(1.0f, 2.8f, -0.5f));
        m = glm::scale(m, glm::vec3(1.6f, 1.0f, 1.5f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();

        // Top crown
        standardShader->setVec3("solidColor", tree.foliageColor * 1.12f);
        m = glm::translate(base, glm::vec3(0.05f, 3.5f, 0.0f));
        m = glm::scale(m, glm::vec3(1.4f, 1.05f, 1.4f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();
    } else {
        // Deciduous: Oak & Autumn Maple with root flares, scaffold branches, and wide horizontal boughs
        drawRootFlare(0.68f, 2.0f, glm::vec3(0.65f, 0.55f, 0.45f));
        glBindTexture(GL_TEXTURE_2D, texBark);
        standardShader->setVec2("texScale", glm::vec2(1.0f, 2.0f));
        standardShader->setVec3("solidColor", glm::vec3(0.68f, 0.58f, 0.48f));

        glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(0.65f, 2.6f, 0.65f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();

        // 3 Visible branching boughs extending into the canopy
        m = glm::translate(base, glm::vec3(0.35f, 2.2f, 0.25f));
        m = glm::rotate(m, glm::radians(28.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m = glm::scale(m, glm::vec3(0.24f, 1.3f, 0.24f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();

        m = glm::translate(base, glm::vec3(-0.35f, 2.3f, -0.2f));
        m = glm::rotate(m, glm::radians(-30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m = glm::scale(m, glm::vec3(0.24f, 1.2f, 0.24f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();

        m = glm::translate(base, glm::vec3(-0.1f, 2.6f, 0.35f));
        m = glm::rotate(m, glm::radians(24.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(0.22f, 1.1f, 0.22f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();

        // Broad spreading horizontal leaf boughs with volume depth shading
        glBindTexture(GL_TEXTURE_2D, texLeaves);
        standardShader->setVec2("texScale", glm::vec2(2.0f, 2.0f));

        // Core volume (deep shadow green)
        standardShader->setVec3("solidColor", tree.foliageColor * 0.85f);
        m = glm::translate(base, glm::vec3(0.0f, 3.2f, 0.0f));
        m = glm::scale(m, glm::vec3(2.3f, 1.25f, 2.2f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();

        // East bough
        standardShader->setVec3("solidColor", tree.foliageColor * 0.98f);
        m = glm::translate(base, glm::vec3(1.0f, 3.0f, 0.45f));
        m = glm::scale(m, glm::vec3(1.7f, 1.05f, 1.6f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();

        // West bough
        standardShader->setVec3("solidColor", tree.foliageColor * 0.96f);
        m = glm::translate(base, glm::vec3(-0.95f, 3.1f, -0.4f));
        m = glm::scale(m, glm::vec3(1.65f, 1.05f, 1.6f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();

        // Front / South bough
        standardShader->setVec3("solidColor", tree.foliageColor * 1.04f);
        m = glm::translate(base, glm::vec3(0.2f, 2.9f, 0.9f));
        m = glm::scale(m, glm::vec3(1.5f, 0.95f, 1.5f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();

        // Top crown bough (brightest sunlit highlights)
        standardShader->setVec3("solidColor", tree.foliageColor * 1.12f);
        m = glm::translate(base, glm::vec3(-0.05f, 4.0f, 0.0f));
        m = glm::scale(m, glm::vec3(1.75f, 1.2f, 1.75f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();
    }

    standardShader->setVec2("texScale", glm::vec2(1.0f, 1.0f));
    standardShader->setVec3("solidColor", glm::vec3(1.0f, 1.0f, 1.0f));
}

void Park::drawBench(glm::vec3 position, float rotationY, bool registerBench) {
    if (registerBench) {
        benchList.push_back({position, rotationY});
    }

    standardShader->use();
    standardShader->setBool("useTexture", false);
    standardShader->setFloat("emissive", 0.0f);

    glm::mat4 base = glm::mat4(1.0f);
    base = glm::translate(base, position);
    base = glm::rotate(base, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));

    // Dark iron side legs & armrests
    standardShader->setVec3("solidColor", glm::vec3(0.12f, 0.12f, 0.14f));

    // Left leg assembly
    glm::mat4 m = glm::translate(base, glm::vec3(-0.9f, 0.22f, 0.0f));
    m = glm::scale(m, glm::vec3(0.08f, 0.44f, 0.45f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Right leg assembly
    m = glm::translate(base, glm::vec3(0.9f, 0.22f, 0.0f));
    m = glm::scale(m, glm::vec3(0.08f, 0.44f, 0.45f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Left armrest
    m = glm::translate(base, glm::vec3(-0.9f, 0.55f, 0.05f));
    m = glm::scale(m, glm::vec3(0.08f, 0.06f, 0.5f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Right armrest
    m = glm::translate(base, glm::vec3(0.9f, 0.55f, 0.05f));
    m = glm::scale(m, glm::vec3(0.08f, 0.06f, 0.5f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Teak wooden slats for Seat & Backrest
    standardShader->setVec3("solidColor", glm::vec3(0.5f, 0.28f, 0.12f));

    // Seat slats (3 horizontal wood boards)
    for (int s = 0; s < 3; ++s) {
        float zOff = -0.15f + s * 0.16f;
        m = glm::translate(base, glm::vec3(0.0f, 0.44f, zOff));
        m = glm::scale(m, glm::vec3(2.0f, 0.04f, 0.13f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // Backrest slats (2 horizontal wood boards)
    m = glm::translate(base, glm::vec3(0.0f, 0.68f, -0.18f));
    m = glm::scale(m, glm::vec3(2.0f, 0.12f, 0.04f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(base, glm::vec3(0.0f, 0.84f, -0.2f));
    m = glm::scale(m, glm::vec3(2.0f, 0.12f, 0.04f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
}

void Park::drawLampPost(glm::vec3 position, bool lightsOn) {
    standardShader->use();
    standardShader->setBool("useTexture", false);
    standardShader->setFloat("emissive", 0.0f);

    glm::mat4 base = glm::translate(glm::mat4(1.0f), position);

    // Cast iron dark base & post
    standardShader->setVec3("solidColor", glm::vec3(0.15f, 0.15f, 0.17f));

    // Base collar
    glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.25f, 0.0f));
    m = glm::scale(m, glm::vec3(0.55f, 0.5f, 0.55f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Tall pole
    m = glm::translate(base, glm::vec3(0.0f, 2.2f, 0.0f));
    m = glm::scale(m, glm::vec3(0.18f, 3.6f, 0.18f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Decorative top bracket
    m = glm::translate(base, glm::vec3(0.0f, 4.05f, 0.0f));
    m = glm::scale(m, glm::vec3(0.45f, 0.1f, 0.45f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Glowing warm lantern glass
    if (lightsOn) {
        standardShader->setVec3("solidColor", glm::vec3(1.0f, 0.96f, 0.72f));
        standardShader->setFloat("emissive", 2.2f);
    } else {
        standardShader->setVec3("solidColor", glm::vec3(0.85f, 0.83f, 0.75f));
        standardShader->setFloat("emissive", 0.0f);
    }
    m = glm::translate(base, glm::vec3(0.0f, 4.35f, 0.0f));
    m = glm::scale(m, glm::vec3(0.35f, 0.48f, 0.35f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    standardShader->setFloat("emissive", 0.0f);

    // Lantern cap roof
    standardShader->setVec3("solidColor", glm::vec3(0.15f, 0.15f, 0.17f));
    m = glm::translate(base, glm::vec3(0.0f, 4.65f, 0.0f));
    m = glm::scale(m, glm::vec3(0.5f, 0.12f, 0.5f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
}

void Park::drawFlowerBed(glm::vec3 position, float width, float length) {
    standardShader->use();
    standardShader->setBool("useTexture", false);

    position.y = Mesh::getTerrainHeight(position.x, position.z);
    glm::mat4 base = glm::translate(glm::mat4(1.0f), position);

    // Stone retaining border (grey granite)
    standardShader->setVec3("solidColor", glm::vec3(0.58f, 0.56f, 0.52f));
    glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.18f, 0.0f));
    m = glm::scale(m, glm::vec3(width, 0.36f, length));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Dark rich soil inside
    standardShader->setVec3("solidColor", glm::vec3(0.24f, 0.18f, 0.12f));
    m = glm::translate(base, glm::vec3(0.0f, 0.22f, 0.0f));
    m = glm::scale(m, glm::vec3(width - 0.3f, 0.36f, length - 0.3f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Vibrant flowers (red, yellow, magenta, white)
    const glm::vec3 flowerPalette[] = {
        glm::vec3(0.92f, 0.15f, 0.2f),  // Red tulip
        glm::vec3(0.98f, 0.85f, 0.1f),  // Yellow daisy
        glm::vec3(0.75f, 0.2f, 0.75f),  // Purple orchid
        glm::vec3(0.95f, 0.95f, 0.95f)   // White lily
    };

    float stepX = (width - 0.6f) / 3.0f;
    float stepZ = (length - 0.6f) / 3.0f;
    int fIdx = 0;
    for (float fx = -width/2.0f + 0.4f; fx <= width/2.0f - 0.4f; fx += stepX) {
        for (float fz = -length/2.0f + 0.4f; fz <= length/2.0f - 0.4f; fz += stepZ) {
            // Green leaf cluster at base of stem
            standardShader->setVec3("solidColor", glm::vec3(0.18f, 0.52f, 0.22f));
            m = glm::translate(base, glm::vec3(fx, 0.36f, fz));
            m = glm::scale(m, glm::vec3(0.22f, 0.10f, 0.22f));
            standardShader->setMat4("model", m);
            sphereMesh->draw();

            // Slender stem
            m = glm::translate(base, glm::vec3(fx, 0.38f, fz));
            m = glm::scale(m, glm::vec3(0.05f, 0.22f, 0.05f));
            standardShader->setMat4("model", m);
            columnMesh->draw();

            // Organic rounded flower blossom crown
            standardShader->setVec3("solidColor", flowerPalette[fIdx % 4]);
            m = glm::translate(base, glm::vec3(fx, 0.60f, fz));
            m = glm::scale(m, glm::vec3(0.20f, 0.16f, 0.20f));
            standardShader->setMat4("model", m);
            sphereMesh->draw();

            // Blossom bright center pollen core
            standardShader->setVec3("solidColor", glm::vec3(1.0f, 0.90f, 0.25f));
            m = glm::translate(base, glm::vec3(fx, 0.65f, fz));
            m = glm::scale(m, glm::vec3(0.08f, 0.08f, 0.08f));
            standardShader->setMat4("model", m);
            sphereMesh->draw();

            fIdx++;
        }
    }
}

void Park::drawTrashBin(glm::vec3 position) {
    standardShader->use();
    standardShader->setBool("useTexture", false);

    glm::mat4 base = glm::translate(glm::mat4(1.0f), position);

    // Green body
    standardShader->setVec3("solidColor", glm::vec3(0.18f, 0.35f, 0.22f));
    glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.4f, 0.0f));
    m = glm::scale(m, glm::vec3(0.48f, 0.8f, 0.48f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Dark lid
    standardShader->setVec3("solidColor", glm::vec3(0.12f, 0.12f, 0.12f));
    m = glm::translate(base, glm::vec3(0.0f, 0.83f, 0.0f));
    m = glm::scale(m, glm::vec3(0.52f, 0.08f, 0.52f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
}

void Park::drawPlayground(glm::vec3 position) {
    standardShader->use();
    standardShader->setBool("useTexture", false);

    position.y = Mesh::getTerrainHeight(position.x, position.z);
    glm::mat4 base = glm::translate(glm::mat4(1.0f), position);

    // 1. Colorful Soft Rubber Safety Turf
    standardShader->setVec3("solidColor", glm::vec3(0.18f, 0.65f, 0.62f)); // Cyan-teal rubber turf
    glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.03f, 0.0f));
    m = glm::scale(m, glm::vec3(26.0f, 0.06f, 22.0f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Safety curb / border in bright yellow
    standardShader->setVec3("solidColor", glm::vec3(0.95f, 0.82f, 0.15f));
    // North border
    m = glm::translate(base, glm::vec3(0.0f, 0.1f, -11.0f));
    m = glm::scale(m, glm::vec3(26.4f, 0.2f, 0.4f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    // South border
    m = glm::translate(base, glm::vec3(0.0f, 0.1f, 11.0f));
    m = glm::scale(m, glm::vec3(26.4f, 0.2f, 0.4f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    // West border
    m = glm::translate(base, glm::vec3(-13.0f, 0.1f, 0.0f));
    m = glm::scale(m, glm::vec3(0.4f, 0.2f, 22.0f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    // East border
    m = glm::translate(base, glm::vec3(13.0f, 0.1f, 0.0f));
    m = glm::scale(m, glm::vec3(0.4f, 0.2f, 22.0f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // -------------------------------------------------------------
    // 2. SWING SET (at offset -6, -4)
    // -------------------------------------------------------------
    glm::mat4 swingBase = glm::translate(base, glm::vec3(-6.0f, 0.0f, -4.0f));

    // Steel A-frame legs (Royal Blue)
    standardShader->setVec3("solidColor", glm::vec3(0.15f, 0.45f, 0.9f));

    // Left A-frame (2 legs tilted)
    m = glm::translate(swingBase, glm::vec3(-3.0f, 1.8f, -0.6f));
    m = glm::rotate(m, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.12f, 3.8f, 0.12f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(swingBase, glm::vec3(-3.0f, 1.8f, 0.6f));
    m = glm::rotate(m, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.12f, 3.8f, 0.12f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Right A-frame (2 legs tilted)
    m = glm::translate(swingBase, glm::vec3(3.0f, 1.8f, -0.6f));
    m = glm::rotate(m, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.12f, 3.8f, 0.12f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(swingBase, glm::vec3(3.0f, 1.8f, 0.6f));
    m = glm::rotate(m, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.12f, 3.8f, 0.12f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Top crossbar
    m = glm::translate(swingBase, glm::vec3(0.0f, 3.6f, 0.0f));
    m = glm::scale(m, glm::vec3(6.4f, 0.15f, 0.15f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Two Swings (Yellow seats & silver chains)
    float swingOffsets[] = {-1.3f, 1.3f};
    for (float so : swingOffsets) {
        // Chains
        standardShader->setVec3("solidColor", glm::vec3(0.35f, 0.35f, 0.38f));
        m = glm::translate(swingBase, glm::vec3(so - 0.28f, 2.1f, 0.0f));
        m = glm::scale(m, glm::vec3(0.03f, 2.8f, 0.03f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        m = glm::translate(swingBase, glm::vec3(so + 0.28f, 2.1f, 0.0f));
        m = glm::scale(m, glm::vec3(0.03f, 2.8f, 0.03f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Yellow seat
        standardShader->setVec3("solidColor", glm::vec3(0.98f, 0.85f, 0.12f));
        m = glm::translate(swingBase, glm::vec3(so, 0.7f, 0.0f));
        m = glm::scale(m, glm::vec3(0.66f, 0.08f, 0.28f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // -------------------------------------------------------------
    // 3. SLIDE TOWER WITH ROOF & RAMP (at offset 6, -3)
    // -------------------------------------------------------------
    glm::mat4 slideBase = glm::translate(base, glm::vec3(6.0f, 0.0f, -3.0f));

    // 4 Red corner posts
    standardShader->setVec3("solidColor", glm::vec3(0.9f, 0.2f, 0.15f));
    float postX[] = {-1.0f, 1.0f};
    float postZ[] = {-1.0f, 1.0f};
    for (float px : postX) {
        for (float pz : postZ) {
            m = glm::translate(slideBase, glm::vec3(px, 1.9f, pz));
            m = glm::scale(m, glm::vec3(0.12f, 3.8f, 0.12f));
            standardShader->setMat4("model", m);
            boxMesh->draw();
        }
    }

    // Wooden observation platform at height 2.0m
    standardShader->setVec3("solidColor", glm::vec3(0.85f, 0.6f, 0.35f));
    m = glm::translate(slideBase, glm::vec3(0.0f, 2.0f, 0.0f));
    m = glm::scale(m, glm::vec3(2.2f, 0.12f, 2.2f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Platform safety railings (Red)
    standardShader->setVec3("solidColor", glm::vec3(0.9f, 0.2f, 0.15f));
    // Back rail
    m = glm::translate(slideBase, glm::vec3(0.0f, 2.5f, -1.0f));
    m = glm::scale(m, glm::vec3(2.1f, 0.8f, 0.06f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    // Left rail
    m = glm::translate(slideBase, glm::vec3(-1.0f, 2.5f, 0.0f));
    m = glm::scale(m, glm::vec3(0.06f, 0.8f, 2.1f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    // Right rail
    m = glm::translate(slideBase, glm::vec3(1.0f, 2.5f, 0.0f));
    m = glm::scale(m, glm::vec3(0.06f, 0.8f, 2.1f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Peaked Canopy Roof (Bright Yellow)
    standardShader->setVec3("solidColor", glm::vec3(0.98f, 0.88f, 0.15f));
    m = glm::translate(slideBase, glm::vec3(0.0f, 3.9f, 0.0f));
    m = glm::scale(m, glm::vec3(2.6f, 0.4f, 2.6f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(slideBase, glm::vec3(0.0f, 4.2f, 0.0f));
    m = glm::scale(m, glm::vec3(1.8f, 0.35f, 1.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Slide Chute (Sloping ramp in bright orange-yellow extending forward)
    standardShader->setVec3("solidColor", glm::vec3(1.0f, 0.58f, 0.1f));
    m = glm::translate(slideBase, glm::vec3(0.0f, 1.0f, 2.6f));
    m = glm::rotate(m, glm::radians(28.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.85f, 0.08f, 3.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Slide side guide rails (Red)
    standardShader->setVec3("solidColor", glm::vec3(0.9f, 0.2f, 0.15f));
    m = glm::translate(slideBase, glm::vec3(-0.45f, 1.12f, 2.6f));
    m = glm::rotate(m, glm::radians(28.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.08f, 0.22f, 3.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(slideBase, glm::vec3(0.45f, 1.12f, 2.6f));
    m = glm::rotate(m, glm::radians(28.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.08f, 0.22f, 3.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Ladder steps at back
    standardShader->setVec3("solidColor", glm::vec3(0.2f, 0.5f, 0.85f));
    for (int l = 1; l <= 5; ++l) {
        m = glm::translate(slideBase, glm::vec3(0.0f, l * 0.38f, -1.05f));
        m = glm::scale(m, glm::vec3(0.8f, 0.06f, 0.1f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // -------------------------------------------------------------
    // 4. MERRY-GO-ROUND / CAROUSEL (at offset 5, 5)
    // -------------------------------------------------------------
    glm::mat4 carouselBase = glm::translate(base, glm::vec3(5.0f, 0.0f, 5.0f));

    // Green round deck
    standardShader->setVec3("solidColor", glm::vec3(0.25f, 0.75f, 0.35f));
    m = glm::translate(carouselBase, glm::vec3(0.0f, 0.15f, 0.0f));
    m = glm::scale(m, glm::vec3(3.8f, 0.15f, 3.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Center pivot post
    standardShader->setVec3("solidColor", glm::vec3(0.15f, 0.45f, 0.9f));
    m = glm::translate(carouselBase, glm::vec3(0.0f, 0.6f, 0.0f));
    m = glm::scale(m, glm::vec3(0.25f, 1.0f, 0.25f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Four colorful safety grab bars
    const glm::vec3 handleColors[] = {
        glm::vec3(0.92f, 0.2f, 0.18f),
        glm::vec3(0.98f, 0.82f, 0.12f),
        glm::vec3(0.18f, 0.5f, 0.95f),
        glm::vec3(0.92f, 0.2f, 0.18f)
    };
    for (int h = 0; h < 4; ++h) {
        float rot = glm::radians(h * 90.0f);
        standardShader->setVec3("solidColor", handleColors[h]);
        glm::mat4 hm = glm::rotate(carouselBase, rot, glm::vec3(0.0f, 1.0f, 0.0f));
        // Vertical outer post
        m = glm::translate(hm, glm::vec3(1.3f, 0.6f, 0.0f));
        m = glm::scale(m, glm::vec3(0.08f, 0.9f, 0.08f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
        // Horizontal connect to center
        m = glm::translate(hm, glm::vec3(0.7f, 0.95f, 0.0f));
        m = glm::scale(m, glm::vec3(1.3f, 0.08f, 0.08f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // -------------------------------------------------------------
    // 5. SEESAW / TEETER-TOTTER (at offset -6, 5)
    // -------------------------------------------------------------
    glm::mat4 seesawBase = glm::translate(base, glm::vec3(-6.0f, 0.0f, 5.0f));

    // Green fulcrum base
    standardShader->setVec3("solidColor", glm::vec3(0.2f, 0.6f, 0.3f));
    m = glm::translate(seesawBase, glm::vec3(0.0f, 0.35f, 0.0f));
    m = glm::scale(m, glm::vec3(0.5f, 0.65f, 0.5f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Pivoted Plank (Tilted 12 degrees, yellow)
    glm::mat4 plankBase = glm::translate(seesawBase, glm::vec3(0.0f, 0.65f, 0.0f));
    plankBase = glm::rotate(plankBase, glm::radians(12.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    standardShader->setVec3("solidColor", glm::vec3(0.98f, 0.82f, 0.12f));
    m = glm::scale(plankBase, glm::vec3(4.2f, 0.08f, 0.35f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Two Red Seats with handles
    standardShader->setVec3("solidColor", glm::vec3(0.9f, 0.2f, 0.15f));
    // Left seat
    m = glm::translate(plankBase, glm::vec3(-1.85f, 0.07f, 0.0f));
    m = glm::scale(m, glm::vec3(0.45f, 0.06f, 0.4f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    // Left handle
    m = glm::translate(plankBase, glm::vec3(-1.6f, 0.22f, 0.0f));
    m = glm::scale(m, glm::vec3(0.05f, 0.32f, 0.28f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Right seat
    m = glm::translate(plankBase, glm::vec3(1.85f, 0.07f, 0.0f));
    m = glm::scale(m, glm::vec3(0.45f, 0.06f, 0.4f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    // Right handle
    m = glm::translate(plankBase, glm::vec3(1.6f, 0.22f, 0.0f));
    m = glm::scale(m, glm::vec3(0.05f, 0.32f, 0.28f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // -------------------------------------------------------------
    // 6. SANDBOX WITH TOYS (at offset 0, 6.5)
    // -------------------------------------------------------------
    glm::mat4 sandBase = glm::translate(base, glm::vec3(0.0f, 0.0f, 6.5f));

    // Wooden timber border (4.5m x 4.5m)
    standardShader->setVec3("solidColor", glm::vec3(0.6f, 0.38f, 0.2f));
    m = glm::translate(sandBase, glm::vec3(0.0f, 0.18f, -2.1f));
    m = glm::scale(m, glm::vec3(4.5f, 0.32f, 0.3f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    m = glm::translate(sandBase, glm::vec3(0.0f, 0.18f, 2.1f));
    m = glm::scale(m, glm::vec3(4.5f, 0.32f, 0.3f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    m = glm::translate(sandBase, glm::vec3(-2.1f, 0.18f, 0.0f));
    m = glm::scale(m, glm::vec3(0.3f, 0.32f, 4.5f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    m = glm::translate(sandBase, glm::vec3(2.1f, 0.18f, 0.0f));
    m = glm::scale(m, glm::vec3(0.3f, 0.32f, 4.5f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Golden play sand filling
    standardShader->setVec3("solidColor", glm::vec3(0.94f, 0.86f, 0.62f));
    m = glm::translate(sandBase, glm::vec3(0.0f, 0.12f, 0.0f));
    m = glm::scale(m, glm::vec3(3.9f, 0.2f, 3.9f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Toy sand buckets & spades
    standardShader->setVec3("solidColor", glm::vec3(0.9f, 0.18f, 0.15f)); // Red bucket
    m = glm::translate(sandBase, glm::vec3(-0.6f, 0.28f, -0.4f));
    m = glm::scale(m, glm::vec3(0.25f, 0.26f, 0.25f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    standardShader->setVec3("solidColor", glm::vec3(0.15f, 0.45f, 0.95f)); // Blue bucket
    m = glm::translate(sandBase, glm::vec3(0.7f, 0.26f, 0.5f));
    m = glm::scale(m, glm::vec3(0.22f, 0.22f, 0.22f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // -------------------------------------------------------------
    // 7. PLAYGROUND PARENT BENCHES
    // -------------------------------------------------------------
    drawBench(position + glm::vec3(0.0f, 0.0f, -12.5f), 0.0f);
    drawBench(position + glm::vec3(-14.5f, 0.0f, 0.0f), glm::radians(90.0f));
}

void Park::drawFountain(glm::vec3 position, float time) {
    standardShader->use();
    standardShader->setBool("useTexture", false);
    standardShader->setFloat("emissive", 0.0f);

    position.y = Mesh::getTerrainHeight(position.x, position.z);
    glm::mat4 base = glm::translate(glm::mat4(1.0f), position);

    // 1. Classical Carved Stone Basin Rim (24-sided beveled circular wall)
    standardShader->setVec3("solidColor", glm::vec3(0.80f, 0.77f, 0.72f)); // Warm French limestone
    const int basinSegs = 24;
    float basinRadius = 6.8f;
    for (int i = 0; i < basinSegs; ++i) {
        float angle = i * (6.283185f / basinSegs);
        glm::mat4 m = glm::rotate(base, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        // Upright wall slab
        m = glm::translate(m, glm::vec3(basinRadius, 0.42f, 0.0f));
        m = glm::scale(m, glm::vec3(0.55f, 0.85f, 1.82f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Top beveled coping rim
        glm::mat4 cm = glm::rotate(base, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        cm = glm::translate(cm, glm::vec3(basinRadius, 0.86f, 0.0f));
        cm = glm::scale(cm, glm::vec3(0.70f, 0.12f, 1.84f));
        standardShader->setMat4("model", cm);
        boxMesh->draw();
    }

    // 2. Basin Water Surface — Perfectly fitted CIRCULAR DISK (R = 6.4m, strictly inside 6.8m wall)
    // Completely eliminates any clipping corners / blue wings!
    standardShader->setBool("useTexture", true);
    standardShader->setBool("isWater", true);
    standardShader->setFloat("time", time);
    standardShader->setVec2("texScale", glm::vec2(4.0f, 4.0f));
    standardShader->setVec3("solidColor", glm::vec3(0.65f, 0.88f, 0.98f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texWater);

    glm::mat4 waterM = glm::translate(base, glm::vec3(0.0f, 0.58f, 0.0f));
    waterM = glm::scale(waterM, glm::vec3(6.4f, 1.0f, 6.4f));
    standardShader->setMat4("model", waterM);
    diskMesh->draw();

    standardShader->setBool("isWater", false);
    standardShader->setBool("useTexture", false);

    // 3. Central Sculpted Stone Pedestal & Tiered Bowls
    standardShader->setVec3("solidColor", glm::vec3(0.78f, 0.75f, 0.70f));

    // Base pedestal plinth
    glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.45f, 0.0f));
    m = glm::scale(m, glm::vec3(2.4f, 0.9f, 2.4f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    // Tier 1 (Large lower basin bowl)
    m = glm::translate(base, glm::vec3(0.0f, 0.95f, 0.0f));
    m = glm::scale(m, glm::vec3(1.8f, 0.4f, 1.8f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    m = glm::translate(base, glm::vec3(0.0f, 1.25f, 0.0f));
    m = glm::scale(m, glm::vec3(3.9f, 0.35f, 3.9f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    // Tier 2 (Middle bowl & fluted shaft)
    m = glm::translate(base, glm::vec3(0.0f, 1.55f, 0.0f));
    m = glm::scale(m, glm::vec3(1.1f, 0.85f, 1.1f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    m = glm::translate(base, glm::vec3(0.0f, 2.25f, 0.0f));
    m = glm::scale(m, glm::vec3(2.5f, 0.32f, 2.5f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    // Tier 3 (Top bowl & finial column)
    m = glm::translate(base, glm::vec3(0.0f, 2.52f, 0.0f));
    m = glm::scale(m, glm::vec3(0.75f, 0.65f, 0.75f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    m = glm::translate(base, glm::vec3(0.0f, 3.08f, 0.0f));
    m = glm::scale(m, glm::vec3(1.35f, 0.26f, 1.35f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    // Decorative pineapple/acorn finial
    m = glm::translate(base, glm::vec3(0.0f, 3.38f, 0.0f));
    m = glm::scale(m, glm::vec3(0.42f, 0.62f, 0.42f));
    standardShader->setMat4("model", m);
    sphereMesh->draw();

    // 4. Cascading Water Sheets Falling Between Tiers
    standardShader->setBool("useTexture", true);
    standardShader->setBool("isWater", true);
    standardShader->setFloat("time", time);
    standardShader->setFloat("emissive", 0.35f);
    standardShader->setVec3("solidColor", glm::vec3(0.75f, 0.94f, 1.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texWater);

    // Tier 3 -> Tier 2 cascade curtain
    standardShader->setVec2("texScale", glm::vec2(2.0f, 4.0f));
    m = glm::translate(base, glm::vec3(0.0f, 2.3f, 0.0f));
    m = glm::scale(m, glm::vec3(1.36f, 0.82f, 1.36f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    // Tier 2 -> Tier 1 cascade curtain
    standardShader->setVec2("texScale", glm::vec2(3.0f, 4.0f));
    m = glm::translate(base, glm::vec3(0.0f, 1.3f, 0.0f));
    m = glm::scale(m, glm::vec3(2.52f, 0.98f, 2.52f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    // Tier 1 -> Lower Basin cascade curtain
    standardShader->setVec2("texScale", glm::vec2(4.0f, 4.0f));
    m = glm::translate(base, glm::vec3(0.0f, 0.62f, 0.0f));
    m = glm::scale(m, glm::vec3(3.92f, 0.68f, 3.92f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    standardShader->setBool("isWater", false);
    standardShader->setBool("useTexture", false);

    // 5. Parabolic Arcing Water Jets (6 streams arching gracefully into Tier 2 bowl)
    standardShader->setVec3("solidColor", glm::vec3(0.86f, 0.96f, 1.0f));
    standardShader->setFloat("emissive", 0.60f);

    const int numStreams = 6;
    for (int s = 0; s < numStreams; ++s) {
        float baseRot = glm::radians(s * (360.0f / numStreams) + time * 12.0f);
        glm::mat4 sm = glm::rotate(base, baseRot, glm::vec3(0.0f, 1.0f, 0.0f));

        // 6 sample points along parabolic arc
        for (int p = 1; p <= 6; ++p) {
            float t = (float)p / 6.0f;
            float r = 0.25f + t * 1.75f;
            float y = 3.45f + (0.95f * t - 1.85f * t * t) + 0.05f * std::sin(time * 6.0f + p + s);
            float beadSize = 0.07f + 0.02f * std::sin(time * 8.0f + p);

            m = glm::translate(sm, glm::vec3(r, y, 0.0f));
            m = glm::scale(m, glm::vec3(beadSize, beadSize, beadSize));
            standardShader->setMat4("model", m);
            sphereMesh->draw();
        }
    }

    // 6. Central Water Spout Plume (Shooting vertically from finial)
    float plumeHeight = 1.7f + 0.35f * std::sin(time * 5.5f);
    m = glm::translate(base, glm::vec3(0.0f, 3.6f + plumeHeight * 0.5f, 0.0f));
    m = glm::scale(m, glm::vec3(0.20f, plumeHeight, 0.20f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    // Spray crest droplets at apex
    m = glm::translate(base, glm::vec3(0.0f, 3.6f + plumeHeight, 0.0f));
    m = glm::scale(m, glm::vec3(0.46f, 0.36f, 0.46f));
    standardShader->setMat4("model", m);
    sphereMesh->draw();

    // Secondary mist crest
    m = glm::translate(base, glm::vec3(0.0f, 3.6f + plumeHeight + 0.25f, 0.0f));
    m = glm::scale(m, glm::vec3(0.28f, 0.24f, 0.28f));
    standardShader->setMat4("model", m);
    sphereMesh->draw();

    standardShader->setFloat("emissive", 0.0f);
}

void Park::drawLakePier(glm::vec3 startPos, float time) {
    standardShader->use();
    standardShader->setBool("useTexture", false);
    standardShader->setFloat("emissive", 0.0f);

    // Lake surface is at y = -0.45f. Pier deck is at y = -0.1f.
    glm::mat4 base = glm::translate(glm::mat4(1.0f), glm::vec3(startPos.x, -0.1f, startPos.z));

    // Wooden deck planks (extends 20m along -X, width 3.6m along Z)
    standardShader->setVec3("solidColor", glm::vec3(0.52f, 0.36f, 0.22f)); // Rustic weathered cedar
    glm::mat4 m = glm::translate(base, glm::vec3(-10.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(20.0f, 0.15f, 3.6f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Planking lines detail
    standardShader->setVec3("solidColor", glm::vec3(0.42f, 0.28f, 0.16f));
    for (int p = 0; p <= 16; ++p) {
        float px = -p * 1.25f;
        m = glm::translate(base, glm::vec3(px, 0.08f, 0.0f));
        m = glm::scale(m, glm::vec3(0.04f, 0.02f, 3.58f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // Heavy submerged wooden pilings into water bed (5 pairs along length)
    standardShader->setVec3("solidColor", glm::vec3(0.24f, 0.18f, 0.13f)); // Wet dark timber
    float pilingX[] = {-1.0f, -5.5f, -10.0f, -14.5f, -19.5f};
    for (float px : pilingX) {
        // Left piling (Z = -1.6)
        m = glm::translate(base, glm::vec3(px, -1.2f, -1.6f));
        m = glm::scale(m, glm::vec3(0.28f, 2.5f, 0.28f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Right piling (Z = +1.6)
        m = glm::translate(base, glm::vec3(px, -1.2f, 1.6f));
        m = glm::scale(m, glm::vec3(0.28f, 2.5f, 0.28f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Cross-brace beams under deck
        m = glm::translate(base, glm::vec3(px, -0.2f, 0.0f));
        m = glm::scale(m, glm::vec3(0.2f, 0.25f, 3.4f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Handrail vertical posts extending above deck
        standardShader->setVec3("solidColor", glm::vec3(0.48f, 0.34f, 0.20f));
        m = glm::translate(base, glm::vec3(px, 0.6f, -1.7f));
        m = glm::scale(m, glm::vec3(0.12f, 1.1f, 0.12f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        m = glm::translate(base, glm::vec3(px, 0.6f, 1.7f));
        m = glm::scale(m, glm::vec3(0.12f, 1.1f, 0.12f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // Top Handrails along North and South sides
    standardShader->setVec3("solidColor", glm::vec3(0.56f, 0.39f, 0.24f));
    // North rail (Z = -1.7)
    m = glm::translate(base, glm::vec3(-10.0f, 1.15f, -1.7f));
    m = glm::scale(m, glm::vec3(20.0f, 0.08f, 0.14f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    // South rail (Z = +1.7)
    m = glm::translate(base, glm::vec3(-10.0f, 1.15f, 1.7f));
    m = glm::scale(m, glm::vec3(20.0f, 0.08f, 0.14f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Pier-Head End rail connecting North and South rails (X = -19.9)
    m = glm::translate(base, glm::vec3(-19.9f, 1.15f, 0.0f));
    m = glm::scale(m, glm::vec3(0.14f, 0.08f, 3.4f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Pier Entrance Portal Posts (Timber posts with warm nautical brass lanterns)
    standardShader->setVec3("solidColor", glm::vec3(0.35f, 0.24f, 0.15f));
    m = glm::translate(base, glm::vec3(-0.2f, 0.65f, -1.8f));
    m = glm::scale(m, glm::vec3(0.22f, 1.3f, 0.22f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(base, glm::vec3(-0.2f, 0.65f, 1.8f));
    m = glm::scale(m, glm::vec3(0.22f, 1.3f, 0.22f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Nautical lantern globes
    standardShader->setVec3("solidColor", glm::vec3(1.0f, 0.85f, 0.45f));
    standardShader->setFloat("emissive", 0.7f);
    m = glm::translate(base, glm::vec3(-0.2f, 1.38f, -1.8f));
    m = glm::scale(m, glm::vec3(0.16f, 0.16f, 0.16f));
    standardShader->setMat4("model", m);
    sphereMesh->draw();

    m = glm::translate(base, glm::vec3(-0.2f, 1.38f, 1.8f));
    m = glm::scale(m, glm::vec3(0.16f, 0.16f, 0.16f));
    standardShader->setMat4("model", m);
    sphereMesh->draw();
    standardShader->setFloat("emissive", 0.0f);

    // Lifebuoy ring mounted on North railing
    standardShader->setVec3("solidColor", glm::vec3(0.92f, 0.92f, 0.92f));
    glm::mat4 buoyM = glm::translate(base, glm::vec3(-10.0f, 0.65f, -1.75f));
    buoyM = glm::scale(buoyM, glm::vec3(0.6f, 0.6f, 0.08f));
    standardShader->setMat4("model", buoyM);
    cylinderMesh->draw();

    standardShader->setVec3("solidColor", glm::vec3(0.85f, 0.15f, 0.12f));
    buoyM = glm::translate(base, glm::vec3(-10.0f, 0.65f, -1.77f));
    buoyM = glm::scale(buoyM, glm::vec3(0.62f, 0.14f, 0.09f));
    standardShader->setMat4("model", buoyM);
    boxMesh->draw();

    // End Bollards / Mooring Cleats at Pier Head
    standardShader->setVec3("solidColor", glm::vec3(0.15f, 0.15f, 0.17f)); // Cast iron
    m = glm::translate(base, glm::vec3(-19.8f, 0.3f, -1.5f));
    m = glm::scale(m, glm::vec3(0.2f, 0.45f, 0.2f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    m = glm::translate(base, glm::vec3(-19.8f, 0.3f, 1.5f));
    m = glm::scale(m, glm::vec3(0.2f, 0.45f, 0.2f));
    standardShader->setMat4("model", m);
    cylinderMesh->draw();

    // Scenic Pier-End Bench facing open lake West!
    drawBench(startPos + glm::vec3(-18.2f, -0.1f, 0.0f), glm::radians(90.0f));

    // Two Floating Wooden Rowboats moored alongside
    auto drawRowboat = [&](glm::vec3 boatPos, float timeOffset) {
        float bob = std::sin(time * 2.2f + timeOffset) * 0.035f;
        float roll = std::sin(time * 1.7f + timeOffset) * 0.04f;

        glm::mat4 bm = glm::translate(glm::mat4(1.0f), boatPos + glm::vec3(0.0f, bob, 0.0f));
        bm = glm::rotate(bm, roll, glm::vec3(1.0f, 0.0f, 0.0f));

        // Boat Hull - Navy Blue exterior
        standardShader->setVec3("solidColor", glm::vec3(0.14f, 0.32f, 0.55f));
        // Bottom plank
        glm::mat4 b = glm::translate(bm, glm::vec3(0.0f, 0.0f, 0.0f));
        b = glm::scale(b, glm::vec3(4.5f, 0.12f, 1.6f));
        standardShader->setMat4("model", b);
        boxMesh->draw();

        // Left gunwale
        b = glm::translate(bm, glm::vec3(0.0f, 0.35f, -0.75f));
        b = glm::scale(b, glm::vec3(4.5f, 0.6f, 0.1f));
        standardShader->setMat4("model", b);
        boxMesh->draw();

        // Right gunwale
        b = glm::translate(bm, glm::vec3(0.0f, 0.35f, 0.75f));
        b = glm::scale(b, glm::vec3(4.5f, 0.6f, 0.1f));
        standardShader->setMat4("model", b);
        boxMesh->draw();

        // Tapered bow wedge (pointing towards open water -X)
        b = glm::translate(bm, glm::vec3(-2.4f, 0.3f, 0.0f));
        b = glm::rotate(b, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        b = glm::scale(b, glm::vec3(0.5f, 0.8f, 1.4f));
        standardShader->setMat4("model", b);
        coneMesh->draw();

        // Transom / stern back
        b = glm::translate(bm, glm::vec3(2.25f, 0.35f, 0.0f));
        b = glm::scale(b, glm::vec3(0.12f, 0.6f, 1.5f));
        standardShader->setMat4("model", b);
        boxMesh->draw();

        // Wooden interior seats (Teak)
        standardShader->setVec3("solidColor", glm::vec3(0.6f, 0.4f, 0.22f));
        // Center rower seat
        b = glm::translate(bm, glm::vec3(0.0f, 0.28f, 0.0f));
        b = glm::scale(b, glm::vec3(0.55f, 0.08f, 1.45f));
        standardShader->setMat4("model", b);
        boxMesh->draw();

        // Bow seat
        b = glm::translate(bm, glm::vec3(-1.4f, 0.35f, 0.0f));
        b = glm::scale(b, glm::vec3(0.55f, 0.08f, 1.1f));
        standardShader->setMat4("model", b);
        boxMesh->draw();

        // Stern seat
        b = glm::translate(bm, glm::vec3(1.5f, 0.35f, 0.0f));
        b = glm::scale(b, glm::vec3(0.55f, 0.08f, 1.3f));
        standardShader->setMat4("model", b);
        boxMesh->draw();

        // Pair of wooden oars stowed neatly INSIDE the rowboat along gunwales (Zero pier clipping!)
        standardShader->setVec3("solidColor", glm::vec3(0.72f, 0.52f, 0.32f));
        // Left Oar - stowed lengthwise resting across seats inside the boat
        b = glm::translate(bm, glm::vec3(0.1f, 0.36f, -0.50f));
        b = glm::rotate(b, glm::radians(3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        b = glm::scale(b, glm::vec3(2.1f, 0.05f, 0.05f));
        standardShader->setMat4("model", b);
        boxMesh->draw();
        // Flat paddle blade at forward end of oar
        b = glm::translate(bm, glm::vec3(-1.0f, 0.36f, -0.55f));
        b = glm::scale(b, glm::vec3(0.55f, 0.02f, 0.16f));
        standardShader->setMat4("model", b);
        boxMesh->draw();

        // Right Oar - stowed lengthwise resting across seats inside the boat
        b = glm::translate(bm, glm::vec3(0.1f, 0.36f, 0.50f));
        b = glm::rotate(b, glm::radians(-3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        b = glm::scale(b, glm::vec3(2.1f, 0.05f, 0.05f));
        standardShader->setMat4("model", b);
        boxMesh->draw();
        // Flat paddle blade at forward end of oar
        b = glm::translate(bm, glm::vec3(-1.0f, 0.36f, 0.55f));
        b = glm::scale(b, glm::vec3(0.55f, 0.02f, 0.16f));
        standardShader->setMat4("model", b);
        boxMesh->draw();

        // Mooring rope tied from bow to pier post
        standardShader->setVec3("solidColor", glm::vec3(0.85f, 0.78f, 0.58f));
        float ropeTargetZ = (boatPos.z < startPos.z) ? 2.0f : -2.0f;
        b = glm::translate(bm, glm::vec3(1.8f, 0.3f, ropeTargetZ * 0.5f));
        b = glm::scale(b, glm::vec3(0.06f, 0.06f, std::abs(ropeTargetZ)));
        standardShader->setMat4("model", b);
        boxMesh->draw();
    };

    drawRowboat(startPos + glm::vec3(-8.5f, -0.42f, -3.8f), 0.0f);
    drawRowboat(startPos + glm::vec3(-15.0f, -0.42f, 3.8f), 1.8f);
}

void Park::drawShoreline(glm::vec3 center, float radius) {
    standardShader->use();
    standardShader->setBool("useTexture", false);
    standardShader->setFloat("emissive", 0.0f);

    // 1. Natural Sand & Pebble Beach Embankment Ring
    // 48 trapezoidal quad segments forming a gentle sandy/gravelly beach fringe around the lake rim
    int segments = 48;
    float innerR = radius - 1.2f; // ~19.6m (submerged slightly)
    float outerR = radius + 2.4f; // ~23.2m (blends into grass)
    
    for (int i = 0; i < segments; ++i) {
        float a1 = (float)i * 2.0f * 3.14159265f / (float)segments;
        float a2 = (float)(i + 1) * 2.0f * 3.14159265f / (float)segments;
        
        float aMid = (a1 + a2) * 0.5f;
        float cosMid = std::cos(aMid);
        float sinMid = std::sin(aMid);
        float midR = (innerR + outerR) * 0.5f;
        float segWidth = (outerR - innerR);
        float segLen = midR * (a2 - a1) * 1.06f; // slight overlap
        
        float px = center.x + cosMid * midR;
        float pz = center.z + sinMid * midR;
        float py = Mesh::getTerrainHeight(px, pz) + 0.02f;
        
        // Warm wet-sand / fine pebble beach tone with subtle segment shade variation
        float shade = 0.95f + 0.08f * std::sin((float)i * 1.7f);
        glm::vec3 beachColor = glm::vec3(0.70f * shade, 0.65f * shade, 0.54f * shade);
        standardShader->setVec3("solidColor", beachColor);
        
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(px, py, pz));
        m = glm::rotate(m, -aMid + 1.5707963f, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(segLen, 0.06f, segWidth));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // 2. Natural Weathered River Boulders & Shoreline Stones
    // Scattered organically along the water edge
    struct BoulderDef {
        float angleDeg;
        float distOffset;
        float scaleX, scaleY, scaleZ;
        float rotY;
        glm::vec3 color;
    };
    
    static const BoulderDef boulders[] = {
        // Northwest rocky outcropping
        { 120.0f, 0.4f, 1.4f, 0.65f, 1.1f, 25.0f, glm::vec3(0.42f, 0.42f, 0.40f) },
        { 128.0f, -0.3f, 0.85f, 0.45f, 0.9f, -15.0f, glm::vec3(0.36f, 0.38f, 0.35f) },
        { 135.0f, 0.8f, 1.6f, 0.85f, 1.3f, 40.0f, glm::vec3(0.48f, 0.46f, 0.42f) },
        { 142.0f, 0.2f, 0.9f, 0.4f, 0.8f, -30.0f, glm::vec3(0.38f, 0.40f, 0.36f) },
        // Southwest bank stones
        { 210.0f, 0.5f, 1.5f, 0.7f, 1.2f, 10.0f, glm::vec3(0.44f, 0.42f, 0.39f) },
        { 220.0f, -0.2f, 1.1f, 0.5f, 1.0f, -45.0f, glm::vec3(0.35f, 0.37f, 0.33f) },
        { 232.0f, 0.6f, 1.8f, 0.9f, 1.4f, 30.0f, glm::vec3(0.46f, 0.45f, 0.43f) },
        { 245.0f, -0.4f, 0.75f, 0.35f, 0.7f, 60.0f, glm::vec3(0.38f, 0.38f, 0.35f) },
        // South shore pebbles & rock cluster
        { 280.0f, 0.3f, 1.3f, 0.6f, 1.0f, -20.0f, glm::vec3(0.42f, 0.41f, 0.38f) },
        { 290.0f, -0.1f, 0.95f, 0.45f, 0.85f, 15.0f, glm::vec3(0.37f, 0.39f, 0.35f) },
        // Northeast quiet bend
        { 38.0f, 0.5f, 1.2f, 0.55f, 1.1f, -10.0f, glm::vec3(0.45f, 0.44f, 0.41f) },
        { 50.0f, 0.2f, 1.7f, 0.8f, 1.3f, 35.0f, glm::vec3(0.40f, 0.41f, 0.38f) },
        { 62.0f, -0.3f, 0.8f, 0.4f, 0.75f, -50.0f, glm::vec3(0.35f, 0.36f, 0.34f) },
        // North shore
        { 85.0f, 0.6f, 1.4f, 0.7f, 1.1f, 20.0f, glm::vec3(0.43f, 0.42f, 0.40f) },
        { 98.0f, -0.2f, 1.0f, 0.5f, 0.9f, -15.0f, glm::vec3(0.38f, 0.40f, 0.37f) },
        // Near pier bank (safely offset from walkway)
        { 345.0f, 0.4f, 1.1f, 0.5f, 0.95f, 45.0f, glm::vec3(0.44f, 0.43f, 0.40f) },
        { 18.0f, 0.4f, 1.2f, 0.55f, 1.0f, -30.0f, glm::vec3(0.41f, 0.40f, 0.38f) }
    };

    for (const auto& b : boulders) {
        float rad = glm::radians(b.angleDeg);
        float r = radius + b.distOffset;
        float bx = center.x + std::cos(rad) * r;
        float bz = center.z + std::sin(rad) * r;
        float by = Mesh::getTerrainHeight(bx, bz);

        standardShader->setVec3("solidColor", b.color);
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(bx, by + b.scaleY * 0.35f, bz));
        m = glm::rotate(m, glm::radians(b.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(b.scaleX, b.scaleY, b.scaleZ));
        standardShader->setMat4("model", m);
        sphereMesh->draw();
    }

    // 3. Aquatic Marsh Cattail Reeds & Wild Grass Tufts
    // Clusters of tall dark-green reeds with brown velvet heads in quiet coves
    static const float reedClusters[][2] = {
        { 75.0f, 20.0f },   // North cove
        { 155.0f, 19.8f },  // West-Northwest bend
        { 185.0f, 19.5f },  // Far West point
        { 225.0f, 20.2f },  // Southwest cove
        { 310.0f, 20.0f }   // Southeast bend
    };

    for (const auto& cluster : reedClusters) {
        float baseAngle = glm::radians(cluster[0]);
        float baseR = cluster[1];
        
        for (int k = 0; k < 9; ++k) {
            float dAngle = ((k % 3) - 1) * 0.045f + (k * 0.012f);
            float dR = ((k / 3) - 1) * 0.45f;
            float angle = baseAngle + dAngle;
            float r = baseR + dR;
            
            float rx = center.x + std::cos(angle) * r;
            float rz = center.z + std::sin(angle) * r;
            float ry = Mesh::getTerrainHeight(rx, rz);

            // Tall slender green stalk
            standardShader->setVec3("solidColor", glm::vec3(0.24f, 0.40f, 0.16f));
            glm::mat4 sm = glm::translate(glm::mat4(1.0f), glm::vec3(rx, ry + 0.65f, rz));
            sm = glm::rotate(sm, 0.1f * std::sin((float)k * 2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            sm = glm::scale(sm, glm::vec3(0.04f, 1.3f, 0.04f));
            standardShader->setMat4("model", sm);
            columnMesh->draw();

            // Velvety brown cattail head on taller stalks
            if (k % 2 == 0) {
                standardShader->setVec3("solidColor", glm::vec3(0.32f, 0.20f, 0.12f));
                glm::mat4 hm = glm::translate(glm::mat4(1.0f), glm::vec3(rx, ry + 1.15f, rz));
                hm = glm::scale(hm, glm::vec3(0.08f, 0.28f, 0.08f));
                standardShader->setMat4("model", hm);
                columnMesh->draw();
            }
        }
    }

    // 4. Floating Water Lily Pads & Blossoms
    // Resting gently on the lake surface at Y = -0.43m (just above water at -0.45m)
    static const glm::vec3 lilyClusters[] = {
        glm::vec3(-112.0f, -0.43f, -94.0f), // West quiet water
        glm::vec3(-106.0f, -0.43f, -112.0f), // North quiet water
        glm::vec3(-90.0f, -0.43f, -110.0f)   // Northeast bay
    };

    for (const auto& lPos : lilyClusters) {
        for (int p = 0; p < 5; ++p) {
            float ox = ((p == 0) ? 0.0f : ((p == 1) ? 0.9f : ((p == 2) ? -0.8f : ((p == 3) ? 0.3f : -0.6f))));
            float oz = ((p == 0) ? 0.0f : ((p == 1) ? 0.5f : ((p == 2) ? 0.7f : ((p == 3) ? -0.9f : -0.7f))));
            float padScale = 0.55f + 0.12f * (p % 3);

            // Waxy dark-green lily pad
            standardShader->setVec3("solidColor", glm::vec3(0.16f, 0.38f, 0.14f));
            glm::mat4 lm = glm::translate(glm::mat4(1.0f), lPos + glm::vec3(ox, 0.0f, oz));
            lm = glm::scale(lm, glm::vec3(padScale, 0.015f, padScale));
            standardShader->setMat4("model", lm);
            diskMesh->draw();

            // Water lily flower on central pad
            if (p == 0 || p == 3) {
                // Petals: soft cream / blush pink
                standardShader->setVec3("solidColor", (p == 0) ? glm::vec3(0.96f, 0.92f, 0.88f) : glm::vec3(0.95f, 0.78f, 0.85f));
                glm::mat4 fm = glm::translate(glm::mat4(1.0f), lPos + glm::vec3(ox, 0.04f, oz));
                fm = glm::scale(fm, glm::vec3(0.14f, 0.10f, 0.14f));
                standardShader->setMat4("model", fm);
                sphereMesh->draw();

                // Yellow center stamen
                standardShader->setVec3("solidColor", glm::vec3(0.96f, 0.85f, 0.15f));
                glm::mat4 stm = glm::translate(glm::mat4(1.0f), lPos + glm::vec3(ox, 0.09f, oz));
                stm = glm::scale(stm, glm::vec3(0.05f, 0.04f, 0.05f));
                standardShader->setMat4("model", stm);
                sphereMesh->draw();
            }
        }
    }
}

void Park::drawGazebo(glm::vec3 position) {
    standardShader->use();
    standardShader->setBool("useTexture", false);
    standardShader->setFloat("emissive", 0.0f);

    position.y = Mesh::getTerrainHeight(position.x, position.z);
    glm::mat4 base = glm::translate(glm::mat4(1.0f), position);

    // -------------------------------------------------------------
    // 1. TWO-TIERED CIRCULAR STONE PODIUM & ENTRANCE STEPS
    // -------------------------------------------------------------
    standardShader->setVec3("solidColor", glm::vec3(0.74f, 0.72f, 0.68f)); // French limestone

    // Lower podium tier (R = 4.4m, height 0.25m, from Y = 0.0 to 0.25)
    glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(8.8f, 0.25f, 8.8f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Upper podium tier (R = 3.9m, height 0.25m, from Y = 0.25 to 0.50)
    standardShader->setVec3("solidColor", glm::vec3(0.78f, 0.76f, 0.72f));
    m = glm::translate(base, glm::vec3(0.0f, 0.25f, 0.0f));
    m = glm::scale(m, glm::vec3(7.8f, 0.25f, 7.8f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Front access steps leading from ground to upper platform (at Z = -3.9m to -4.4m)
    standardShader->setVec3("solidColor", glm::vec3(0.72f, 0.70f, 0.66f));
    // Step 1
    m = glm::translate(base, glm::vec3(0.0f, 0.125f, -4.25f));
    m = glm::scale(m, glm::vec3(2.6f, 0.125f, 0.6f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    // Step 2
    m = glm::translate(base, glm::vec3(0.0f, 0.25f, -3.75f));
    m = glm::scale(m, glm::vec3(2.4f, 0.25f, 0.5f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Polished hardwood pavilion floor (R = 3.75m, from Y = 0.50 to 0.55)
    standardShader->setVec3("solidColor", glm::vec3(0.56f, 0.36f, 0.20f)); // Weathered teak / oak
    m = glm::translate(base, glm::vec3(0.0f, 0.50f, 0.0f));
    m = glm::scale(m, glm::vec3(7.5f, 0.05f, 7.5f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Floor perimeter stone curb ring
    standardShader->setVec3("solidColor", glm::vec3(0.82f, 0.80f, 0.76f));
    m = glm::translate(base, glm::vec3(0.0f, 0.51f, 0.0f));
    m = glm::scale(m, glm::vec3(7.65f, 0.06f, 7.65f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // -------------------------------------------------------------
    // 2. EIGHT CLASSICAL FLUTED PILLARS & ARCHITRAVE BEAMS
    // -------------------------------------------------------------
    const int numPillars = 8;
    const float pillarRadius = 3.20f;
    const float PI = 3.14159265359f;

    for (int i = 0; i < numPillars; ++i) {
        float angle = (float)i * (2.0f * PI / (float)numPillars);
        float px = std::sin(angle) * pillarRadius;
        float pz = std::cos(angle) * pillarRadius;

        // Pillar Square Plinth Base (from Y = 0.55 to 0.75, height = 0.20m)
        standardShader->setVec3("solidColor", glm::vec3(0.92f, 0.92f, 0.90f));
        m = glm::translate(base, glm::vec3(px, 0.65f, pz));
        m = glm::scale(m, glm::vec3(0.42f, 0.20f, 0.42f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Fluted Classical Column Shaft (straight untapered, from Y = 0.75 to 3.05, height = 2.30m)
        standardShader->setVec3("solidColor", glm::vec3(0.96f, 0.96f, 0.95f)); // Crisp white
        m = glm::translate(base, glm::vec3(px, 0.75f, pz));
        m = glm::scale(m, glm::vec3(0.30f, 2.30f, 0.30f));
        standardShader->setMat4("model", m);
        columnMesh->draw();

        // Tuscan Capital Molding (from Y = 3.05 to 3.23, height = 0.18m)
        standardShader->setVec3("solidColor", glm::vec3(0.92f, 0.92f, 0.90f));
        m = glm::translate(base, glm::vec3(px, 3.14f, pz));
        m = glm::scale(m, glm::vec3(0.44f, 0.18f, 0.44f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // White Balustrades between columns (leave open between column 7 and 0 for front entrance!)
        if (i > 0 && i < numPillars) {
            float nextAngle = ((float)(i + 1)) * (2.0f * PI / (float)numPillars);
            if (i < numPillars - 1) { // Skip entrance span (between index 7 and 0)
                float midAngle = (angle + nextAngle) * 0.5f;
                float chordDist = 2.0f * pillarRadius * std::sin(PI / (float)numPillars);
                glm::mat4 rm = glm::rotate(base, midAngle, glm::vec3(0.0f, 1.0f, 0.0f));
                float wallZ = pillarRadius * std::cos(PI / (float)numPillars);

                // Lower balustrade rail
                standardShader->setVec3("solidColor", glm::vec3(0.90f, 0.90f, 0.88f));
                m = glm::translate(rm, glm::vec3(0.0f, 0.78f, wallZ));
                m = glm::scale(m, glm::vec3(chordDist * 0.90f, 0.08f, 0.12f));
                standardShader->setMat4("model", m);
                boxMesh->draw();

                // Molded top handrail
                m = glm::translate(rm, glm::vec3(0.0f, 1.25f, wallZ));
                m = glm::scale(m, glm::vec3(chordDist * 0.90f, 0.09f, 0.15f));
                standardShader->setMat4("model", m);
                boxMesh->draw();

                // Turned baluster spindles (5 per span)
                standardShader->setVec3("solidColor", glm::vec3(0.95f, 0.95f, 0.95f));
                for (int s = -2; s <= 2; ++s) {
                    m = glm::translate(rm, glm::vec3(s * (chordDist * 0.17f), 0.82f, wallZ));
                    m = glm::scale(m, glm::vec3(0.07f, 0.40f, 0.07f));
                    standardShader->setMat4("model", m);
                    columnMesh->draw();
                }
            }
        }

        // -------------------------------------------------------------
        // CONTINUOUS OCTAGONAL ARCHITRAVE / ENTABLATURE BEAM
        // Connects the tops of all columns (from Y = 3.23 to 3.48)
        // -------------------------------------------------------------
        float nextAngle = ((float)(i + 1)) * (2.0f * PI / (float)numPillars);
        float midAngle = (angle + nextAngle) * 0.5f;
        float chordDist = 2.0f * pillarRadius * std::sin(PI / (float)numPillars);
        glm::mat4 rm = glm::rotate(base, midAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        float wallZ = pillarRadius * std::cos(PI / (float)numPillars);

        // Lower timber frieze beam
        standardShader->setVec3("solidColor", glm::vec3(0.92f, 0.92f, 0.90f));
        m = glm::translate(rm, glm::vec3(0.0f, 3.32f, wallZ));
        m = glm::scale(m, glm::vec3(chordDist * 1.06f, 0.18f, 0.28f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Upper cornice molding
        standardShader->setVec3("solidColor", glm::vec3(0.88f, 0.88f, 0.86f));
        m = glm::translate(rm, glm::vec3(0.0f, 3.44f, wallZ));
        m = glm::scale(m, glm::vec3(chordDist * 1.09f, 0.08f, 0.36f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // -------------------------------------------------------------
    // 3. VICTORIAN BELL-CAST ROOF & VENTED CUPOLA
    // -------------------------------------------------------------
    // Roof cornice eave ring (flared overhang sitting directly on entablature)
    standardShader->setVec3("solidColor", glm::vec3(0.25f, 0.38f, 0.34f)); // Forest green / copper patina
    m = glm::translate(base, glm::vec3(0.0f, 3.48f, 0.0f));
    m = glm::scale(m, glm::vec3(8.2f, 0.22f, 8.2f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Decorative white eave fascia band
    standardShader->setVec3("solidColor", glm::vec3(0.92f, 0.92f, 0.90f));
    m = glm::translate(base, glm::vec3(0.0f, 3.60f, 0.0f));
    m = glm::scale(m, glm::vec3(8.0f, 0.08f, 8.0f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Main roof cone (slate green / oxidized copper, slopes gracefully from Y = 3.68 to 5.45)
    standardShader->setVec3("solidColor", glm::vec3(0.24f, 0.36f, 0.32f));
    m = glm::translate(base, glm::vec3(0.0f, 3.68f, 0.0f));
    m = glm::scale(m, glm::vec3(4.0f, 1.85f, 4.0f));
    standardShader->setMat4("model", m);
    coneMesh->draw();

    // Vented Louver Lantern Cupola (from Y = 5.35 to 6.10, height 0.75m)
    // White louver housing
    standardShader->setVec3("solidColor", glm::vec3(0.94f, 0.94f, 0.92f));
    m = glm::translate(base, glm::vec3(0.0f, 5.35f, 0.0f));
    m = glm::scale(m, glm::vec3(1.5f, 0.75f, 1.5f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Louver horizontal dark slats
    standardShader->setVec3("solidColor", glm::vec3(0.2f, 0.25f, 0.28f));
    for (int l = 1; l <= 3; ++l) {
        m = glm::translate(base, glm::vec3(0.0f, 5.35f + l * 0.18f, 0.0f));
        m = glm::scale(m, glm::vec3(1.54f, 0.04f, 1.54f));
        standardShader->setMat4("model", m);
        columnMesh->draw();
    }

    // Cupola Roof Hat (from Y = 6.10 to 6.75)
    standardShader->setVec3("solidColor", glm::vec3(0.25f, 0.38f, 0.34f));
    m = glm::translate(base, glm::vec3(0.0f, 6.10f, 0.0f));
    m = glm::scale(m, glm::vec3(1.05f, 0.65f, 1.05f));
    standardShader->setMat4("model", m);
    coneMesh->draw();

    // Antique Brass Weathervane Spire
    standardShader->setVec3("solidColor", glm::vec3(0.85f, 0.74f, 0.32f)); // Polished brass
    // Vertical spire rod
    m = glm::translate(base, glm::vec3(0.0f, 6.75f, 0.0f));
    m = glm::scale(m, glm::vec3(0.06f, 0.75f, 0.06f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Directional cardinal cross arms (N-S-E-W)
    m = glm::translate(base, glm::vec3(0.0f, 7.20f, 0.0f));
    m = glm::scale(m, glm::vec3(0.65f, 0.035f, 0.035f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(base, glm::vec3(0.0f, 7.20f, 0.0f));
    m = glm::scale(m, glm::vec3(0.035f, 0.035f, 0.65f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Weather vane pointer arrow on top
    m = glm::translate(base, glm::vec3(0.12f, 7.42f, 0.0f));
    m = glm::scale(m, glm::vec3(0.40f, 0.06f, 0.03f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(base, glm::vec3(0.0f, 7.50f, 0.0f));
    m = glm::scale(m, glm::vec3(0.10f, 0.10f, 0.10f));
    standardShader->setMat4("model", m);
    sphereMesh->draw();

    // -------------------------------------------------------------
    // 4. INTERIOR CEILING, BENCH & WARM LANTERN
    // -------------------------------------------------------------
    // Circular indoor bench for resting
    glm::vec3 insideBenchPos = position + glm::vec3(0.0f, 0.0f, 1.8f);
    drawBench(insideBenchPos, glm::radians(180.0f));

    // Octagonal tea table
    standardShader->setVec3("solidColor", glm::vec3(0.52f, 0.34f, 0.18f));
    m = glm::translate(base, glm::vec3(0.0f, 0.90f, 0.0f));
    m = glm::scale(m, glm::vec3(1.5f, 0.08f, 1.5f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Table pedestal base
    m = glm::translate(base, glm::vec3(0.0f, 0.55f, 0.0f));
    m = glm::scale(m, glm::vec3(0.28f, 0.35f, 0.28f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Hanging Victorian pendant lantern
    standardShader->setVec3("solidColor", glm::vec3(0.15f, 0.15f, 0.18f)); // Dark iron
    m = glm::translate(base, glm::vec3(0.0f, 3.10f, 0.0f));
    m = glm::scale(m, glm::vec3(0.04f, 0.55f, 0.04f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Lantern fixture
    m = glm::translate(base, glm::vec3(0.0f, 2.85f, 0.0f));
    m = glm::scale(m, glm::vec3(0.26f, 0.35f, 0.26f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Glowing warm lantern core
    standardShader->setVec3("solidColor", glm::vec3(1.0f, 0.88f, 0.55f));
    standardShader->setFloat("emissive", 0.95f);
    m = glm::translate(base, glm::vec3(0.0f, 2.85f, 0.0f));
    m = glm::scale(m, glm::vec3(0.18f, 0.24f, 0.18f));
    standardShader->setMat4("model", m);
    sphereMesh->draw();
    standardShader->setFloat("emissive", 0.0f);

    // -------------------------------------------------------------
    // 5. WHITE DIAMOND LATTICE TRELLIS ARBOR & CLIMBING ROSES
    // -------------------------------------------------------------
    // Flanking the entrance path at Z = -4.6m (ground level Y = 0.0m)
    standardShader->setVec3("solidColor", glm::vec3(0.96f, 0.96f, 0.95f)); // White painted wood

    // Left trellis post
    m = glm::translate(base, glm::vec3(-1.60f, 0.0f, -4.60f));
    m = glm::scale(m, glm::vec3(0.16f, 2.85f, 0.16f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Right trellis post
    m = glm::translate(base, glm::vec3(1.60f, 0.0f, -4.60f));
    m = glm::scale(m, glm::vec3(0.16f, 2.85f, 0.16f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Overhead arbor header crossbeam
    m = glm::translate(base, glm::vec3(0.0f, 2.85f, -4.60f));
    m = glm::scale(m, glm::vec3(3.60f, 0.14f, 0.20f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Decorative pergolas / rafter tails across arbor top
    for (int r = -2; r <= 2; ++r) {
        m = glm::translate(base, glm::vec3(r * 0.70f, 2.94f, -4.60f));
        m = glm::scale(m, glm::vec3(0.10f, 0.10f, 0.75f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // Side lattice panels (left and right)
    standardShader->setVec3("solidColor", glm::vec3(0.92f, 0.92f, 0.90f));
    for (int h = 1; h <= 6; ++h) {
        float ly = 0.40f * h;
        // Left horizontal lattice slats
        m = glm::translate(base, glm::vec3(-1.60f, ly, -4.85f));
        m = glm::scale(m, glm::vec3(0.05f, 0.05f, 0.65f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
        // Right horizontal lattice slats
        m = glm::translate(base, glm::vec3(1.60f, ly, -4.85f));
        m = glm::scale(m, glm::vec3(0.05f, 0.05f, 0.65f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // Climbing Dark Green Ivy Vines wrapping around posts
    standardShader->setVec3("solidColor", glm::vec3(0.18f, 0.48f, 0.20f));
    for (int v = 0; v < 8; ++v) {
        float vy = 0.35f + v * 0.30f;
        float vRot = v * 0.8f;
        // Left post ivy leaves
        m = glm::translate(base, glm::vec3(-1.60f + std::sin(vRot) * 0.12f, vy, -4.60f + std::cos(vRot) * 0.12f));
        m = glm::scale(m, glm::vec3(0.18f, 0.14f, 0.18f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();

        // Right post ivy leaves
        m = glm::translate(base, glm::vec3(1.60f + std::cos(vRot) * 0.12f, vy, -4.60f + std::sin(vRot) * 0.12f));
        m = glm::scale(m, glm::vec3(0.18f, 0.14f, 0.18f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();
    }

    // Overhead arbor vine runners
    for (int a = -3; a <= 3; ++a) {
        m = glm::translate(base, glm::vec3(a * 0.45f, 2.88f, -4.60f + ((a % 2) ? 0.06f : -0.06f)));
        m = glm::scale(m, glm::vec3(0.24f, 0.16f, 0.22f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();
    }

    // Blooming Tea Rose Blossoms (delicate multi-toned clusters: blush pink, scarlet red, cream)
    glm::vec3 roseColors[] = {
        glm::vec3(0.96f, 0.45f, 0.62f), // Blush pink
        glm::vec3(0.90f, 0.18f, 0.24f), // Crimson ruby
        glm::vec3(0.98f, 0.88f, 0.82f), // Cream white
        glm::vec3(0.95f, 0.35f, 0.52f)  // Rose pink
    };

    struct RoseSpot { float x, y, z; int col; float sz; };
    RoseSpot roses[] = {
        {-1.65f, 0.9f, -4.50f, 0, 0.12f}, {-1.55f, 1.4f, -4.70f, 1, 0.14f},
        {-1.62f, 1.9f, -4.52f, 2, 0.11f}, {-1.58f, 2.4f, -4.65f, 3, 0.13f},
        { 1.65f, 0.8f, -4.55f, 1, 0.13f}, { 1.58f, 1.3f, -4.68f, 0, 0.12f},
        { 1.62f, 1.8f, -4.50f, 3, 0.14f}, { 1.55f, 2.3f, -4.68f, 2, 0.11f},
        {-1.00f, 2.92f, -4.55f, 0, 0.13f}, {-0.45f, 2.90f, -4.65f, 1, 0.14f},
        { 0.15f, 2.94f, -4.52f, 2, 0.12f}, { 0.75f, 2.91f, -4.64f, 0, 0.14f},
        { 1.20f, 2.93f, -4.56f, 3, 0.13f}
    };

    for (const auto& r : roses) {
        standardShader->setVec3("solidColor", roseColors[r.col]);
        m = glm::translate(base, glm::vec3(r.x, r.y, r.z));
        m = glm::scale(m, glm::vec3(r.sz, r.sz * 0.85f, r.sz));
        standardShader->setMat4("model", m);
        sphereMesh->draw();
    }
}

void Park::drawPicnicArea(glm::vec3 position) {
    standardShader->use();
    standardShader->setBool("useTexture", false);
    standardShader->setFloat("emissive", 0.0f);

    position.y = Mesh::getTerrainHeight(position.x, position.z);
    glm::mat4 base = glm::translate(glm::mat4(1.0f), position);

    // 1. Three Wooden Picnic Tables with attached benches
    auto drawPicnicTable = [&](glm::vec3 offset, float rotY, bool checkeredCloth) {
        glm::mat4 tm = glm::translate(base, offset);
        tm = glm::rotate(tm, rotY, glm::vec3(0.0f, 1.0f, 0.0f));

        // Wooden A-frame legs
        standardShader->setVec3("solidColor", glm::vec3(0.38f, 0.24f, 0.12f));
        float legX[] = {-0.9f, 0.9f};
        for (float lx : legX) {
            // Diagonal leg 1
            glm::mat4 m = glm::translate(tm, glm::vec3(lx, 0.38f, -0.4f));
            m = glm::rotate(m, glm::radians(18.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            m = glm::scale(m, glm::vec3(0.1f, 0.8f, 0.1f));
            standardShader->setMat4("model", m);
            boxMesh->draw();

            // Diagonal leg 2
            m = glm::translate(tm, glm::vec3(lx, 0.38f, 0.4f));
            m = glm::rotate(m, glm::radians(-18.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            m = glm::scale(m, glm::vec3(0.1f, 0.8f, 0.1f));
            standardShader->setMat4("model", m);
            boxMesh->draw();

            // Cross beam connecting benches
            m = glm::translate(tm, glm::vec3(lx, 0.38f, 0.0f));
            m = glm::scale(m, glm::vec3(0.1f, 0.08f, 1.6f));
            standardShader->setMat4("model", m);
            boxMesh->draw();
        }

        // Tabletop wooden planks
        if (checkeredCloth) {
            standardShader->setVec3("solidColor", glm::vec3(0.88f, 0.22f, 0.22f)); // Red checkered cloth
        } else {
            standardShader->setVec3("solidColor", glm::vec3(0.55f, 0.36f, 0.18f));
        }
        glm::mat4 m = glm::translate(tm, glm::vec3(0.0f, 0.76f, 0.0f));
        m = glm::scale(m, glm::vec3(2.3f, 0.08f, 0.95f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Attached Benches (Z = -0.75m and +0.75m)
        standardShader->setVec3("solidColor", glm::vec3(0.52f, 0.34f, 0.16f));
        // North bench
        m = glm::translate(tm, glm::vec3(0.0f, 0.42f, -0.72f));
        m = glm::scale(m, glm::vec3(2.3f, 0.06f, 0.32f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // South bench
        m = glm::translate(tm, glm::vec3(0.0f, 0.42f, 0.72f));
        m = glm::scale(m, glm::vec3(2.3f, 0.06f, 0.32f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Register table benches for sitting
        glm::vec3 worldTablePos = position + offset;
        benchList.push_back({worldTablePos + glm::vec3(0.0f, 0.0f, -0.72f), rotY});
        benchList.push_back({worldTablePos + glm::vec3(0.0f, 0.0f, 0.72f), rotY + 3.14159f});
    };

    drawPicnicTable(glm::vec3(0.0f, 0.0f, 5.0f), 0.0f, true);
    drawPicnicTable(glm::vec3(-5.5f, 0.0f, -3.0f), glm::radians(35.0f), false);
    drawPicnicTable(glm::vec3(5.5f, 0.0f, -3.0f), glm::radians(-35.0f), false);

    // 2. Heavy-duty Park BBQ Charcoal Grill on cast-iron pedestal
    glm::mat4 gm = glm::translate(base, glm::vec3(0.0f, 0.0f, -5.5f));
    standardShader->setVec3("solidColor", glm::vec3(0.12f, 0.12f, 0.14f)); // Cast iron black

    // Post (planted firmly from ground Y=0 to 0.85m)
    glm::mat4 m = glm::translate(gm, glm::vec3(0.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.16f, 0.85f, 0.16f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Firebox
    m = glm::translate(gm, glm::vec3(0.0f, 0.95f, 0.0f));
    m = glm::scale(m, glm::vec3(1.1f, 0.35f, 0.85f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Grill steel grate
    standardShader->setVec3("solidColor", glm::vec3(0.35f, 0.35f, 0.38f));
    m = glm::translate(gm, glm::vec3(0.0f, 1.14f, 0.0f));
    m = glm::scale(m, glm::vec3(1.05f, 0.03f, 0.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Glowing red charcoal embers inside grill
    standardShader->setVec3("solidColor", glm::vec3(1.0f, 0.25f, 0.05f));
    standardShader->setFloat("emissive", 1.2f);
    m = glm::translate(gm, glm::vec3(0.0f, 1.05f, 0.0f));
    m = glm::scale(m, glm::vec3(0.95f, 0.08f, 0.7f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    standardShader->setFloat("emissive", 0.0f);

    // 3. Campfire Stone Ring & Firewood
    glm::mat4 cm = glm::translate(base, glm::vec3(0.0f, 0.0f, -0.5f));

    // Ring of 12 river stones
    standardShader->setVec3("solidColor", glm::vec3(0.55f, 0.52f, 0.48f));
    for (int st = 0; st < 12; ++st) {
        float stAngle = st * (6.283185f / 12);
        m = glm::rotate(cm, stAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::translate(m, glm::vec3(1.5f, 0.12f, 0.0f));
        m = glm::scale(m, glm::vec3(0.35f, 0.22f, 0.32f));
        standardShader->setMat4("model", m);
        sphereMesh->draw();
    }

    // Charred ash bed in center
    standardShader->setVec3("solidColor", glm::vec3(0.15f, 0.14f, 0.13f));
    m = glm::translate(cm, glm::vec3(0.0f, 0.02f, 0.0f));
    m = glm::scale(m, glm::vec3(2.5f, 0.05f, 2.5f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // Stacked firewood logs
    standardShader->setVec3("solidColor", glm::vec3(0.42f, 0.26f, 0.14f));
    for (int l = 0; l < 4; ++l) {
        float logAngle = l * glm::radians(45.0f);
        m = glm::rotate(cm, logAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::translate(m, glm::vec3(0.0f, 0.16f + l * 0.06f, 0.0f));
        m = glm::scale(m, glm::vec3(0.2f, 0.18f, 1.4f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }
}

void Park::drawBicycle(glm::vec3 position, float yaw, float steerAngle, float wheelSpin) {
    standardShader->use();
    standardShader->setBool("useTexture", false);
    standardShader->setFloat("emissive", 0.0f);

    position.y = Mesh::getTerrainHeight(position.x, position.z);
    glm::mat4 base = glm::translate(glm::mat4(1.0f), position);
    base = glm::rotate(base, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));

    // Wheels (radius 0.42m)
    auto drawWheel = [&](glm::vec3 center, float spin, float steer) {
        glm::mat4 wm = glm::translate(base, center);
        if (steer != 0.0f) wm = glm::rotate(wm, glm::radians(steer), glm::vec3(0.0f, 1.0f, 0.0f));
        wm = glm::rotate(wm, spin, glm::vec3(1.0f, 0.0f, 0.0f));

        // Black Rubber Tire
        standardShader->setVec3("solidColor", glm::vec3(0.12f, 0.12f, 0.14f));
        glm::mat4 m = glm::scale(wm, glm::vec3(0.08f, 0.84f, 0.84f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();

        // Silver Rim & Spokes
        standardShader->setVec3("solidColor", glm::vec3(0.75f, 0.75f, 0.78f));
        for (int sp = 0; sp < 4; ++sp) {
            m = glm::rotate(wm, glm::radians(sp * 45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            m = glm::scale(m, glm::vec3(0.025f, 0.72f, 0.025f));
            standardShader->setMat4("model", m);
            boxMesh->draw();
        }

        // Center chrome hub
        m = glm::scale(wm, glm::vec3(0.12f, 0.12f, 0.12f));
        standardShader->setMat4("model", m);
        cylinderMesh->draw();
    };

    // Draw Rear Wheel
    drawWheel(glm::vec3(0.0f, 0.42f, -0.7f), wheelSpin, 0.0f);

    // Draw Front Wheel (steered)
    drawWheel(glm::vec3(0.0f, 0.42f, 0.7f), wheelSpin, steerAngle);

    // Bicycle Frame (Tubular Diamond Frame - Bright Azure Blue)
    standardShader->setVec3("solidColor", glm::vec3(0.08f, 0.58f, 0.92f));

    glm::vec3 bb(0.0f, 0.42f, -0.08f);
    glm::vec3 seatNode(0.0f, 0.88f, -0.28f);
    glm::vec3 headNode(0.0f, 0.92f, 0.52f);
    glm::vec3 rearAxle(0.0f, 0.42f, -0.7f);

    // Seat Tube (from BB up to seat node)
    glm::mat4 m = glm::translate(base, (bb + seatNode) * 0.5f);
    m = glm::rotate(m, glm::radians(-18.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.045f, 0.52f, 0.045f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Top Tube (from seat node to head node)
    m = glm::translate(base, (seatNode + headNode) * 0.5f);
    m = glm::scale(m, glm::vec3(0.045f, 0.045f, 0.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Down Tube (from BB to head node)
    m = glm::translate(base, (bb + headNode) * 0.5f);
    m = glm::rotate(m, glm::radians(35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.048f, 0.75f, 0.048f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Chain Stays (from BB to rear axle)
    m = glm::translate(base, (bb + rearAxle) * 0.5f);
    m = glm::scale(m, glm::vec3(0.07f, 0.035f, 0.62f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Seat Stays (from seat node down to rear axle)
    m = glm::translate(base, (seatNode + rearAxle) * 0.5f);
    m = glm::rotate(m, glm::radians(-38.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.07f, 0.62f, 0.035f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Front Fork (from head node down to front axle)
    standardShader->setVec3("solidColor", glm::vec3(0.75f, 0.75f, 0.78f)); // Chrome fork
    glm::vec3 frontAxle(0.0f, 0.42f, 0.7f);
    m = glm::translate(base, (headNode + frontAxle) * 0.5f);
    m = glm::rotate(m, glm::radians(steerAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::rotate(m, glm::radians(18.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.08f, 0.56f, 0.04f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Handlebar Stem & Crossbar
    m = glm::translate(base, headNode + glm::vec3(0.0f, 0.12f, 0.02f));
    m = glm::rotate(m, glm::radians(steerAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.65f, 0.035f, 0.035f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Black Rubber Handlebar Grips
    standardShader->setVec3("solidColor", glm::vec3(0.1f, 0.1f, 0.12f));
    m = glm::translate(base, headNode + glm::vec3(-0.28f, 0.12f, 0.02f));
    m = glm::rotate(m, glm::radians(steerAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.12f, 0.045f, 0.045f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(base, headNode + glm::vec3(0.28f, 0.12f, 0.02f));
    m = glm::rotate(m, glm::radians(steerAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.12f, 0.045f, 0.045f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Cushioned Saddle Seat
    m = glm::translate(base, seatNode + glm::vec3(0.0f, 0.12f, -0.02f));
    m = glm::scale(m, glm::vec3(0.18f, 0.06f, 0.32f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Pedal Cranks & Pedals
    standardShader->setVec3("solidColor", glm::vec3(0.3f, 0.3f, 0.35f));
    m = glm::translate(base, bb + glm::vec3(-0.1f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.03f, 0.18f, 0.03f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(base, bb + glm::vec3(0.1f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.03f, 0.18f, 0.03f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
}

void Park::draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos, float yaw, float time, bool isSitting, bool isRidingBike, float bikeSpeed) {
    benchList.clear();

    // Update player interaction prompt
    if (isSitting) {
        currentPrompt = "Press [E] or [WASD] to Stand Up";
    } else if (isRidingBike) {
        currentPrompt = "BICYCLE: [W] Pedal  [S] Brake  [A/D] Steer  [F] Dismount";
    } else {
        if (checkNearBicycle(viewPos)) {
            currentPrompt = "Press [F] to Ride Bicycle";
        } else {
            glm::vec3 sitPos;
            float sitYaw;
            if (checkNearBench(viewPos, sitPos, sitYaw)) {
                currentPrompt = "Press [E] to Sit on Bench";
            } else {
                currentPrompt = "";
            }
        }
    }

    // Dynamic Lighting & Atmosphere based on Time of Day
    glm::vec3 lightPos;
    glm::vec3 lightColor;
    glm::vec3 fogColor;
    float fogDensity = 0.0035f;
    bool lightsOn = false;

    if (currentTimeOfDay == TIME_NOON) {
        lightPos = glm::vec3(50.0f, 120.0f, 50.0f);
        lightColor = glm::vec3(1.05f, 1.05f, 0.98f);
        fogColor = glm::vec3(0.53f, 0.81f, 0.92f);
        fogDensity = 0.0035f;
        lightsOn = false;
    } else if (currentTimeOfDay == TIME_SUNSET) {
        lightPos = glm::vec3(150.0f, 25.0f, 100.0f);
        lightColor = glm::vec3(1.2f, 0.65f, 0.35f);
        fogColor = glm::vec3(0.85f, 0.48f, 0.32f);
        fogDensity = 0.0045f;
        lightsOn = true;
    } else { // TIME_NIGHT
        lightPos = glm::vec3(10.0f, 60.0f, 10.0f);
        lightColor = glm::vec3(0.18f, 0.22f, 0.35f);
        fogColor = glm::vec3(0.04f, 0.06f, 0.12f);
        fogDensity = 0.0075f;
        lightsOn = true;
    }

    // --- Render Realistic Atmospheric Sky Dome ---
    drawSky(view, projection, time);

    standardShader->use();
    standardShader->setMat4("view", view);
    standardShader->setMat4("projection", projection);
    standardShader->setVec3("viewPos", viewPos);
    standardShader->setVec3("lightPos", lightPos);
    standardShader->setVec3("lightColor", lightColor);
    standardShader->setVec3("fogColor", fogColor);
    standardShader->setFloat("fogDensity", fogDensity);
    standardShader->setFloat("time", time);
    standardShader->setBool("isWater", false);
    standardShader->setFloat("emissive", 0.0f);

    glm::mat4 model = glm::mat4(1.0f);
    standardShader->setMat4("model", model);
    standardShader->setVec2("texScale", glm::vec2(1.0f, 1.0f));
    standardShader->setVec3("solidColor", glm::vec3(1.0f, 1.0f, 1.0f));
    standardShader->setBool("useTexture", true);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texGrass);
    groundMesh->draw();

    // --- Massively Expanded Path Network ---
    float pathWidth = 10.0f;
    
    // Vertical Axis (runs completely from front to back)
    drawPathStraight(glm::vec3(0.0f, 0.0f, -380.0f), glm::vec3(0.0f, 0.0f, 380.0f), pathWidth);
    
    // Horizontal Axis (runs completely left to right)
    drawPathStraight(glm::vec3(-380.0f, 0.0f, 0.0f), glm::vec3(380.0f, 0.0f, 0.0f), pathWidth);

    // Center Circular Plaza (radius 40m)
    drawPathCurve(glm::vec3(0.0f, 0.0f, 0.0f), 40.0f, 0.0f, glm::radians(360.0f), 12.0f);

    // Massive Outer Ring Road (radius 250m)
    drawPathCurve(glm::vec3(0.0f, 0.0f, 0.0f), 250.0f, 0.0f, glm::radians(360.0f), pathWidth);

    // Branch Path leading to the Sign
    drawPathStraight(glm::vec3(0.0f, 0.0f, -80.0f), glm::vec3(80.0f, 0.0f, -80.0f), 6.0f);

    // Branch Paths to Restaurants and Shop
    drawPathStraight(glm::vec3(0.0f, 0.0f, 150.0f), glm::vec3(250.0f, 0.0f, 150.0f), 6.0f); // East Rest
    drawPathStraight(glm::vec3(-100.0f, 0.0f, 0.0f), glm::vec3(-100.0f, 0.0f, -25.0f), 6.0f); // West Rest
    drawPathStraight(glm::vec3(0.0f, 0.0f, 50.0f), glm::vec3(50.0f, 0.0f, 50.0f), 4.0f); // Shop

    // Branch Path to Kids Playground Area (-60, 60)
    drawPathStraight(glm::vec3(-28.0f, 0.0f, 28.0f), glm::vec3(-52.0f, 0.0f, 52.0f), 5.0f);

    // Branch Path to Victorian Gazebo (-40, -40)
    drawPathStraight(glm::vec3(-20.0f, 0.0f, -20.0f), glm::vec3(-35.0f, 0.0f, -35.0f), 4.0f);

    // Branch Path to Picnic Area (40, -40)
    drawPathStraight(glm::vec3(20.0f, 0.0f, -20.0f), glm::vec3(35.0f, 0.0f, -35.0f), 4.0f);

    // Branch Path to Lake Boardwalk Pier (-76, -100)
    drawPathStraight(glm::vec3(0.0f, 0.0f, -100.0f), glm::vec3(-76.0f, 0.0f, -100.0f), 5.0f);

    // --- Draw Trees with diverse species, scaling, and organic colors ---
    for (const auto& tree : trees) {
        drawTree(tree);
    }
    
    // Draw Kids Playground!
    drawPlayground(glm::vec3(-60.0f, 0.0f, 60.0f));

    // --- Draw NEW Attractions & Landmarks ---
    // 0. Grand Central Plaza Terrace (Elevated French limestone foundation)
    drawPlazaTerrace();

    // 1. Grand Central Plaza Fountain at (0, 0)
    drawFountain(glm::vec3(0.0f, 0.0f, 0.0f), time);

    // 2. Lake Boardwalk Pier & Floating Rowboats at (-76, 0, -100)
    drawLakePier(glm::vec3(-76.0f, 0.0f, -100.0f), time);

    // 3. Victorian Gazebo at (-40, 0, -40) with botanical garden borders
    drawGazebo(glm::vec3(-40.0f, 0.0f, -40.0f));
    drawFlowerBed(glm::vec3(-43.5f, 0.0f, -44.5f), 2.2f, 3.5f);
    drawFlowerBed(glm::vec3(-36.5f, 0.0f, -44.5f), 2.2f, 3.5f);

    // 4. Picnic & BBQ Area at (40, 0, -40)
    drawPicnicArea(glm::vec3(40.0f, 0.0f, -40.0f));

    // 5. Driveable Bicycle
    if (isRidingBike) {
        drawBicycle(viewPos + glm::vec3(0.0f, -1.9f, 0.0f), yaw, 0.0f, time * bikeSpeed * 5.0f);
    } else {
        drawBicycle(bikePos, bikeYaw, 0.0f, 0.0f);
    }

    // --- Central Plaza Amenities ---
    // 8 radial benches arranged inside the 4 grass quadrants (NE, NW, SE, SW),
    // strictly avoiding the crossroad axes (0, 90, 180, 270)
    const float plazaAngles[] = {
        glm::radians(32.0f), glm::radians(58.0f),   // Quadrant 1 (NE)
        glm::radians(122.0f), glm::radians(148.0f), // Quadrant 2 (SE)
        glm::radians(212.0f), glm::radians(238.0f), // Quadrant 3 (SW)
        glm::radians(302.0f), glm::radians(328.0f)  // Quadrant 4 (NW)
    };
    for (int i = 0; i < 8; ++i) {
        float angle = plazaAngles[i];
        float radius = 22.0f; // Safely on grass lawn facing fountain
        glm::vec3 benchPos(sin(angle) * radius, 0.0f, cos(angle) * radius);
        benchPos.y = Mesh::getTerrainHeight(benchPos.x, benchPos.z);
        drawBench(benchPos, angle + 3.14159f);

        // Place trash bins at alternating benches (tangentially offset on grass)
        if (i % 2 == 0) {
            glm::vec3 binPos = benchPos + glm::vec3(-cos(angle) * 1.8f, 0.0f, sin(angle) * 1.8f);
            binPos.y = Mesh::getTerrainHeight(binPos.x, binPos.z);
            drawTrashBin(binPos);
        }
    }

    // Plaza Corner Flowerbeds
    drawFlowerBed(glm::vec3(22.0f, 0.0f, 22.0f), 4.5f, 4.5f);
    drawFlowerBed(glm::vec3(-22.0f, 0.0f, 22.0f), 4.5f, 4.5f);
    drawFlowerBed(glm::vec3(22.0f, 0.0f, -22.0f), 4.5f, 4.5f);
    drawFlowerBed(glm::vec3(-22.0f, 0.0f, -22.0f), 4.5f, 4.5f);

    // Plaza Corner Street Lamps (with day/night emissive lighting)
    drawLampPost(glm::vec3(18.0f, Mesh::getTerrainHeight(18.0f, 18.0f), 18.0f), lightsOn);
    drawLampPost(glm::vec3(-18.0f, Mesh::getTerrainHeight(-18.0f, 18.0f), 18.0f), lightsOn);
    drawLampPost(glm::vec3(18.0f, Mesh::getTerrainHeight(18.0f, -18.0f), -18.0f), lightsOn);
    drawLampPost(glm::vec3(-18.0f, Mesh::getTerrainHeight(-18.0f, -18.0f), -18.0f), lightsOn);

    // Promenade Benches & Street Lamps along North/South path (Z axis)
    const float benchZ[] = {-315.0f, -200.0f, -135.0f, -65.0f, 65.0f, 120.0f, 180.0f, 215.0f, 315.0f};
    for (float bz : benchZ) {
        // Right side of path
        glm::vec3 b1(9.0f, 0.0f, bz);
        b1.y = Mesh::getTerrainHeight(b1.x, b1.z);
        drawBench(b1, glm::radians(-90.0f)); // Facing path (West)
        
        // Left side of path
        glm::vec3 b2(-9.0f, 0.0f, bz);
        b2.y = Mesh::getTerrainHeight(b2.x, b2.z);
        drawBench(b2, glm::radians(90.0f)); // Facing path (East)

        // Street lamps
        glm::vec3 lamp1(9.5f, 0.0f, bz + 10.0f);
        lamp1.y = Mesh::getTerrainHeight(lamp1.x, lamp1.z);
        drawLampPost(lamp1, lightsOn);

        glm::vec3 lamp2(-9.5f, 0.0f, bz + 10.0f);
        lamp2.y = Mesh::getTerrainHeight(lamp2.x, lamp2.z);
        drawLampPost(lamp2, lightsOn);
    }

    // Lake Walkway Street Lamps (illuminating path from promenade to pier entrance)
    drawLampPost(glm::vec3(-25.0f, Mesh::getTerrainHeight(-25.0f, -104.0f), -104.0f), lightsOn);
    drawLampPost(glm::vec3(-55.0f, Mesh::getTerrainHeight(-55.0f, -104.0f), -104.0f), lightsOn);

    // Lake Pier Entrance Flanking Benches (safely on solid grass bank flanking walkway - zero pier clipping!)
    glm::vec3 pierB1(-72.0f, 0.0f, -106.0f);
    pierB1.y = Mesh::getTerrainHeight(pierB1.x, pierB1.z);
    drawBench(pierB1, glm::radians(90.0f)); // Facing path / lake North-West
    drawTrashBin(glm::vec3(pierB1.x - 1.8f, pierB1.y, pierB1.z));

    glm::vec3 pierB2(-72.0f, 0.0f, -94.0f);
    pierB2.y = Mesh::getTerrainHeight(pierB2.x, pierB2.z);
    drawBench(pierB2, glm::radians(-90.0f)); // Facing path / lake South-West
    drawTrashBin(glm::vec3(pierB2.x - 1.8f, pierB2.y, pierB2.z));

    // North Shore Scenic Lake Bench
    glm::vec3 pondB2(-100.0f, 0.0f, -76.0f);
    pondB2.y = Mesh::getTerrainHeight(pondB2.x, pondB2.z);
    drawBench(pondB2, glm::radians(180.0f)); // Facing South toward open water
    drawTrashBin(glm::vec3(pondB2.x + 1.8f, pondB2.y, pondB2.z));

    // Draw 3D Landmark Sign in the mountainous grass!
    glm::vec3 signPos(80.0f, Mesh::getTerrainHeight(80.0f, -80.0f), -80.0f);
    drawSign(signPos, glm::radians(-90.0f));
    
    // Draw the Water Lake at (-100, -100) with animated ripples and high-res circular geometry
    standardShader->setBool("useTexture", true);
    standardShader->setBool("isWater", true);
    standardShader->setFloat("time", time);
    standardShader->setVec2("texScale", glm::vec2(3.0f, 3.0f));
    standardShader->setVec3("solidColor", glm::vec3(0.68f, 0.88f, 0.98f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texWater);
    
    glm::mat4 pondModel = glm::mat4(1.0f);
    pondModel = glm::translate(pondModel, glm::vec3(-100.0f, -0.45f, -100.0f));
    pondModel = glm::scale(pondModel, glm::vec3(21.2f, 1.0f, 21.2f));
    standardShader->setMat4("model", pondModel);
    lakeMesh->draw();

    standardShader->setBool("isWater", false);

    // Natural Shoreline: sand/pebble beach ring, river boulders, cattail reeds & water lilies
    drawShoreline(glm::vec3(-100.0f, 0.0f, -100.0f), 20.8f);
    
    // Draw Restaurants (Craftsman Alpine Park Lodges)
    glm::vec3 eastRestPos(250.0f, Mesh::getTerrainHeight(250.0f, 150.0f), 150.0f);
    drawRestaurant(eastRestPos, glm::radians(180.0f)); // Facing North
    drawFlowerBed(eastRestPos + glm::vec3(-15.0f, 0.0f, -14.0f), 3.0f, 5.0f);
    drawFlowerBed(eastRestPos + glm::vec3(15.0f, 0.0f, -14.0f), 3.0f, 5.0f);

    glm::vec3 westRestPos(-100.0f, Mesh::getTerrainHeight(-100.0f, -30.0f), -30.0f);
    drawRestaurant(westRestPos, glm::radians(0.0f)); // Facing South towards path
    drawFlowerBed(westRestPos + glm::vec3(-15.0f, 0.0f, 14.0f), 3.0f, 5.0f);
    drawFlowerBed(westRestPos + glm::vec3(15.0f, 0.0f, 14.0f), 3.0f, 5.0f);
    
    // Draw Small Coffee Shop
    glm::vec3 shopPos(50.0f, Mesh::getTerrainHeight(50.0f, 50.0f), 50.0f);
    drawShop(shopPos, glm::radians(180.0f)); // Facing North towards path
    
    // Draw Border Fence (expanded to 380x380)
    drawFence(380.0f);
    
    // Draw North Entrance Reception
    drawBuilding(glm::vec3(-12.0f, 0.0f, -380.0f));
    drawBuilding(glm::vec3(12.0f, 0.0f, -380.0f));
    
    // Entrance flowerbeds
    drawFlowerBed(glm::vec3(-18.0f, 0.0f, -376.0f), 3.0f, 5.0f);
    drawFlowerBed(glm::vec3(18.0f, 0.0f, -376.0f), 3.0f, 5.0f);

    // Draw South Entrance Reception
    drawBuilding(glm::vec3(-12.0f, 0.0f, 380.0f));
    drawBuilding(glm::vec3(12.0f, 0.0f, 380.0f));
    
    drawHUD(viewPos, yaw, isSitting, isRidingBike, bikeSpeed);
}

void Park::drawRestaurant(glm::vec3 position, float rotationY) {
    standardShader->use();
    standardShader->setFloat("emissive", 0.0f);

    glm::mat4 base = glm::mat4(1.0f);
    base = glm::translate(base, position);
    base = glm::rotate(base, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));

    // -------------------------------------------------------------
    // 1. RUSTIC FIELDSTONE FOUNDATION / WAINSCOTING
    // -------------------------------------------------------------
    standardShader->setBool("useTexture", false);
    standardShader->setVec3("solidColor", glm::vec3(0.68f, 0.65f, 0.60f)); // River stone / granite
    glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.70f, 0.0f));
    m = glm::scale(m, glm::vec3(30.6f, 1.40f, 15.6f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Stone water table sill trim
    standardShader->setVec3("solidColor", glm::vec3(0.58f, 0.55f, 0.52f));
    m = glm::translate(base, glm::vec3(0.0f, 1.44f, 0.0f));
    m = glm::scale(m, glm::vec3(30.8f, 0.12f, 15.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // -------------------------------------------------------------
    // 2. MAIN TIMBER LOG WALLS
    // -------------------------------------------------------------
    standardShader->setBool("useTexture", true);
    standardShader->setVec2("texScale", glm::vec2(2.5f, 1.2f));
    standardShader->setVec3("solidColor", glm::vec3(0.95f, 0.90f, 0.85f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texRestaurant);

    m = glm::translate(base, glm::vec3(0.0f, 4.80f, 0.0f));
    m = glm::scale(m, glm::vec3(30.0f, 6.60f, 15.0f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Dark cedar corner trim posts
    standardShader->setBool("useTexture", false);
    standardShader->setVec3("solidColor", glm::vec3(0.32f, 0.22f, 0.14f));
    float cornersX[] = {-15.0f, 15.0f};
    float cornersZ[] = {-7.5f, 7.5f};
    for (float cx : cornersX) {
        for (float cz : cornersZ) {
            m = glm::translate(base, glm::vec3(cx, 4.80f, cz));
            m = glm::scale(m, glm::vec3(0.60f, 6.80f, 0.60f));
            standardShader->setMat4("model", m);
            boxMesh->draw();
        }
    }

    // Horizontal timber frieze band below roof
    m = glm::translate(base, glm::vec3(0.0f, 8.15f, 0.0f));
    m = glm::scale(m, glm::vec3(30.8f, 0.35f, 15.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // -------------------------------------------------------------
    // 3. PITCHED TIMBER GABLE ROOF WITH EXPOSED RAFTER TAILS
    // -------------------------------------------------------------
    // Lower eave fascia slab (wide overhang)
    standardShader->setVec3("solidColor", glm::vec3(0.22f, 0.22f, 0.25f)); // Dark slate
    m = glm::translate(base, glm::vec3(0.0f, 8.40f, 0.0f));
    m = glm::scale(m, glm::vec3(32.4f, 0.35f, 17.4f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Pitched Gable Roof Layers (sloping up to central ridge beam at Y = 11.8m)
    const int roofSteps = 7;
    for (int rs = 1; rs <= roofSteps; ++rs) {
        float frac = (float)rs / (float)roofSteps;
        float ry = 8.55f + frac * 2.80f;
        float rz = 17.2f * (1.0f - frac * 0.88f);
        float rx = 32.2f * (1.0f - frac * 0.12f);

        standardShader->setVec3("solidColor", glm::vec3(0.20f - frac * 0.04f, 0.22f - frac * 0.04f, 0.25f - frac * 0.04f));
        m = glm::translate(base, glm::vec3(0.0f, ry, 0.0f));
        m = glm::scale(m, glm::vec3(rx, 0.42f, rz));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // Ridge cap beam along the peak
    standardShader->setVec3("solidColor", glm::vec3(0.14f, 0.14f, 0.16f));
    m = glm::translate(base, glm::vec3(0.0f, 11.50f, 0.0f));
    m = glm::scale(m, glm::vec3(32.4f, 0.25f, 0.60f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Triangular Gable End Walls (West and East)
    standardShader->setVec3("solidColor", glm::vec3(0.42f, 0.30f, 0.18f));
    m = glm::translate(base, glm::vec3(-14.95f, 9.8f, 0.0f));
    m = glm::scale(m, glm::vec3(0.20f, 2.6f, 12.0f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(base, glm::vec3(14.95f, 9.8f, 0.0f));
    m = glm::scale(m, glm::vec3(0.20f, 2.6f, 12.0f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // -------------------------------------------------------------
    // 4. FIELDSTONE FIREPLACE CHIMNEY
    // -------------------------------------------------------------
    standardShader->setVec3("solidColor", glm::vec3(0.55f, 0.52f, 0.48f)); // River stone
    m = glm::translate(base, glm::vec3(12.0f, 6.5f, -7.2f));
    m = glm::scale(m, glm::vec3(2.2f, 12.5f, 2.2f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Terracotta chimney pots on top
    standardShader->setVec3("solidColor", glm::vec3(0.68f, 0.32f, 0.18f));
    m = glm::translate(base, glm::vec3(11.6f, 12.9f, -7.2f));
    m = glm::scale(m, glm::vec3(0.40f, 0.65f, 0.40f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    m = glm::translate(base, glm::vec3(12.4f, 12.9f, -7.2f));
    m = glm::scale(m, glm::vec3(0.40f, 0.65f, 0.40f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // -------------------------------------------------------------
    // 5. PICTURE WINDOWS WITH WARM GLOWING INTERIOR LIGHTS
    // -------------------------------------------------------------
    // 4 large windows on front facade (Z = 7.55m)
    float windowX[] = {-9.0f, -4.0f, 4.0f, 9.0f};
    for (float wx : windowX) {
        // Dark timber window frame
        standardShader->setVec3("solidColor", glm::vec3(0.22f, 0.15f, 0.10f));
        m = glm::translate(base, glm::vec3(wx, 4.6f, 7.55f));
        m = glm::scale(m, glm::vec3(2.8f, 2.4f, 0.16f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Warm glowing golden window panes
        standardShader->setVec3("solidColor", glm::vec3(1.0f, 0.88f, 0.52f));
        standardShader->setFloat("emissive", 0.68f);
        m = glm::translate(base, glm::vec3(wx, 4.6f, 7.60f));
        m = glm::scale(m, glm::vec3(2.5f, 2.1f, 0.10f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
        standardShader->setFloat("emissive", 0.0f);

        // Window mullion crossbars
        standardShader->setVec3("solidColor", glm::vec3(0.22f, 0.15f, 0.10f));
        m = glm::translate(base, glm::vec3(wx, 4.6f, 7.64f));
        m = glm::scale(m, glm::vec3(0.08f, 2.1f, 0.06f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        m = glm::translate(base, glm::vec3(wx, 4.6f, 7.64f));
        m = glm::scale(m, glm::vec3(2.5f, 0.08f, 0.06f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
    }

    // Main entrance double oak door in center (Z = 7.55m)
    standardShader->setVec3("solidColor", glm::vec3(0.42f, 0.26f, 0.14f));
    m = glm::translate(base, glm::vec3(0.0f, 3.2f, 7.58f));
    m = glm::scale(m, glm::vec3(2.4f, 3.6f, 0.15f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Covered Portico over entrance
    standardShader->setVec3("solidColor", glm::vec3(0.35f, 0.24f, 0.15f));
    m = glm::translate(base, glm::vec3(0.0f, 5.2f, 8.8f));
    m = glm::scale(m, glm::vec3(3.8f, 0.25f, 2.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Portico support columns
    m = glm::translate(base, glm::vec3(-1.6f, 0.0f, 9.8f));
    m = glm::scale(m, glm::vec3(0.28f, 5.2f, 0.28f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    m = glm::translate(base, glm::vec3(1.6f, 0.0f, 9.8f));
    m = glm::scale(m, glm::vec3(0.28f, 5.2f, 0.28f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // -------------------------------------------------------------
    // 6. OUTDOOR DINING VERANDA DECK WITH UMBRELLAS & CAFE TABLES
    // -------------------------------------------------------------
    // Wooden deck floor (width 28m, depth 4.5m, raised Y = 0.25m)
    standardShader->setVec3("solidColor", glm::vec3(0.55f, 0.38f, 0.22f)); // Deck stain
    m = glm::translate(base, glm::vec3(0.0f, 0.12f, 10.0f));
    m = glm::scale(m, glm::vec3(28.0f, 0.24f, 5.0f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Veranda handrails along front (Z = 12.4m)
    standardShader->setVec3("solidColor", glm::vec3(0.44f, 0.28f, 0.16f));
    // Left railing
    m = glm::translate(base, glm::vec3(-8.5f, 0.65f, 12.4f));
    m = glm::scale(m, glm::vec3(11.0f, 0.80f, 0.12f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    // Right railing
    m = glm::translate(base, glm::vec3(8.5f, 0.65f, 12.4f));
    m = glm::scale(m, glm::vec3(11.0f, 0.80f, 0.12f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // 3 Outdoor Cafe Bistro Sets with Canvas Sun Umbrellas
    struct CafeTable { float x; glm::vec3 umbrellaColor; };
    CafeTable tables[] = {
        {-8.0f, glm::vec3(0.18f, 0.42f, 0.30f)}, // Forest green
        { 0.0f, glm::vec3(0.72f, 0.20f, 0.22f)}, // Warm burgundy
        { 8.0f, glm::vec3(0.20f, 0.35f, 0.60f)}  // Navy blue
    };

    for (const auto& ct : tables) {
        glm::vec3 tPos(ct.x, 0.25f, 10.0f);

        // Round wooden bistro table
        standardShader->setVec3("solidColor", glm::vec3(0.48f, 0.32f, 0.18f));
        m = glm::translate(base, tPos + glm::vec3(0.0f, 0.68f, 0.0f));
        m = glm::scale(m, glm::vec3(1.3f, 0.06f, 1.3f));
        standardShader->setMat4("model", m);
        columnMesh->draw();

        // Table center stem
        m = glm::translate(base, tPos + glm::vec3(0.0f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(0.12f, 0.68f, 0.12f));
        standardShader->setMat4("model", m);
        columnMesh->draw();

        // 2 Cafe chairs
        standardShader->setVec3("solidColor", glm::vec3(0.25f, 0.25f, 0.28f)); // Dark metal
        // Chair 1
        m = glm::translate(base, tPos + glm::vec3(-0.85f, 0.42f, 0.0f));
        m = glm::scale(m, glm::vec3(0.45f, 0.05f, 0.45f));
        standardShader->setMat4("model", m);
        boxMesh->draw();
        // Chair 2
        m = glm::translate(base, tPos + glm::vec3(0.85f, 0.42f, 0.0f));
        m = glm::scale(m, glm::vec3(0.45f, 0.05f, 0.45f));
        standardShader->setMat4("model", m);
        boxMesh->draw();

        // Umbrella mast pole
        standardShader->setVec3("solidColor", glm::vec3(0.75f, 0.70f, 0.60f));
        m = glm::translate(base, tPos + glm::vec3(0.0f, 0.0f, 0.0f));
        m = glm::scale(m, glm::vec3(0.06f, 2.65f, 0.06f));
        standardShader->setMat4("model", m);
        columnMesh->draw();

        // Open Canvas Sun Umbrella Canopy
        standardShader->setVec3("solidColor", ct.umbrellaColor);
        m = glm::translate(base, tPos + glm::vec3(0.0f, 2.15f, 0.0f));
        m = glm::scale(m, glm::vec3(2.6f, 0.75f, 2.6f));
        standardShader->setMat4("model", m);
        coneMesh->draw();
    }
}

void Park::drawShop(glm::vec3 position, float rotationY) {
    standardShader->use();
    standardShader->setFloat("emissive", 0.0f);

    glm::mat4 base = glm::mat4(1.0f);
    base = glm::translate(base, position);
    base = glm::rotate(base, rotationY, glm::vec3(0.0f, 1.0f, 0.0f));

    // 1. Stone foundation base
    standardShader->setBool("useTexture", false);
    standardShader->setVec3("solidColor", glm::vec3(0.70f, 0.68f, 0.64f));
    glm::mat4 m = glm::translate(base, glm::vec3(0.0f, 0.35f, 0.0f));
    m = glm::scale(m, glm::vec3(12.4f, 0.70f, 8.4f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // 2. Main timber shop walls
    standardShader->setBool("useTexture", true);
    standardShader->setVec2("texScale", glm::vec2(1.5f, 1.0f));
    standardShader->setVec3("solidColor", glm::vec3(0.95f, 0.90f, 0.85f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texShop);

    m = glm::translate(base, glm::vec3(0.0f, 3.40f, 0.0f));
    m = glm::scale(m, glm::vec3(12.0f, 5.40f, 8.0f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    standardShader->setBool("useTexture", false);

    // 3. Service counter window with glowing warmth
    standardShader->setVec3("solidColor", glm::vec3(1.0f, 0.85f, 0.50f));
    standardShader->setFloat("emissive", 0.75f);
    m = glm::translate(base, glm::vec3(0.0f, 3.2f, 4.05f));
    m = glm::scale(m, glm::vec3(4.5f, 2.0f, 0.12f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
    standardShader->setFloat("emissive", 0.0f);

    // Counter wooden service shelf
    standardShader->setVec3("solidColor", glm::vec3(0.48f, 0.30f, 0.16f));
    m = glm::translate(base, glm::vec3(0.0f, 2.15f, 4.45f));
    m = glm::scale(m, glm::vec3(5.2f, 0.12f, 0.90f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // 4. Striped Canvas Awning over service window
    standardShader->setVec3("solidColor", glm::vec3(0.90f, 0.25f, 0.20f)); // Red & cream awning
    m = glm::translate(base, glm::vec3(0.0f, 4.35f, 4.8f));
    m = glm::rotate(m, glm::radians(22.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(5.8f, 0.08f, 1.8f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    // Awning support struts
    standardShader->setVec3("solidColor", glm::vec3(0.25f, 0.25f, 0.28f));
    m = glm::translate(base, glm::vec3(-2.6f, 3.4f, 4.5f));
    m = glm::scale(m, glm::vec3(0.08f, 1.8f, 0.08f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    m = glm::translate(base, glm::vec3(2.6f, 3.4f, 4.5f));
    m = glm::scale(m, glm::vec3(0.08f, 1.8f, 0.08f));
    standardShader->setMat4("model", m);
    columnMesh->draw();

    // 5. Pitched cedar shake roof
    standardShader->setVec3("solidColor", glm::vec3(0.25f, 0.28f, 0.32f)); // Dark slate
    m = glm::translate(base, glm::vec3(0.0f, 6.45f, 0.0f));
    m = glm::scale(m, glm::vec3(13.4f, 0.40f, 9.4f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(base, glm::vec3(0.0f, 7.10f, 0.0f));
    m = glm::scale(m, glm::vec3(12.8f, 0.90f, 6.5f));
    standardShader->setMat4("model", m);
    boxMesh->draw();

    m = glm::translate(base, glm::vec3(0.0f, 7.75f, 0.0f));
    m = glm::scale(m, glm::vec3(12.2f, 0.60f, 3.2f));
    standardShader->setMat4("model", m);
    boxMesh->draw();
}

void Park::drawHUD(const glm::vec3& viewPos, float yaw, bool isSitting, bool isRidingBike, float bikeSpeed) {
    if (!showHUD) return;
    (void)isSitting;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    uiShader->use();
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
    uiShader->setMat4("projection", projection);

    // 1. Mini-map background: dark translucent box
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(580.0f, 380.0f, 0.0f));
    model = glm::scale(model, glm::vec3(200.0f, 200.0f, 1.0f));
    uiShader->setMat4("model", model);
    uiShader->setVec4("color", glm::vec4(0.1f, 0.1f, 0.14f, 0.7f));
    uiMesh->draw();

    // Map border
    glm::mat4 borderM = glm::translate(glm::mat4(1.0f), glm::vec3(578.0f, 378.0f, 0.0f));
    borderM = glm::scale(borderM, glm::vec3(204.0f, 2.0f, 1.0f));
    uiShader->setMat4("model", borderM);
    uiShader->setVec4("color", glm::vec4(0.3f, 0.35f, 0.45f, 0.8f));
    uiMesh->draw();

    auto drawDot = [&](float worldX, float worldZ, float size, glm::vec4 color) {
        float mapX = 580.0f + (worldX + 400.0f) * (200.0f / 800.0f);
        float mapY = 380.0f + (worldZ + 400.0f) * (200.0f / 800.0f);
        
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3(mapX - size/2.0f, mapY - size/2.0f, 0.0f));
        m = glm::scale(m, glm::vec3(size, size, 1.0f));
        uiShader->setMat4("model", m);
        uiShader->setVec4("color", color);
        uiMesh->draw();
    };

    // Draw Entrance (Yellow)
    drawDot(0.0f, -380.0f, 8.0f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

    // Draw Grand Fountain (Cyan)
    drawDot(0.0f, 0.0f, 8.0f, glm::vec4(0.1f, 0.9f, 1.0f, 1.0f));

    // Draw Victorian Gazebo (Pastel Rose)
    drawDot(-40.0f, -40.0f, 7.0f, glm::vec4(1.0f, 0.45f, 0.65f, 1.0f));

    // Draw Picnic & BBQ Area (Warm Orange-Red)
    drawDot(40.0f, -40.0f, 7.0f, glm::vec4(1.0f, 0.45f, 0.2f, 1.0f));

    // Draw Sign (Red)
    drawDot(80.0f, -80.0f, 6.0f, glm::vec4(1.0f, 0.2f, 0.2f, 1.0f));

    // Draw Lake & Pier (Blue)
    drawDot(-100.0f, -100.0f, 9.0f, glm::vec4(0.2f, 0.6f, 1.0f, 1.0f));
    
    // Draw Restaurants (Orange)
    drawDot(250.0f, 150.0f, 7.0f, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
    drawDot(-100.0f, -30.0f, 7.0f, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));

    // Draw Shop (Purple)
    drawDot(50.0f, 50.0f, 6.0f, glm::vec4(0.8f, 0.2f, 0.8f, 1.0f));

    // Draw Kids Playground (Bright Teal)
    drawDot(-60.0f, 60.0f, 8.0f, glm::vec4(0.2f, 0.85f, 0.85f, 1.0f));

    // Draw Park Bicycle (Lime Green)
    if (!isRidingBike) {
        drawDot(bikePos.x, bikePos.z, 6.0f, glm::vec4(0.2f, 1.0f, 0.35f, 1.0f));
    }

    // Draw Player (White Arrow)
    float mapX = 580.0f + (viewPos.x + 400.0f) * (200.0f / 800.0f);
    float mapY = 380.0f + (viewPos.z + 400.0f) * (200.0f / 800.0f);
    
    glm::mat4 playerModel = glm::mat4(1.0f);
    playerModel = glm::translate(playerModel, glm::vec3(mapX, mapY, 0.0f));
    playerModel = glm::rotate(playerModel, glm::radians(yaw + 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    playerModel = glm::scale(playerModel, glm::vec3(10.0f, 12.0f, 1.0f));
    uiShader->setMat4("model", playerModel);
    uiShader->setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    arrowMesh->draw();

    // 2. Time-of-Day Pill Badge at Top-Right (with [N] Cycle Hint)
    glm::mat4 todBg = glm::translate(glm::mat4(1.0f), glm::vec3(580.0f, 20.0f, 0.0f));
    todBg = glm::scale(todBg, glm::vec3(200.0f, 32.0f, 1.0f));
    uiShader->setMat4("model", todBg);
    uiShader->setVec4("color", glm::vec4(0.08f, 0.08f, 0.12f, 0.75f));
    uiMesh->draw();

    // Time Indicator Dot
    glm::vec4 todColor = (currentTimeOfDay == TIME_NOON) ? glm::vec4(1.0f, 0.85f, 0.15f, 1.0f) :
                         (currentTimeOfDay == TIME_SUNSET) ? glm::vec4(1.0f, 0.45f, 0.15f, 1.0f) :
                                                             glm::vec4(0.45f, 0.65f, 1.0f, 1.0f);
    glm::mat4 todDot = glm::translate(glm::mat4(1.0f), glm::vec3(592.0f, 28.0f, 0.0f));
    todDot = glm::scale(todDot, glm::vec3(16.0f, 16.0f, 1.0f));
    uiShader->setMat4("model", todDot);
    uiShader->setVec4("color", todColor);
    uiMesh->draw();

    // Three cycle segments: Noon, Sunset, Night
    for (int t = 0; t < 3; ++t) {
        glm::vec4 segColor = (currentTimeOfDay == t) ? todColor : glm::vec4(0.25f, 0.28f, 0.35f, 0.7f);
        glm::mat4 segM = glm::translate(glm::mat4(1.0f), glm::vec3(625.0f + t * 45.0f, 32.0f, 0.0f));
        segM = glm::scale(segM, glm::vec3(38.0f, 8.0f, 1.0f));
        uiShader->setMat4("model", segM);
        uiShader->setVec4("color", segColor);
        uiMesh->draw();
    }

    // 3. Bicycle Speedometer HUD
    if (isRidingBike) {
        // Outer dark plate
        glm::mat4 speedBg = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 545.0f, 0.0f));
        speedBg = glm::scale(speedBg, glm::vec3(180.0f, 28.0f, 1.0f));
        uiShader->setMat4("model", speedBg);
        uiShader->setVec4("color", glm::vec4(0.08f, 0.08f, 0.12f, 0.85f));
        uiMesh->draw();

        // Speed fill bar
        float speedFrac = std::max(0.0f, std::min(bikeSpeed / 22.0f, 1.0f));
        glm::vec4 speedBarColor = (speedFrac < 0.6f) ? glm::vec4(0.2f, 0.85f, 0.35f, 1.0f) :
                                  (speedFrac < 0.85f) ? glm::vec4(1.0f, 0.75f, 0.15f, 1.0f) :
                                                        glm::vec4(1.0f, 0.25f, 0.2f, 1.0f);
        glm::mat4 speedBar = glm::translate(glm::mat4(1.0f), glm::vec3(34.0f, 549.0f, 0.0f));
        speedBar = glm::scale(speedBar, glm::vec3(std::max(4.0f, speedFrac * 172.0f), 20.0f, 1.0f));
        uiShader->setMat4("model", speedBar);
        uiShader->setVec4("color", speedBarColor);
        uiMesh->draw();
    }

    // 4. Interactive Action Prompt Bar
    if (!currentPrompt.empty()) {
        glm::mat4 promptBg = glm::translate(glm::mat4(1.0f), glm::vec3(230.0f, 545.0f, 0.0f));
        promptBg = glm::scale(promptBg, glm::vec3(340.0f, 32.0f, 1.0f));
        uiShader->setMat4("model", promptBg);
        uiShader->setVec4("color", glm::vec4(0.06f, 0.07f, 0.1f, 0.88f));
        uiMesh->draw();

        // Yellow / cyan action badge pill
        glm::vec4 badgeColor = isRidingBike ? glm::vec4(0.15f, 0.85f, 0.95f, 1.0f) :
                                              glm::vec4(1.0f, 0.82f, 0.12f, 1.0f);
        glm::mat4 badgeM = glm::translate(glm::mat4(1.0f), glm::vec3(236.0f, 550.0f, 0.0f));
        badgeM = glm::scale(badgeM, glm::vec3(22.0f, 22.0f, 1.0f));
        uiShader->setMat4("model", badgeM);
        uiShader->setVec4("color", badgeColor);
        uiMesh->draw();
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
