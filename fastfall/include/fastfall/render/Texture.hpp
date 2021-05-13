#pragma once

#include <string_view>

#include "glm/glm.hpp"

namespace ff {

class Texture;

class TextureRef {
public:
	TextureRef(const Texture& tex);
	TextureRef();

	void operator=(const Texture& tex) {
		texture = &tex;
	}

	void bind() const;
	bool exists() const noexcept;
	const Texture* get() const noexcept;


private:
	const Texture* texture;
};


class Texture {
public:

	enum class ImageFormat {
		BMP,
		PNG
	};

	Texture();
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(Texture&&) = default;
	Texture& operator=(Texture&&) = default;

	bool loadFromFile(const std::string_view filename);
	bool loadFromData(const void* data, unsigned width, unsigned height, ImageFormat format = ImageFormat::BMP);

	bool create(glm::uvec2 size);
	bool create(unsigned sizeX, unsigned sizeY);

	void bind() const;

	void clear();

	bool exists() const noexcept;

	static const Texture& getNullTexture();


	unsigned int getID() const { return texture_id; };

	glm::uvec2 size() const { return m_size; };
	glm::fvec2 inverseSize() const { return m_invSize; };

private:
	glm::uvec2 m_size;
	glm::fvec2 m_invSize;

	static Texture NullTexture;

	unsigned int texture_id = NULL;
};

}