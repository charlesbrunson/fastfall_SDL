#include "fastfall/resource/asset/AnimAssetTypes.hpp"

#include "fastfall/resource/Resources.hpp"

namespace ff {


AnimID AnimIDRef::id() const
{
	if (m_id == AnimID::NONE && !m_sprite.empty() && !m_anim.empty()) {
		m_id = AnimDB::get_animation_id(m_sprite, m_anim);
	}
	return m_id;
};

AnimID AnimID::NONE = AnimID{ 0 };
unsigned int AnimID::counter = 0;

}