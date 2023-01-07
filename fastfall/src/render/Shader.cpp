#include "fastfall/render/Shader.hpp"

#include "detail/error.hpp"

#include "fastfall/render.hpp"

#include <assert.h>

namespace ff {

namespace {

const static std::string gl_header =
#if defined(__EMSCRIPTEN__)
    "#version 300 es\n";
#else
    "#version 330 core\n";
#endif


const static std::string vertex_default = 
gl_header +
R"(
layout(location = 0) in vec2 i_position;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec2 i_texcoord;

out vec4 v_color;
out vec2 v_texcoord;

uniform mat3 model;
uniform mat3 view;

void main() {
	v_texcoord = i_texcoord;
	v_color = i_color;
	gl_Position = vec4( view * model * vec3( i_position, 1.0 ), 1.0);
})";

const static std::string fragment_default = 
gl_header +
R"(
precision mediump float;
in vec4 v_color;
in vec2 v_texcoord;

out vec4 FragColor;

uniform sampler2D Texture;

void main() {
	FragColor = texture(Texture, v_texcoord) * v_color;
})";

}

std::string_view ShaderProgram::getGLSLVersionString() {
    return gl_header;
}

ShaderProgram DefaultProgram;
//ShaderProgram TileArrayProgram;
//ShaderProgram TextProgram;

const ShaderProgram& ShaderProgram::getDefaultProgram() {
	if (!DefaultProgram.isLinked() && render_glew_is_init()) {
		DefaultProgram.add(ff::ShaderType::VERTEX, vertex_default);
		DefaultProgram.add(ff::ShaderType::FRAGMENT, fragment_default);
		DefaultProgram.link();
		//LOG_INFO("default program: {}", DefaultProgram.getID());
	}
	return DefaultProgram;
}

/*
const ShaderProgram& ShaderProgram::getTileArrayProgram() {
	if (!TileArrayProgram.isLinked() && render_glew_is_init()) {
		TileArrayProgram.add(ff::ShaderType::VERTEX,   tilearray_vertex);
		TileArrayProgram.add(ff::ShaderType::FRAGMENT, tilearray_fragment);
		TileArrayProgram.link();
		TileArrayProgram.cacheUniform("columns");
		//LOG_INFO("tile program: {}", TileArrayProgram.getID());
	}
	return TileArrayProgram;
}
const ShaderProgram& ShaderProgram::getTextProgram() {
	if (!TextProgram.isLinked() && render_glew_is_init()) {
		TextProgram.add(ff::ShaderType::VERTEX, text_vertex);
		TextProgram.add(ff::ShaderType::FRAGMENT, text_fragment);
		TextProgram.link();
		TextProgram.cacheUniform("char_size");
		//LOG_INFO("text program: {}", TextProgram.getID());
	}
	return TextProgram;
}
*/

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
		glCheck(id = glCreateProgram());
	}

	GLint shader_id = -1;
	switch (type)
	{
		case ShaderType::GEOMETRY: 
			glCheck(shader_id = glCreateShader(GL_GEOMETRY_SHADER));
			break;
		case ShaderType::VERTEX: 
			glCheck(shader_id = glCreateShader(GL_VERTEX_SHADER));
			break;
		case ShaderType::FRAGMENT: 
			glCheck(shader_id = glCreateShader(GL_FRAGMENT_SHADER));
			break;
	}

	if (shader_id != -1) {
		shaders.push_back(shader_id);
	}
	else {
		LOG_ERR_("Could not create shader");
		return;
	}

	GLint len = shader_code.length();
	const GLchar* data = shader_code.data();
	glCheck(glShaderSource(shaders.back(), 1, &data, NULL));
	glCheck(glCompileShader(shaders.back()));

	GLint status;
	glGetShaderiv(shaders.back(), GL_COMPILE_STATUS, &status);
	if (GL_FALSE == status) {
		GLint loglen;
		glGetShaderiv(shaders.back(), GL_INFO_LOG_LENGTH, &loglen);

		std::string log((int)loglen + 1, ' ');
		glGetShaderInfoLog(shaders.back(), log.size(), &loglen, log.data());

		glDeleteShader(shaders.back());
		shaders.pop_back();

		LOG_ERR_("Could not compile shader: {}", log);
		LOG_ERR_("\n{}", shader_code);

		throw Error(std::string("Could not compile shader: \n") + log);
	}
	glCheck(glAttachShader(id, shaders.back()));
}

void ShaderProgram::link() {
	if (isLinked())
		return;

	glCheck(glLinkProgram(id));

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

	mdl_loc  	= glGetUniformLocation(id, "model");
	view_loc 	= glGetUniformLocation(id, "view");
	//columns_loc = glGetUniformLocation(id, "columns");

	m_is_linked = true;
}

bool ShaderProgram::isLinked() const {
	return m_is_linked;
}

void ShaderProgram::use() const {
	if (isLinked()) {
		glCheck(glUseProgram(id));
	}
	else {
		LOG_ERR_("Attempted to use unlinked shader id: {}", id);
		glCheck(glUseProgram(0));
	}
}


int ShaderProgram::getOtherUniformID(std::string_view uniform_name) const {
	auto it = other_locs.find(uniform_name);
	if (it != other_locs.end()) {
		return it->second;
	}
	return -1;
}

void ShaderProgram::cacheUniform(std::string_view uniform_name) {
	int uniform_id = glGetUniformLocation(id, uniform_name.data());
	if (uniform_id != -1) {
		other_locs.emplace(uniform_name, uniform_id);
	}
	else {
		LOG_ERR_("Could not cache uniform for shader {}: {}", id, uniform_name);
	}
}





}
