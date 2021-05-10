#pragma once

#include <string>
#include <string_view>

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>

#include "detail/error.hpp"

#include <vector>

namespace ff {

enum class ShaderType {
	VERTEX,
	FRAGMENT
};

class ShaderProgram {
public:

	ShaderProgram();
	~ShaderProgram();

	void add(ShaderType type, const std::string_view shader_code);

	void link();
	bool isLinked() const;

	void use() const;

	inline int getViewUniformID() const { return view_loc; };
	inline int getMdlUniformID() const { return mdl_loc; };
	//inline int getSubTexUniformID() const { return subtex_loc; };


	static const ShaderProgram& getDefaultProgram();

	bool isInitialized() const { return id != 0; };
	GLuint getID() const { return id; };

private:

	bool initialized = false;

	GLuint id = 0;

	static ShaderProgram DefaultProgram;

	bool m_is_linked = false;

	int view_loc = -1;
	int mdl_loc  = -1;
	//int subtex_loc = -1;

	std::vector<GLuint> shaders;
};

}