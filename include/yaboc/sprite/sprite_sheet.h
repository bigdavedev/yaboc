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
#ifndef YABOC_INCLUDE_YABOC_SPRITE_SPRITE_SHEET_H
#define YABOC_INCLUDE_YABOC_SPRITE_SPRITE_SHEET_H

#include "glm/glm.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace yaboc::sprite
{
enum class image_format : int
{
	rgba_8888
};

struct sprite_sheet_meta final
{
	std::string  name{};
	glm::ivec2   dimensions{};
	image_format format{};
};

struct sprite_frame_data final
{
	struct subtexture_bounds final
	{
		glm::ivec2 min{};
		glm::ivec2 max{};
	};

	std::string       name{};
	subtexture_bounds bounds{};
	glm::ivec2        size{};
};

class sprite_sheet final
{
	std::unordered_map<std::string, std::size_t> m_id_lookup{};

	std::vector<sprite_frame_data> m_sprite_frame_data{};

	sprite_sheet_meta m_meta_data{};

	unsigned int m_renderer_id{};

public:
	explicit sprite_sheet(std::string&& specification_path);

	auto meta_data() const -> sprite_sheet_meta const&
	{
		return m_meta_data;
	}

	auto id_from_name(std::string const& name) const -> std::size_t;

	auto frame_data(std::size_t sprite_id) const -> sprite_frame_data
	{
		assert(sprite_id < std::size(m_sprite_frame_data));
		return m_sprite_frame_data[sprite_id];
	}

	void renderer_id(unsigned int id)
	{
		m_renderer_id = id;
	}

	auto renderer_id() const -> unsigned int
	{
		return m_renderer_id;
	}
};
} // namespace yaboc::sprite

#endif // YABOC_INCLUDE_YABOC_SPRITE_SPRITE_SHEET_H
