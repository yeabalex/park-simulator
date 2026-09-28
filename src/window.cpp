#include "window.h"
#include "camera.h"
#include "park.h"

int windowWidth = 800;
int windowHeight = 600;

static Camera camera;
static Park park;

static bool cursorCaptured = true;
static bool firstMouse = true;
static double lastX = 400.0;
static double lastY = 300.0;

void initOpenGL() {
    // Fog color matching sky
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    // Core profile no longer has GL_LIGHTING or GL_FOG built-in!
    // We implement these entirely in our custom shaders now.
    park.init();
}

void renderFrame() {
    // Dynamic sky clear color matching time of day fog
    if (park.currentTimeOfDay == TIME_NOON) {
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    } else if (park.currentTimeOfDay == TIME_SUNSET) {
        glClearColor(0.85f, 0.48f, 0.32f, 1.0f);
    } else {
        glClearColor(0.04f, 0.06f, 0.12f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute matrices (Increased far plane to 300.0f for deeper LOS)
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 300.0f);
    glm::mat4 view = camera.GetViewMatrix();

    float time = static_cast<float>(glfwGetTime());

    // Draw the modern park with full interactive state & lighting cycle
    park.draw(view, projection, camera.Position, camera.Yaw, time, camera.isSitting, camera.isRidingBike, camera.bikeSpeed);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;
    if (height == 0) height = 1;
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (!cursorCaptured) {
            cursorCaptured = true;
            firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (cursorCaptured) {
            cursorCaptured = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    if (cursorCaptured) {
        float xoffset = static_cast<float>(xpos - lastX);
        float yoffset = static_cast<float>(ypos - lastY);
        lastX = xpos;
        lastY = ypos;
        camera.processMouseMovement(xoffset, yoffset);
    } else {
        lastX = xpos;
        lastY = ypos;
    }
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Toggle cursor capture with TAB key
    static bool tabPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (!tabPressed) {
            cursorCaptured = !cursorCaptured;
            firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, cursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            tabPressed = true;
        }
    } else {
        tabPressed = false;
    }

    // Teleport to North Gate
    static bool rKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!rKeyPressed) {
            camera.teleportToGate();
            rKeyPressed = true;
        }
    } else {
        rKeyPressed = false;
    }

    // Cycle Time of Day [N]
    static bool nKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        if (!nKeyPressed) {
            park.nextTimeOfDay();
            nKeyPressed = true;
        }
    } else {
        nKeyPressed = false;
    }

    // Toggle Clean Photo Mode HUD [H]
    static bool hKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        if (!hKeyPressed) {
            park.toggleHUD();
            hKeyPressed = true;
        }
    } else {
        hKeyPressed = false;
    }

    // Sit on Bench / Stand Up [E]
    static bool eKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (!eKeyPressed) {
            if (camera.isSitting) {
                camera.standUp();
            } else {
                glm::vec3 sitPos;
                float sitYaw;
                if (park.checkNearBench(camera.Position, sitPos, sitYaw)) {
                    camera.sit(sitPos, sitYaw);
                }
            }
            eKeyPressed = true;
        }
    } else {
        eKeyPressed = false;
    }

    // Mount / Dismount Bicycle [F]
    static bool fKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fKeyPressed) {
            if (camera.isRidingBike) {
                // Park bicycle at current position
                park.setBicyclePosition(camera.Position + glm::vec3(0.0f, -1.9f, 0.0f), camera.Yaw);
                camera.dismountBike();
            } else {
                if (park.checkNearBicycle(camera.Position)) {
                    camera.mountBike(park.getBicyclePosition(), camera.Yaw);
                }
            }
            fKeyPressed = true;
        }
    } else {
        fKeyPressed = false;
    }

    // Sprinting
    bool isSprinting = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ||
                        glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

    if (isSprinting) {
        camera.MovementSpeed = 20.0f;
    } else {
        camera.MovementSpeed = 5.0f;
    }

    // Movement (WASD)
    bool w = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    bool s = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    bool a = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    bool d = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
    camera.processKeyboard(w, s, a, d, deltaTime);

    // 360 Rotation via Keyboard (Arrow keys & Q)
    float keyTurnSpeed = 90.0f * deltaTime; // 90 degrees per second
    float yawOffset = 0.0f;
    float pitchOffset = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        yawOffset -= keyTurnSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        yawOffset += keyTurnSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        pitchOffset -= keyTurnSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        pitchOffset += keyTurnSpeed;
    }

    if (yawOffset != 0.0f || pitchOffset != 0.0f) {
        camera.processMouseMovement(yawOffset / camera.MouseSensitivity, pitchOffset / camera.MouseSensitivity);
    }
}
