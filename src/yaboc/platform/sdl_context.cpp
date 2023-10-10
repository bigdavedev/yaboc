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
#include "yaboc/platform/sdl_context.h"

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"

namespace yaboc::platform
{
	sdl_context::sdl_context(int gl_major, int gl_minor)
	{
		// NOLINTNEXTLINE(*-signed-bitwise)
		SDL_Init(SDL_INIT_EVERYTHING);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
		                    SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
		                    SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
	}

	sdl_context::~sdl_context()
	{
		SDL_Quit();
	}
}

