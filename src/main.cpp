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
#include "yaboc/shader.h"

#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"

#include <array>
#include <iostream>

namespace
{
constexpr int window_default_width{800};
constexpr int window_default_height{600};

void message_callback(GLenum        source,
                      GLenum        type,
                      GLuint        id,
                      GLenum        severity,
                      GLsizei       length,
                      GLchar const* message,
                      void const*   user_param);
} // namespace

namespace yaboc
{
class sprite_renderer final
{
	unsigned int m_shader{};

	unsigned int m_vao{};
	unsigned int m_vbo{};
	unsigned int m_ebo{};

public:
	~sprite_renderer()
	{
		glDeleteProgram(m_shader);
		glDeleteBuffers(1, &m_vbo);
		glDeleteBuffers(1, &m_ebo);
		glDeleteVertexArrays(1, &m_vao);
	}

	explicit sprite_renderer(unsigned int shader_id)
	    : m_shader{shader_id}
	{
		auto const verts =
		    std::array{-0.5F, -0.5F, 0.5F, -0.5F, 0.5F, 0.5F, -0.5F, 0.5F};
		glCreateBuffers(1, &m_vbo);
		glNamedBufferStorage(m_vbo,
		                     std::size(verts) * sizeof(float),
		                     std::data(verts),
		                     GL_DYNAMIC_STORAGE_BIT);

		auto const indices = std::array{0U, 1U, 2U, 2U, 3U, 0U};
		glCreateBuffers(1, &m_ebo);
		glNamedBufferStorage(m_ebo,
		                     std::size(indices) * sizeof(unsigned int),
		                     std::data(indices),
		                     GL_DYNAMIC_STORAGE_BIT);

		glCreateVertexArrays(1, &m_vao);
		glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, 2 * sizeof(float));
		glVertexArrayElementBuffer(m_vao, m_ebo);

		glEnableVertexArrayAttrib(m_vao, 0);
		glVertexArrayAttribFormat(m_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_vao, 0, 0);
	}

	sprite_renderer(sprite_renderer const&) = delete;
	sprite_renderer(sprite_renderer&&) = default;

	auto operator=(sprite_renderer const&) -> sprite_renderer& = delete;
	auto operator=(sprite_renderer&&) -> sprite_renderer& = default;

	auto submit_sprite(glm::vec2 const position, glm::vec2 const size) const
	    -> void
	{
		auto model =
		    glm::translate(glm::mat4{1.0F}, glm::vec3{position, 1.0F});
		model = glm::scale(model, glm::vec3{size, 1.0F});

		glUseProgram(m_shader);
		auto const model_loc = glGetUniformLocation(m_shader, "model");
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(m_vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}
};
} // namespace yaboc

auto main(int argc, char* argv[]) -> int
{
	static_cast<void>(argc);
	static_cast<void>(argv);

	// NOLINTNEXTLINE(*-signed-bitwise)
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                    SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
	                    SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

	SDL_Window* window = SDL_CreateWindow("Yet Another Breakout Clone",
	                                      window_default_width,
	                                      window_default_height,
	                                      SDL_WINDOW_OPENGL);

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);

	gladLoadGL(SDL_GL_GetProcAddress);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(message_callback, nullptr);
	glDebugMessageControl(GL_DONT_CARE,
	                      GL_DONT_CARE,
	                      GL_DEBUG_SEVERITY_NOTIFICATION,
	                      0,
	                      nullptr,
	                      GL_FALSE);

	auto sprite_shader =
	    yaboc::make_shader(std::vector<yaboc::shader_builder_input>{
	        {.type = yaboc::shader_builder_input::shader_type::vertex,
	         .path = "assets/shaders/sprite.vert.glsl"},
	        {.type = yaboc::shader_builder_input::shader_type::fragment,
	         .path = "assets/shaders/sprite.frag.glsl"}
    });

	int window_drawable_width  = 0;
	int window_drawable_height = 0;
	SDL_GetWindowSizeInPixels(window,
	                          &window_drawable_width,
	                          &window_drawable_height);

	auto projection = glm::ortho(0.0F,
	                             static_cast<float>(window_drawable_width),
	                             static_cast<float>(window_drawable_height),
	                             0.0F,
	                             -1.0F,
	                             1.0F);

	glViewport(0, 0, window_drawable_width, window_drawable_height);

	glUseProgram(sprite_shader);
	auto const proj_loc = glGetUniformLocation(sprite_shader, "projection");
	glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(projection));

	auto renderer = yaboc::sprite_renderer{sprite_shader};

	bool running{true};
	while (running)
	{
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

		glm::vec4 clear_colour{};
		glClearBufferfv(GL_COLOR, 0, glm::value_ptr(clear_colour));

		renderer.submit_sprite(glm::vec2{400, 100}, glm::vec2{100, 10});

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}

namespace
{
void message_callback(GLenum        source,
                      GLenum        type,
                      GLuint        id,
                      GLenum        severity,
                      GLsizei       length,
                      GLchar const* message,
                      void const*   user_param)
{
	static_cast<void>(length);
	static_cast<void>(user_param);
	auto const src_str = [source]() {
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		default: break;
		}
		return "UNKNOWN";
	};

	auto const type_str = [type]() {
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		default: break;
		}
		return "UNKNOWN";
	};

	auto const severity_str = [severity]() {
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		default: break;
		}
		return "UNKNOWN";
	};
	std::cout << src_str() << ", " << type_str() << ", " << severity_str()
	          << ", " << id << ": " << message << '\n';
}
} // namespace
