#pragma once


#include "fastfall/engine/imgui/ImGuiContent.hpp"
#include "fastfall/util/math.hpp"

#include "flatbuffers/flatbuffers.h"

#include <iostream>

namespace ff {

//#include <SFML/System.hpp>


/*
class AssetStream : public sf::InputStream {

	std::ifstream* source;

	std::streampos start, end;

public:
	explicit AssetStream(std::ifstream* stream, int st, int en) :
		source(stream)
	{
		start = st;
		end = en;
	};

	inline std::ifstream* getSource() { return source; };

	sf::Int64 read(void* data, sf::Int64 size) override;
	sf::Int64 seek(sf::Int64 position) override;
	sf::Int64 tell() override;
	sf::Int64 getSize() override;
};
*/

// base class for files to be loaded
class Asset : public ImGuiContent {
public:
	Asset(const std::string& filename);

	virtual bool loadFromFile(const std::string& relpath) = 0;
	virtual bool reloadFromFile() = 0;

public:

	inline bool isLoaded() { return loaded; };
	inline const std::string& getAssetName() const { return assetName; };
	inline const std::string& getFilePath() const { return assetFilePath; }

protected:

	std::string assetFilePath;
	std::string assetName;
	bool loaded = false;
};

template<typename T>
struct flat_type
{
};

std::vector<int8_t> readFile(const char* filename);

constexpr const char* spriteExt = ".sax";
constexpr const char* tilesetExt = ".tsx";
constexpr const char* levelExt = ".tmx";

}