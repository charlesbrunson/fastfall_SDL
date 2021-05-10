#include "Texture.hpp"

#include <SDL.h>
#include <gl/glew.h>
#include <SDL_opengl.h>
#include <gl/glu.h>

#include "SDL_image.h"

#include <iostream>


namespace ff {


TextureRef::TextureRef(const Texture& tex) 
	: texture{ &tex }
{

}

TextureRef::TextureRef()
	: texture{ nullptr }
{

}

void TextureRef::bind() const noexcept {
	if (texture) {
		texture->bind();
	}
	else {
		Texture::getNullTexture().bind();
	}
}

bool TextureRef::exists() const noexcept {
	return texture ? texture->exists() : false;
}


const Texture* TextureRef::get() const noexcept {
	return texture; 
}

Texture Texture::NullTexture;

Texture::Texture()
	: m_size{ 0, 0 },
	m_invSize{ 0.f, 0.f }
{

}

Texture::~Texture() {
	glDeleteTextures(1, &texture_id);
}

bool Texture::loadFromFile(const std::string_view filename) {

	SDL_Surface* surf = IMG_Load(filename.data());
	if (surf) {
		// determine the image type
		// SDL_image is configured to only support BMP and PNG
		// and BMP does not support an alpha channel
		if (surf->format->Amask > 0) {
			loadFromData(surf->pixels, surf->w, surf->h, ImageFormat::PNG);
		}
		else {
			loadFromData(surf->pixels, surf->w, surf->h, ImageFormat::BMP);
		}

		SDL_FreeSurface(surf);
	}
	else {
		std::cout << IMG_GetError() << std::endl;
	}

	return exists();
}

bool Texture::loadFromData(const void* data, unsigned width, unsigned height, ImageFormat format) {
	if (data) {
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (format == ImageFormat::BMP) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
		}
		else if (format == ImageFormat::PNG) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		m_size = { width, height };
		m_invSize = 1.f / glm::fvec2{ m_size };

	}
	return exists();
}

void Texture::bind() const noexcept {
	if (exists()) {
		glBindTexture(GL_TEXTURE_2D, texture_id);
	}
	else {
		getNullTexture().bind();
	}
}

void Texture::clear() {
	glDeleteTextures(1, &texture_id);
}

bool Texture::exists() const noexcept {
	return texture_id != NULL;
}

const Texture& Texture::getNullTexture() {
	if (!NullTexture.exists()) {

		// generate null texture
		glGenTextures(1, &NullTexture.texture_id);

		GLubyte data[] = { 255, 255, 255, 255 };

		NullTexture.m_size = { 1, 1 };
		NullTexture.m_invSize = { 1.f, 1.f };

		glBindTexture(GL_TEXTURE_2D, NullTexture.texture_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	return NullTexture;
}

bool Texture::create(glm::uvec2 size) {
	return create(size.x, size.y);
}

bool Texture::create(unsigned sizeX, unsigned sizeY) {
	if (exists()) {
		glDeleteTextures(1, &texture_id);
		texture_id = NULL;
	}
	m_size = { sizeX, sizeY };
	m_invSize = 1.f / glm::fvec2{ m_size };

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	return exists();
}

}