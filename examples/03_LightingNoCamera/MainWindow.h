#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
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

	// Rendering scene (OpenGL)
	void RenderScene();
	// Rendering interface ImGUI
	void RenderImgui();

	// Create geomtry for the mesh
	void updateGeometry();
private:
	// settings
	const unsigned int SCR_WIDTH = 512;
	const unsigned int SCR_HEIGHT = 512;
	const unsigned int COUNT = 18;
	GLFWwindow* m_window = nullptr;

	// ImGui variables (controlling geometry)
	float m_rotationX = 0.0;
	bool m_showNormals = false;
	bool m_flatNormals = false;
	bool m_phongShading = true;

	// Buffers (voir example Position and Color pour explication)
	enum VAO_IDs { Triangles, NumVAOs };
	enum Buffer_IDs { Position, Normal, NumBuffers };
	GLuint m_VAOs[NumVAOs];
	GLuint m_VBOs[NumBuffers];

	// Can be std::vector<float> or std::vector<GLfloat>
	std::vector<glm::vec3> m_vertices; // Array holding vertices
	std::vector<glm::vec3> m_normals; // Array holding normals

	std::unique_ptr<ShaderProgram> m_phongShader = nullptr;
	std::unique_ptr<ShaderProgram> m_gouraudShader = nullptr;

};
