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
uniform uint columns;

layout (location = 0) in uint aTileId;

flat out uint tileId;

void main()
{
	//if (aTileId == 0)
	//	discard; // empty tile

	// calc topleft position of tile
	float x = float(uint(gl_VertexID) % columns) * 16.0;
	float y = float(uint(gl_VertexID) / columns) * 16.0;

	// apply transform
	gl_Position = vec4( x, y, 1.0, 1.0);

	// pass tile id to geometry shader
    tileId = aTileId;
})";

const static std::string tilearray_geometry = 
gl_header +
R"(
uniform mat3 model;
uniform mat3 view;
uniform sampler2D texture0;

flat in uint tileId[];

out vec2 texCoord;

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

void main() {
	uint tid = tileId[0];
	if (tid != 255u) {

		uvec2 tex_size = uvec2(textureSize(texture0, 0));
		uvec2 tile_size = tex_size / 16u;
		vec2 tex_fsize = vec2(tex_size);

		float tileX = float(tid % 16u) * 16.0 / tex_fsize.x;
		float tileY = float(tid / 16u) * 16.0 / tex_fsize.y;

		const float B = 1.0 / 16384.0;
		float Sx = 16.0 / tex_fsize.x;
		float Sy = 16.0 / tex_fsize.y;

		gl_Position = vec4( view * model * vec3(gl_in[0].gl_Position.xy + vec2( 0.0,  0.0), 1.0), 1.0);
		texCoord = vec2(tileX + B, tileY + B);
		EmitVertex();

		gl_Position = vec4( view * model * vec3(gl_in[0].gl_Position.xy + vec2(16.0,  0.0), 1.0), 1.0);
		texCoord = vec2(tileX + Sx - B, tileY + B);
		EmitVertex();

		gl_Position = vec4( view * model * vec3(gl_in[0].gl_Position.xy + vec2( 0.0, 16.0), 1.0), 1.0);
		texCoord = vec2(tileX + B, tileY + Sy - B);
		EmitVertex();

		gl_Position = vec4( view * model * vec3(gl_in[0].gl_Position.xy + vec2(16.0, 16.0), 1.0), 1.0);
		texCoord = vec2(tileX + Sx - B, tileY + Sy - B);
		EmitVertex();
	}
})";

const static std::string tilearray_fragment = 
gl_header +
R"(
uniform sampler2D texture0;
in vec2 texCoord;
out vec4 FragColor;

void main()
{
    FragColor = texture(texture0, texCoord);
})";








/*
#if defined(__EMSCRIPTEN__)
	static const std::string_view vertex_default = 
	R"(#version 300 es

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

	static const std::string_view fragment_default = 
	R"(#version 300 es

	precision mediump float;
	in vec4 v_color;
	in vec2 v_texcoord;

	out vec4 FragColor;

	uniform sampler2D Texture;

	void main() {
		FragColor = texture(Texture, v_texcoord) * v_color;
	})";

#else
	static const std::string_view vertex_default = R"(
	#version 330 core

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

	static const std::string_view fragment_default = R"(
	#version 330 core

	in vec4 v_color;
	in vec2 v_texcoord;

	out vec4 FragColor;

	uniform sampler2D Texture;

	void main() {
		FragColor = texture(Texture, v_texcoord) * v_color;
	})";
#endif

static const std::string_view tilearray_vertex = 
R"(#version 330 core

uniform mat3 model;
uniform mat3 view;
uniform uint columns;

layout (location = 0) in uint aTileId;

out uint tileId;

void main()
{
	//if (aTileId == 0)
	//	discard; // empty tile

	// calc topleft position of tile
	float x = float(uint(gl_VertexID) % columns) * 16.0;
	float y = float(uint(gl_VertexID) / columns) * 16.0;

	// apply transform
	gl_Position = vec4( x, y, 1.0, 1.0);

	// pass tile id to geometry shader
    tileId = aTileId;
})";

static const std::string_view tilearray_geometry = 
R"(#version 330 core

uniform mat3 model;
uniform mat3 view;
uniform sampler2D texture0;

in uint tileId[];

out vec2 texCoord;

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

void main() {
	uint tid = tileId[0];
	if (tid != 255u) {

		uvec2 tex_size = uvec2(textureSize(texture0, 0));
		uvec2 tile_size = tex_size / 16u;
		vec2 tex_fsize = vec2(tex_size);

		float tileX = float(tid % 16u) * 16.0 / tex_fsize.x;
		float tileY = float(tid / 16u) * 16.0 / tex_fsize.y;

		const float B = 1.0 / 16384.0;
		float Sx = 16.0 / tex_fsize.x;
		float Sy = 16.0 / tex_fsize.y;

		gl_Position = vec4( view * model * vec3(gl_in[0].gl_Position.xy + vec2( 0.0,  0.0), 1.0), 1.0);
		texCoord = vec2(tileX + B, tileY + B);
		EmitVertex();

		gl_Position = vec4( view * model * vec3(gl_in[0].gl_Position.xy + vec2(16.0,  0.0), 1.0), 1.0);
		texCoord = vec2(tileX + Sx - B, tileY + B);
		EmitVertex();

		gl_Position = vec4( view * model * vec3(gl_in[0].gl_Position.xy + vec2( 0.0, 16.0), 1.0), 1.0);
		texCoord = vec2(tileX + B, tileY + Sy - B);
		EmitVertex();

		gl_Position = vec4( view * model * vec3(gl_in[0].gl_Position.xy + vec2(16.0, 16.0), 1.0), 1.0);
		texCoord = vec2(tileX + Sx - B, tileY + Sy - B);
		EmitVertex();
	}
})";

static const std::string_view tilearray_fragment = 
R"(#version 330 core

uniform sampler2D texture0;
in vec2 texCoord;
out vec4 FragColor;

void main()
{
    FragColor = texture(texture0, texCoord);
})";
*/
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
		TileArrayProgram.add(ff::ShaderType::GEOMETRY, tilearray_geometry);
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
