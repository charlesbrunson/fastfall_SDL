#pragma once

#include "VertexArray.hpp"
#include "Drawable.hpp"
#include "Font.hpp"

#include "fastfall/util/math.hpp"

#include <string_view>
#include <optional>

namespace ff {

	class Text : public Transformable, public Drawable
	{
	public:
		Text();
		Text(Font& font);
		Text(Font& font, unsigned size);
		Text(Font& font, unsigned size, std::string_view text);

		void setText(std::optional<std::reference_wrapper<Font>> font, std::optional<unsigned> pixel_size, std::optional<std::string_view> text);
		void clear();

		void setColor(Color color);

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
		virtual void draw(RenderTarget& target, RenderState state = RenderState()) const;

		void update_varray();

		std::string m_text;
		VertexArray m_varr;
		Rectf bounding_size;

		TextureRef bitmap_texture;

		unsigned px_size = 0;

		float v_spacing = 1.f;

		Font* m_font = nullptr;
		Color m_color = Color::White;
	};
}