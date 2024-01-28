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

	void updateGeometry();

private:
	// settings
	const unsigned int SCR_WIDTH = 900;
	const unsigned int SCR_HEIGHT = 720;
	const unsigned int COUNT = 18;
	GLFWwindow* m_window = nullptr;

	// Transformations
	int m_selected_mode = 0;
	// - angles euler (x, y, z)
	glm::vec3 m_rot1 = glm::vec3(0);
	glm::vec3 m_rot2 = glm::vec3(0);
	// - quaternion
	float m_quat1_angle = 0;
	glm::vec3 m_quat1_axis = glm::vec3(0, 0, 1);
	float m_quat2_angle = 0;
	glm::vec3 m_quat2_axis = glm::vec3(0, 0, 1);
	bool m_linear = true;
	// - time
	float m_time = 0;
	bool m_animate = true;

	// VAO, VBO
	enum VAO_IDs { Triangles, NumVAOs };
	enum Buffer_IDs { Position, Normal, NumBuffers };
	GLuint m_VAOs[NumVAOs];
	GLuint m_VBOs[NumBuffers];

	// Object
	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec3> m_normals;

	std::unique_ptr<ShaderProgram> m_mainShader = nullptr;
	GLint m_mainShader_matrix = 0;
	GLint m_mainShader_matrixNormal = 1;
};
