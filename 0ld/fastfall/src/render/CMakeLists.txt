cmake_minimum_required (VERSION 3.16)

target_sources(fastfall PRIVATE
    render.cpp
    DebugDraw.cpp
    external/freetype.cpp
    drawable/ChunkVertexArray.cpp
    drawable/Sprite.cpp
    drawable/ShapeLine.cpp
    drawable/TileArray.cpp
    drawable/VertexArray.cpp
    drawable/TileVertexArray.cpp
    drawable/Text.cpp
    drawable/ShapeCircle.cpp
    drawable/AnimatedSprite.cpp
    drawable/ShapeRectangle.cpp
    detail/error.hpp
    detail/error.cpp
    target/RenderTexture.cpp
    target/Window.cpp
    target/RenderTarget.cpp
    util/RenderState.cpp
    util/Transform.cpp
    util/View.cpp
    util/Shader.cpp
    util/Texture.cpp
    util/Font.cpp
    util/Transformable.cpp
)