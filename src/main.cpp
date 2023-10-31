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
#include "yaboc/platform/sdl_context.h"
#include "yaboc/platform/sdl_gl_window.h"
#include "yaboc/sprite/sprite_renderer.h"
#include "yaboc/sprite/sprite_sheet.h"

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
#include <numeric>
#include <string>

namespace
{
constexpr int window_default_width{1'280};
constexpr int window_default_height{720};

constexpr int       opengl_major_version{4};
constexpr int       opengl_minor_version{6};
constexpr glm::vec4 clear_colour{0.157F, 0.157F, 0.157F, 1.0F};

using namespace std::chrono_literals;

constexpr auto dt = std::chrono::duration<std::int64_t, std::ratio<1, 60>>{1};
constexpr auto dt_f =
    std::chrono::duration_cast<std::chrono::duration<float>>(dt);
using duration = decltype(std::chrono::steady_clock::duration{} + dt);
using time_point = std::chrono::time_point<std::chrono::steady_clock, duration>;
} // namespace

namespace yaboc
{
auto load_sprite_sheet(sprite::sprite_sheet_meta const& sprite_sheet_meta_data)
    -> GLuint;
} // namespace yaboc

namespace yaboc
{
auto load_level(entt::registry&    registry,
                std::string const& level_file,
                std::size_t        sprite_id) -> void;
auto create_sprite_entity(entt::registry& registry,
                          std::size_t     sprite_id,
                          glm::vec2       position,
                          glm::vec2       size) -> entt::entity;

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

namespace ecs::components
{
struct direction final
{
	float horizontal{};
	float vertical{};
};

struct velocity final
{
	float x{};
	float y{};
};
} // namespace ecs::components

namespace ecs::system
{
class move_entity_system final
{
	entt::registry* m_registry{};

public:
	explicit move_entity_system(entt::registry& registry)
	    : m_registry{&registry}
	{}

	void operator()(entt::entity                entity,
	                components::velocity const  velocity,
	                components::direction const direction)
	{
		auto& transform = m_registry->get<components::transform>(entity);
		transform.position.x +=
		    direction.horizontal * velocity.x * dt_f.count();
		transform.position.y += direction.vertical * velocity.y * dt_f.count();
	}
};
} // namespace ecs::system
} // namespace yaboc

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) -> int
{
	yaboc::platform::sdl_context sdl{opengl_major_version,
	                                 opengl_minor_version};

	yaboc::platform::sdl_gl_window window{window_default_width,
	                                      window_default_height,
	                                      "Yet Another Breakout Clone"};

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	bool running{true};

	auto sprite_sheet =
	    yaboc::sprite::sprite_sheet{"assets/data/sprites/sprite_sheet.json"};

	auto sprite_sheet_texture_id =
	    yaboc::load_sprite_sheet(sprite_sheet.meta_data());

	sprite_sheet.renderer_id(sprite_sheet_texture_id);

	entt::registry registry{};

	auto paddle = registry.create();
	registry.emplace<yaboc::ecs::components::transform>(paddle,
	                                                    glm::vec2{5.0F, 5.25F});
	registry.emplace<yaboc::ecs::components::sprite>(
	    paddle,
	    sprite_sheet.id_from_name("entity/paddleRed"),
	    glm::vec2{1.0F, 0.25F},
	    glm::vec4{1.0F});
	registry.emplace<yaboc::ecs::components::velocity>(paddle, 10.0F, 0.0F);
	registry.emplace<yaboc::ecs::components::direction>(paddle);
	registry.emplace<yaboc::ecs::tags::player>(paddle);

	auto ball = registry.create();
	registry.emplace<yaboc::ecs::components::transform>(ball,
	                                                    glm::vec2{5.0F, 5.0F});
	registry.emplace<yaboc::ecs::components::sprite>(
	    ball,
	    sprite_sheet.id_from_name("entity/ballGrey"),
	    glm::vec2{0.25F, 0.25F},
	    glm::vec4{1.0F});
	registry.emplace<yaboc::ecs::components::velocity>(ball, 0.2F, 0.0F);
	registry.emplace<yaboc::ecs::components::direction>(ball, -1.0f, 0.0F);
	registry.emplace<yaboc::ecs::tags::ball>(ball);

	yaboc::load_level(
	    registry,
	    "assets/data/levels/level_01.txt",
	    sprite_sheet.id_from_name("entity/element_grey_rectangle"));

	auto renderer = std::make_unique<yaboc::sprite::sprite_renderer>(
	    yaboc::sprite::sprite_renderer::configuration{});

	auto render_system =
	    yaboc::ecs::system::sprite_render_system{std::move(renderer),
	                                             &sprite_sheet};

	std::span sdl_key_states = [] {
		int         num_keys{};
		auto const* key_states = SDL_GetKeyboardState(&num_keys);
		return std::span{key_states, static_cast<std::size_t>(num_keys)};
	}();

	// Setup timing
	time_point time_step{};
	time_point current_time = std::chrono::steady_clock::now();
	duration   accumulator{0s};

	while (running)
	{
		time_point new_time = std::chrono::steady_clock::now();
		auto const frame_time =
		    std::min(new_time - current_time, duration{250ms});
		current_time = new_time;

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

		registry.get<yaboc::ecs::components::direction>(paddle).horizontal =
		    [sdl_key_states] {
			    if (sdl_key_states[SDL_SCANCODE_LEFT] != 0U) return -1.0F;
			    if (sdl_key_states[SDL_SCANCODE_RIGHT] != 0U) return 1.0F;
			    return 0.0F;
		    }();

		accumulator += frame_time;

		while (accumulator >= dt)
		{
			time_step += dt;
			accumulator -= dt;

			registry
			    .view<yaboc::ecs::components::velocity,
			          yaboc::ecs::components::direction>()
			    .each(yaboc::ecs::system::move_entity_system{registry});
		}

		glClearBufferfv(GL_COLOR, 0, glm::value_ptr(clear_colour));

		render_system(registry);

		window.swap_buffers();
	}

	glDeleteTextures(1, &sprite_sheet_texture_id);

	return 0;
}

namespace yaboc
{
auto load_sprite_sheet(sprite::sprite_sheet_meta const& sprite_sheet_meta_data)
    -> GLuint
{
	glm::ivec2 dimensions{};
	int        stb_num_channels{};
	auto*      sprite_sheet_pixel_data =
	    stbi_load(sprite_sheet_meta_data.name.c_str(),
	              &dimensions.x,
	              &dimensions.y,
	              &stb_num_channels,
	              0);

	assert(dimensions == sprite_sheet_meta_data.dimensions);

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
	                    GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(sprite_sheet_texture_id,
	                    GL_TEXTURE_MAG_FILTER,
	                    GL_LINEAR);

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

	stbi_image_free(sprite_sheet_pixel_data);

	return sprite_sheet_texture_id;
}
} // namespace yaboc
