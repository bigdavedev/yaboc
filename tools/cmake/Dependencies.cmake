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

FetchContent_MakeAvailable (SDL GLM)

add_subdirectory (external/glad SYSTEM)
