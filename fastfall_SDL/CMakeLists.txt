# CMakeList.txt : CMake project for fastfall_SDL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


add_executable (fastfall_SDL 
	"main.cpp" 
	"render/Texture.hpp" 
	"render/Sprite.hpp" 
	"render/Window.hpp" 
	"render/RenderTexture.hpp" 
	"render/render.hpp" 
	"render/render.cpp" 
	"render/detail/error.hpp" 
	"render/detail/error.cpp" 
	"render/Color.hpp" 
	"render/Window.cpp" 
	"render/Color.cpp" 
	"render/View.hpp"  
	"render/Shader.hpp"
	"render/Shader.cpp" 
	"render/Transform.hpp" 
	"render/View.cpp" 
	"render/Drawable.hpp" 
	"render/Texture.cpp" 
	"render/RenderState.hpp" 
	"render/RenderState.cpp" 
	"render/RenderTarget.hpp" 
	"render/RenderTarget.cpp" 
	"render/Vertex.hpp" 
	"render/Primitives.hpp" 
	"render/Transformable.hpp" 
	"render/Transformable.cpp" 
	"render/VertexArray.hpp" 
	"render/VertexArray.cpp"
	"render/shapes/ShapeRectangle.hpp" 
	"render/shapes/ShapeRectangle.cpp"
	"render/shapes/ShapeLine.hpp"
	"render/shapes/ShapeLine.cpp"
	"render/shapes/ShapeCircle.hpp" 
	"render/shapes/ShapeCircle.cpp"  
	"render/RenderTexture.cpp" 
	"util/Rect.hpp" 
	"render/Sprite.cpp" 
	"render/Transform.cpp" 
	 )

target_include_directories(fastfall_SDL PRIVATE "${glew_SOURCE_DIR}/include")
target_include_directories(fastfall_SDL PRIVATE "${sdl_SOURCE_DIR}/include")
target_include_directories(fastfall_SDL PRIVATE "${sdl_image_SOURCE_DIR}")
target_include_directories(fastfall_SDL PRIVATE "${PROJECT_SOURCE_DIR}/external/imgui")
target_include_directories(fastfall_SDL PRIVATE "${PROJECT_SOURCE_DIR}/external/imgui")

target_link_libraries(fastfall_SDL PRIVATE 
	glm
	SDL2
	SDL2main
	SDL2image
	glew
	flatbuffers
	fmt::fmt
	imgui
)