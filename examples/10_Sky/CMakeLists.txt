cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(10_Sky)

# Add source files
SET(SOURCE_FILES 
	Main.cpp
	Mainwindow.cpp)
set(HEADER_FILES 
	MainWindow.h)
set(SHADER_FILES 
	main.vert
	main.frag)

# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${SHADER_FILES} ${SHARED_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC SHADERS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Define the link libraries
target_link_libraries(${PROJECT_NAME} ${LIBS})
