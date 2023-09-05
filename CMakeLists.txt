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

add_executable (yaboc main.cpp)

target_link_libraries (
	yaboc

	PRIVATE
	SDL3::SDL3
)