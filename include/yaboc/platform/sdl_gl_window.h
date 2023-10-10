// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2023 David Brown <d.brown@bigdavedev.com>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program. If not, see <https://www.gnu.org/licenses/>.
#ifndef YABOC_INCLUDE_YABOC_PLATFORM_SDL_GL_WINDOW_H
#define YABOC_INCLUDE_YABOC_PLATFORM_SDL_GL_WINDOW_H

#include "glm/glm.hpp"
#include "SDL3/SDL_video.h"

#include <string>

namespace yaboc::platform
{
class sdl_gl_window final
{
	SDL_Window*   m_window_handle{};
	SDL_GLContext m_gl_context{};

public:
	~sdl_gl_window();

	sdl_gl_window(int width, int height, std::string const& title);

	sdl_gl_window(sdl_gl_window const&) = delete;
	auto operator=(sdl_gl_window const&) -> sdl_gl_window& = delete;

	sdl_gl_window(sdl_gl_window&&) = default;
	auto operator=(sdl_gl_window&&) -> sdl_gl_window& = default;

	[[nodiscard]]
	auto drawable_area() const noexcept -> glm::ivec2;

	void title(std::string const& new_title)
	{
		SDL_SetWindowTitle(m_window_handle, new_title.c_str());
	}

	void swap_buffers()
	{
		SDL_GL_SwapWindow(m_window_handle);
	}
};
} // namespace yaboc::platform

#endif // YABOC_INCLUDE_YABOC_PLATFORM_SDL_GL_WINDOW_H
