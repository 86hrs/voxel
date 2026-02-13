// camera.h
#pragma once
#include "glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 13.0f;
constexpr float SENSITIVITY = 0.1f;
constexpr float ZOOM = 90.0f;

class Camera {
  public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW,
           float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
          MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        updateCameraVectors();
    }
    glm::mat4 get_view_matrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        float c = Position.y;

        if (direction == FORWARD) {
            Position += Front * velocity;
            Position.y = c;
        }
        if (direction == BACKWARD) {
            Position -= Front * velocity;
            Position.y = c;
        }
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += Up * velocity;
        if (direction == DOWN)
            Position -= Up * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset,
                              GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        this->Yaw += xoffset;
        this->Pitch += yoffset;

        if (constrainPitch)
            Pitch = glm::clamp(Pitch, -89.0f, 89.0f);
        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset) {
        this->Zoom -= (float)yoffset * 5;
        this->Zoom = glm::clamp(this->Zoom, 45.0f, 120.0f);
    }

  private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        this->Front = glm::normalize(front);
        this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
        this->Up = glm::normalize(glm::cross(this->Right, this->Front));
    }
};
