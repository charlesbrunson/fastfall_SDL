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
		Color get_color() const { return m_color; }

	private:
		virtual void draw(RenderTarget& target, RenderState state = RenderState()) const;

		void update_varray();

		std::string m_text;
		VertexArray m_varr;
		Rectf bounding_size;

		const Font* m_font = nullptr;
		Color m_color = Color::White;
	};
}