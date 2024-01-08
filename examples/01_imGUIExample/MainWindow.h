#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

class MainWindow
{
public:
	MainWindow();

	// Main functions (initialization, run)
	int Initialisation();
	int RenderLoop();

	// Callback to intersept GLFW calls
	void FramebufferSizeCallback(int width, int height);
	void CursorPositionCallback(double xpos, double ypos);
	void MouseButtonCallback(int button, int action, int mods);

private:
	// Initialize GLFW callbacks
	void InitializeCallback();
	// Intiialize OpenGL objects (shaders, ...)
	int InitializeGL();

	// Rendering scene (OpenGL)
	void RenderScene();
	// Rendering interface ImGUI
	void RenderImgui();

private:
	// settings
	const unsigned int SCR_WIDTH = 800;
	const unsigned int SCR_HEIGHT = 600;

	// GLFW Window
	GLFWwindow* m_window = nullptr;

	// Clear color (controlled by imGui)
	glm::vec3 m_clearColor = glm::vec3(0.0);
	float m_slider = 0.0f;
	int m_counter = 0;

	// Information about the cursor and state
	glm::vec2 m_cursor_position = glm::vec2(0.0);
	bool m_mouse_right_pressed = false;
};