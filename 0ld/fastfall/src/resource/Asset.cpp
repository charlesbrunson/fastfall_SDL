#include "fastfall/resource/Asset.hpp"

namespace ff {

Asset::Asset(const std::filesystem::path& t_asset_path)
    : asset_path(t_asset_path)
    , asset_name(t_asset_path.filename().generic_string())
    , ImGuiContent(ImGuiContentType::NONE, t_asset_path.generic_string())
{
};

}