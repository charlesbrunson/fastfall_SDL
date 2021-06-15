#include "AnimLogic.hpp"

#include "fastfall/resource/Resources.hpp"

using namespace ff;

TileLogicType type = {
	"anim",
	[](GameContext context) -> std::unique_ptr<TileLogic> {
		return std::make_unique<AnimLogic>(context);
	}
};

void AnimLogic::addTile(Vec2u tilePos, Tile tile, std::string arg) {
	tile_timers.push_back(TileTimer{});
	tile_timers.back().time_to_anim = ms_to_secs(std::stoi(arg));
	tile_timers.back().tile_impacted = tilePos;
	tile_timers.back().tile = tile;
}

void AnimLogic::update(secs deltaTime) {

	bool changedTile = false;

	// update tile timers
	for (auto& timer : tile_timers) {
		timer.time_to_anim -= deltaTime;

		if (timer.time_to_anim < 0.0) {
			changedTile = true;
		}
	}

	// update tiles
	if (changedTile) {
		auto iter = std::partition(tile_timers.begin(), tile_timers.end(), [](const TileTimer& timer) {
			return timer.time_to_anim > 0.0;
			});

		if (iter != tile_timers.end()) {
			//std::vector<TileTimer> copy_end{ iter, tile_timers.end() };

			//tile_timers.erase(iter, tile_timers.end());

			for (auto timer = iter; timer != tile_timers.end(); timer++) {

				Vec2u tex_pos;
				const TilesetAsset* next = timer->tile.origin;

				if (timer->tile.has_next_tileset()) {
					std::string_view next_name = timer->tile.origin->getTilesetRef(timer->tile.next_tileset);
					next = Resources::get<TilesetAsset>(next_name);

					// if different tileset, use as absolute
					tex_pos = Vec2u{ timer->tile.next_offset };
				}
				else {
					// if same tileset, use as relative
					tex_pos = timer->tile.pos + timer->tile.next_offset;
				}

				pushCommand({
					.type = TileLogicCommand::Type::Set,
					.position = timer->tile_impacted,
					.texposition = tex_pos,
					.tileset = std::ref(*next)
					});

			}
			tile_timers.erase(iter, tile_timers.end());
		}
	}
}



