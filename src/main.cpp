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
#include "yaboc/ecs/components/all.h"
#include "yaboc/ecs/systems/sprite_render_system.h"
#include "yaboc/sprite_renderer.h"
#include "yaboc/sprite_sheet.h"

#include "entt/entt.hpp"
#include "glad/gl.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "nlohmann/json.hpp"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "stb_image.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace
{
constexpr int window_default_width{1'280};
constexpr int window_default_height{720};

constexpr int       opengl_major_version{4};
constexpr int       opengl_minor_version{6};
constexpr glm::vec4 clear_colour{0.157F, 0.157F, 0.157F, 1.0F};

void message_callback(GLenum        source,
                      GLenum        type,
                      GLuint        id,
                      GLenum        severity,
                      GLsizei       length,
                      GLchar const* message,
                      void const*   user_param);

using namespace std::chrono_literals;

constexpr auto dt = std::chrono::duration<long long, std::ratio<1, 60>>{1};
using duration = decltype(std::chrono::steady_clock::duration{} + dt);
using time_point = std::chrono::time_point<std::chrono::steady_clock, duration>;
} // namespace

namespace yaboc
{
stbi_uc* load_sprite_sheet(sprite_sheet_meta const& sprite_sheet_meta_data);
} // namespace yaboc

namespace yaboc
{
auto load_level(entt::registry&    registry,
                std::string const& level_file,
                std::size_t        sprite_id) -> void;
auto create_paddle(entt::registry& registry,
                   std::size_t     sprite_id,
                   glm::vec2       position,
                   glm::vec2       size) -> entt::entity;
auto create_ball(entt::registry& registry,
                 std::size_t     sprite_id,
                 glm::vec2       size,
                 entt::entity    parent) -> entt::entity;

void load_level(entt::registry&    registry,
                std::string const& level_file,
                std::size_t        sprite_id)
{
	std::ifstream level_stream{level_file};
	std::string   line{};

	float     gap = 0.03125F;
	glm::vec2 brick_size{0.75F, 0.25F};

	glm::vec2 start_point{};

	int bricks_per_row{};
	int y{};
	while (std::getline(level_stream, line))
	{
		unsigned int      brick_type{};
		std::stringstream line_stream{line};

		int x{};
		while (line_stream >> brick_type)
		{
			if (brick_type != 0)
			{
				entt::entity brick = registry.create();

				using transform_component = ecs::components::transform;
				using sprite_component = ecs::components::sprite;

				registry.emplace<transform_component>(
				    brick,
				    glm::vec2{start_point.x + (static_cast<float>(x) * gap) +
				                  (static_cast<float>(x) * brick_size.x),
				              start_point.y + static_cast<float>(y) * gap +
				                  (static_cast<float>(y) * brick_size.y)});
				registry.emplace<sprite_component>(brick,
				                                   sprite_id,
				                                   brick_size,
				                                   glm::vec4{1.0F});

				registry.emplace<ecs::tags::brick>(brick);
			}

			++x;
		}
		bricks_per_row = std::max(bricks_per_row, x);

		++y;
	}

	// Ensure the bricks are centered along the X-axis.
	auto brick_row_midpoint = static_cast<float>(bricks_per_row) / 2.0F;
	registry.ctx().emplace<ecs::components::brick_group>(glm::vec2{
	    (gap / 2.0F + 5.0F -
	     (brick_row_midpoint * (brick_size.x) + brick_row_midpoint * gap)) +
	        brick_size.x / 2.0F,
	    0.25F + brick_size.y / 2.0F});
}

auto create_paddle(entt::registry& registry,
                   std::size_t     sprite_id,
                   glm::vec2       position,
                   glm::vec2       size) -> entt::entity
{
	static_cast<void>(sprite_id);
	auto paddle = registry.create();
	registry.emplace<yaboc::ecs::components::transform>(paddle, position);
	registry.emplace<yaboc::ecs::components::sprite>(paddle,
	                                                 sprite_id,
	                                                 size,
	                                                 glm::vec4{1.0F});
	registry.emplace<yaboc::ecs::tags::player>(paddle);
	return paddle;
}

auto create_ball(entt::registry& registry,
                 std::size_t     sprite_id,
                 glm::vec2       size,
                 entt::entity    parent) -> entt::entity
{
	auto ball = registry.create();

	auto [parent_transform, parent_sprite] =
	    registry.get<ecs::components::transform, ecs::components::sprite>(
	        parent);

	auto position =
	    glm::vec2{parent_transform.position.x,
	              parent_transform.position.y - parent_sprite.size.y};

	registry.emplace<ecs::components::transform>(ball, position);
	registry.emplace<ecs::components::sprite>(ball,
	                                          sprite_id,
	                                          size,
	                                          glm::vec4{1.0F});
	registry.emplace<ecs::tags::ball>(ball);
	registry.emplace<ecs::components::relationship>(ball, parent);

	return ball;
}
} // namespace yaboc

struct sdl_context final
{
	sdl_context()
	{
		// NOLINTNEXTLINE(*-signed-bitwise)
		SDL_Init(SDL_INIT_EVERYTHING);
	}

	sdl_context(sdl_context const&) = delete;
	sdl_context(sdl_context&&) = delete;
	auto operator=(sdl_context const&) -> sdl_context& = delete;
	auto operator=(sdl_context&&) -> sdl_context& = delete;

	~sdl_context()
	{
		SDL_GL_DeleteContext(m_gl_context);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}

	SDL_Window*   m_window{};
	SDL_GLContext m_gl_context{};
};

auto main(int argc, char* argv[]) -> int
{
	static_cast<void>(argc);
	static_cast<void>(argv);

	sdl_context sdl{};

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                    SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
	                    SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, opengl_major_version);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, opengl_minor_version);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

	SDL_Window* window = SDL_CreateWindow("Yet Another Breakout Clone",
	                                      window_default_width,
	                                      window_default_height,
	                                      SDL_WINDOW_OPENGL);

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1);

	sdl.m_window = window;
	sdl.m_gl_context = gl_context;

	gladLoadGL(SDL_GL_GetProcAddress);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(message_callback, nullptr);
	glDebugMessageControl(GL_DONT_CARE,
	                      GL_DONT_CARE,
	                      GL_DEBUG_SEVERITY_NOTIFICATION,
	                      0,
	                      nullptr,
	                      GL_FALSE);

	int window_drawable_width = 0;
	int window_drawable_height = 0;
	SDL_GetWindowSizeInPixels(window,
	                          &window_drawable_width,
	                          &window_drawable_height);

	glViewport(0, 0, window_drawable_width, window_drawable_height);

	bool running{true};

	auto sprite_sheet =
	    yaboc::sprite_sheet{"assets/data/sprites/sprite_sheet.json"};

	auto paddle_sprite_id = sprite_sheet.id_from_name("entity/paddleRed");
	auto paddle_sprite_data = sprite_sheet.frame_data(paddle_sprite_id);

	auto const& sprite_sheet_meta_data = sprite_sheet.meta_data();
	auto* sprite_sheet_pixel_data = load_sprite_sheet(sprite_sheet_meta_data);

	GLuint sprite_sheet_texture_id{};
	glCreateTextures(GL_TEXTURE_2D, 1, &sprite_sheet_texture_id);

	glTextureParameteri(sprite_sheet_texture_id,
	                    GL_TEXTURE_WRAP_S,
	                    GL_CLAMP_TO_EDGE);
	glTextureParameteri(sprite_sheet_texture_id,
	                    GL_TEXTURE_WRAP_T,
	                    GL_CLAMP_TO_EDGE);
	glTextureParameteri(sprite_sheet_texture_id,
	                    GL_TEXTURE_MIN_FILTER,
	                    GL_NEAREST);
	glTextureParameteri(sprite_sheet_texture_id,
	                    GL_TEXTURE_MAG_FILTER,
	                    GL_NEAREST);

	glTextureStorage2D(sprite_sheet_texture_id,
	                   1,
	                   GL_RGBA8,
	                   sprite_sheet_meta_data.dimensions.x,
	                   sprite_sheet_meta_data.dimensions.y);

	glTextureSubImage2D(sprite_sheet_texture_id,
	                    0,
	                    0,
	                    0,
	                    sprite_sheet_meta_data.dimensions.x,
	                    sprite_sheet_meta_data.dimensions.y,
	                    GL_RGBA,
	                    GL_UNSIGNED_BYTE,
	                    sprite_sheet_pixel_data);

	glGenerateTextureMipmap(sprite_sheet_texture_id);

	entt::registry registry{};

	auto paddle =
	    yaboc::create_paddle(registry,
	                         sprite_sheet.id_from_name("entity/paddleRed"),
	                         glm::vec2{5.0F, 5.25F},
	                         glm::vec2{1.0F, 0.25F});
	yaboc::create_ball(registry,
	                   sprite_sheet.id_from_name("entity/ballGrey"),
	                   glm::vec2{0.25F, 0.25F},
	                   paddle);

	yaboc::load_level(
	    registry,
	    "assets/data/levels/level_01.txt",
	    sprite_sheet.id_from_name("entity/element_grey_rectangle"));

	auto renderer = std::make_unique<yaboc::sprite_renderer>(
	    yaboc::sprite_renderer::configuration{});

	auto render_system =
	    yaboc::ecs::system::sprite_render_system{std::move(renderer),
	                                             &sprite_sheet};

	// Setup timing
	time_point time_step{};
	time_point current_time = std::chrono::steady_clock::now();
	duration   accumulator{0s};

	auto seconds_now = std::chrono::time_point_cast<std::chrono::seconds>(
	    std::chrono::steady_clock::now());
	int frame_rate{};
	int frame_count{};

	while (running)
	{
		time_point new_time = std::chrono::steady_clock::now();
		auto const frame_time = new_time - current_time;
		current_time = new_time;

		auto const seconds =
		    std::chrono::duration_cast<std::chrono::duration<float>>(
		        frame_time);

		auto prev_seconds = seconds_now;
		seconds_now = std::chrono::time_point_cast<std::chrono::seconds>(
		    std::chrono::steady_clock::now());
		frame_count++;
		if (seconds_now != prev_seconds)
		{
			frame_rate = frame_count;
			frame_count = 0;
		}
		auto const title = std::format(
		    "Yet Another Breakout Clone. Frame time: {:1.5f} FPS {:d}",
		    seconds.count(),
		    frame_rate);
		SDL_SetWindowTitle(window, title.c_str());

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

		accumulator += frame_time;

		while (accumulator >= dt)
		{
			time_step += dt;
			accumulator -= dt;
		}

		glClearBufferfv(GL_COLOR, 0, glm::value_ptr(clear_colour));

		glBindTextureUnit(0, sprite_sheet_texture_id);
		render_system(registry);

		SDL_GL_SwapWindow(window);
	}

	glDeleteTextures(1, &sprite_sheet_texture_id);

	return 0;
}

namespace yaboc
{
auto load_sprite_sheet(sprite_sheet_meta const& sprite_sheet_meta_data)
    -> stbi_uc*
{
	glm::ivec2 dimensions{};
	int        stb_num_channels{};
	auto*      sprite_sheet_pizel_data =
	    stbi_load(sprite_sheet_meta_data.name.c_str(),
	              &dimensions.x,
	              &dimensions.y,
	              &stb_num_channels,
	              0);

	assert(dimensions == sprite_sheet_meta_data.dimensions);
	return sprite_sheet_pizel_data;
}
} // namespace yaboc

namespace
{
// This function's signature is specified by OpenGL and cannot be changed.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void message_callback(
    GLenum        source,
    GLenum        type,
    GLuint        id,
    GLenum        severity,
    GLsizei       length,
    GLchar const* message,
    void const*   user_param) // NOLINTEND(bugprone-easily-swappable-parameters)
{
	static_cast<void>(length);
	static_cast<void>(user_param);
	auto const src_str = [source]() {
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		default: break;
		}
		return "UNKNOWN";
	};

	auto const type_str = [type]() {
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		default: break;
		}
		return "UNKNOWN";
	};

	auto const severity_str = [severity]() {
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		default: break;
		}
		return "UNKNOWN";
	};
	auto formatted_message = std::format("{}, {}, {}, {}: {}\n",
	                                     src_str(),
	                                     type_str(),
	                                     severity_str(),
	                                     id,
	                                     message);
	std::cout << formatted_message;
}
} // namespace
