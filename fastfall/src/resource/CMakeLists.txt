cmake_minimum_required (VERSION 4.0)

target_sources(fastfall PRIVATE
    Resources.cpp
    ResourceWatcher.cpp
    ResourceSubscriber.cpp
    Asset.cpp
    asset/TileLayerData.cpp
    asset/AnimAsset.cpp
    asset/FontAsset.cpp
    asset/LevelAsset.cpp
    asset/SpriteAsset.cpp
    asset/TilesetAsset.cpp
    asset/TextureAsset.cpp
    asset/AnimAssetTypes.cpp
    asset/MusicAsset.cpp
    asset/SoundAsset.cpp
    asset/ShaderAsset.cpp
    asset/leveltypes/TileTMX.hpp
    asset/leveltypes/ObjectTMX.cpp
    asset/leveltypes/TileTMX.cpp
    asset/leveltypes/TilesetTMX.hpp
    asset/leveltypes/ObjectTMX.hpp
    asset/leveltypes/TilesetTMX.cpp
)