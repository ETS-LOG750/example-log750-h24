#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <memory>

#include "ShaderProgram.h"
#include "Camera.h"

class MainWindow
{
public:
	MainWindow();

	// Main functions (initialization, run)
	int Initialisation();
	int RenderLoop();

	// Callback to intersept GLFW calls
	void FramebufferSizeCallback(int width, int height);
	void MouseButtonCallback(int button, int action, int mods);
	void CursorPositionCallback(double xpos, double ypos);

private:
	// Initialize GLFW callbacks
	void InitializeCallback();
	// Intiialize OpenGL objects (shaders, ...)
	int InitializeGL();
	// Load spiral geometry (0 = success)
	int InitGeometrySpiral();

	// Rendering scene (OpenGL)
	void RenderScene();
	// Rendering interface ImGUI
	void RenderImgui();
	
	// Perform selection on the object
	void PerformSelection(int x, int y);

private:
	// settings
	unsigned int m_windowWidth = 1200;
	unsigned int m_windowHeight = 800;

	// GLFW Window
	GLFWwindow* m_window = nullptr;

	// Camera
	Camera m_camera;

	// VAOs and VBOs
	enum VAO_IDs { VAO_Spiral, VAO_SpiralSelected, VAO_SpiralPicking, VAO_Ray, NumVAOs };
	enum Buffer_IDs { VBO_Spiral, VBO_Ray, NumBuffers };

	GLuint m_VAOs[NumVAOs];
	GLuint m_buffers[NumBuffers];
	
	// Render shaders & locations
	std::unique_ptr<ShaderProgram> m_mainShader = nullptr;
	struct {
		GLint uProjMatrix;
		GLint uMatrix;
		GLint uNormalMatrix;
	} m_mainShaderLocations;
	std::unique_ptr<ShaderProgram> m_pickingShader = nullptr;
	struct {
		GLint uProjMatrix;
		GLint uColor;
		GLint uMatrix;
	} m_pickingShaderLocations;

	// Shader constant location 
	const GLint SHADER_POSTION_LOCATION = 0;
	const GLint SHADER_NORMAL_LOCATION = 1;
	const GLint SHADER_COLOR_LOCATION = 2;

	// Picking parameters
	int m_selectedSpiral = -1;
	glm::vec3 m_point = glm::vec3(0.0);
};
