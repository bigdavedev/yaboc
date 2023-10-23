cmake_minimum_required (VERSION 3.26)

project (
	Yaboc
	LANGUAGES C CXX
	VERSION 0.1.0
	DESCRIPTION "Yet Another BreakOut Clone"
)

list (APPEND CMAKE_MODULE_PATH ${Yaboc_SOURCE_DIR}/tools/cmake)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

option (YABOC_ENABLE_SANITZERS "" OFF)

include (StageConfig)
include (Dependencies)
include (CompilerConfig)

add_compile_definitions (-DJSON_HAS_RANGES=0)

add_executable (yaboc)

target_include_directories (
	yaboc

	PRIVATE
	${Yaboc_SOURCE_DIR}/include
)

target_sources (
	yaboc

	PRIVATE
	include/yaboc/graphics/shader.h
	include/yaboc/platform/sdl_gl_window.h
	include/yaboc/platform/sdl_context.h
	include/yaboc/sprite/sprite_renderer.h
	include/yaboc/sprite/sprite_sheet.h

	include/yaboc/ecs/components/all.h
	include/yaboc/ecs/components/tags.h
	include/yaboc/ecs/systems/sprite_render_system.h

	src/yaboc/graphics/shader.cpp
	src/yaboc/platform/sdl_gl_window.cpp
	src/yaboc/platform/sdl_context.cpp
	src/yaboc/sprite/sprite_renderer.cpp
	src/yaboc/sprite/sprite_sheet.cpp
	src/yaboc/ecs/systems/sprite_render_system.cpp
	src/main.cpp
)

target_link_libraries (
	yaboc

	PRIVATE
	Yaboc::CompilerOptions
	SDL3::SDL3
	OpenGL::GL
	glm::glm
	Glad::Glad
	EnTT::EnTT
	STB::Image
	nlohmann_json::nlohmann_json
)

install (TARGETS SDL3-shared)
install (TARGETS yaboc)
install (DIRECTORY ${PROJECT_SOURCE_DIR}/assets TYPE DATA)

include (CPack)
