// ---------------------------------------------------
/* Code based on Learn openGL shader class
 *  
 * https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h
 * Copyright (C) Joey de Vries -
 * licensed under the terms of the CC BY-NC 4.0 license as published by Creative Commons
 *
 * https://creativecommons.org/licenses/by-nc/4.0/legalcode
 *
 */

#include "ShaderProgram.h"
#include <iostream>


// utility function that translate OpenGL error code to console output
// --------------------------------------------------------------------
void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
bool checkCompileErrors(GLuint shader, std::string type, std::string filename)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n";
			if (filename != "")
				std::cerr << "Filename: " << filename << "\n";
			std::cerr << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n";
			if (filename != "")
				std::cerr << "Filename: " << filename << "\n";
			std::cerr << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	return success;
}

ShaderProgram::ShaderProgram()
{
	// Note that the Glad need to be initialized before calling this line
	m_ID = glCreateProgram();
}

bool ShaderProgram::addShaderFromSource(GLenum shader_type, const std::string& path) {
	std::string shader_type_str = [&]() -> std::string {
		if (shader_type == GL_VERTEX_SHADER) {
			return "VERTEX";
		}
		else if (shader_type == GL_FRAGMENT_SHADER) {
			return "FRAGMENT";
		}
		else if (shader_type == GL_TESS_CONTROL_SHADER) {
			return "TCONTROL";
		}
		else if (shader_type == GL_TESS_EVALUATION_SHADER) {
			return "TEVAL";
		} 
		else if (shader_type == GL_GEOMETRY_SHADER) {
			return "GEOMETRY";
		}
		else if (shader_type == GL_COMPUTE_SHADER) {
			return "COMPUTE";
		}
		else {
			return "UNKNOW";
		}
	}();
	if (shader_type_str == "UNKNOW") {
		std::cerr << "BAD shader type: " << shader_type_str << "\n";
		return false;
	}

	// Read file
	std::string code;
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		file.open(path);
		std::stringstream ss;
		ss << file.rdbuf();
		file.close();
		code = ss.str();
	} 
	catch (std::ifstream::failure& e)
	{
		std::cerr << "Impossible to read: " << path << std::endl;
		std::cerr << e.what() << std::endl;
		return false;
	}
	GLuint shader_id = glCreateShader(shader_type);
	const char* code_c_str = code.c_str();
	glShaderSource(shader_id, 1, &code_c_str, NULL);
	glCompileShader(shader_id);
	bool success = checkCompileErrors(shader_id, shader_type_str, path);
	glAttachShader(m_ID, shader_id);
	if (success) {
		m_shaders_ids[shader_type_str] = shader_id;
	}
	return success;
}

bool ShaderProgram::link() {
	glLinkProgram(m_ID);
	m_linked = checkCompileErrors(m_ID, "PROGRAM", "");
	return m_linked;
}
