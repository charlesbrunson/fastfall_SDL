#include "fastfall/render/Shader.hpp"

#include "detail/error.hpp"

#include "fastfall/render.hpp"

#include <assert.h>

namespace ff {

namespace {

	static const std::string_view vertex_default = R"(
	#version 330 core

	layout(location = 0) in vec2 i_position;
	layout(location = 1) in vec4 i_color;
	layout(location = 2) in vec2 i_texcoord;

	out vec4 v_color;
	out vec2 v_texcoord;

	uniform mat3 model;
	uniform mat3 view;
	uniform mat3 subtexture;

	void main() {
		v_texcoord = i_texcoord;
		v_color = i_color;
		gl_Position = vec4( view * model * vec3( i_position, 1.0 ), 1.0);
	})";

	static const std::string_view fragment_default = R"(
	#version 330 core

	in vec4 v_color;
	in vec2 v_texcoord;

	out vec4 FragColor;

	uniform sampler2D Texture;

	void main() {
		FragColor = texture(Texture, v_texcoord) * v_color;
	})";

}


ShaderProgram ShaderProgram::DefaultProgram;

const ShaderProgram& ShaderProgram::getDefaultProgram() {
	if (!DefaultProgram.isLinked() && FFisGLEWInit()) {
		DefaultProgram.add(ff::ShaderType::VERTEX, vertex_default);
		DefaultProgram.add(ff::ShaderType::FRAGMENT, fragment_default);
		DefaultProgram.link();
	}
	return DefaultProgram;
}

ShaderProgram::ShaderProgram()
{

}

ShaderProgram::~ShaderProgram() {
	if (initialized) {
		if (!isLinked()) {
			for (auto shader : shaders) {
				glDetachShader(id, shader);
				glDeleteShader(shader);
			}
		}
		glDeleteProgram(id);
	}
}

void ShaderProgram::add(ShaderType type, const std::string_view shader_code) {
	assert(!isLinked());

	if (!isInitialized()) {
		id = glCreateProgram();
	}

	shaders.push_back(glCreateShader(type == ShaderType::VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER));

	GLint len = shader_code.length();
	const GLchar* data = shader_code.data();
	glShaderSource(shaders.back(), 1, &data, NULL);
	glCompileShader(shaders.back());

	GLint status;
	glGetShaderiv(shaders.back(), GL_COMPILE_STATUS, &status);
	if (GL_FALSE == status) {
		GLint loglen;
		glGetShaderiv(shaders.back(), GL_INFO_LOG_LENGTH, &loglen);

		std::string log((int)loglen + 1, ' ');
		glGetShaderInfoLog(shaders.back(), log.size(), &loglen, log.data());

		glDeleteShader(shaders.back());
		shaders.pop_back();

		throw Error(std::string("Could not compile shader: \n") + log);
	}
	glAttachShader(id, shaders.back());
}

void ShaderProgram::link() {
	if (isLinked())
		return;

	glLinkProgram(id);

	GLint status;
	glGetProgramiv(id, GL_LINK_STATUS, &status);

	if (GL_FALSE == status) {
		GLint loglen;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &loglen);

		std::string log((int)loglen + 1, ' ');
		glGetProgramInfoLog(id, log.size(), &loglen, log.data());

		throw Error(std::string("Could not link shader program: \n") + log);
	}
	for (auto shader : shaders) {
		glDetachShader(id, shader);
		glDeleteShader(shader);
	}

	mdl_loc  = glGetUniformLocation(id, "model");
	view_loc = glGetUniformLocation(id, "view");
	//subtex_loc = glGetUniformLocation(id, "subtexture");

	m_is_linked = true;
}

bool ShaderProgram::isLinked() const {
	return m_is_linked;
}

void ShaderProgram::use() const {
	if (isLinked()) {
		glUseProgram(id);
	}
	else {
		glUseProgram(0);
	}
}





}