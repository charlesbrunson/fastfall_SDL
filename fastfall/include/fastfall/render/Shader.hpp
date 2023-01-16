#pragma once

#include <string>
#include <string_view>

#include <vector>
#include <map>
#include <string>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

namespace ff {

enum class ShaderType {
	VERTEX,
	FRAGMENT,
};

class ShaderProgram {
public:
    struct gl_attribute {
        GLint       id;
        GLint       loc;
        GLenum      type;
        std::string name;
    };
    struct gl_uniform {
        GLint       id;
        GLint       loc;
        GLenum      type;
        std::string name;
    };

	ShaderProgram() = default;
    ShaderProgram(const ShaderProgram& other) = delete;
    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(const ShaderProgram& other) = delete;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;
	~ShaderProgram();

	void add(ShaderType type, std::string_view shader_code);

	void link();
	bool isLinked() const;

	void use() const;

    const gl_uniform*   getUniform(std::string_view name) const;
    const gl_attribute* getAttribute(std::string_view name) const;

    const std::vector<gl_attribute>& all_attributes() const { return attributes; }
    const std::vector<gl_uniform>&   all_uniforms()   const { return uniforms; }

    static std::string_view getGLSLVersionString();

	static const ShaderProgram& getDefaultProgram();

	bool isInitialized() const { return id != 0; };
	unsigned int getID() const { return id; };

private:
    unsigned int id  = 0;
	bool initialized = false;
    bool m_is_linked = false;

    std::vector<gl_attribute> attributes;
    std::vector<gl_uniform>   uniforms;
	std::vector<unsigned int> shaders;
};

std::string_view uniformTypeEnumToString(GLenum type);

}
