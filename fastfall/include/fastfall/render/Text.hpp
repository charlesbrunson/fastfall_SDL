#pragma once

#include "VertexArray.hpp"
#include "Drawable.hpp"
#include "Font.hpp"

#include "fastfall/util/math.hpp"

#include <string_view>
#include <optional>

namespace ff {

	class Text : public Transformable
	{
	public:
		Text();
		Text(Font& font);
		Text(Font& font, unsigned size);
		Text(Font& font, unsigned size, std::string_view text);

		using const_font_ref = std::reference_wrapper<const Font>;

		void setText(std::optional<const_font_ref> font, std::optional<unsigned> pixel_size, std::optional<std::string_view> text);
		void clear();

		void setColor(Color color);

		void predraw();

		std::string_view getText() const { return m_text; };
		const Font* getFont() const { return m_font; };
		Rectf getBounds() const  { return bounding_size; };
		Rectf getScaledBounds() const { 
			return Rectf{
				bounding_size.left * getScale().x,
				bounding_size.top * getScale().y,
				bounding_size.width * getScale().x,
				bounding_size.height * getScale().y
			};
		};
		Color getColor() const { return m_color; }

		void setVertSpacing(float spacing_factor) { v_spacing = spacing_factor; };

	private:

		struct glChar {
			Vec2f offset;
			Color color;
			uint8_t character;
		};

		bool gl_text_fresh = false;
		std::vector<glChar> gl_text;

		struct GPUState {
			GLuint m_array = 0;
			GLuint m_buffer = 0;

			size_t m_bufsize = 0;
			bool m_bound = false;

			bool sync = false;
		} mutable gl;

		friend class RenderTarget;
		void glTransfer() const;

		//virtual void draw(RenderTarget& target, RenderState state = RenderState()) const;

		void update_text();

		std::string m_text;
		//VertexArray m_varr;
		Rectf bounding_size;

		TextureRef bitmap_texture;

		unsigned px_size = 0;

		float v_spacing = 1.f;

		const Font* m_font = nullptr;
		Color m_color = Color::White;
	};
}