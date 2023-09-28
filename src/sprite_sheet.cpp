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
#include "yaboc/sprite_sheet.h"

#include <cstddef>
#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"

namespace yaboc
{

	NLOHMANN_JSON_SERIALIZE_ENUM(image_format,
	                             {
	                                 {image_format::rgba_8888, "RGBA8888"}
    })

	void from_json(nlohmann::json const& json, sprite_sheet_meta& meta);
	void from_json(nlohmann::json const& json, sprite_frame_data& sprite);
	void from_json(nlohmann::json const&              json,
	               sprite_frame_data::subtexture_bounds& bounds);
} // namespace yaboc

namespace nlohmann
{
template <int Dimensions, class T>
struct adl_serializer<glm::vec<Dimensions, T>>
{
	static void from_json(json const& j, glm::vec<Dimensions, T>& vec)
	{
		if constexpr (Dimensions >= 1)
		{
			vec.x = j.contains("w") ? j.at("w").get<T>() : j.at("x").get<T>();
		}

		if constexpr (Dimensions >= 2)
		{
			vec.y = j.contains("h") ? j.at("h").get<T>() : j.at("y").get<T>();
		}

		if constexpr (Dimensions >= 3)
		{
			j.at("z").get_to<T>(vec.z);
		}
	}
};
} // namespace nlohmann

namespace yaboc
{
sprite_sheet::sprite_sheet(std::string&& specification_path)
{
	auto sprite_sheet_info =
	    nlohmann::json::parse(std::ifstream{specification_path});

	sprite_sheet_info["meta"].get_to<sprite_sheet_meta>(m_meta_data);

	std::size_t current_id{};
	for (auto&& frame: sprite_sheet_info["frames"])
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
} // namespace yaboc

namespace yaboc
{
	void from_json(nlohmann::json const& json, sprite_frame_data& sprite)
	{
		json.at("filename").get_to<std::string>(sprite.name);
		json.at("frame").get_to<sprite_frame_data::subtexture_bounds>(
		    sprite.bounds);
		json.at("sourceSize").get_to<glm::ivec2>(sprite.size);
	}

	void from_json(nlohmann::json const&              json,
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
} // namespace yaboc
