project (Glad)

find_package (OpenGL REQUIRED)

add_library (glad)
add_library (Glad::Glad ALIAS glad)

target_include_directories (
	glad

	PUBLIC
	$<BUILD_INTERFACE:${Glad_SOURCE_DIR}/include>
)

target_sources (
	glad

	PRIVATE
	${Glad_SOURCE_DIR}/include/glad/gl.h
	${Glad_SOURCE_DIR}/include/KHR/khrplatform.h

	${Glad_SOURCE_DIR}/src/gl.c
)

set_target_properties (
	glad

	PROPERTIES
	SYSTEM TRUE
)

target_link_libraries (
	glad

	PUBLIC
	OpenGL::GL
)
