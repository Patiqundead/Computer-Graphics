cmake_minimum_required (VERSION 2.6)

set(PROJECT_NAME "Trabalho" CACHE STRING "Projeto")

project (${PROJECT_NAME})

find_package(OpenGL REQUIRED)

add_subdirectory (external)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-1c73e35")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) 

if(INCLUDE_DISTRIB) 
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)

include_directories(
	external/AntTweakBar-1.16/include/
	external/glfw-2.7.6/include/
	external/glm-0.9.4.0/
	external/glew-1.9.0/include/
	common/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	GLFW_276
	GLEW_190
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

add_executable(main main.cpp
			common/controls.hpp
			common/controls.cpp
			common/objloader.hpp
			common/objloader.cpp
			common/quaternion_utils.hpp
			common/quaternion_utils.cpp
			common/reflection.h
			common/reflection.cpp
			common/shader.hpp
			common/shader.cpp
			common/tangentspace.hpp
			common/tangentspace.cpp
			common/text2D.hpp
			common/text2D.cpp
			common/texture.hpp
			common/texture.cpp
			common/vboindexer.hpp
			common/vboindexer.cpp
)

target_link_libraries(main ${ALL_LIBS}
				ANTTWEAKBAR_116_OGLCORE_GLFW)

set_target_properties(main PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")

create_target_launcher(main WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")

create_default_target_launcher(main WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )

if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
add_custom_command(
   TARGET main POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/main${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/"
)

elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

