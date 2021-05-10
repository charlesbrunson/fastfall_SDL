#include "render/Transformable.hpp"

namespace ff {

Transformable::Transformable()
{
}

Transformable::~Transformable()
{
}

void Transformable::setPosition(float posX, float posY)
{
	m_transform.setPosition({ posX, posY });
}

void Transformable::setPosition(const glm::fvec2& pos)
{
	m_transform.setPosition(pos);
}

void Transformable::setRotation(float angle)
{
	m_transform.setRotation(angle);
}

void Transformable::setScale(float factor)
{
	m_transform.setScale(factor);
}
void Transformable::setScale(float factorX, float factorY)
{
	m_transform.setScale({factorX, factorY});
}
void Transformable::setScale(const glm::fvec2& factor)
{
	m_transform.setScale(factor);
}

void Transformable::setOrigin(float posX, float posY)
{
	m_transform.setOrigin({ posX, posY });
}
void Transformable::setOrigin(const glm::fvec2& pos)
{
	m_transform.setOrigin(pos);
}

const glm::fvec2& Transformable::getPosition() const
{
	return m_transform.getPosition();
}
float Transformable::getRotation() const
{
	return m_transform.getRotation();
}
const glm::fvec2& Transformable::getScale() const
{
	return m_transform.getScale();
}
const glm::fvec2& Transformable::getOrigin() const
{
	return m_transform.getOrigin();
}

void Transformable::move(float offsetX, float offsetY)
{
	m_transform.setPosition(m_transform.getPosition() + glm::fvec2{ offsetX, offsetY });
}
void Transformable::move(const glm::fvec2& offset)
{
	m_transform.setPosition(m_transform.getPosition() + offset);
}

void Transformable::rotate(float angle)
{
	m_transform.setRotation(m_transform.getRotation() + angle);
}

void Transformable::magnify(float factor)
{
	m_transform.setScale(m_transform.getScale() * factor);
}
void Transformable::magnify(float factorX, float factorY)
{
	m_transform.setScale(m_transform.getScale() * glm::fvec2{ factorX, factorY });
}
void Transformable::magnify(const glm::fvec2& factor)
{
	m_transform.setScale(m_transform.getScale() * factor);
}

const Transform& Transformable::getTransform() const
{
	return m_transform;
}
const Transform& Transformable::getInvTransform() const
{
	Transform inv;
	inv.setPosition(-1.f * m_transform.getPosition());
	inv.setOrigin(-1.f * m_transform.getOrigin());
	inv.setScale(1.f / m_transform.getScale());
	inv.setRotation(-1.f * m_transform.getRotation());
	return inv;
}

}