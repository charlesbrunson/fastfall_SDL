#include "fastfall/resource/asset/AnimAssetTypes.hpp"

#include "fastfall/resource/Resources.hpp"

namespace ff {

AnimIDRef::AnimIDRef(std::string_view sprite, std::string_view anim)
	: m_sprite(sprite), m_anim(anim)
{
}

AnimID AnimIDRef::id() const
{
	if (m_id == AnimID::NONE && !m_sprite.empty() && !m_anim.empty()) {
		m_id = Resources::get_animation_id(m_sprite, m_anim);
	}
	return m_id;
};

AnimID AnimID::NONE = AnimID{ 0 };
unsigned int AnimID::counter = 0;

}