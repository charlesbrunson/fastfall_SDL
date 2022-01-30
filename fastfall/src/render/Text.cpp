#include "fastfall/render/Text.hpp"
#include "fastfall/util/log.hpp"

namespace ff {
	Text::Text()
		: m_varr(Primitive::TRIANGLES, 0)
	{

	}

	Text::Text(Font& font)
		: m_varr(Primitive::TRIANGLES, 0)
	{
		setText(font, {}, {});
	}

	Text::Text(Font& font, unsigned size)
		: m_varr(Primitive::TRIANGLES, 0)
	{
		setText(font, size, {});
	}

	Text::Text(Font& font, unsigned size, std::string_view text)
		: m_varr(Primitive::TRIANGLES, 0)
	{
		setText(font, size, text);
	}

	void Text::setText(std::optional<const_font_ref> font, std::optional<unsigned> pixel_size, std::optional<std::string_view> text)
	{
		bool update = false;

		if (font && m_font != &font.value().get())
		{
			m_font = &font.value().get();
			update = true;
		}

		if (text && *text != m_text)
		{
			m_text = *text;
			update = true;
		}

		if (pixel_size && *pixel_size != px_size)
		{
			px_size = *pixel_size;
			update = true;
		}

		if (update)
		{
			update_varray();
		}
	}

	void Text::clear()
	{
		m_text = {};
		m_varr.clear();
		bitmap_texture = {};
	}


	void Text::setColor(Color color)
	{
		for (size_t i = 0; i < m_varr.size(); i++)
		{
			m_varr[i].color = color;
		}
		m_color = color;
	}

	void Text::update_varray()
	{

		if (m_font 
			&& px_size > 0 
			&& !m_text.empty()
			&& m_font->setPixelSize(px_size))
		{
			if (!m_font->setPixelSize(px_size))
			{
				clear();
				return;
			}

			size_t vertex_count = 6 * m_text.size();
			if (m_varr.size() < vertex_count) {
				m_varr.insert(m_varr.size(), vertex_count - m_varr.size(), Vertex{});
			}
			else if (m_varr.size() > vertex_count) {
				m_varr.erase(vertex_count, m_varr.size() - vertex_count);
			}

			bounding_size = { 0, 0, 0, 0 };

			size_t ndx = 0;

			glm::fvec2 pen = {0, m_font->getYMax() };

			for (auto ch : m_text)
			{
				if (ch >= 128)
					continue;

				if (ch == '\n') {
					pen = { 0, pen.y + (m_font->getHeight() * v_spacing) };
					continue;
				}

				if (ch == '\t') {
					unsigned space_width = m_font->getMetrics(' ').advance_x;

					float x = 0.f;
					while (x <= pen.x)
					{
						x += 4.f * space_width;
					}
					pen.x = x;

					//pen.x = pen.x /
					continue;

				}

				auto& metrics = m_font->getMetrics(ch);

				Rectf draw_rect{
					pen.x + metrics.bearing.x,
					pen.y - metrics.bearing.y,
					(float)metrics.size.x,
					(float)metrics.size.y
				};

				glm::fvec2 size = { 
					m_font->getBitmapTex().inverseSize().x * m_font->getGlyphSize().x,
					m_font->getBitmapTex().inverseSize().y * m_font->getGlyphSize().y
				};

				Rectf tex_rect{
					(float)(ch % 16) * size.x,
					(float)(ch / 16) * size.y,
					(float)metrics.size.x * m_font->getBitmapTex().inverseSize().x,
					(float)metrics.size.y * m_font->getBitmapTex().inverseSize().y
				};

				m_varr[ndx + 0] = Vertex{
					math::rect_topleft(draw_rect),
					m_color,
					math::rect_topleft(tex_rect),
				};
				m_varr[ndx + 1] = Vertex{
					math::rect_topright(draw_rect),
					m_color,
					math::rect_topright(tex_rect),
				};
				m_varr[ndx + 2] = Vertex{
					math::rect_botleft(draw_rect),
					m_color,
					math::rect_botleft(tex_rect),
				};
				m_varr[ndx + 3] = Vertex{
					math::rect_topright(draw_rect),
					m_color,
					math::rect_topright(tex_rect),
				};
				m_varr[ndx + 4] = Vertex{
					math::rect_botleft(draw_rect),
					m_color,
					math::rect_botleft(tex_rect),
				};
				m_varr[ndx + 5] = Vertex{
					math::rect_botright(draw_rect),
					m_color,
					math::rect_botright(tex_rect),
				};

				pen.x += metrics.advance_x;
				ndx += 6;

				bounding_size = math::rect_bound(Rectf{
						(float)pen.x + metrics.bearing.x,
						(float)pen.y - m_font->getYMax(),
						(float)metrics.size.x,
						(float)m_font->getYMax() - m_font->getYMin()
					}, bounding_size);

				bitmap_texture = m_font->getBitmapTex();
			}

			
			ndx++;
			for (; ndx < m_varr.size(); ndx++)
			{
				m_varr[ndx] = Vertex{};
			}
			
		}
		else {
			clear();
		}
	}

	void Text::draw(RenderTarget& target, RenderState state) const
	{
		if (m_font && !bitmap_texture.exists())
		{
			m_font->loadBitmapTex(px_size);
		}

		state.transform = Transform::combine(state.transform, getTransform());
		state.texture	= bitmap_texture;
		target.draw(m_varr, state);
	}

}