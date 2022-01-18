#pragma once

#include "VertexArray.hpp"
#include "Drawable.hpp"
#include "Font.hpp"

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

		std::string_view get_text() const { return m_text; };
		const Font* get_font() const { return m_font; };

		glm::fvec2 get_bounds() { return bounding_size; };

	private:
		virtual void draw(RenderTarget& target, RenderState state = RenderState()) const;

		void update_varray();

		std::string m_text;
		VertexArray m_varr;
		const Font* m_font;
		glm::fvec2 bounding_size;

	};
}