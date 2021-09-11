#include "fastfall/game/level/LevelEditor.hpp"

namespace ff {

[[nodiscard]]
LevelEditor&& LevelEditor::attach(Level* lvl) 
{
	LevelEditor editor{};
	editor.level = lvl;
	return std::move(editor);
}

[[nodiscard]]
LevelEditor&& LevelEditor::create_level(GameContext context, std::string name, Vec2u tile_size)
{
	assert(tile_size.x >= LevelEditor::MIN_LEVEL_SIZE.x);
	assert(tile_size.y >= LevelEditor::MIN_LEVEL_SIZE.y);

	LevelEditor editor{};
	editor.created_level = std::make_unique<Level>(context);
	editor.level = editor.created_level.get();

	return std::move(editor);
}





















}