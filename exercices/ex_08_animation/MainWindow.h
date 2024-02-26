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
	void RenderScene(float t);
	void RenderImgui();

	glm::mat4 transform(float v) const;

private:
	// settings
	const unsigned int SCR_WIDTH = 800;
	const unsigned int SCR_HEIGHT = 800;
	GLFWwindow* m_window = nullptr;

	bool m_animate = true;
	float m_time = 0.0;

	// Geometries
	enum VAO_IDs { SharedVAO, SimpleVAO, NumVAOs };
	enum Buffer_IDs { SharedVertexBuffer, SharedIndexBuffer, LinesBuffers, NumBuffers };
	GLuint m_VAOs[NumVAOs];
	GLuint m_buffers[NumBuffers];


	// Square with shared vertices
	GLfloat VerticesShared[5][3] = {
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.5f, 1.5f, 0.0f }
	};

	// Triangle indices
	GLint IndicesShared[3][3] = {
		{ 0, 1, 2 },
		{ 0, 2, 3 },
		{ 4, 0, 1 }
	};

	std::unique_ptr<ShaderProgram> m_mainShader = nullptr ;
	struct {
		GLint mvMatrix;
		GLint color;
	} m_mainShaderUniforms;
};
