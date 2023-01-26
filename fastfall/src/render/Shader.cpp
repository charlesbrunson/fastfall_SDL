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

const ShaderProgram& ShaderProgram::getDefaultProgram() {
	if (!DefaultProgram.isLinked() && render::glew_is_init()) {
		DefaultProgram.add(ff::ShaderType::VERTEX, vertex_default);
		DefaultProgram.add(ff::ShaderType::FRAGMENT, fragment_default);
		DefaultProgram.link();
	}
	return DefaultProgram;
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept {
    if (initialized) {
        if (!isLinked()) {
            for (auto shader : shaders) {
                glDetachShader(id, shader);
                glDeleteShader(shader);
            }
        }
        glDeleteProgram(id);
    }

    initialized = other.initialized;
    m_is_linked = other.m_is_linked;
    id          = other.id;
    attributes  = other.attributes;
    uniforms    = other.uniforms;
    shaders     = std::move(other.shaders);

    other.initialized = false;
    other.m_is_linked = false;
    other.id          = 0;
    other.attributes  = {};
    other.uniforms    = {};
    other.shaders     = {};
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (initialized) {
        if (!isLinked()) {
            for (auto shader : shaders) {
                glDetachShader(id, shader);
                glDeleteShader(shader);
            }
        }
        glDeleteProgram(id);
    }

    initialized = other.initialized;
    m_is_linked = other.m_is_linked;
    id          = other.id;
    attributes  = other.attributes;
    uniforms    = other.uniforms;
    shaders     = std::move(other.shaders);

    other.initialized = false;
    other.m_is_linked = false;
    other.id          = 0;
    other.attributes  = {};
    other.uniforms    = {};
    other.shaders     = {};
    return *this;
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
        initialized = true;
	}

	GLint shader_id = -1;
	switch (type)
	{
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

    GLint i;
    GLint count;
    GLint size;
    GLenum type;

    constexpr GLsizei bufSize = 128;
    GLchar name[bufSize];
    GLsizei length;

    GLint location;

    glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &count);
    for (i = 0; i < count; i++) {
        glGetActiveAttrib(id, (GLuint)i, bufSize, &length, &size, &type, name);
        location = glGetAttribLocation(id, name);
        attributes.push_back({ i, location, type, name });
    }

    glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &count);
    for (i = 0; i < count; i++) {
        glGetActiveUniform(id, (GLuint)i, bufSize, &length, &size, &type, name);
        location = glGetUniformLocation(id, name);
        uniforms.push_back({ i, location, type, name });
    }

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

const ShaderProgram::gl_uniform* ShaderProgram::getUniform(std::string_view name) const
{
    auto it = std::find_if(uniforms.begin(), uniforms.end(),
       [&name](const auto& uni) { return uni.name == name; });
    return (it != uniforms.end() ? &*it : nullptr);
}

const ShaderProgram::gl_attribute* ShaderProgram::getAttribute(std::string_view name) const
{
    auto it = std::find_if(attributes.begin(), attributes.end(),
           [&name](const auto& uni) { return uni.name == name; });
    return (it != attributes.end() ? &*it : nullptr);
}

std::string_view uniformTypeEnumToString(GLenum type) {
    switch (type) {
        case GL_FLOAT: return "float";
        case GL_FLOAT_VEC2: return "vec2";
        case GL_FLOAT_VEC3: return "vec3";
        case GL_FLOAT_VEC4: return "vec4";
        case GL_DOUBLE: return "double";
        case GL_DOUBLE_VEC2: return "dvec2";
        case GL_DOUBLE_VEC3: return "dvec3";
        case GL_DOUBLE_VEC4: return "dvec4";
        case GL_INT: return "int";
        case GL_INT_VEC2: return "ivec2";
        case GL_INT_VEC3: return "ivec3";
        case GL_INT_VEC4: return "ivec4";
        case GL_UNSIGNED_INT: return "unsigned int";
        case GL_UNSIGNED_INT_VEC2: return "uvec2";
        case GL_UNSIGNED_INT_VEC3: return "uvec3";
        case GL_UNSIGNED_INT_VEC4: return "uvec4";
        case GL_BOOL: return "bool";
        case GL_BOOL_VEC2: return "bvec2";
        case GL_BOOL_VEC3: return "bvec3";
        case GL_BOOL_VEC4: return "bvec4";
        case GL_FLOAT_MAT2: return "mat2";
        case GL_FLOAT_MAT3: return "mat3";
        case GL_FLOAT_MAT4: return "mat4";
        case GL_FLOAT_MAT2x3: return "mat2x3";
        case GL_FLOAT_MAT2x4: return "mat2x4";
        case GL_FLOAT_MAT3x2: return "mat3x2";
        case GL_FLOAT_MAT3x4: return "mat3x4";
        case GL_FLOAT_MAT4x2: return "mat4x2";
        case GL_FLOAT_MAT4x3: return "mat4x3";
        case GL_DOUBLE_MAT2: return "dmat2";
        case GL_DOUBLE_MAT3: return "dmat3";
        case GL_DOUBLE_MAT4: return "dmat4";
        case GL_DOUBLE_MAT2x3: return "dmat2x3";
        case GL_DOUBLE_MAT2x4: return "dmat2x4";
        case GL_DOUBLE_MAT3x2: return "dmat3x2";
        case GL_DOUBLE_MAT3x4: return "dmat3x4";
        case GL_DOUBLE_MAT4x2: return "dmat4x2";
        case GL_DOUBLE_MAT4x3: return "dmat4x3";
        case GL_SAMPLER_1D: return "sampler1D";
        case GL_SAMPLER_2D: return "sampler2D";
        case GL_SAMPLER_3D: return "sampler3D";
        case GL_SAMPLER_CUBE: return "samplerCube";
        case GL_SAMPLER_1D_SHADOW: return "sampler1DShadow";
        case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
        case GL_SAMPLER_1D_ARRAY: return "sampler1DArray";
        case GL_SAMPLER_2D_ARRAY: return "sampler2DArray";
        case GL_SAMPLER_1D_ARRAY_SHADOW: return "sampler1DArrayShadow";
        case GL_SAMPLER_2D_ARRAY_SHADOW: return "sampler2DArrayShadow";
        case GL_SAMPLER_2D_MULTISAMPLE: return "sampler2DMS";
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: return "sampler2DMSArray";
        case GL_SAMPLER_CUBE_SHADOW: return "samplerCubeShadow";
        case GL_SAMPLER_BUFFER: return "samplerBuffer";
        case GL_SAMPLER_2D_RECT: return "sampler2DRect";
        case GL_SAMPLER_2D_RECT_SHADOW: return "sampler2DRectShadow";
        case GL_INT_SAMPLER_1D: return "isampler1D";
        case GL_INT_SAMPLER_2D: return "isampler2D";
        case GL_INT_SAMPLER_3D: return "isampler3D";
        case GL_INT_SAMPLER_CUBE: return "isamplerCube";
        case GL_INT_SAMPLER_1D_ARRAY: return "isampler1DArray";
        case GL_INT_SAMPLER_2D_ARRAY: return "isampler2DArray";
        case GL_INT_SAMPLER_2D_MULTISAMPLE: return "isampler2DMS";
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "isampler2DMSArray";
        case GL_INT_SAMPLER_BUFFER: return "isamplerBuffer";
        case GL_INT_SAMPLER_2D_RECT: return "isampler2DRect";
        case GL_UNSIGNED_INT_SAMPLER_1D: return "usampler1D";
        case GL_UNSIGNED_INT_SAMPLER_2D: return "usampler2D";
        case GL_UNSIGNED_INT_SAMPLER_3D: return "usampler3D";
        case GL_UNSIGNED_INT_SAMPLER_CUBE: return "usamplerCube";
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY: return "usampler2DArray";
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return "usampler2DArray";
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE: return "usampler2DMS";
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "usampler2DMSArray";
        case GL_UNSIGNED_INT_SAMPLER_BUFFER: return "usamplerBuffer";
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT: return "usampler2DRect";
        case GL_IMAGE_1D: return "image1D";
        case GL_IMAGE_2D: return "image2D";
        case GL_IMAGE_3D: return "image3D";
        case GL_IMAGE_2D_RECT: return "image2DRect";
        case GL_IMAGE_CUBE: return "imageCube";
        case GL_IMAGE_BUFFER: return "imageBuffer";
        case GL_IMAGE_1D_ARRAY: return "image1DArray";
        case GL_IMAGE_2D_ARRAY: return "image2DArray";
        case GL_IMAGE_2D_MULTISAMPLE: return "image2DMS";
        case GL_IMAGE_2D_MULTISAMPLE_ARRAY: return "image2DMSArray";
        case GL_INT_IMAGE_1D: return "iimage1D";
        case GL_INT_IMAGE_2D: return "iimage2D";
        case GL_INT_IMAGE_3D: return "iimage3D";
        case GL_INT_IMAGE_2D_RECT: return "iimage2DRect";
        case GL_INT_IMAGE_CUBE: return "iimageCube";
        case GL_INT_IMAGE_BUFFER: return "iimageBuffer";
        case GL_INT_IMAGE_1D_ARRAY: return "iimage1DArray";
        case GL_INT_IMAGE_2D_ARRAY: return "iimage2DArray";
        case GL_INT_IMAGE_2D_MULTISAMPLE: return "iimage2DMS";
        case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return "iimage2DMSArray";
        case GL_UNSIGNED_INT_IMAGE_1D: return "uimage1D";
        case GL_UNSIGNED_INT_IMAGE_2D: return "uimage2D";
        case GL_UNSIGNED_INT_IMAGE_3D: return "uimage3D";
        case GL_UNSIGNED_INT_IMAGE_2D_RECT: return "uimage2DRect";
        case GL_UNSIGNED_INT_IMAGE_CUBE: return "uimageCube";
        case GL_UNSIGNED_INT_IMAGE_BUFFER: return "uimageBuffer";
        case GL_UNSIGNED_INT_IMAGE_1D_ARRAY: return "uimage1DArray";
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY: return "uimage2DArray";
        case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE: return "uimage2DMS";
        case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return "uimage2DMSArray";
        case GL_UNSIGNED_INT_ATOMIC_COUNTER: return "atomic_uint";
        default: return "";
    }
}

}
