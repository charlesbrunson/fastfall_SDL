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

	auto iter = std::partition(tile_timers.begin(), tile_timers.end(), [deltaTime](TileTimer& timer) {
		timer.time_to_anim -= deltaTime;
		return timer.time_to_anim > 0.0;
		});

	if (iter != tile_timers.end()) {
		for (auto timer = iter; timer != tile_timers.end(); timer++) {

			Vec2u tex_pos;
			Tile& tile = timer->tile;
			const TilesetAsset* next = tile.origin;

			if (tile.has_next_tileset()) {
				std::string_view next_name = tile.origin->getTilesetRef(tile.next_tileset);
				next = Resources::get<TilesetAsset>(next_name);

				tex_pos = Vec2u{ tile.next_offset };
			}
			else {
				tex_pos = tile.pos + tile.next_offset;
			}

			bool nLogic = true;
			if (auto logic = next->getTileLogic(tex_pos); logic && logic->logicType == getName()) {
				timer->time_to_anim += ms_to_secs(std::stoi(logic->logicArg));
				timer->tile = next->getTile(tex_pos);
				nLogic = false;
			}

			pushCommand({
				.type = TileLogicCommand::Type::Set,
				.position = timer->tile_impacted,
				.texposition = tex_pos,
				.tileset = std::ref(*next),
				.updateLogic = nLogic
			});
		}

		// update partition
		iter = std::partition(tile_timers.begin(), tile_timers.end(), [](TileTimer& timer) {
			return timer.time_to_anim > 0.0;
			});
		tile_timers.erase(iter, tile_timers.end());
	}
}



