cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(11_SimpleFBO)

# Add source files
set(SOURCE_FILES 
	Main.cpp
	MainWindow.cpp
)
set(HEADER_FILES 
	MainWindow.h
)
set(SHADER_FILES 
	basicShader.vert
	basicShader.frag)

# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${SHADER_FILES} ${SHARED_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC SHADERS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Define the link libraries
target_link_libraries(${PROJECT_NAME} ${LIBS})
