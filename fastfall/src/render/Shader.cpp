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

const static std::string tilearray_vertex = 
gl_header +
R"(
uniform mat3 model;
uniform mat3 view;
uniform sampler2D texture0;

uniform uint columns;

layout (location = 0) in uint aTileId;

out vec2 texCoord;

void main()
{
	const float TILESIZE = 16.0;

	const uint PAD_TOP_MASK 	= (1 << 16);
	const uint PAD_RIGHT_MASK 	= (1 << 15);
	const uint PAD_BOT_MASK 	= (1 << 14);
	const uint PAD_LEFT_MASK 	= (1 << 13);
	const uint Y_MASK 			= (63u << 6);
	const uint X_MASK 			= (63u);
	const uint XY_MASK = (X_MASK | Y_MASK);
	const uint INVALID = 4095u;

	// non-empty tile
	if ((aTileId & XY_MASK) != INVALID) {

		uint pad_top 	= (aTileId >> 16) & 1;
		uint pad_right 	= (aTileId >> 15) & 1;
		uint pad_bot 	= (aTileId >> 14) & 1;
		uint pad_left 	= (aTileId >> 13) & 1;

		uvec2 tile_id = uvec2(
			aTileId & X_MASK,
			(aTileId & Y_MASK) >> 6
		);

		vec2 tileset_size = vec2(textureSize(texture0, 0)) / TILESIZE;

		// +1 for right/bot, -1 for left/top
		float horz_side = float(gl_VertexID & 1) * 2.0 - 1.0;
		float vert_side = float((gl_VertexID & 2) >> 1) * 2.0 - 1.0;

		vec2 t_offset = vec2(
			horz_side,
			vert_side
		) / -16384.0;

		vec2 p_offset = vec2(
			(0.5 + float(horz_side) * 0.5),
			(0.5 + float(vert_side) * 0.5)
		);

		// add padding to position offset
		p_offset.x = p_offset.x 
			+ (-1.0 * pad_left  * float((gl_VertexID + 1) & 1)) 
			+ ( 1.0 * pad_right * float((gl_VertexID)     & 1));

		p_offset.y = p_offset.y
			+ (-1.0 * pad_top * float( ( ( (gl_VertexID & 2) >> 1) + 1) & 1) ) 
			+ ( 1.0 * pad_bot * float(((gl_VertexID & 2) >> 1)));

		// position of tile based on instance id + position offset
		vec2 position = p_offset + vec2(
			float(uint(gl_InstanceID) % columns),
			float(uint(gl_InstanceID) / columns)
		);

		// apply transform
		gl_Position = vec4( view * model * vec3(position * TILESIZE, 1.0), 1.0);

		// calc texture coords
		const uint tileset_columns = 64u;
		texCoord    = t_offset + vec2(
			(float(tile_id.x) + p_offset.x) / tileset_size.x,
			(float(tile_id.y) + p_offset.y) / tileset_size.y
		);
	}
})";

const static std::string tilearray_fragment = 
gl_header +
R"(
precision mediump float;
uniform sampler2D texture0;
in vec2 texCoord;
out vec4 FragColor;

void main()
{
    FragColor = texture(texture0, texCoord);
})";

}


ShaderProgram DefaultProgram;
ShaderProgram TileArrayProgram;

const ShaderProgram& ShaderProgram::getDefaultProgram() {
	if (!DefaultProgram.isLinked() && FFisGLEWInit()) {
		DefaultProgram.add(ff::ShaderType::VERTEX, vertex_default);
		DefaultProgram.add(ff::ShaderType::FRAGMENT, fragment_default);
		DefaultProgram.link();
		LOG_INFO("default program: {}", DefaultProgram.getID());
	}
	return DefaultProgram;
}
const ShaderProgram& ShaderProgram::getTileArrayProgram() {
	if (!TileArrayProgram.isLinked() && FFisGLEWInit()) {
		TileArrayProgram.add(ff::ShaderType::VERTEX,   tilearray_vertex);
		TileArrayProgram.add(ff::ShaderType::FRAGMENT, tilearray_fragment);
		TileArrayProgram.link();
		LOG_INFO("tile program: {}", TileArrayProgram.getID());
	}
	return TileArrayProgram;
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
		glCheck(id = glCreateProgram());
	}

	GLint shader_id;
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
	shaders.push_back(shader_id);

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
		LOG_ERR_("{}", shader_code);

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
	columns_loc = glGetUniformLocation(id, "columns");

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





}
