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

    virtual bool postLoad() { return true; };

	virtual bool reloadFromFile() = 0;

    virtual std::vector<std::filesystem::path> getDependencies() const = 0;

public:

	inline bool isLoaded() { return loaded; };
	inline std::filesystem::path get_path() const { return asset_path; };
    inline std::string_view get_name() const { return asset_name; }

protected:
	bool out_of_date = false;
    std::filesystem::path asset_path;
    std::string asset_name;
	bool loaded = false;
};

std::vector<int8_t> readFile(const std::filesystem::path& filename);

}