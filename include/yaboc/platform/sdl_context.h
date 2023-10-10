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
#ifndef YABOC_INCLUDE_YABOC_PLATFORM_SDL_CONTEXT_H
#define YABOC_INCLUDE_YABOC_PLATFORM_SDL_CONTEXT_H

namespace yaboc::platform
{
struct sdl_context final
{
	sdl_context(int gl_major, int gl_minor);

	sdl_context(sdl_context const&) = delete;
	sdl_context(sdl_context&&) = delete;
	auto operator=(sdl_context const&) -> sdl_context& = delete;
	auto operator=(sdl_context&&) -> sdl_context& = delete;

	~sdl_context();
};
} // namespace yaboc::platform

#endif // YABOC_INCLUDE_YABOC_PLATFORM_SDL_CONTEXT_H
