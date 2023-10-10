#include "yaboc/platform/sdl_gl_window.h"

#include "glad/gl.h"

namespace yaboc::platform
{
sdl_gl_window::~sdl_gl_window()
{
	SDL_GL_DeleteContext(m_gl_context);
	SDL_DestroyWindow(m_window_handle);
}

sdl_gl_window::sdl_gl_window(int width, int height, std::string const& title)
    : m_window_handle{SDL_CreateWindow(title.c_str(),
                                       width,
                                       height,
                                       SDL_WINDOW_OPENGL)}
    , m_gl_context{SDL_GL_CreateContext(m_window_handle)}
{
	SDL_GL_MakeCurrent(m_window_handle, m_gl_context);
	gladLoadGL(SDL_GL_GetProcAddress);

	auto const window_drawable_area = drawable_area();
	glViewport(0, 0, window_drawable_area.x, window_drawable_area.y);
}

[[nodiscard]]
auto sdl_gl_window::drawable_area() const noexcept -> glm::ivec2
{
	glm::ivec2 dimensions{};
	SDL_GetWindowSizeInPixels(m_window_handle, &dimensions.x, &dimensions.y);
	return dimensions;
}
} // namespace yaboc::platform
