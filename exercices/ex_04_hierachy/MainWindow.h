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

	void exerice(float v) const;
	void drawCircle(float Sx, float Sy, glm::mat4 T, glm::vec3 color) const;

private:
	// settings
	const unsigned int SCR_WIDTH = 800;
	const unsigned int SCR_HEIGHT = 800;
	GLFWwindow* m_window = nullptr;

	bool m_animate = true;
	float m_time = 0.0;

	// Geometries
	enum VAO_IDs { CircleVAO, LineVAO, NumVAOs };
	enum Buffer_IDs { Circle, LinesBuffers, NumBuffers };
	GLuint m_VAOs[NumVAOs];
	GLuint m_buffers[NumBuffers];

	std::unique_ptr<ShaderProgram> m_mainShader = nullptr ;
	GLint m_mainShader_matrix = -1;
	GLint m_mainShader_color = -1;
};
