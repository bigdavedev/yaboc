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
#ifndef YABOC_ECS_SYSTEMS_SPRITE_RENDER_SYSTEM_H
#define YABOC_ECS_SYSTEMS_SPRITE_RENDER_SYSTEM_H

#include "entt/fwd.hpp"
#include "glad/gl.h"

#include <memory>

namespace yaboc
{
class sprite_renderer;
} // namespace yaboc

namespace yaboc::ecs::system
{
class sprite_render_system final
{
	std::unique_ptr<sprite_renderer> m_renderer{};

public:
	explicit sprite_render_system(std::unique_ptr<sprite_renderer>&& renderer);

	static auto components(entt::registry& registry, entt::entity entity);

	void operator()(entt::registry& registry) const;
};
} // namespace yaboc::ecs::system

#endif // YABOC_ECS_SYSTEMS_SPRITE_RENDER_SYSTEM_H
