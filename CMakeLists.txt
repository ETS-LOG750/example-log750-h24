cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(ETS-LOG750)

#######################################
# LOOK for the packages that we need! #
#######################################

if (APPLE)
    set (CMAKE_CXX_FLAGS "-std=c++17")
endif()

# GLM: Math library
include_directories(3rdparty/glm)
add_subdirectory(3rdparty/glm)

# GLFW: Creating windows and OpenGL context
set(GLFW_BUILD_DOCS OFF CACHE BOOL OFF FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL OFF FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL OFF FORCE)
set(GLFW_INSTALL OFF CACHE BOOL OFF FORCE)
set(GLFW_LIBRARY_TYPE OFF CACHE BOOL "STATIC" FORCE)
add_subdirectory(3rdparty/glfw)
include_directories(3rdparty/glfw/include) # For windows builds

# GLAD: Load OpenGL extensions
include_directories(3rdparty)
add_library(GLAD 3rdparty/glad/glad.c)

# imGUI: Interface
include_directories(3rdparty/imgui 3rdparty/imgui/backends)
add_library(IMGUI
        3rdparty/imgui/imgui.cpp
        3rdparty/imgui/imgui.h
        3rdparty/imgui/imgui_demo.cpp
        3rdparty/imgui/imgui_draw.cpp
        3rdparty/imgui/imgui_widgets.cpp
        3rdparty/imgui/imgui_tables.cpp
        3rdparty/imgui/imgui_internal.h
        3rdparty/imgui/imstb_rectpack.h
        3rdparty/imgui/imstb_textedit.h
        3rdparty/imgui/imstb_truetype.h
		# Here implementation
        3rdparty/imgui/backends/imgui_impl_opengl3.cpp
        3rdparty/imgui/backends/imgui_impl_opengl3.h
	    3rdparty/imgui/backends/imgui_impl_opengl3_loader.h
        3rdparty/imgui/backends/imgui_impl_glfw.cpp
        3rdparty/imgui/backends/imgui_impl_glfw.h

	    3rdparty/imgui/imconfig.h)

# STB (header only library): Load images
include_directories(3rdparty/stbImage)

# List of libs to link each projects
set(LIBS GLAD IMGUI glfw)

####################################################
# The different projects that we are interested in #
####################################################
add_subdirectory(examples)