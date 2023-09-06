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

#include <cassert>
#include <fstream>
#include <sstream>

namespace yaboc
{
namespace
{
	auto gl_check_shader_compile_status(unsigned int shader_id) -> bool;

	auto gl_check_program_link_status(unsigned int program_id) -> bool;

	constexpr auto to_gl_shader_type(shader_builder_input::shader_type input)
	{
		switch (input)
		{
		case shader_builder_input::shader_type::vertex: return GL_VERTEX_SHADER;
		case shader_builder_input::shader_type::tess_control:
			return GL_TESS_CONTROL_SHADER;
		case shader_builder_input::shader_type::tess_eval:
			return GL_TESS_EVALUATION_SHADER;
		case shader_builder_input::shader_type::geometry:
			return GL_GEOMETRY_SHADER;
		case shader_builder_input::shader_type::fragment:
			return GL_FRAGMENT_SHADER;
		case shader_builder_input::shader_type::compute:
			return GL_COMPUTE_SHADER;
		}

		assert(false && "Invalid shader type provided");
	}
} // namespace

auto make_shader(std::vector<shader_builder_input> const& inputs) -> unsigned int
{
	auto program_id{glCreateProgram()};

	std::vector<unsigned int> attached_shaders{};

	for (auto const& input: inputs)
	{
		auto shader_id = glCreateShader(to_gl_shader_type(input.type));

		auto file = std::ifstream{input.path};

		std::stringstream file_contents{};
		file_contents << file.rdbuf();
		auto        code      = file_contents.str();
		auto const* code_cstr = code.c_str();
		glShaderSource(shader_id, 1, &code_cstr, nullptr);
		glCompileShader(shader_id);
		gl_check_shader_compile_status(shader_id);
		glAttachShader(program_id, shader_id);
		attached_shaders.push_back(shader_id);
	}

	glLinkProgram(program_id);

	gl_check_program_link_status(program_id);

	for (auto id: attached_shaders)
	{
		glDetachShader(program_id, id);
		glDeleteShader(id);
	}

	return program_id;
}

namespace
{
	auto gl_check_shader_compile_status(unsigned int shader_id) -> bool
	{
		int success{};
		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
		if (success == 0)
		{
			int log_length{};
			glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
			std::string message{};
			message.resize(static_cast<std::size_t>(log_length));
			glGetShaderInfoLog(shader_id, log_length, nullptr, message.data());
//			spdlog::default_logger()->error(
//			    "Failed to compiler shader (id: {}): {}",
//			    shader_id,
//			    message);
			return false;
		}
		return true;
	}

	auto gl_check_program_link_status(unsigned int program_id) -> bool
	{
		int success{};
		glGetProgramiv(program_id, GL_LINK_STATUS, &success);
		if (success == 0)
		{
			int log_length{};
			glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
			std::string message{};
			message.resize(static_cast<std::size_t>(log_length));
			glGetProgramInfoLog(program_id,
			                    log_length,
			                    nullptr,
			                    message.data());
//			spdlog::default_logger()->error(
//			    "Failed to compiler shader (id: {}): {}",
//			    program_id,
//			    message);
			return false;
		}
		return true;
	}
} // namespace
} // namespace yaboc
