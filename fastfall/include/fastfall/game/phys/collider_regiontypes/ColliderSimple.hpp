
#pragma once

#include "fastfall/game/phys/ColliderRegion.hpp"
#include "fastfall/util/math.hpp"

// collider region containing a single collider

namespace ff {

class ColliderSimple : public ColliderRegion {
public:
	ColliderSimple(Rectf shape) :
		ColliderRegion{},
		quad(shape)
	{

		boundingBox = shape;

		//surf = sf::VertexArray(sf::PrimitiveType::Quads, 16);

		quad.setID(0u);

		//render collision surfaces
		/*
		auto normal = [](Line<float> line) -> Vec2f {
			Vec2f v = (line.p2 - line.p1);

			v /= sqrtf(v.x * v.x + v.y * v.y);

			std::swap(v.x, v.y);
			v.y *= -1.f;

			return v;
		};

		constexpr static unsigned colors[4] = {
			0xFF0000FF, // red, north
			0x00FF00FF, // green, east
			0x0000FFFF, // blue, south
			0xFFFF00FF // yellow, west
		};

		for (int i = 0; i < 4; i++) {

			const ColliderSurface* s = quad.getSurface((Cardinal)i);

			if (s) {

				sf::Vector2f n = normal(Linef(Vec2f(s->surface.p1), Vec2f(s->surface.p2)));
				sf::Color c = sf::Color(colors[i]);

				surf[(i * 4)].color = c;
				surf[(i * 4)].position = Vec2f(s->surface.p1);

				surf[(i * 4) + 1].color = c;
				surf[(i * 4) + 1].position = Vec2f(s->surface.p2);

				c.a = 128;

				surf[(i * 4) + 2].color = c;
				surf[(i * 4) + 2].position = s->surface.p2;
				surf[(i * 4) + 2].position -= n;

				surf[(i * 4) + 3].color = c;
				surf[(i * 4) + 3].position = s->surface.p1;
				surf[(i * 4) + 3].position -= n;
			}
		}
		*/

	}

	void update(secs deltaTime) override {
		debugDrawQuad(quad, getPosition(), this);
	}
	const ColliderQuad* get_quad(int quad_id) const noexcept override {
		return quad_id == 0u ? &quad : nullptr;
	}

	void getQuads(Rectf area, std::vector<std::pair<Rectf, const ColliderQuad*>>& buffer) const override {
		//std::vector<std::pair<Rectf, const ColliderQuad*>> r;

		Rectf box = boundingBox;
		box.left += getPosition().x;
		box.top += getPosition().y;


		// Compute the min and max of the first rectangle on both axes
		float r1MinX = std::min(box.left, (box.left + box.width));
		float r1MaxX = std::max(box.left, (box.left + box.width));
		float r1MinY = std::min(box.top, (box.top + box.height));
		float r1MaxY = std::max(box.top, (box.top + box.height));

		// Compute the min and max of the second rectangle on both axes
		float r2MinX = std::min(area.left, (area.left + area.width));
		float r2MaxX = std::max(area.left, (area.left + area.width));
		float r2MinY = std::min(area.top, (area.top + area.height));
		float r2MaxY = std::max(area.top, (area.top + area.height));

		// Compute the intersection boundaries
		float interLeft = std::max(r1MinX, r2MinX);
		float interTop = std::max(r1MinY, r2MinY);
		float interRight = std::min(r1MaxX, r2MaxX);
		float interBottom = std::min(r1MaxY, r2MaxY);

		if ((interLeft <= interRight) && (interTop <= interBottom)) {
			buffer.push_back(std::make_pair(box, &quad));
		}
		//return r;
	}

private:

	/*
	void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates()) const override {
		states.transform.translate(getPrevPosition());
		target.draw(surf, states);
	}
	*/

	ColliderQuad quad;
	//sf::VertexArray surf;
};

}