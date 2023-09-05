include (FetchContent)

FetchContent_Declare (
	SDL
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG fd7cd91dc9c6c5c3882b69793642b7915184535b
	SYSTEM
)
FetchContent_MakeAvailable (SDL)