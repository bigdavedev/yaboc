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
#include "yaboc/sprite/sprite_sheet.h"

#include "nlohmann/json.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>

namespace yaboc::sprite
{
// clang-format off
NLOHMANN_JSON_SERIALIZE_ENUM(image_format,
{
	{image_format::rgba_8888, "RGBA8888"}
})
// clang-format on

void from_json(nlohmann::json const& json, sprite_sheet_meta& meta);
void from_json(nlohmann::json const& json, sprite_frame_data& sprite);
void from_json(nlohmann::json const&                 json,
               sprite_frame_data::subtexture_bounds& bounds);
} // namespace yaboc::sprite

namespace nlohmann
{
template <int Dimensions, class T>
struct adl_serializer<glm::vec<Dimensions, T>>
{
	static void from_json(json const& json_object, glm::vec<Dimensions, T>& vec)
	{
		if constexpr (Dimensions >= 1)
		{
			vec.x = json_object.contains("w") ? json_object.at("w").get<T>()
			                                  : json_object.at("x").get<T>();
		}

		if constexpr (Dimensions >= 2)
		{
			vec.y = json_object.contains("h") ? json_object.at("h").get<T>()
			                                  : json_object.at("y").get<T>();
		}

		// 3 is a well-understood number in the context of vector dimensions
		// NOLINTNEXTLINE(*-magic-numbers)
		if constexpr (Dimensions >= 3)
		{
			json_object.at("z").get_to<T>(vec.z);
		}
	}
};
} // namespace nlohmann

namespace yaboc::sprite
{
sprite_sheet::sprite_sheet(std::string&& specification_path)
{
	auto sprite_sheet_info =
	    nlohmann::json::parse(std::ifstream{specification_path});

	sprite_sheet_info["meta"].get_to<sprite_sheet_meta>(m_meta_data);

	for (std::size_t current_id{}; auto&& frame: sprite_sheet_info["frames"])
	{
		auto data = frame.get<sprite_frame_data>();
		m_id_lookup.emplace(std::piecewise_construct,
		                    std::forward_as_tuple(data.name),
		                    std::forward_as_tuple(current_id));
		m_sprite_frame_data.push_back(std::move(data));
		++current_id;
	}
}

auto sprite_sheet::id_from_name(std::string const& name) const -> std::size_t
{
	auto iterator = m_id_lookup.find(name);
	assert(iterator != std::end(m_id_lookup));
	return iterator->second;
}

void from_json(nlohmann::json const& json, sprite_frame_data& sprite)
{
	json.at("filename").get_to<std::string>(sprite.name);
	json.at("frame").get_to<sprite_frame_data::subtexture_bounds>(
	    sprite.bounds);
	json.at("sourceSize").get_to<glm::ivec2>(sprite.size);
}

void from_json(nlohmann::json const&                 json,
               sprite_frame_data::subtexture_bounds& bounds)
{
	bounds.min.x = json.at("x").get<int>();
	bounds.min.y = json.at("y").get<int>();
	bounds.max.x = bounds.min.x + json.at("w").get<int>();
	bounds.max.y = bounds.min.y + json.at("h").get<int>();
}

void from_json(nlohmann::json const& json, sprite_sheet_meta& meta)
{
	meta.name = std::filesystem::path{"assets/data/sprites/"} /
	            json.at("image").get<std::string>();
	json.at("size").get_to<glm::ivec2>(meta.dimensions);
	json.at("format").get_to<image_format>(meta.format);
}
} // namespace yaboc::sprite
