#define GLM_ENABLE_EXPERIMENTAL
#if defined (_APPLE_)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION

#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include <iostream>
#include <vector>
#include <random>
// window

float randomFloat(float min, float max) {
    static std::random_device rd;  // Seed for random number generation
    static std::mt19937 generator(rd()); // Mersenne Twister RNG
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
}


gps::Window myWindow;

//Shader uniforms
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;


//SHadow mapping uniforms
const unsigned int SHADOW_WIDTH = 8192;
const unsigned int SHADOW_HEIGHT = 8192;
GLuint shadowMapFBO;
GLuint depthMapTexture;

// Fireworks
bool isFireworkActive = false;
struct FireworkParticle {
    glm::vec3 position;    
    glm::vec3 velocity;   
    glm::vec3 color;	   
    float lifetime;        
    bool hasExploded;      
};
std::vector<FireworkParticle> fireworks;
gps::Model3D fireworkParticle;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;


//Music notes animation:
gps::Model3D musicNote;
struct Note {
    glm::vec3 position;
    glm::vec3 velocity;
    float rotation;
    float scale;
    float lifetime;
    glm::vec3 color;
};
std::vector<Note> jukebox1Notes;
std::vector<Note> jukebox2Notes;
std::vector<Note> jukebox3Notes;
std::vector<Note> extraLocationsNotes;

// gold rain
gps::Model3D token;
struct Coin {
    glm::vec3 position;
    glm::vec3 velocity;
    float size;
    float rotationAngle;
    float rotationSpeed;
};
std::vector<Coin> coins;

// camera
gps::Camera myCamera(
    glm::vec3(30.0f, 3.0f, 0.0f),
    glm::vec3(29.0f, 3.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D house;
gps::Model3D lightCube;
GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::Shader depthShader;
//
GLfloat lightAngle;

//global vars for the  image filters
bool isInverted = false;
bool isBlackAndWhite = false;
bool isSepia = false;
bool isCartoon = false;
bool isHueShift = false;
float hueShift = 0.0f; 
bool isNightVision = false;
bool isOldTV = false;
bool isPixelated = false;
glm::vec2 pixelSize = glm::vec2(0.01f, 0.01f);


//global vars
bool toggleRain = false;
bool fogToggle = false;
bool isWireframe = false;
bool isPointView = false;
bool isTourActive = false;
bool isDirectionalLightEnabled = true; // Tracks the state of the directional light
bool isPointLightEnabled = true;      // Tracks the state of the point light

int currentTourIndex = 0;
float tourSpeed = 0.1f; 
float tourProgress = 0.0f;

//skybox vars
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

// Tour camera positions and targets.
std::vector<glm::vec3> cameraPositions = {
	glm::vec3(-1.0f, 1.8f, 0.18f),
	glm::vec3(-0.5f, 1.5f, -1.3f),
	glm::vec3(-0.4f, 1.2f, 3.0f),
	glm::vec3(24.5f, 2.7f, -0.3f),
	glm::vec3(12.5f, 2.2f, -18.6f),
	glm::vec3(-4.5f, 2.0f, -25.6f),
	glm::vec3(-25.27f, 2.4f, -3.0f),
	glm::vec3(-17.0f, 2.0f, 13.0f),
	glm::vec3(4.0f, 2.0f, 22.0f),
	glm::vec3(30.0f, 2.8f, -0.01f)
};
std::vector<glm::vec3> cameraTargets = {
    glm::vec3(0.0f, 1.8f, 0.2f),
    glm::vec3(-0.5f, 1.5f, -2.3f),
    glm::vec3(-0.3f, 1.2f, 4.0f),
    glm::vec3(25.5f, 2.7f, -0.3f),
    glm::vec3(11.8f, 2.3f, -19.5f),
    glm::vec3(-4.1f, 2.0f, -26.5f),
    glm::vec3(-25.9f, 2.3f, -2.2f),
    glm::vec3(-16.4f, 2.0f, 13.9f),
    glm::vec3(4.0f, 2.2f, 23.4f),
    glm::vec3(29.5f, 2.7f, -0.0f)
};

glm::vec3 static bezierCurve(const std::vector<glm::vec3>& controlPoints, float t) {
    size_t n = controlPoints.size() - 1;
    glm::vec3 point(0.0f);

    auto binomialCoeff = [](size_t n, size_t k) {
        if (k > n) return 0.0f;
        float res = 1.0f;
        for (size_t i = 1; i <= k; ++i) {
            res *= (n - (k - i));
            res /= i;
        }
        return res;
        };

    for (size_t i = 0; i <= n; ++i) {
        float coeff = binomialCoeff(n, i) * glm::pow(1 - t, n - i) * glm::pow(t, i);
        point += coeff * controlPoints[i];
    }

    return point;
}

// Directional Light
struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 color;
    bool enabled;
};
// Point Light
struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float constant;
    float linear;
    float quadratic;
    bool enabled;
};

DirectionalLight dirLight = {
    glm::vec3(-0.2f, -1.0f, -0.3f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    true
};
PointLight pointLight = {
    glm::vec3(2.0f, 3.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 0.8f),
    1.0f, 
    0.01f, 
    0.002f, 
    true
};
PointLight lamp1 = {
    glm::vec3(1.84588f, 1.98045f, -29.1487f),
    glm::vec3(1.0f, 1.0f, 0.8f),
    1.0f, 
    0.22f, 
    0.20f, 
    false
};
PointLight lamp2 = {
    glm::vec3(-2.57556f, 1.98314f, 28.5561f),
    glm::vec3(1.0f, 1.0f, 0.8f),
    1.0f, 
    0.22f, 
    0.20f, 
    false
};
PointLight dogLamp = {
    glm::vec3(-0.580192f, 2.48697f, -3.17624f),
    glm::vec3(0.545, 0.0, 0.545),
    1.0f,
    0.35f, 
    0.44f, 
    false
};

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void updateTour() {
    if (!isTourActive) return;

    static double lastFrameTime = 0.0;
    double currentFrameTime = glfwGetTime();
    double deltaTime = currentFrameTime - lastFrameTime;
    lastFrameTime = currentFrameTime;

    tourProgress += tourSpeed * deltaTime;
    if (tourProgress >= 1.0f) {
        tourProgress = 0.0f;
        currentTourIndex = (currentTourIndex + 1) % cameraPositions.size();
    }
    // Define control points for position and target
    std::vector<glm::vec3> positionControlPoints = {
        cameraPositions[currentTourIndex],
        cameraPositions[(currentTourIndex + 1) % cameraPositions.size()],
        cameraPositions[(currentTourIndex + 2) % cameraPositions.size()]
    };
    std::vector<glm::vec3> targetControlPoints = {
        cameraTargets[currentTourIndex],
        cameraTargets[(currentTourIndex + 1) % cameraTargets.size()],
        cameraTargets[(currentTourIndex + 2) % cameraTargets.size()]
    };
    glm::vec3 newPosition = bezierCurve(positionControlPoints, tourProgress);
    glm::vec3 newTarget = bezierCurve(targetControlPoints, tourProgress);
    myCamera.setCameraPosition(newPosition);
    myCamera.setCameraTarget(newTarget);
    view = glm::lookAt(newPosition, newTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}
void togglePointLights() {
    bool newState = !lamp1.enabled;
    lamp1.enabled = newState;
    lamp2.enabled = newState;
    dogLamp.enabled = newState;

    GLint lamp1EnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp1.enabled");
    GLint lamp2EnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp2.enabled");
    GLint dogLampEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dogLamp.enabled");

    myBasicShader.useShaderProgram();
    glUniform1i(lamp1EnabledLoc, lamp1.enabled);
    glUniform1i(lamp2EnabledLoc, lamp2.enabled);
    glUniform1i(dogLampEnabledLoc, dogLamp.enabled);
}
void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    if (width == 0 || height == 0)
        return;
    glViewport(0, 0, width, height);

    myWindow.setWindowDimensions({ width, height });
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    std::cout << "Window resized! New width: " << width << " and height: " << height << std::endl;
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;
    static double lastX = 1024.0 / 2.0;
    static double lastY = 768.0 / 2.0;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        return;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    myCamera.rotate(yoffset, xoffset);

    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}
void spawnFirework(glm::vec3 origin) {
    FireworkParticle particle;
    particle.position = origin;  
    particle.velocity = glm::vec3(0.0f, 5.0f, 0.0f);  
    particle.color = glm::vec3(randomFloat(0.0f, 1.0f), randomFloat(0.0f, 1.0f), randomFloat(0.0f, 1.0f));  
    particle.lifetime = 1.1f; 
    particle.hasExploded = false;
    fireworks.push_back(particle);
}
void rotateScene(float angle, glm::vec3 axis)
{
    glm::mat4 translationToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    glm::mat4 translationBack = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    model = translationBack * rotationMatrix * translationToOrigin * model;

    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}

void processMovement() {
    // Camera controls for movement Forward
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    // Camera controls for movement Backward
    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    // Camera controls for movement Left
    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    // Camera controls for movement Right
    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    
    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    // Toggle Wireframe view mode
    if (pressedKeys[GLFW_KEY_R]){
        isWireframe = !isWireframe;
        if (isWireframe){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }else{
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        pressedKeys[GLFW_KEY_R] = false;
    }
    // Toggle point view mode
    if (pressedKeys[GLFW_KEY_T]){
        isPointView = !isPointView;
        if (isPointView){
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            std::cout << "Point view enabled" << std::endl;
        }else{
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            std::cout << "Point view disabled" << std::endl;
        }
        pressedKeys[GLFW_KEY_T] = false;
    }
    // Toggle directional light
    if (pressedKeys[GLFW_KEY_1]) {
        isDirectionalLightEnabled = !isDirectionalLightEnabled; // Toggle the state
        GLint dirLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dirLight.enabled");
        myBasicShader.useShaderProgram();
        glUniform1i(dirLightEnabledLoc, isDirectionalLightEnabled);
        std::cout << "Directional Light: " << (isDirectionalLightEnabled ? "Enabled" : "Disabled") << std::endl;
        pressedKeys[GLFW_KEY_1] = false; // Prevent repeated toggling
    }
    // Toggle point light
    if (pressedKeys[GLFW_KEY_2]) {
        isPointLightEnabled = !isPointLightEnabled; // Toggle the state
        GLint pointLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.enabled");
        myBasicShader.useShaderProgram();
        glUniform1i(pointLightEnabledLoc, isPointLightEnabled);
        std::cout << "Point Light: " << (isPointLightEnabled ? "Enabled" : "Disabled") << std::endl;
        pressedKeys[GLFW_KEY_2] = false; // Prevent repeated toggling
    }
    // Toggle the extra 3 point lights
    if (pressedKeys[GLFW_KEY_3]) {
        togglePointLights();
        GLint lamp1EnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp1.enabled");
        GLint lamp2EnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp2.enabled");
        GLint dogLampEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dogLamp.enabled");
        myBasicShader.useShaderProgram();
        glUniform1i(lamp1EnabledLoc, lamp1.enabled);
        glUniform1i(lamp2EnabledLoc, lamp2.enabled);
        glUniform1i(dogLampEnabledLoc, dogLamp.enabled); // Corrected from glUniform1d to glUniform1i
        std::cout << "Lamps: " << (lamp1.enabled ? "Enabled" : "Disabled") << std::endl;
        pressedKeys[GLFW_KEY_3] = false;
    }
    //ON K press to see the current coordinates of the camera on the console and the viewing direction
    if (pressedKeys[GLFW_KEY_K]) {
		std::cout << "Camera Position: " << myCamera.getCameraPosition().x << " " << myCamera.getCameraPosition().y << " " << myCamera.getCameraPosition().z << std::endl;
		std::cout << "Camera Target: " << myCamera.getCameraTarget().x << " " << myCamera.getCameraTarget().y << " " << myCamera.getCameraTarget().z << std::endl;
		pressedKeys[GLFW_KEY_K] = false;
    }
    //ON L press to enable/disable the tour
    if (pressedKeys[GLFW_KEY_L]) {
        isTourActive = !isTourActive;
        std::cout << "Tour: " << (isTourActive ? "Enabled" : "Disabled") << std::endl;
        pressedKeys[GLFW_KEY_L] = false;
    }
    //On 0 press to enable/disable fog
    if (pressedKeys[GLFW_KEY_0])
    {
        fogToggle = !fogToggle;
        myBasicShader.useShaderProgram();
        GLint fogEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogToggle");
        glUniform1i(fogEnabledLoc, fogToggle);
        std::cout << "Fog: " << (fogToggle ? "Enabled" : "Disabled") << std::endl;
        pressedKeys[GLFW_KEY_0] = false;
    }
    //On C press to enable/disable THE GOLDEN rain
    if (pressedKeys[GLFW_KEY_C])
    {
		toggleRain = !toggleRain;
		std::cout << "Gold Rain: " << (toggleRain ? "Enabled" : "Disabled") << std::endl;
		pressedKeys[GLFW_KEY_C] = false;
	}
    //On X press to increase the light angle
    if (pressedKeys[GLFW_KEY_X]) {
        lightAngle += 0.4f; // Adjust the light's angle
    }
    if (pressedKeys[GLFW_KEY_Z]) {
        lightAngle -= 0.4f; // Adjust the light's angle
    }

    // On B press to enable/disable black and white filter
    if (pressedKeys[GLFW_KEY_B]) {
        isBlackAndWhite = !isBlackAndWhite;
        GLint bwFilterLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isBlackAndWhite");
        myBasicShader.useShaderProgram();
        glUniform1i(bwFilterLoc, isBlackAndWhite);
        pressedKeys[GLFW_KEY_B] = false; // Prevent repeated toggles
    }
    // On N press to enable/disable inverted colors filter
    if (pressedKeys[GLFW_KEY_N]) {
        isInverted = !isInverted;
        GLint invertedFilterLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isInverted");
        myBasicShader.useShaderProgram();
        glUniform1i(invertedFilterLoc, isInverted);
        pressedKeys[GLFW_KEY_N] = false; // Prevent repeated toggles
    }
    // On M press to enable/disable sepia filter
    if (pressedKeys[GLFW_KEY_M]) {
        isSepia = !isSepia;
        GLint sepiaFilterLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isSepia");
        myBasicShader.useShaderProgram();
        glUniform1i(sepiaFilterLoc, isSepia);
        pressedKeys[GLFW_KEY_M] = false; // Prevent repeated toggles
    }
    // On V press to enable/disable cartoon filter
    if (pressedKeys[GLFW_KEY_V]) {
        isCartoon = !isCartoon;
        GLint cartoonFilterLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isCartoon");
        myBasicShader.useShaderProgram();
        glUniform1i(cartoonFilterLoc, isCartoon);
        pressedKeys[GLFW_KEY_V] = false; // Prevent repeated toggles
    }
    // On H increase hue shift
    if (pressedKeys[GLFW_KEY_H]) {
        hueShift += 1.0f; // Adjust increment speed if needed
        if (hueShift > 360.0f) hueShift -= 360.0f; // Wrap around
        GLint hueShiftLoc = glGetUniformLocation(myBasicShader.shaderProgram, "hueShift");
        myBasicShader.useShaderProgram();
        glUniform1f(hueShiftLoc, hueShift);
    }
    // On G decrease hue shift
    if (pressedKeys[GLFW_KEY_G]) {
        hueShift -= 1.0f; // Adjust decrement speed if needed
        if (hueShift < 0.0f) hueShift += 360.0f; // Wrap around
        GLint hueShiftLoc = glGetUniformLocation(myBasicShader.shaderProgram, "hueShift");
        myBasicShader.useShaderProgram();
        glUniform1f(hueShiftLoc, hueShift);
    }
    // On J press to enable/disable hue shift
    if (pressedKeys[GLFW_KEY_J])
    {
        isHueShift = !isHueShift;
        GLint hueShiftEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isHueShift");
        myBasicShader.useShaderProgram();
        glUniform1i(hueShiftEnabledLoc, isHueShift);
        pressedKeys[GLFW_KEY_J] = false;
    }
    // On 4 press to enable/disable night vision
    if (pressedKeys[GLFW_KEY_4]) {
        isNightVision = !isNightVision;
        GLint nightVisionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isNightVision");
        myBasicShader.useShaderProgram();
        glUniform1i(nightVisionLoc, isNightVision);
        pressedKeys[GLFW_KEY_4] = false; // Prevent repeated toggles
    }
    // On 5 press to enable/disable old TV effect
    if (pressedKeys[GLFW_KEY_5]) {
        isOldTV = !isOldTV;
        GLint oldTVLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isOldTV");
        myBasicShader.useShaderProgram();
        glUniform1i(oldTVLoc, isOldTV);
        pressedKeys[GLFW_KEY_5] = false; // Prevent repeated toggles
    }
    // On 6 press to enable/disable pixelated effect
    if (pressedKeys[GLFW_KEY_6]) {
        isPixelated = !isPixelated;
        GLint pixelatedLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isPixelated");
        GLint pixelSizeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pixelSize");
        myBasicShader.useShaderProgram();
        glUniform1i(pixelatedLoc, isPixelated);
        glUniform2f(pixelSizeLoc, pixelSize.x, pixelSize.y); // Pass pixel size
        pressedKeys[GLFW_KEY_6] = false; // Prevent repeated toggles
    }
    // On = press to increase pixel size
    if (pressedKeys[GLFW_KEY_EQUAL]) { // Increase pixel size with '=' key
        pixelSize += glm::vec2(0.0005f, 0.0005f);
        GLint pixelSizeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pixelSize");
        myBasicShader.useShaderProgram();
        glUniform2f(pixelSizeLoc, pixelSize.x, pixelSize.y);
    }
    // On - press to decrease pixel size
    if (pressedKeys[GLFW_KEY_MINUS]) { 
        pixelSize -= glm::vec2(0.0005f, 0.0005f);
        pixelSize = glm::max(pixelSize, glm::vec2(0.0005f));
        GLint pixelSizeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pixelSize");
        myBasicShader.useShaderProgram();
        glUniform2f(pixelSizeLoc, pixelSize.x, pixelSize.y);
    }
    //On F press start a firework
    if (pressedKeys[GLFW_KEY_F]) {
        isFireworkActive=!isFireworkActive;
        std::cout<<"Firework: "<<(isFireworkActive ? "Enabled" : "Disabled")<<std::endl;
		pressedKeys[GLFW_KEY_F] = false;
	}
}

void updateFireworks(float deltaTime) {
    for (auto& particle : fireworks) {
        if (!particle.hasExploded && particle.position.y >= 5.2f) {
            // Explosion phase
            particle.hasExploded = true;

            // Spawn fragments
            for (int i = 0; i < 75; ++i) {
                FireworkParticle fragment;
                fragment.position = particle.position;
                fragment.velocity = glm::vec3(
                    randomFloat(-2.0f, 2.0f),
                    randomFloat(-2.0f, 2.0f),
                    randomFloat(-2.0f, 2.0f)
                );
                fragment.color = particle.color;
                fragment.lifetime = 2.0f; // Shorter lifetime for fragments
                fragment.hasExploded = true;
                fireworks.push_back(fragment);
            }
        }
        else {
            // Regular motion (launch or explosion fragments)
            particle.position += particle.velocity * deltaTime;
            particle.lifetime -= deltaTime;
        }
    }

    // Remove expired particles
    fireworks.erase(
        std::remove_if(fireworks.begin(), fireworks.end(),
            [](const FireworkParticle& p) { return p.lifetime <= 0.0f; }),
        fireworks.end()
    );
}
void renderFireworks(gps::Shader& shader) {
    shader.useShaderProgram();

    // Enable firework mode in the shader
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "isFirework"), 1);

    for (const auto& particle : fireworks) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), particle.position);
        model = glm::scale(model, glm::vec3(0.1f)); // Scale down the particles
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Pass the color of the particle to the shader
        GLint colorLoc = glGetUniformLocation(shader.shaderProgram, "particleColor");
        glUniform3f(colorLoc, particle.color.r, particle.color.g, particle.color.b);

        fireworkParticle.Draw(shader);
    }

    // Disable firework mode in the shader for subsequent rendering
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "isFirework"), 0);
}


//Gold rain functions:
void initGold(int no_coins, float ceiling) {
    for (int i = 0; i < no_coins; i++) {
		Coin coin;
        coin.position = glm::vec3(
			(rand() % 62) - 31,
			(rand() % 18),
			(rand() % 62) - 31
		);
		coin.velocity = glm::vec3(0.0f, -1.4f, 0.0f);
		coin.size = 2.5f;
        coin.rotationAngle = 0.0f;
        coin.rotationSpeed = ((rand() % 180) - 90);
		coins.push_back(coin);
	}
}
void updateGold(float timeG, float ceiling) {
    for (auto& coin : coins) {
        coin.velocity.y -= 1.4f * timeG;
        coin.position += coin.velocity * timeG;
        
        coin.rotationAngle +=coin.rotationSpeed*timeG;

        if(coin.rotationAngle > 360.0f)
			coin.rotationAngle -= 360.0f;
        else if (coin.rotationAngle < -360.0f) {
            coin.rotationAngle += 360.0f;
        }

        if (coin.position.y < 0.0f) {
			coin.position.x = (rand() % 62) - 31;
            coin.position.y = (rand() % 20);
            coin.position.z = (rand() % 62) - 31;

            coin.velocity = glm::vec3(0.0f, -1.4f, 0.0f);
            coin.rotationAngle = 0.0f;
            coin.rotationSpeed = ((rand() % 180) - 90);
		}
        //std::cout<<coin.position.x<<" "<<coin.position.y<<" "<<coin.position.z<<std::endl;
    }
}
void spawnFireworkInCircle(float radius) {
    // Generate random position within a circle on the XZ plane
    float angle = randomFloat(0.0f, 2.0f * 3.1456); // Random angle in radians
    float r = randomFloat(0.0f, radius);          // Random distance from the center
    glm::vec3 origin = glm::vec3(r * cos(angle), 0.0f, r * sin(angle)); // XZ position
    spawnFirework(origin);
}

//Music notes functions:
void spawnMusicNote(std::vector<Note>& jukeboxNotes, glm::vec3 origin) {
    Note note;
    note.position = origin;
    note.velocity = glm::vec3(
        randomFloat(-0.5f, 0.5f),  
        randomFloat(0.5f, 1.5f),   
        randomFloat(-1.5f, 1.7f)  
    );
    note.rotation = randomFloat(0.0f, 360.0f); 
    note.scale = randomFloat(0.8f, 1.2f);       
    note.lifetime = randomFloat(4.0f, 8.0f);   
    note.color = glm::vec3(randomFloat(0.5f, 1.0f), randomFloat(0.5f, 1.0f), randomFloat(0.5f, 1.0f));  
    jukeboxNotes.push_back(note);
}

void updateMusicNotes(std::vector<Note>& jukeboxNotes, float deltaTime) {
    for (auto& note : jukeboxNotes) {
        note.position += note.velocity * deltaTime; // Move note
        note.rotation += 90.0f * deltaTime;         // Rotate note
        note.scale += 0.2f * deltaTime;            // Scale note up
        note.lifetime -= deltaTime;                // Decrease lifetime
    }

    jukeboxNotes.erase(
        std::remove_if(jukeboxNotes.begin(), jukeboxNotes.end(),
            [](const Note& note) { return note.lifetime <= 0.0f; }),
        jukeboxNotes.end());
}
void renderMusicNotes(const std::vector<Note>& jukeboxNotes, gps::Shader& shader) {
    shader.useShaderProgram();
    for (const auto& note : jukeboxNotes) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), note.position);
        model = glm::rotate(model, glm::radians(note.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(note.scale));
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        musicNote.Draw(shader);
    }
}


void initOpenGLWindow() {
    myWindow.Create(1900, 1080, "OpenGL Project Core");
    glfwSetCursorPos(myWindow.getWindow(), 1500.0f / 2.0f, 850.0f / 2.0f);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(myWindow.getWindow(), GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
}


void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    //glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    house.LoadModel("models/house/Audition_Plaza.obj");
    token.LoadModel("models/coin/coin.obj");
    musicNote.LoadModel("models/music/music_notes.obj");
    fireworkParticle.LoadModel("models/cube/cube.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    myBasicShader.useShaderProgram();
    skyboxShader.loadShader(
        "shaders/skybox.vert",
        "shaders/skybox.frag");
    skyboxShader.useShaderProgram();
    depthShader.loadShader(
		"shaders/depthMap.vert",
		"shaders/depthMap.frag");
    depthShader.useShaderProgram();
}
void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // Attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
glm::mat4 computeLightSpaceMatrix() {
    glm::mat4 lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, 1.0f, 100.0f); 

    
    float radius = 36.0f; 
    glm::vec3 lightPos = glm::vec3(
        radius * cos(glm::radians(lightAngle)), // Rotate X
        29.0f,                                 // Fixed height
        radius * sin(glm::radians(lightAngle))  // Rotate Z
    );

    // Center of the scene
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, up);
    return lightProjection * lightView;
}



void initUniforms() {
    // Use the basic shader program
    myBasicShader.useShaderProgram();

    // For the fog.
    GLint fogEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogToggle");
    glUniform1i(fogEnabledLoc, fogToggle); 
    //For shadow mapping I HOPE
    GLint viewPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "viewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(myCamera.getCameraPosition()));
    
    // --- Model Matrix ---
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // --- View Matrix ---
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // --- Normal Matrix ---
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // --- Projection Matrix ---
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 100.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // --- Directional Light ---
    glm::vec3 dirLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
    glm::vec3 dirLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    bool isDirLightEnabled = true;

    GLint dirLightDirectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dirLight.direction");
    GLint dirLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dirLight.color");
    GLint dirLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dirLight.enabled");

    glUniform3fv(dirLightDirectionLoc, 1, glm::value_ptr(dirLightDirection));
    glUniform3fv(dirLightColorLoc, 1, glm::value_ptr(dirLightColor));
    glUniform1i(dirLightEnabledLoc, isDirLightEnabled);

    // --- Point Light ---
    glm::vec3 pointLightPosition = glm::vec3(2.0f, 3.0f, 1.0f);
    glm::vec3 pointLightColor = glm::vec3(1.0f, 1.0f, 0.8f);
    float pointLightConstant = 1.0f;
    float pointLightLinear = 0.09f;
    float pointLightQuadratic = 0.032f;
    bool isPointLightEnabled = true;

    GLint pointLightPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.position");
    GLint pointLightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.color");
    GLint pointLightConstantLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.constant");
    GLint pointLightLinearLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.linear");
    GLint pointLightQuadraticLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.quadratic");
    GLint pointLightEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pointLight.enabled");

    glUniform3fv(pointLightPositionLoc, 1, glm::value_ptr(pointLightPosition));
    glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));
    glUniform1f(pointLightConstantLoc, pointLightConstant);
    glUniform1f(pointLightLinearLoc, pointLightLinear);
    glUniform1f(pointLightQuadraticLoc, pointLightQuadratic);
    glUniform1i(pointLightEnabledLoc, isPointLightEnabled);

    //Point light lamp1
    glm::vec3 lamp1Position = lamp1.position;
    glm::vec3 lamp1Color = lamp1.color;
    float lamp1Constant = lamp1.constant;
    float lamp1Linear = lamp1.linear;
    float lamp1Quadratic = lamp1.quadratic;
    bool lamp1Enabled = lamp1.enabled;

    GLint lamp1PositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp1.position");
    GLint lamp1ColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp1.color");
    GLint lamp1ConstantLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp1.constant");
    GLint lamp1LinearLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp1.linear");
    GLint lamp1QuadraticLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp1.quadratic");
    GLint lamp1EnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp1.enabled");

    glUniform3fv(lamp1PositionLoc, 1, glm::value_ptr(lamp1Position));
    glUniform3fv(lamp1ColorLoc, 1, glm::value_ptr(lamp1Color));
    glUniform1f(lamp1ConstantLoc, lamp1Constant);
    glUniform1f(lamp1LinearLoc, lamp1Linear);
    glUniform1f(lamp1QuadraticLoc, lamp1Quadratic);
    glUniform1i(lamp1EnabledLoc, lamp1Enabled);

    //Point light lamp2
    glm::vec3 lamp2Position = lamp2.position;
    glm::vec3 lamp2Color = lamp2.color;
    float lamp2Constant = lamp2.constant;
    float lamp2Linear = lamp2.linear;
    float lamp2Quadratic = lamp2.quadratic;
    bool lamp2Enabled = lamp2.enabled;

    GLint lamp2PositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp2.position");
    GLint lamp2ColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp2.color");
    GLint lamp2ConstantLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp2.constant");
    GLint lamp2LinearLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp2.linear");
    GLint lamp2QuadraticLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp2.quadratic");
    GLint lamp2EnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lamp2.enabled");

    glUniform3fv(lamp2PositionLoc, 1, glm::value_ptr(lamp2Position));
    glUniform3fv(lamp2ColorLoc, 1, glm::value_ptr(lamp2Color));
    glUniform1f(lamp2ConstantLoc, lamp2Constant);
    glUniform1f(lamp2LinearLoc, lamp2Linear);
    glUniform1f(lamp2QuadraticLoc, lamp2Quadratic);
    glUniform1i(lamp2EnabledLoc, lamp2Enabled);

	//Point light dogLamp
    glm::vec3 dogLampPosition = dogLamp.position;
    glm::vec3 dogLampColor = dogLamp.color;
    float dogLampConstant = dogLamp.constant;
    float dogLampLinear = dogLamp.linear;
    float dogLampQuadratic = dogLamp.quadratic;
    bool dogLampEnabled = dogLamp.enabled;

    GLint dogLampPositionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dogLamp.position");
    GLint dogLampColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dogLamp.color");
    GLint dogLampConstantLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dogLamp.constant");
    GLint dogLampLinearLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dogLamp.linear");
    GLint dogLampQuadraticLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dogLamp.quadratic");
    GLint dogLampEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "dogLamp.enabled");

    glUniform3fv(dogLampPositionLoc, 1, glm::value_ptr(dogLampPosition));
    glUniform3fv(dogLampColorLoc, 1, glm::value_ptr(dogLampColor));
    glUniform1f(dogLampConstantLoc, dogLampConstant);
    glUniform1f(dogLampLinearLoc, dogLampLinear);
    glUniform1f(dogLampQuadraticLoc, dogLampQuadratic);
    glUniform1i(dogLampEnabledLoc, dogLampEnabled);

    // --- Black and white filter uniforms ---

    GLint bwFilterLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isBlackAndWhite");
    myBasicShader.useShaderProgram();
    glUniform1i(bwFilterLoc, isBlackAndWhite);

    // --- Inverted colors filter uniforms ---
    GLint invertedFilterLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isInverted");
    myBasicShader.useShaderProgram();
    glUniform1i(invertedFilterLoc, isInverted);

    // --- Sepia filter uniforms ---
    GLint sepiaFilterLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isSepia");
    myBasicShader.useShaderProgram();
    glUniform1i(sepiaFilterLoc, isSepia);

    // --- Cartoon filter uniforms ---
    GLint cartoonFilterLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isCartoon");
    myBasicShader.useShaderProgram();
    glUniform1i(cartoonFilterLoc, isCartoon);

    // --- Hue shift uniforms ---
    GLint hueShiftLoc = glGetUniformLocation(myBasicShader.shaderProgram, "hueShift");
    myBasicShader.useShaderProgram();
    glUniform1f(hueShiftLoc, hueShift);
    
    // --- Hue shift toggle ---
    GLint hueShiftEnabledLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isHueShift");
    myBasicShader.useShaderProgram();
    glUniform1i(hueShiftEnabledLoc, isHueShift);

    // --- Night vision toggle ---
    GLint nightVisionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isNightVision");
    myBasicShader.useShaderProgram();
    glUniform1i(nightVisionLoc, isNightVision);

    // --- Old TV effect toggle ---
    GLint oldTVLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isOldTV");
    myBasicShader.useShaderProgram();
    glUniform1i(oldTVLoc, isOldTV);

    // --- Pixelated effect toggle ---
    GLint pixelatedLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isPixelated");
    GLint pixelSizeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pixelSize");
    myBasicShader.useShaderProgram();
    glUniform1i(pixelatedLoc, isPixelated);
    glUniform2f(pixelSizeLoc, pixelSize.x, pixelSize.y);


    // --- Skybox ---
    skyboxShader.useShaderProgram();

    GLint skyboxProjectionLoc = glGetUniformLocation(skyboxShader.shaderProgram, "projection");
    glUniformMatrix4fv(skyboxProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set view matrix (without translation)
    GLint skyboxViewLoc = glGetUniformLocation(skyboxShader.shaderProgram, "view");
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view)); // Remove translation
    glUniformMatrix4fv(skyboxViewLoc, 1, GL_FALSE, glm::value_ptr(viewNoTranslation));

    // Set model matrix (optional, typically identity for skybox)
    GLint skyboxModelLoc = glGetUniformLocation(skyboxShader.shaderProgram, "model");
    glUniformMatrix4fv(skyboxModelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f))); // Identity matrix

}

void initSkybox() {
    std::vector<const GLchar*> faces;
    faces.push_back("models/skybox/redeclipse_rt.png");
    faces.push_back("models/skybox/redeclipse_lf.png");

    faces.push_back("models/skybox/redeclipse_up.png");
    faces.push_back("models/skybox/redeclipse_dn.png");

    faces.push_back("models/skybox/redeclipse_bk.png");
    faces.push_back("models/skybox/redeclipse_ft.png");

    mySkyBox.Load(faces);
}

void renderTeapot(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();

    if(!depthPass)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glCheckError();

    if(!depthPass)
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glCheckError();

    house.Draw(shader);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Update model matrix uniform
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix)); // Update normal matrix uniform

    depthShader.useShaderProgram();
    glm::mat4 lightSpaceMatrix = computeLightSpaceMatrix();
    glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "lightSpaceMatrix"),1,GL_FALSE,glm::value_ptr(lightSpaceMatrix));

    glm::mat4 depthModelMatrix = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(depthModelMatrix));
    renderTeapot(depthShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //Unbind the framebuffer

    // Step 2: Render the scene with shadows
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height); // Reset viewport
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
    
    glDepthFunc(GL_LESS);
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 0); // Shadow map is bound to texture unit 0

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture); // Bind the shadow map texture

    // Set other uniforms like view, projection, and model matrices
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(myCamera.getViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(depthModelMatrix));

    // Render the house model with shadows
    renderTeapot(myBasicShader, false);

    glDepthFunc(GL_LEQUAL); 
    skyboxShader.useShaderProgram();

    mySkyBox.Draw(skyboxShader, view, projection);
    
    //Render the music notes
    renderMusicNotes(jukebox1Notes, myBasicShader);
    renderMusicNotes(jukebox2Notes, myBasicShader);
    renderMusicNotes(jukebox3Notes, myBasicShader);
    renderMusicNotes(extraLocationsNotes, myBasicShader);

    //Render the gold coins
    if (toggleRain) {
        for (auto& coin : coins) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), coin.position);
            model = glm::rotate(model, glm::radians(coin.rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(coin.size));
            glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            token.Draw(myBasicShader);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    initGold(150, 20.0f);
    initSkybox();
    initFBO();

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    glCheckError();
    
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        updateGold(0.01f, 20.0f);

        static float spawnTimer1 = 0.0f, spawnTimer2 = 0.0f, spawnTimer3 = 0.0f, extraSpawnTimer = 0.0f;
        static float fireworkSpawnTimer = 0.0f;
        float deltaTime = 0.01f;

        spawnTimer1 += deltaTime;
        spawnTimer2 += deltaTime;
        spawnTimer3 += deltaTime;
        extraSpawnTimer += deltaTime;

        if (spawnTimer1 > 0.5f) { 
            spawnMusicNote(jukebox1Notes, glm::vec3(25.7689f, 3.3093f, 6.76255f));  
            spawnTimer1 = 0.0f;
        }
        if (spawnTimer2 > 0.5f) { 
            spawnMusicNote(jukebox2Notes, glm::vec3(4.54648f, 3.4914f, 29.774f));  
            spawnTimer2 = 0.0f;
        }
        if (spawnTimer3 > 0.5f) { 
            spawnMusicNote(jukebox3Notes, glm::vec3(-6.20397f, 3.7623f, -29.1902f));  
            spawnTimer3 = 0.0f;
        }
        if (extraSpawnTimer > 0.5f) {  
            spawnMusicNote(extraLocationsNotes, glm::vec3(26.8443f, 2.66127f, -7.52428f)); 
            spawnMusicNote(extraLocationsNotes, glm::vec3(0.975846f, 2.71715f, -30.6594f));  
            spawnMusicNote(extraLocationsNotes, glm::vec3(-25.5434f, 2.57449f, 7.00975f));  
            spawnMusicNote(extraLocationsNotes, glm::vec3(-26.1442f, 2.10231f, -7.00535f));  
            extraSpawnTimer = 0.0f;
        }
        if (isFireworkActive) {
            fireworkSpawnTimer += deltaTime;
            if (fireworkSpawnTimer >= 0.5f) { 
                spawnFireworkInCircle(15.0f); 
                fireworkSpawnTimer = 0.0f;    
            }
        }


        // Update all notes
        updateMusicNotes(jukebox1Notes, deltaTime);
        updateMusicNotes(jukebox2Notes, deltaTime);
        updateMusicNotes(jukebox3Notes, deltaTime);
        updateMusicNotes(extraLocationsNotes, deltaTime);
        updateFireworks(deltaTime);

        // Render all notes
        renderMusicNotes(jukebox1Notes, myBasicShader);
        renderMusicNotes(jukebox2Notes, myBasicShader);
        renderMusicNotes(jukebox3Notes, myBasicShader);
        renderMusicNotes(extraLocationsNotes, myBasicShader);

        processMovement();
        updateTour();
        renderScene();
        renderFireworks(myBasicShader);
        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());
        glCheckError();
    }
    cleanup();
    return EXIT_SUCCESS;
}