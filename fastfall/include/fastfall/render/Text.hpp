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
		Text(const Font& font);
		Text(const Font& font, std::string_view text);

		void set(std::optional<std::reference_wrapper<const Font>> font, std::optional<std::string_view> text);
		void clear();

		void set_color(Color color);

		std::string_view get_text() const { return m_text; };
		const Font* get_font() const { return m_font; };
		Rectf get_bounds() const  { return bounding_size; };
		Rectf get_scaled_bounds() const { 
			return Rectf{
				bounding_size.left * getScale().x,
				bounding_size.top * getScale().y,
				bounding_size.width * getScale().x,
				bounding_size.height * getScale().y
			};
		};
		Color get_color() const { return m_color; }

		void set_vert_spacing(float spacing_factor) { v_spacing = spacing_factor; };

	private:
		virtual void draw(RenderTarget& target, RenderState state = RenderState()) const;

		void update_varray();

		std::string m_text;
		VertexArray m_varr;
		Rectf bounding_size;

		float v_spacing = 1.f;

		const Font* m_font = nullptr;
		Color m_color = Color::White;
	};
}