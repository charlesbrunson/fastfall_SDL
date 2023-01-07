#pragma once

#include <string>
#include <string_view>

#include <vector>
#include <map>
#include <string>

namespace ff {

enum class ShaderType {
	VERTEX,
	FRAGMENT,
	GEOMETRY
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

	int getOtherUniformID(std::string_view uniform_name) const;
	void cacheUniform(std::string_view uniform_name);

    static std::string_view getGLSLVersionString();

	static const ShaderProgram& getDefaultProgram();
	//static const ShaderProgram& getTileArrayProgram();
	//static const ShaderProgram& getTextProgram();

	bool isInitialized() const { return id != 0; };
	unsigned int getID() const { return id; };

private:

	bool initialized = false;

	unsigned int id = 0;

	bool m_is_linked = false;

	int view_loc 	= -1;
	int mdl_loc  	= -1;

	std::map<std::string, int, std::less<>> other_locs;

	std::vector<unsigned int> shaders;
};

}
