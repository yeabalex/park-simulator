#ifndef PARK_H
#define PARK_H

#include <vector>
#include <string>
#include <utility>
#include <glm/glm.hpp>
#include "shader.h"
#include "mesh.h"

enum TreeType {
    TREE_OAK,      // Classic lush green multi-tiered canopy
    TREE_PINE,     // Conical 3-tier evergreen conifer
    TREE_AUTUMN,   // Glowing autumn maple with rich amber/orange foliage
    TREE_CHERRY,   // Cherry blossom with delicate pink petals
    TREE_BIRCH     // Birch tree with pale white trunk and spring-green leaves
};

struct TreeInstance {
    glm::vec3 pos;
    TreeType type;
    float scale;
    float rotationY;
    glm::vec3 foliageColor;
};

enum TimeOfDay {
    TIME_NOON = 0,
    TIME_SUNSET = 1,
    TIME_NIGHT = 2
};

class Park {
public:
    Park();
    ~Park();
    
    // Initialize OpenGL assets (textures, shaders, meshes)
    void init();

    // Main render function to draw the entire park
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos, float yaw, float time, bool isSitting, bool isRidingBike, float bikeSpeed);

    // Time of day cycle
    TimeOfDay currentTimeOfDay;
    void nextTimeOfDay();

    // Interaction queries
    bool checkNearBench(const glm::vec3& playerPos, glm::vec3& outSitPos, float& outSitYaw);
    bool checkNearBicycle(const glm::vec3& playerPos);
    std::string getPrompt() const { return currentPrompt; }

    glm::vec3 getBicyclePosition() const { return bikePos; }
    void setBicyclePosition(const glm::vec3& pos, float yaw) { bikePos = pos; bikeYaw = yaw; }

    bool showHUD = true;
    void toggleHUD() { showHUD = !showHUD; }

private:
    std::vector<TreeInstance> trees;
    std::vector<std::pair<glm::vec3, float>> benchList;
    std::string currentPrompt;

    glm::vec3 bikePos;
    float bikeYaw;

    GLuint texGrass;
    GLuint texBark;
    GLuint texLeaves;
    GLuint texAsphalt;
    GLuint texSign;
    GLuint texWater;
    GLuint texRestaurant;
    GLuint texShop;
    
    Shader* standardShader;
    Shader* uiShader;
    Shader* skyShader;
    
    Mesh* groundMesh;
    Mesh* trunkMesh;
    Mesh* leafMesh;
    Mesh* boxMesh;
    Mesh* waterMesh;
    Mesh* uiMesh;
    Mesh* arrowMesh;
    Mesh* cylinderMesh;
    Mesh* columnMesh;
    Mesh* sphereMesh;
    Mesh* coneMesh;
    Mesh* diskMesh;
    Mesh* lakeMesh;
    Mesh* skyDomeMesh;

    void generateTrees(int count, float areaSize);
    
    // Sky rendering
    void drawSky(const glm::mat4& view, const glm::mat4& projection, float time);

    // Path rendering helpers
    void drawPathStraight(glm::vec3 start, glm::vec3 end, float width);
    void drawPathCurve(glm::vec3 center, float radius, float startAngle, float endAngle, float width);
    void drawCurbStraight(glm::vec3 start, glm::vec3 end, float width);
    void drawCurbCurve(glm::vec3 center, float radius, float startAngle, float endAngle, float width);
    void drawPlazaTerrace();
    
    // Props & Amenities
    void drawFence(float size);
    void drawBuilding(glm::vec3 position);
    void drawSign(glm::vec3 position, float rotationY);
    void drawRestaurant(glm::vec3 position, float rotationY);
    void drawShop(glm::vec3 position, float rotationY);
    void drawHUD(const glm::vec3& viewPos, float yaw, bool isSitting, bool isRidingBike, float bikeSpeed);

    // Park life & playground additions
    void drawTree(const TreeInstance& tree);
    void drawBench(glm::vec3 position, float rotationY, bool registerBench = true);
    void drawLampPost(glm::vec3 position, bool lightsOn);
    void drawFlowerBed(glm::vec3 position, float width, float length);
    void drawTrashBin(glm::vec3 position);
    void drawPlayground(glm::vec3 position);

    // New Landmarks & Attractions
    void drawFountain(glm::vec3 position, float time);
    void drawLakePier(glm::vec3 startPos, float time);
    void drawShoreline(glm::vec3 center, float radius);
    void drawGazebo(glm::vec3 position);
    void drawPicnicArea(glm::vec3 position);
    void drawBicycle(glm::vec3 position, float yaw, float steerAngle, float wheelSpin);
};

#endif // PARK_H
