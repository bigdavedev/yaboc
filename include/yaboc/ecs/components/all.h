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
#ifndef YASIC_ECS_COMPONENTS_ALL_H
#define YASIC_ECS_COMPONENTS_ALL_H

#include "yaboc/ecs/components/tags.h"

#include "entt/entt.hpp"
#include "glm/glm.hpp"

#include <cstddef>

namespace yaboc::ecs::components
{
struct transform final
{
	glm::vec2 position;
};

struct relationship final
{
	entt::entity parent;
};

struct sprite final
{
	std::size_t id;
	glm::vec2   size;
	glm::vec4   tint;
};

enum class brick_type
{
	unbreakable,
	normal
};

struct brick_group final
{
	glm::vec2 offset{};
};
} // namespace yaboc::ecs::components

#endif // YASIC_ECS_COMPONENTS_ALL_H
