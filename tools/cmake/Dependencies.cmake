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

FetchContent_MakeAvailable (SDL GLM EnTT)

add_subdirectory (external/glad SYSTEM)
