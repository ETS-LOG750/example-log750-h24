#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <array>

#include "ShaderProgram.h"

class MainWindow
{
public:
	MainWindow();

	// Main functions (initialization, run)
	int Initialisation();
	int RenderLoop();

	// Callback to intersept GLFW calls
	void FramebufferSizeCallback(int width, int height);
private:
	// Initialize GLFW callbacks
	void InitializeCallback();
	// Intiialize OpenGL objects (shaders, ...)
	int InitializeGL();

	// Rendering scene (OpenGL)
	void RenderScene();
	// Rendering interface ImGUI
	void RenderImgui();

	void updateCameraEye();

private:
	// settings (window)
	const unsigned int SCR_WIDTH = 900;
	const unsigned int SCR_HEIGHT = 900;

	float m_longitude = 0.0f ;
	float m_latitude = 0.0f;
	float m_distance = 8.0f;

	// Simple camera
	glm::vec3 m_eye, m_at, m_up;
	glm::mat4 m_proj;

	// Rendering mode
	bool m_showNormal = true;

	// Informations for the geometry
	enum VAO_IDs { Triangles, NumVAOs };
	enum Buffer_IDs { ArrayBuffer, ElementBuffer, NumVertexBuffers };
	GLuint m_VAOs[NumVAOs];
	GLuint m_buffers[NumVertexBuffers];
	std::array<glm::vec4, 32> m_colors;

	// Tesselation
	GLfloat  m_inner = 16.0;
	GLfloat  m_outer = 16.0;

	// GLFW Window
	GLFWwindow* m_window = nullptr;

	std::unique_ptr<ShaderProgram> m_mainShader = nullptr;
	std::unique_ptr<ShaderProgram> m_constantColorShader = nullptr;
	GLint m_mainShader_showNormal = -1; 
	GLint m_mainShader_inner = -1;
	GLint m_mainShader_outer = -1;

	const GLint SHADER_MATRIX = 0;
	const GLint SHADER_MATRIX_NORMAL = 1;
	const GLint SHADER_PROJECTION = 2;
	const GLint SHADER_COLOR = 3;
};
