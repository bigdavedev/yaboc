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
#include "yaboc/sprite_renderer.h"

#include "entt/entt.hpp"

namespace yaboc::ecs::system
{
sprite_render_system::sprite_render_system(GLuint shader_id)
    : m_renderer{std::make_unique<sprite_renderer>(shader_id, 500)}
{}

void sprite_render_system::operator()(entt::registry& registry) const
{
	m_renderer->begin_batch();

	auto render_components = [&registry](entt::entity entity) {
		return registry.get<components::transform, components::sprite>(entity);
	};

	for (auto entity: registry.view<tags::brick>())
	{
		auto [transform, sprite] = render_components(entity);

		auto position = transform.position +
		                registry.ctx().get<components::brick_group>().offset;

		m_renderer->submit_sprite(position, sprite.size);
	}

	for (auto entity: registry.view<tags::player>())
	{
		auto [transform, sprite] = render_components(entity);

		m_renderer->submit_sprite(transform.position, sprite.size);
	}

	for (auto entity: registry.view<tags::ball>())
	{
		auto [transform, sprite] = render_components(entity);

		m_renderer->submit_sprite(transform.position, sprite.size);
	}

	m_renderer->end_batch();
}
} // namespace yaboc::ecs::system
