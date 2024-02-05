#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <memory>
#include <glm/gtx/euler_angles.hpp>

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

private:
	// settings
	const unsigned int SCR_WIDTH = 900;
	const unsigned int SCR_HEIGHT = 900;
	GLFWwindow* m_window = nullptr;

	// Transformation object
	glm::vec3 m_rot1 = glm::vec3(0);

	// Show normal (geometry shader)
	bool m_showNormal = false;
	float m_scale = 0.3f;

	enum VAO_IDs { Triangles, NumVAOs };
	enum Buffer_IDs { ArrayBuffer, NumBuffers };
	size_t m_nbVertices = 3; 

	GLuint m_VAOs[NumVAOs];
	GLuint m_buffers[NumBuffers];

	std::vector<GLfloat> m_vertices; // Array holding vertices
	std::vector<GLfloat> m_normals; // Array holding normals
	GLint m_numCoordinatesPerVertices; // Number of coordinates per vertex in the m_vertices array

	std::unique_ptr<ShaderProgram> m_mainShader = nullptr;
	std::unique_ptr<ShaderProgram> m_normalShader = nullptr;

	// Note only for OpenGL 4.3 
	// you can use the old technique that query the uniform location
	const GLint SHADER_MATRIX = 0;
	const GLint SHADER_MATRIX_NORMAL = 1;
	const GLint SHADER_SCALE = 2;
};
