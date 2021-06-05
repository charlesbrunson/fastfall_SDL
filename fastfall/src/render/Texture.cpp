#include "fastfall/render/Texture.hpp"

#include "fastfall/render/opengl.hpp"

#include "fastfall/util/log.hpp"

#include <iostream>

#include "detail/error.hpp"


namespace ff {


TextureRef::TextureRef(const Texture& tex) 
	: texture{ &tex }
{

}

TextureRef::TextureRef()
	: texture{ nullptr }
{

}

void TextureRef::bind() const {
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
	if (texture_id != 0) {
		glCheck(glDeleteTextures(1, &texture_id));
	}
}

bool Texture::loadFromFile(const std::string_view filename) {

	SDL_Surface* surf = IMG_Load(filename.data());
	checkSDL(surf);

	loadFromSurface(surf);
	SDL_FreeSurface(surf);

	return exists();
}


bool Texture::loadFromStream(const void* data, short length) {
	SDL_RWops* io = SDL_RWFromConstMem(data, length);
	checkSDL(io);

	SDL_Surface* surf = IMG_Load_RW(io, 1);
	checkSDL(surf);

	loadFromSurface(surf);
	SDL_FreeSurface(surf);

	return exists();
}

bool Texture::loadFromSurface(SDL_Surface* surface) {
	checkSDL(surface);
	if (surface) {
		if (surface->format->Amask > 0) {
			load(surface->pixels, surface->w, surface->h, ImageFormat::PNG);
		}
		else {
			load(surface->pixels, surface->w, surface->h, ImageFormat::BMP);
		}
	}
	return exists();
}

bool Texture::load(const void* data, unsigned width, unsigned height, ImageFormat format) {
	if (data) {
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (format == ImageFormat::BMP) {
			glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data));
		}
		else if (format == ImageFormat::PNG) {
			glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
		}

		m_size = { width, height };
		m_invSize = 1.f / glm::fvec2{ m_size };

	}
	return exists();
}

void Texture::bind() const {
	if (exists()) {
		glCheck(glBindTexture(GL_TEXTURE_2D, texture_id));
	}
	else {
		if (!NullTexture.exists() && this != &NullTexture) {
			getNullTexture().bind();
		}
		else {
			throw Error("Unable to bind a texture!");
		}
	}
}

void Texture::clear() {
	if (texture_id != 0) {
		glCheck(glDeleteTextures(1, &texture_id));
	}
	texture_id = 0;
}

bool Texture::exists() const noexcept {
	return texture_id != 0;
}

const Texture& Texture::getNullTexture() {
	if (!NullTexture.exists()) {

		// generate null texture
		glGenTextures(1, &NullTexture.texture_id);
		glBindTexture(GL_TEXTURE_2D, NullTexture.texture_id);

		GLubyte data[] = { 255, 255, 255, 255 };

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));

		NullTexture.m_size = { 1, 1 };
		NullTexture.m_invSize = { 1.f, 1.f };
	}
	return NullTexture;
}


void Texture::destroyNullTexture() {
	if (NullTexture.exists()) {
		glCheck(glDeleteTextures(1, &NullTexture.texture_id));
		NullTexture.texture_id = 0;
	}
}

bool Texture::create(glm::uvec2 size) {
	return create(size.x, size.y);
}

bool Texture::create(unsigned sizeX, unsigned sizeY) {
	if (exists()) {
		glDeleteTextures(1, &texture_id);
		texture_id = 0;
	}
	m_size = { sizeX, sizeY };
	m_invSize = 1.f / glm::fvec2{ m_size };

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
	return exists();
}

}