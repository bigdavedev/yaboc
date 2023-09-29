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
#include "yaboc/ecs/systems/sprite_render_system.h"

#include "yaboc/ecs/components/all.h"
#include "yaboc/sprite/sprite_renderer.h"

#include "entt/entt.hpp"

namespace yaboc::ecs::system
{
sprite_render_system::sprite_render_system(
    std::unique_ptr<sprite::sprite_renderer>&& renderer,
    sprite::sprite_sheet*                      sheet)
    : m_renderer{std::move(renderer)}
    , m_sprite_sheet{sheet}
{}

void sprite_render_system::operator()(entt::registry& registry) const
{
	m_renderer->begin_batch();

	auto scale_uv = [sheet_size = m_sprite_sheet->meta_data().dimensions](
	                    sprite::sprite_frame_data::subtexture_bounds bounds) {
		float sheet_width{static_cast<float>(sheet_size.x)};
		float sheet_height{static_cast<float>(sheet_size.y)};

		auto min = glm::vec2{static_cast<float>(bounds.min.x) / sheet_width,
		                     static_cast<float>(bounds.min.y) / sheet_height};

		auto max = glm::vec2{static_cast<float>(bounds.max.x) / sheet_width,
		                     static_cast<float>(bounds.max.y) / sheet_height};

		return sprite::sprite_renderer::subtexture_bounds{min, max};
	};

	auto render_components = [&registry](entt::entity entity) {
		return registry.get<components::transform, components::sprite>(entity);
	};

	for (auto entity: registry.view<tags::brick>())
	{
		auto [transform, sprite] = render_components(entity);

		auto offset = registry.ctx().get<components::brick_group>().offset;
		auto position = transform.position + offset;

		auto frame = m_sprite_sheet->frame_data(sprite.id);

		m_renderer->submit_sprite(position,
		                          sprite.size,
		                          sprite.tint,
		                          scale_uv(frame.bounds));
	}

	for (auto entity: registry.view<tags::player>())
	{
		auto [transform, sprite] = render_components(entity);

		auto frame = m_sprite_sheet->frame_data(sprite.id);

		m_renderer->submit_sprite(transform.position,
		                          sprite.size,
		                          sprite.tint,
		                          scale_uv(frame.bounds));
	}

	for (auto entity: registry.view<tags::ball>())
	{
		auto [transform, sprite] = render_components(entity);

		auto frame = m_sprite_sheet->frame_data(sprite.id);

		m_renderer->submit_sprite(transform.position,
		                          sprite.size,
		                          sprite.tint,
		                          scale_uv(frame.bounds));
	}

	m_renderer->end_batch();
}
} // namespace yaboc::ecs::system
