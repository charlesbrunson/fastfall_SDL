#pragma once

#include "Transform.hpp"

namespace ff {

class Transformable {
public:
	Transformable();
	virtual ~Transformable();

	void setPosition(float posX, float posY);
	void setPosition(const glm::fvec2& pos);

	void setRotation(float angle);

	void setScale(float factor);
	void setScale(float factorX, float factorY);
	void setScale(const glm::fvec2& factor);

	void setOrigin(float posX, float posY);
	void setOrigin(const glm::fvec2& pos);

	const glm::fvec2& getPosition() const;
	float getRotation() const;
	const glm::fvec2& getScale() const;
	const glm::fvec2& getOrigin() const;

	void move(float offsetX, float offsetY);
	void move(const glm::fvec2& offset);

	void rotate(float angle);

	void magnify(float factor);
	void magnify(float factorX, float factorY);
	void magnify(const glm::fvec2& factor);

	Transform getTransform() const;
	Transform getInvTransform() const;

private:
	Transform m_transform;


};

}