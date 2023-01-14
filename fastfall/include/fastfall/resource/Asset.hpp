#pragma once


#include "fastfall/engine/imgui/ImGuiContent.hpp"
#include "fastfall/util/math.hpp"

//#include "flatbuffers/flatbuffers.h"

#include <iostream>
#include <filesystem>

namespace ff {

// base class for files to be loaded
class Asset : public ImGuiContent {
public:
	Asset(const std::filesystem::path& t_asset_path);

	virtual bool loadFromFile() = 0;

	void setOutOfDate(bool is_OOD) { out_of_date = is_OOD; };
	bool isOutOfDate() const { return out_of_date; };

	virtual bool reloadFromFile() = 0;

    virtual std::vector<std::filesystem::path> getDependencies() const = 0;

public:

	inline bool isLoaded() { return loaded; };
	inline std::filesystem::path get_path() const { return asset_path; };

protected:
	bool out_of_date = false;
    std::filesystem::path asset_path;
	bool loaded = false;
};

template<typename T>
struct flat_type
{
};

std::vector<int8_t> readFile(const char* filename);

}