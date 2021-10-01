#pragma once

//#include "util/Updatable.hpp"
#include "fastfall/engine/time/time.hpp"
#include "level/LevelLayerContainer.hpp"

#include "fastfall/resource/asset/LevelAsset.hpp"

#include "fastfall/render/Drawable.hpp"

#include <list>
#include <concepts>

namespace ff {

class LevelEditor;

class Level  {
public:
	Level(GameContext context);
	Level(GameContext context, const LevelAsset& levelData);

	void init(const LevelAsset& levelData);

	void update(secs deltaTime);

	void predraw(secs deltaTime);

	inline const Color& getBGColor() const { return bgColor; };
	inline const Vec2u& size() const { return levelSize; };
	inline const std::string& name() const { return levelName; };

	GameContext getContext() { return context; }
	inline InstanceID getInstanceID() { return context.getID(); };

	bool is_attached(LevelEditor* editor) { return editor == attached_editor; };

	bool attach(LevelEditor* editor) { 
		bool already_attached = is_attached(editor);
		attached_editor = editor; 
		return already_attached;
	};

	void detach(LevelEditor* editor) { 
		if (is_attached(editor)) 
		{ 
			attached_editor = nullptr; 
		} 
	};

	void resize(Vec2u n_size);
	void set_name(std::string name) { levelName = name; };
	void set_bg_color(Color color) { bgColor = color; };

	LevelLayerContainer& get_layers() { return layers; };
	const LevelLayerContainer& get_layers() const { return layers; };

	bool hasEditorHooked = false;

private:
	const LevelEditor* attached_editor = nullptr;

	GameContext context;

	std::string levelName;
	Color bgColor;
	Vec2u levelSize;

	LevelLayerContainer layers;

};

}
