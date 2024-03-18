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
	void CursorPositionCallback(double xpos, double ypos);
private:
	// Initialize GLFW callbacks
	void InitializeCallback();
	// Intiialize OpenGL objects (shaders, ...)
	int InitializeGL();

	// Rendering scene (OpenGL)
	void RenderScene();
	// Rendering interface ImGUI
	void RenderImgui();

	bool loadTexture(const std::string& path, 
		unsigned int& textureID, 
		GLint uvMode, // UV/ST warp mode
		GLint minMode,  // Minfication
		GLint magMode); // Magnification

	void initGeometrySphere();

private:
	// settings
	unsigned int m_windowWidth = 900;
	unsigned int m_windowHeight = 720;

	// Skybox shader
	std::unique_ptr<ShaderProgram> m_skydomeShader = nullptr;
	int m_vSkyPos = -1;
	struct {
		GLint viewRotMatrix;
		GLint projMatrix;
		GLint texSkydome;
	} m_uSky;
	
	// Normal shader
	std::unique_ptr<ShaderProgram> m_mainShader = nullptr;
	int m_vMainPos = -1;
	int m_vMainNormal = -1;
	glm::vec3 m_uMainDiffColor = glm::vec3(0.8);
	glm::vec3 m_uMainLightInt = glm::vec3(1.0);
	struct {
		GLint diffuseColor;
		GLint lightIntensity;
		GLint mvMatrix;
		GLint projMatrix;
		GLint normalMatrix;
	} m_uMain;

	// Camera
	Camera m_camera;
	bool m_imGuiActive = false;

	// Sphere
	int numRowSphere = 20;
	int numColSphere = numRowSphere + 2;
	int numTriSphere = numColSphere * (numRowSphere - 1) * 2 + 2 * numColSphere;

	// VBO/VAO
	enum VAO_IDs { VAO_Sphere_Sky, VAO_Sphere, NumVAOs };
	enum Buffer_IDs { VBO_Sphere_Position, VBO_Sphere_Normal, VBO_Sphere_UV, EBO_Sphere, NumBuffers };
	GLuint m_VAOs[NumVAOs];
	GLuint m_buffers[NumBuffers];

	// Textures
	unsigned int TextureId = -1 ;

	// GLFW Window
	GLFWwindow* m_window = nullptr;
};
