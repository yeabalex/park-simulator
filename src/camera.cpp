#include "camera.h"
#include "mesh.h"
#include <algorithm>

Camera::Camera() {
    Position = glm::vec3(0.0f, 1.8f, 15.0f);
    WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    Yaw = -90.0f;
    Pitch = 0.0f;
    MovementSpeed = 5.0f;
    MouseSensitivity = 0.1f;

    isSitting = false;
    sitPosition = glm::vec3(0.0f);
    isRidingBike = false;
    bikeSpeed = 0.0f;
    bikeYaw = -90.0f;
    bikeWheelRotation = 0.0f;

    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::sit(const glm::vec3& seatPos, float facingYaw) {
    if (isRidingBike) dismountBike();
    isSitting = true;
    sitPosition = seatPos;
    Position = seatPos + glm::vec3(0.0f, 0.65f, 0.0f); // Seated eye height
    Yaw = facingYaw;
    Pitch = 0.0f;
    updateCameraVectors();
}

void Camera::standUp() {
    if (isSitting) {
        isSitting = false;
        // Step slightly forward from the bench
        glm::vec3 flatFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        Position += flatFront * 0.8f;
        Position.y = Mesh::getTerrainHeight(Position.x, Position.z) + 1.8f;
    }
}

void Camera::mountBike(const glm::vec3& startPos, float facingYaw) {
    if (isSitting) standUp();
    isRidingBike = true;
    bikeSpeed = 0.0f;
    bikeYaw = facingYaw;
    Position = startPos;
    Position.y = Mesh::getTerrainHeight(Position.x, Position.z) + 1.9f;
    Yaw = facingYaw;
    Pitch = 0.0f;
    updateCameraVectors();
}

void Camera::dismountBike() {
    if (isRidingBike) {
        isRidingBike = false;
        bikeSpeed = 0.0f;
        Position.y = Mesh::getTerrainHeight(Position.x, Position.z) + 1.8f;
    }
}

void Camera::processKeyboard(bool w, bool s, bool a, bool d, float deltaTime) {
    if (isSitting) {
        // Any movement key stands the player up
        if (w || s || a || d) {
            standUp();
        }
        return;
    }

    if (isRidingBike) {
        // Bicycle physics
        if (w) bikeSpeed += 14.0f * deltaTime; // Accelerate
        if (s) bikeSpeed -= 16.0f * deltaTime; // Brake / reverse
        
        // Rolling resistance / drag
        bikeSpeed *= (1.0f - 1.0f * deltaTime);
        bikeSpeed = std::max(-4.0f, std::min(bikeSpeed, 22.0f));

        // Steering with A/D
        float turnSpeed = 65.0f * deltaTime;
        if (a) Yaw -= turnSpeed;
        if (d) Yaw += turnSpeed;
        bikeYaw = Yaw;
        updateCameraVectors();

        // Move along front direction
        glm::vec3 flatFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        Position += flatFront * (bikeSpeed * deltaTime);
        bikeWheelRotation += bikeSpeed * deltaTime * 5.0f;

        // Keep inside fence
        float bound = 378.0f;
        Position.x = std::max(-bound, std::min(bound, Position.x));
        Position.z = std::max(-bound, std::min(bound, Position.z));

        Position.y = Mesh::getTerrainHeight(Position.x, Position.z) + 1.9f;
        return;
    }

    // Walking / Running
    float velocity = MovementSpeed * deltaTime;
    glm::vec3 flatFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));

    if (w) Position += flatFront * velocity;
    if (s) Position -= flatFront * velocity;
    if (a) Position -= Right * velocity;
    if (d) Position += Right * velocity;

    // Bound camera within the 380x380 fence
    float bound = 379.0f;
    Position.x = std::max(-bound, std::min(bound, Position.x));
    Position.z = std::max(-bound, std::min(bound, Position.z));

    // Keep the camera at eye level on the terrain
    Position.y = Mesh::getTerrainHeight(Position.x, Position.z) + 1.8f;
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch -= yoffset;

    // Constrain pitch
    if (Pitch > 89.0f) Pitch = 89.0f;
    if (Pitch < -89.0f) Pitch = -89.0f;

    updateCameraVectors();
}

void Camera::teleportToGate() {
    if (isSitting) standUp();
    if (isRidingBike) dismountBike();
    Position = glm::vec3(0.0f, 1.8f, -370.0f);
    Yaw = 90.0f; 
    Pitch = 0.0f;
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}
