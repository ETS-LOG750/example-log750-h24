#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>
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

	// Create 2 triangles vertices
	void CreateVertices();

	// Rendering scene (OpenGL)
	void RenderScene();
	
private:
	// settings
	const unsigned int SCR_WIDTH = 500;
	const unsigned int SCR_HEIGHT = 500;
	GLFWwindow* m_window = nullptr;

	// Geometry
	GLuint m_vao;
	// - positions
	GLuint m_vbo_position;
	std::vector<GLfloat> m_vertices; // Array holding vertices
	GLint m_numVertices; // Number of vertex

	std::unique_ptr<ShaderProgram> m_mainShader = nullptr ;
};
