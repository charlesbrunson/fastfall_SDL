#pragma once

//#include ""
#include "fastfall/resource/asset/SpriteAsset.hpp"
#include "fastfall/engine/time/time.hpp"

#include "fastfall/resource/Resources.hpp"

#include "Sprite.hpp"

#include <algorithm>

namespace ff {

class AnimatedSprite : public Drawable {
public:

	AnimatedSprite() :
		curr_anim{ AnimID{} },
		sprite{nullptr}
	{

	}

    void reset();
	bool set_anim(AnimID id, bool reset = true, std::optional<unsigned> start_frame = {}) noexcept;
	bool set_anim_if_not(AnimID id, bool reset = true, std::optional<unsigned> start_frame = {}) noexcept;

	void restart_anim(bool reset_time_buffer = true);

	bool has_anim() noexcept { return animation != nullptr; };
	const Animation* get_anim() { return animation; };

	std::string anim_name()  noexcept { return has_anim() ? animation->anim_name : ""; };

	void set_frame(unsigned frame) { current_frame = frame; };
	unsigned	get_frame()  noexcept { return current_frame; };

	void update(secs deltaTime) override;
	void predraw(predraw_state_t predraw_state) override;

	bool is_complete() const noexcept;
	bool is_complete(AnimID id) const noexcept;
	bool is_complete_any(std::vector<AnimID> ids) const noexcept;

	bool is_playing() const noexcept;
	bool is_playing(AnimID id, unsigned incl_chain_anims_depth = 1) const noexcept;
	bool is_playing_any(std::vector<AnimID> ids, unsigned incl_chain_anims_depth = 1) const noexcept;
	
	void  set_pos(Vec2f pos) noexcept { position = pos; };
	Vec2f get_pos()          noexcept { return position; };

	bool get_hflip()	const noexcept {
		return hflip;
	};
	void set_hflip(bool flipped) noexcept {
		if (hflip != flipped) {
			hflip = flipped;
			flag_dirty = true;
		}
	};


	void set_playback(float rate) noexcept {
		playback_speed = (std::max)(0.f, rate);
	}
	float get_playback() noexcept {
		return playback_speed;
	}

	Sprite& get_sprite() {
		return sprite;
	}


private:

	void draw(RenderTarget& target, RenderState states = RenderState{}) const override;

	Vec2f position = {};

	bool hflip = false;

	AnimID curr_anim;

	Sprite sprite;

	unsigned int loop_counter = 0;
	unsigned int current_frame = 0;
	float playback_speed = 1.f;

	secs time_buffer = 0.0;
	secs curr_frame_duration = 0.0;

	const Animation* animation = nullptr;
	bool flag_dirty = false;

};

}