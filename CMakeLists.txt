cmake_minimum_required (VERSION 3.26)

project (
	Yaboc
	LANGUAGES C CXX
	VERSION 0.1.0
	DESCRIPTION "Yet Another BreakOut Clone"
)

list (APPEND CMAKE_MODULE_PATH ${Yaboc_SOURCE_DIR}/tools/cmake)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

include (StageConfig)
include (Dependencies)
include (CompilerConfig)

add_executable (yaboc)

target_include_directories (
	yaboc

	PRIVATE
	${Yaboc_SOURCE_DIR}/include
)

target_sources (
	yaboc

	PRIVATE
	include/yaboc/shader.h

	src/shader.cpp
	src/main.cpp
)

target_link_libraries (
	yaboc

	PRIVATE
	SDL3::SDL3
	OpenGL::GL
	glm::glm
	Glad::Glad
)
