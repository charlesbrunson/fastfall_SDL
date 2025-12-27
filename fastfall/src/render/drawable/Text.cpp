#include "fastfall/render/drawable/Text.hpp"
#include "fastfall/util/log.hpp"

#include "GL/glew.h"
#include "../detail/error.hpp"
#include "fastfall/render/render.hpp"

namespace ff {
	Text::Text()
	{

	}

	Text::Text(Font& font)
	{
		setText(font, {}, {});
	}

	Text::Text(Font& font, unsigned size)
	{
		setText(font, size, {});
	}

	Text::Text(Font& font, unsigned size, std::string_view text)
	{
		setText(font, size, text);
	}

	void Text::setText(std::optional<const_font_ref> font, std::optional<unsigned> pixel_size, std::optional<std::string_view> text)
	{
		bool update = false;

		if (font && m_font != &font.value().get())
		{
			m_font = &font.value().get();
			gl_text_fresh = false;
		}

		if (text && *text != m_text)
		{
			m_text = *text;
			gl_text_fresh = false;
		}

		if (pixel_size && *pixel_size != px_size)
		{
			px_size = *pixel_size;
			gl_text_fresh = false;
		}

	}

	void Text::clear()
	{
		m_text = {};
		gl_text.clear();
		bitmap_texture = TextureRef{};
	}


	void Text::setColor(Color color)
	{

		for (size_t i = 0; i < gl_text.size(); i++)
		{
			gl_text[i].color = color;
		}
		m_color = color;
		gl.sync = false;
	}


	void Text::predraw() {
		if (!gl_text_fresh) {
			gl_text_fresh = true;
			update_text();
		}
	}

	void Text::update_text()
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

			gl_text.clear();

			bounding_size = { 0, 0, 0, 0 };

			size_t ndx = 0;

			glm::fvec2 pen = {0, m_font->getYMax() };

			for (unsigned char ch : m_text)
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
					while (x < pen.x + space_width)
					{
						x += 4.f * space_width;
					}
					pen.x = x;

					//pen.x = pen.x /
					continue;

				}

				if (ch == ' ') {
					unsigned space_width = m_font->getMetrics(' ').advance_x;
					pen.x += space_width;
					continue;
				}

				auto& metrics = m_font->getMetrics(ch);

				gl_text.push_back({
					.offset = {
						pen.x + metrics.bearing.x,
						pen.y - metrics.bearing.y
					},
					.color = m_color,
					.character = ch,
					});

				pen.x += metrics.advance_x;

				bounding_size = math::rect_bounds(Rectf{
						(float)pen.x + metrics.bearing.x,
						(float)pen.y - m_font->getYMax(),
						(float)metrics.size.x,
						(float)m_font->getYMax() - m_font->getYMin()
					}, bounding_size);

			}

			bitmap_texture = m_font->getBitmapTex();
			
		}
		else {
			clear();
		}
		gl.sync = false;
	}

	void Text::glTransfer() const {

		if (gl.m_array == 0) {
			// do the opengl initializaion
			glCheck(glGenVertexArrays(1, &gl.m_array));
			glCheck(glGenBuffers(1, &gl.m_buffer));

			glCheck(glBindVertexArray(gl.m_array));
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl.m_buffer));
			glCheck(glBufferData(GL_ARRAY_BUFFER, gl_text.size() * sizeof(glChar), NULL, GL_DYNAMIC_DRAW));
			gl.m_bufsize = gl_text.size();

			size_t position = 0lu;

			// offset
			glCheck(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glChar), (void*)position));
			glCheck(glEnableVertexAttribArray(1));
			glCheck(glVertexAttribDivisor(1, 1)); // one per instance
			position += sizeof(Vec2f);

			// color
			glCheck(glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(glChar), (void*)position));
			glCheck(glEnableVertexAttribArray(2));
			glCheck(glVertexAttribDivisor(2, 1)); // one per instance
			position += sizeof(Color);

			// character
			glCheck(glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, sizeof(glChar), (void*)position));
			glCheck(glEnableVertexAttribArray(0));
			glCheck(glVertexAttribDivisor(0, 1)); // one per instance
			position += sizeof(uint8_t);

			if (gl.m_array == 0 || gl.m_buffer == 0) {
				LOG_ERR_("Unable to initialize vertex array for opengl");
				assert(false);
			}
		}
		if (!gl.sync) {
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, gl.m_buffer));
			if (!gl.m_bound || gl_text.size() > gl.m_bufsize) {
				glCheck(glBufferData(GL_ARRAY_BUFFER, gl_text.size() * sizeof(glChar), &gl_text[0], GL_DYNAMIC_DRAW));
				gl.m_bufsize = gl_text.size();
				gl.m_bound = true;
			}
			else {
				glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, gl_text.size() * sizeof(glChar), &gl_text[0]));
			}
			gl.sync = true;
		}

	}

}
