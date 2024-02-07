#pragma once 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <GLFW/glfw3.h>

#include <iostream>

class Camera {
public:
    // Create camera with width and height window
    // at a given position and looking at a point
    Camera(int width, int height,
        const glm::vec3& position,
        const glm::vec3& at);

    // Methods processing GLFW inputs
    void keybordEvents(GLFWwindow * w, const float delta_time);
    void mouseEvents(const glm::vec2& mousePos, bool clicked) ;
    void viewportEvents(int width, int height);

    // Compute view matrix
    glm::mat4 viewMatrix() const {
        return glm::lookAt(m_position, m_position+m_direction, m_up);
    }
    // Compute (perspective) projection matrix
    glm::mat4 projectionMatrix() const {
        return m_proj_matrix;
    }

    void setPosition(const glm::vec3& pos) {
        m_position = pos;
        computeAngles();
    }
    void setDirection(const glm::vec3& dir) {
        m_direction = dir;
        computeAngles();
    }
    void setFar(float far) {
        m_far = far;
        updateProjectionMatrix();
    }
    void setNear(float near) {
        m_near = near;
        updateProjectionMatrix();
    }
    const glm::vec3& position() const { return m_position;  }
    float fieldOfView() const { return m_fov;  }
private:
    // Compute yaw and vertical angles for the view direction
    void computeAngles();

    void updateProjectionMatrix();

private:
    // Camera parameters
    // - camera position
    glm::vec3 m_position;
    // - camera up vector
    const glm::vec3 m_up = glm::vec3(0, 1, 0);
    // - view direction
    glm::vec3 m_direction;

    // Projection matrix
    const float m_fov = glm::radians(45.0f);
	float m_image_ratio;
    float m_near = 0.1f;
    float m_far = 100.0f;
    glm::mat4 m_proj_matrix;

    // Orientation in degrees
    float yaw;
    float pitch;

    // Mouse position tracking
    bool m_mouse_was_clicked = false;
    glm::vec2 m_last_mouse_pos;
};