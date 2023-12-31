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
#ifndef YABOC_INCLUDE_YABOC_SPRITE_SPRITE_RENDERER_H
#define YABOC_INCLUDE_YABOC_SPRITE_SPRITE_RENDERER_H

#include "yaboc/sprite/sprite_sheet.h"

#include "glad/gl.h"
#include "glm/glm.hpp"

#include <array>
#include <span>

namespace yaboc::sprite
{
class sprite_renderer final
{
	unsigned int m_shader{};

	unsigned int m_vao{};
	unsigned int m_vbo{};

	struct vertex final
	{
		glm::vec2 pos{};
		glm::vec4 tint{1.0F};
		glm::vec2 uv{};
	};

	static_assert(sizeof(vertex) ==
	              (sizeof(glm::vec2) + sizeof(glm::vec4) + sizeof(glm::vec2)));

	static constexpr std::size_t num_buffers{3};

	struct vertex_buffer_region final
	{
		std::span<vertex> region{};

		GLsync fence{nullptr};
	};

	std::array<vertex_buffer_region, num_buffers> m_vertex_buffer_regions{};

	std::size_t m_current_vertex_buffer_region{};

	unsigned int m_current_sprite_count{};
	std::size_t  m_sprites_per_batch{};

	static constexpr std::size_t default_sprites_per_batch{1'000};

	int m_pixels_per_metre{};

public:
	struct configuration final
	{
		glm::vec2 reference_resolution{640, 360};
		int pixels_per_metre{static_cast<int>(reference_resolution.x / 10)};
		std::size_t sprites_per_batch{default_sprites_per_batch};
	};

	struct subtexture_bounds final
	{
		glm::vec2 min{};
		glm::vec2 max{};
	};

	~sprite_renderer();

	explicit sprite_renderer(configuration&& config);

	sprite_renderer(sprite_renderer const&) = delete;
	sprite_renderer(sprite_renderer&&) = default;

	auto operator=(sprite_renderer const&) -> sprite_renderer& = delete;
	auto operator=(sprite_renderer&&) -> sprite_renderer& = default;

	void begin_batch();

	void use_sprite_sheet(sprite_sheet const& sheet);

	void end_batch();

	auto submit_sprite(glm::vec2         position,
	                   glm::vec2         size,
	                   glm::vec4         tint,
	                   subtexture_bounds uv_bounds) -> void;

	void flush();
};
} // namespace yaboc::sprite

#endif // YABOC_INCLUDE_YABOC_SPRITE_SPRITE_RENDERER_H
