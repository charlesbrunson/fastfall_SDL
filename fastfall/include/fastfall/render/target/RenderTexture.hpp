#pragma once

#include "RenderTarget.hpp"

namespace ff {

class RenderTexture : public RenderTarget {
public:

	bool create(glm::ivec2 size);
	bool create(int sizeX, int sizeY);

	View getDefaultView() const override;

	const Texture* getTexture() const;

	Vec2i getSize() const override { return m_size; };

private:
	bool m_valid = false;
	Vec2i m_size;

	Texture m_tex;
};

}