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
	// Rendering interface ImGUI
	void RenderImgui();

	// Geometry
	int InitGeometryCube();
	int InitGeometryFloor();
	int InitPlane2D();

	// Shadow map
	void ShadowRender();
	
	// Animation light position
	void UpdateLightPosition(float delta_time);

private:
	// settings
	const unsigned int SCR_WIDTH = 1200;
	const unsigned int SCR_HEIGHT = 800;

	// Camera settings 
	glm::vec3 m_eye, m_at, m_up;
	glm::mat4 m_proj;

	// Main shader
	std::unique_ptr<ShaderProgram> m_mainShader = nullptr;
	struct {
		GLint MVMatrix = -1;
		GLint ProjMatrix = -1;
		GLint MLPMatrix = -1;
		GLint normalMatrix = -1;
		GLint uColor = -1;
		GLint texShadowMap = -1;
		GLint lightPositionCameraSpace = -1;
		GLint biasType = -1;
		GLint biasValue = -1; 
		GLint biasValueMin = -1;
	} m_mainUniforms;

	// Shadow map shader
	std::unique_ptr<ShaderProgram> m_shadowMapShader = nullptr;
	struct {
		GLint MLP = -1;
	} m_shadowMapUniforms;

	// Debug shader
	std::unique_ptr<ShaderProgram> m_debugShader = nullptr;
	struct {
		GLint tex = -1;
		GLint scale = -1;
	} m_debugUniforms;
	bool m_debug = false;
	float m_debugScale = 1.0f;

	// Light position
	// - For animation
	bool m_lightAnimation = true;
	double m_lightRadius;
	double m_lightAngle;
	double m_lightHeight;
	// - Light position 3d
	glm::vec3 m_lightPosition;
	// - Matrix to project to the shadow map
	glm::mat4 m_lightViewProjMatrix;
	float m_lightNear = 0.1f;
	float m_lightFar = 10.0f;

	// Shadow map information
	const int SHADOW_SIZE_X = 2048;
	const int SHADOW_SIZE_Y = 2048;
	int m_biasType = 0;
	float m_biasValue = 0.0005f;
	float m_biasValueMin = 0.00005f;


	// Face cube
	static const int NumFacesCube = 6;
	static const int NumTriCube = NumFacesCube * 2;
	static const int NumVerticesCube = 4 * NumFacesCube;
	static const int NumVerticesFloor = 4;

	unsigned int DepthMapFBO = 0;
	unsigned int TextureId = 0;

	enum VAO_IDs { CubeVAO, FloorVAO, Plane2DVAO, NumVAOs };
	enum VBO_IDs { CubeVBO, CubeEBO, FloorVBO, Plane2DVBO, NumVBOs };

	GLuint m_VAOs[NumVAOs];
	GLuint VBOs[NumVBOs];

	glm::vec4 m_color;
	glm::vec3 m_cubePosition = glm::vec3(0.0, 1.0, 0.0);

	// GLFW Window
	GLFWwindow* m_window = nullptr;
};
