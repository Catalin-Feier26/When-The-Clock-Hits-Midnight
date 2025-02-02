#define GLM_ENABLE_EXPERIMENTAL
#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO

        return glm::lookAt(cameraPosition, cameraTarget, this->cameraUpDirection);
    }
    glm::vec3 Camera::getCameraPosition() const {
        return cameraPosition;
    }

    // Getter for camera target
    glm::vec3 Camera::getCameraTarget() const {
        return cameraTarget;
    }

    void Camera::setCameraPosition(const glm::vec3& position) {
        cameraPosition = position;
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }

    void Camera::setCameraTarget(const glm::vec3& target) {
        cameraTarget = target;
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }
    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        // Calculate the forward vector
        glm::vec3 forward = glm::normalize(cameraTarget - cameraPosition);
        // Calculate the right vector
        glm::vec3 right = glm::normalize(glm::cross(forward, cameraUpDirection));

        switch (direction) {
        case MOVE_FORWARD:
            cameraPosition += forward * speed;
            cameraTarget += forward * speed;
            break;
        case MOVE_BACKWARD:
            cameraPosition -= forward * speed;
            cameraTarget -= forward * speed;
            break;
        case MOVE_LEFT:
            cameraPosition -= right * speed;
            cameraTarget -= right * speed;
            break;
        case MOVE_RIGHT:
            cameraPosition += right * speed;
            cameraTarget += right * speed;
            break;
        }
    }


    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw)
    {
        // Accumulate the pitch and yaw changes
        static float currentPitch = 0.0f;
        static float currentYaw = -90.0f; // Initialize yaw to -90.0f to face forward by default

        currentPitch += pitch;
        currentYaw += yaw;

        // Clamp the pitch to prevent flipping
        if (currentPitch > 89.0f) currentPitch = 89.0f;
        if (currentPitch < -89.0f) currentPitch = -89.0f;

        // Calculate the new direction vector
        glm::vec3 direction;
        direction.x = cos(glm::radians(currentYaw)) * cos(glm::radians(currentPitch));
        direction.y = sin(glm::radians(currentPitch));
        direction.z = sin(glm::radians(currentYaw)) * cos(glm::radians(currentPitch));

        // Update the camera target based on the new direction
        cameraTarget = cameraPosition + glm::normalize(direction);
    }


}