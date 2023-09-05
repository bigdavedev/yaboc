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
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

namespace
{
constexpr int window_default_width{800};
constexpr int window_default_height{600};
} // namespace

auto main(int argc, char* argv[]) -> int
{
	static_cast<void>(argc);
	static_cast<void>(argv);

	// NOLINTNEXTLINE(*-signed-bitwise)
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("Yet Another Breakout Clone",
	                                      window_default_width,
	                                      window_default_height,
	                                      SDL_WindowFlags{});

	SDL_Renderer* renderer = SDL_CreateRenderer(window,
	                                            nullptr,
	                                            SDL_RENDERER_ACCELERATED |
	                                                SDL_RENDERER_PRESENTVSYNC);

	bool running{true};
	while (running)
	{
		SDL_Event sdl_event{};
		while (SDL_PollEvent(&sdl_event) != 0)
		{
			switch (sdl_event.type)
			{
			case SDL_EVENT_QUIT: running = false; break;
			case SDL_EVENT_KEY_DOWN:
			{
				if (sdl_event.key.keysym.sym == SDLK_ESCAPE)
				{
					running = false;
				}
				break;
			}
			}
		}

		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
