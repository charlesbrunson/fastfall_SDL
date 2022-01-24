#include "fastfall/render/Text.hpp"


namespace ff {


	Text::Text()
		: m_varr(Primitive::TRIANGLES, 0)
	{

	}

	Text::Text(const Font& font)
		: m_varr(Primitive::TRIANGLES, 0)
	{
		set(font, {});
	}

	Text::Text(const Font& font, std::string_view text)
		: m_varr(Primitive::TRIANGLES, 0)
	{
		set(font, text);
	}

	void Text::set(std::optional<std::reference_wrapper<const Font>> font, std::optional<std::string_view> text)
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

		if (update)
		{
			update_varray();
		}
	}

	void Text::clear()
	{
		m_text = {};
		m_varr = { Primitive::TRIANGLES, 0 };
	}


	void Text::set_color(Color color)
	{
		for (size_t i = 0; i < m_varr.size(); i++)
		{
			m_varr[i].color = color;
		}
		m_color = color;
	}

	void Text::update_varray()
	{
		if (m_font && !m_text.empty())
		{
			m_varr = { Primitive::TRIANGLES, 6 * m_text.size() };
			bounding_size = { 0, 0, 0, 0 };

			size_t ndx = 0;

			glm::fvec2 pen = {0, m_font->getYMax() };

			std::optional<char> prev;

			for (auto ch : m_text)
			{
				if (ch >= 128)
					continue;

				if (ch == '\n') {
					pen = { 0, pen.y + (m_font->getHeight() * v_spacing) };
					prev = {};
					continue;
				}

				auto& metrics = m_font->getMetrics(ch);

				Vec2i kern;
				if (prev)
					kern = m_font->getKerning(*prev, ch);


				Rectf draw_rect{
					pen.x + kern.x + metrics.bearing.x,
					pen.y + kern.y - metrics.bearing.y,
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

				m_varr[ndx + 0] = Vertex
				{
					math::rect_topleft(draw_rect),
					m_color,
					math::rect_topleft(tex_rect),
				};
				m_varr[ndx + 1] = Vertex
				{
					math::rect_topright(draw_rect),
					m_color,
					math::rect_topright(tex_rect),
				};
				m_varr[ndx + 2] = Vertex
				{
					math::rect_botleft(draw_rect),
					m_color,
					math::rect_botleft(tex_rect),
				};
				m_varr[ndx + 3] = Vertex
				{
					math::rect_topright(draw_rect),
					m_color,
					math::rect_topright(tex_rect),
				};
				m_varr[ndx + 4] = Vertex
				{
					math::rect_botleft(draw_rect),
					m_color,
					math::rect_botleft(tex_rect),
				};
				m_varr[ndx + 5] = Vertex
				{
					math::rect_botright(draw_rect),
					m_color,
					math::rect_botright(tex_rect),
				};

				pen.x += metrics.advance_x;
				ndx += 6;

				prev = ch;

				bounding_size = math::rect_bound(Rectf{ 
						(float)pen.x + metrics.bearing.x, 
						(float)pen.y - m_font->getYMax(),
						(float)metrics.size.x,
						(float)m_font->getHeight()
					}, bounding_size);
			}
		}
		else {
			clear();
		}
	}

	void Text::draw(RenderTarget& target, RenderState state) const
	{
		state.transform = Transform::combine(state.transform, getTransform());
		state.texture	= m_font ? m_font->getBitmapTex() : TextureRef{};
		target.draw(m_varr, state);
	}

}