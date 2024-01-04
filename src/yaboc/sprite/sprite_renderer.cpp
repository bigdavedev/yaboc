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
#include "yaboc/sprite/sprite_renderer.h"

#include "yaboc/graphics/shader.h"

#include "glad/gl.h"
#include "glm/gtc/type_ptr.hpp"

#include <chrono>
#include <limits>
#include <span>
#include <cassert>

static_assert(sizeof(glm::vec2) == (sizeof(float) * 2));

namespace yaboc::sprite
{
namespace
{
constexpr auto mapping_flags =
    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
constexpr auto storage_flags = mapping_flags | GL_DYNAMIC_STORAGE_BIT;

auto create_empty_buffer(std::size_t size)
{
	assert(size < std::numeric_limits<GLsizeiptr>::max());
	unsigned int id{};
	glCreateBuffers(1, &id);
	glNamedBufferStorage(id,
	                     static_cast<GLsizeiptr>(size),
	                     nullptr,
	                     storage_flags);
	return id;
}

template <class T>
auto map_as(unsigned int id, std::size_t size)
{
	assert(size < std::numeric_limits<GLsizeiptr>::max());
	void* buf = glMapNamedBufferRange(id,
	                                  0,
	                                  static_cast<GLsizeiptr>(size),
	                                  mapping_flags);
	return std::span<T>(static_cast<T*>(buf), size / sizeof(T));
}

constexpr auto verts_per_quad = 6;
} // namespace

sprite_renderer::~sprite_renderer()
{
	glUnmapNamedBuffer(m_vbo);

	glDeleteProgram(m_shader);
	glDeleteBuffers(1, &m_vbo);
	glDeleteVertexArrays(1, &m_vao);
}

sprite_renderer::sprite_renderer(sprite_renderer::configuration&& config)
    : m_sprites_per_batch{config.sprites_per_batch}
    , m_pixels_per_metre{config.pixels_per_metre}
{
	auto const verts_per_batch = verts_per_quad * m_sprites_per_batch;
	auto const vbo_size = verts_per_batch * sizeof(vertex);

	m_vbo = create_empty_buffer(vbo_size * num_buffers);
	auto vbo_span = map_as<vertex>(m_vbo, vbo_size * num_buffers);

	m_vertex_buffer_regions[0].region = vbo_span.first(verts_per_batch);
	m_vertex_buffer_regions[1].region =
	    vbo_span.subspan(verts_per_batch, verts_per_batch);
	m_vertex_buffer_regions[2].region = vbo_span.last(verts_per_batch);

	glCreateVertexArrays(1, &m_vao);
	glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(vertex));

	int const pos_attr_loc{0};
	glEnableVertexArrayAttrib(m_vao, pos_attr_loc);
	glVertexArrayAttribFormat(m_vao,
	                          pos_attr_loc,
	                          decltype(vertex::pos)::length(),
	                          GL_FLOAT,
	                          GL_FALSE,
	                          0);
	glVertexArrayAttribBinding(m_vao, pos_attr_loc, 0);

	int const tint_attr_loc{1};
	glEnableVertexArrayAttrib(m_vao, tint_attr_loc);
	glVertexArrayAttribFormat(m_vao,
	                          tint_attr_loc,
	                          decltype(vertex::tint)::length(),
	                          GL_FLOAT,
	                          GL_FALSE,
	                          offsetof(vertex, tint));
	glVertexArrayAttribBinding(m_vao, tint_attr_loc, 0);

	int const uv_attr_loc{2};
	glEnableVertexArrayAttrib(m_vao, uv_attr_loc);
	glVertexArrayAttribFormat(m_vao,
	                          uv_attr_loc,
	                          decltype(vertex::uv)::length(),
	                          GL_FLOAT,
	                          GL_FALSE,
	                          offsetof(vertex, uv));
	glVertexArrayAttribBinding(m_vao, uv_attr_loc, 0);

	m_shader = yaboc::make_shader(std::vector<yaboc::shader_builder_input>{
	    {.type = yaboc::shader_builder_input::shader_type::vertex,
	     .path = "assets/shaders/sprite.vert.glsl"},
	    {.type = yaboc::shader_builder_input::shader_type::fragment,
	     .path = "assets/shaders/sprite.frag.glsl"}
    });

	auto projection = glm::ortho(0.0F,
	                             config.reference_resolution.x,
	                             config.reference_resolution.y,
	                             0.0F,
	                             -1.0F,
	                             1.0F);

	glUseProgram(m_shader);
	auto const proj_loc = glGetUniformLocation(m_shader, "projection");
	glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(projection));
}

void sprite_renderer::begin_batch()
{
	m_current_sprite_count = 0;

	glUseProgram(m_shader);
	glBindVertexArray(m_vao);
}

void sprite_renderer::use_sprite_sheet(sprite_sheet const& sheet)
{
	glBindTextureUnit(0, sheet.renderer_id());
}

void sprite_renderer::end_batch()
{
	if (m_current_sprite_count > 0)
	{
		flush();
	}
	glBindVertexArray(0);
	glUseProgram(0);
}

auto sprite_renderer::submit_sprite(glm::vec2         position,
                                    glm::vec2         size,
                                    glm::vec4         tint,
                                    subtexture_bounds uv_bounds) -> void
{
	if (m_current_sprite_count == m_sprites_per_batch)
	{
		flush();
		auto* current_fence =
		    m_vertex_buffer_regions[m_current_vertex_buffer_region].fence;

		if (current_fence != nullptr)
		{
			while (true)
			{
				GLenum sync_result{
				    glClientWaitSync(current_fence,
				                     GL_SYNC_FLUSH_COMMANDS_BIT,
				                     std::chrono::nanoseconds::max().count())};

				if (sync_result == GL_ALREADY_SIGNALED ||
				    sync_result == GL_CONDITION_SATISFIED)
				{
					glDeleteSync(current_fence);
					m_vertex_buffer_regions[m_current_vertex_buffer_region]
					    .fence = nullptr;
					break;
				}
			}
		}
	}

	auto const current_vertex_count = m_current_sprite_count * verts_per_quad;

	auto current_vbo_span =
	    m_vertex_buffer_regions[m_current_vertex_buffer_region].region;
	auto const vertices =
	    current_vbo_span.subspan(current_vertex_count, verts_per_quad);

	position *= m_pixels_per_metre;
	size *= m_pixels_per_metre;
	size /= 2.0F;

	glm::vec2 min_pos{position - size};
	glm::vec2 max_pos{position + size};

	auto const top_left =
	    vertex{.pos = min_pos, .tint = tint, .uv = uv_bounds.min};
	auto const top_right = vertex{
	    .pos = glm::vec2{max_pos.x,       min_pos.y      },
	    .tint = tint,
	    .uv = {uv_bounds.max.x, uv_bounds.min.y}
    };
	auto const bottom_left = vertex{
	    .pos = glm::vec2{min_pos.x,       max_pos.y      },
	    .tint = tint,
	    .uv = {uv_bounds.min.x, uv_bounds.max.y}
    };
	auto const bottom_right =
	    vertex{.pos = max_pos, .tint = tint, .uv = uv_bounds.max};

	vertices[0] = bottom_left;
	vertices[1] = top_right;
	vertices[2] = top_left;
	vertices[3] = bottom_left;
	vertices[4] = bottom_right;
	vertices[5] = top_right;

	m_current_sprite_count++;
}

void sprite_renderer::flush()
{
	auto const first_quad =
	    m_sprites_per_batch * verts_per_quad * m_current_vertex_buffer_region;
	auto const count = m_current_sprite_count * verts_per_quad;

	glDrawArrays(GL_TRIANGLES, first_quad, count);

	m_current_sprite_count = 0;

	m_vertex_buffer_regions[m_current_vertex_buffer_region].fence =
	    glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	++m_current_vertex_buffer_region;
	if (m_current_vertex_buffer_region == num_buffers)
	{
		m_current_vertex_buffer_region = 0;
	}
}
} // namespace yaboc::sprite
