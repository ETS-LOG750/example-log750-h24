#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <memory>

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

private:
	// settings
	const unsigned int SCR_WIDTH = 900;
	const unsigned int SCR_HEIGHT = 900;
	GLFWwindow* m_window = nullptr;

	// Geometries
	enum VAO_IDs { SeparateVAO, SharedVAO, NumVAOs };
	enum Buffer_IDs { SeparateVertexBuffer, SharedVertexBuffer, SharedIndexBuffer, NumBuffers };
	GLuint m_VAOs[NumVAOs];
	GLuint m_buffers[NumBuffers];

	// Shader
	std::unique_ptr<ShaderProgram> m_mainShader = nullptr ;
	GLint m_mainShader_matrix = -1;
	GLint m_mainShader_color = -1;
};
