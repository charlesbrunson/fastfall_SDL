#include "fastfall/render/drawable/AnimatedSprite.hpp"

#include "fastfall/resource/Resources.hpp"

#include "fastfall/util/log.hpp"
#include "fastfall/engine/time/time.hpp"

#include "fastfall/render/target/RenderTarget.hpp"

namespace ff {

void AnimatedSprite::reset() {
    hflip = false;
    curr_anim = {};
    sprite    = {};
    loop_counter   = 0;
    current_frame  = 0;
    playback_speed = 1.f;
    time_buffer         = 0.0;
    curr_frame_duration = 0.0;
    animation  = nullptr;
    flag_dirty = false;
}

bool AnimatedSprite::set_anim_if_not(AnimID id, bool reset, std::optional<unsigned> start_frame) noexcept {
	if (!is_playing(id)) {
		return set_anim(id, reset, start_frame);
	}
	return false;
}

bool AnimatedSprite::set_anim(AnimID id, bool reset, std::optional<unsigned> start_frame) noexcept {

	const Animation* anim = AnimDB::get_animation(id);

	animation = anim;
	loop_counter = 0;

	if (reset) {
        restart_anim();
	}
	else if (animation && current_frame >= animation->framerateMS.size()) {
		current_frame = current_frame % animation->framerateMS.size();
	}

    if (start_frame)
        set_frame(*start_frame);

	flag_dirty = true;
	return (animation != nullptr);
}

void AnimatedSprite::restart_anim(bool reset_time_buffer) {
	current_frame = 0;
	playback_speed = 1.f;

	if (reset_time_buffer) {
		time_buffer = 0.0;
		loop_counter = 0;
	}

	curr_frame_duration = has_anim() && !animation->framerateMS.empty()
        ? ms_to_secs(animation->framerateMS.front())
        : 0.0;

	flag_dirty = true;
}

void AnimatedSprite::update(secs deltaTime)
{

	if (!animation || deltaTime <= 0.0 || playback_speed == 0.f) {
		return;
	}

	time_buffer += (deltaTime * playback_speed);

	while (time_buffer >= curr_frame_duration) {

		time_buffer -= curr_frame_duration;
		current_frame++;

		// completed animation
		if (current_frame >= animation->framerateMS.size()) {

			loop_counter++;

			if (animation->loop == 0 || loop_counter < animation->loop) {
                restart_anim(false);
			}
			else if (loop_counter >= animation->loop && animation->chain.has_chain) {

				AnimID next = animation->chain.anim_id;
				set_anim(animation->chain.anim_id, false);
				if (!animation) {
					LOG_WARN("Couldn't set_anim for animation id {}!", next.get());
					return;
				}

				current_frame = animation->chain.start_frame;


				if (!animation->framerateMS.empty()) {
					curr_frame_duration = ms_to_secs(animation->framerateMS.at(current_frame));
				}
				else {
					curr_frame_duration = 0.0;
					playback_speed = 0.f;
				}

			}
			else {
				current_frame = animation->framerateMS.size() - 1;
				playback_speed = 0.f;
			}
		}
		// set up for next frame of animation
		else {
			curr_frame_duration = ms_to_secs(animation->framerateMS.at(current_frame));
		}

		flag_dirty = true;
	}
}

void AnimatedSprite::predraw(predraw_state_t predraw_state)
{
	if (animation) {
		if (flag_dirty) {
			if (curr_anim != animation->anim_id) {
				curr_anim = animation->anim_id;
				sprite.setTexture(&animation->get_sprite_texture());
			}

			sprite.setScale(Vec2f{ (hflip ? -1.f : 1.f), 1.f });
			sprite.setOrigin(glm::fvec2(animation->origin.x, animation->origin.y));
			sprite.setColor(Color::White);
			flag_dirty = false;
		}

        Rectf area = Rectf(
                animation->area.getPosition(),
                animation->area.getSize()
        );

        secs time_buffer_interp = time_buffer + (predraw_state.update_dt * predraw_state.interp);
        unsigned int next_frame = time_buffer_interp >= curr_frame_duration ? 1 : 0;

        area.left += area.width * (float)std::min(current_frame + next_frame, (unsigned)animation->framerateMS.size() - 1);
        sprite.setTextureRect(area);
        sprite.setSize(area.getSize());

		sprite.setPosition(position);
	}
	else {
		// reset sprite
		sprite = Sprite{};
	}
}

bool AnimatedSprite::is_complete() const noexcept {
	return animation != nullptr && loop_counter >= animation->loop;
}

bool AnimatedSprite::is_complete(AnimID id) const noexcept {
	return curr_anim == id && is_complete();
}


bool AnimatedSprite::is_complete_any(std::vector<AnimID> ids) const noexcept {
	for (auto id : ids) {
		if (is_complete(id)) 
			return true;
	}
	return false;
}

bool AnimatedSprite::is_playing() const noexcept
{
	return animation != nullptr && playback_speed > 0.f;
}

bool AnimatedSprite::is_playing(AnimID id, unsigned incl_chain_anims_depth) const noexcept
{	
	if (!animation)
		return false;

	const Animation* anim = animation; // start with current animation
	for (unsigned i = 0; i <= incl_chain_anims_depth; i++) {
		if (anim->anim_id == id) {
			return true;
		}
		else if (anim->chain.has_chain){
			if (i < incl_chain_anims_depth) {
				if (anim->chain.anim_id == id) {
					return true;
				}
				else {
					anim = AnimDB::get_animation(anim->chain.anim_id);
				}
			}
		}
		else {
			break;
		}
	}
	return false;
}



bool AnimatedSprite::is_playing_any(std::vector<AnimID> ids, unsigned incl_chain_anims_depth) const noexcept
{
	for (auto id : ids) {
		if (is_playing(id, incl_chain_anims_depth)) 
			return true;
	}
	return false;
}

void AnimatedSprite::draw(RenderTarget& target, RenderState states) const
{
	if (animation) {
		target.draw(sprite, states);
	}
}

}