include (FetchContent)

find_package (OpenGL REQUIRED)

FetchContent_Declare (
	SDL
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG fd7cd91dc9c6c5c3882b69793642b7915184535b
	SYSTEM
)

FetchContent_Declare (
	GLM
	GIT_REPOSITORY https://github.com/g-truc/glm
	GIT_TAG 47585fde0c49fa77a2bf2fb1d2ead06999fd4b6e
	SYSTEM
)

FetchContent_Declare (
	EnTT
	GIT_REPOSITORY https://github.com/skypjack/entt
	GIT_TAG 344e03ac64a1f78424ab1150e2d4778e8df8431d
	SYSTEM
)

FetchContent_Declare (
	json
	GIT_REPOSITORY https://github.com/nlohmann/json
	GIT_TAG bc889afb4c5bf1c0d8ee29ef35eaaf4c8bef8a5d
	SYSTEM
)

FetchContent_Declare (
	STB
	GIT_REPOSITORY https://github.com/nothings/stb
	GIT_TAG 5736b15f7ea0ffb08dd38af21067c314d6a3aae9
	SYSTEM
)

FetchContent_GetProperties (STB)
if (NOT STB_POPULATED)
	FetchContent_Populate (STB)
endif ()

FetchContent_GetProperties (STB SOURCE_DIR STB_SOURCE_DIR)
FetchContent_GetProperties (STB BINARY_DIR STB_BINARY_DIR)

add_library (stb_image)
add_library (STB::Image ALIAS stb_image)

target_sources (
	stb_image

	PRIVATE
	${STB_SOURCE_DIR}/stb_image.h
	${STB_BINARY_DIR}/stb_image.c
)

target_include_directories (
	stb_image

	PUBLIC
	$<BUILD_INTERFACE:${STB_SOURCE_DIR}>
)

file (
	WRITE ${STB_BINARY_DIR}/stb_image.c
	"#define STB_IMAGE_IMPLEMENTATION\n\n"
	"#include \"stb_image.h\""
)

set_target_properties (
	stb_image

	PROPERTIES
	SYSTEM TRUE
)

FetchContent_MakeAvailable (SDL GLM EnTT json STB)

add_subdirectory (external/glad SYSTEM)
