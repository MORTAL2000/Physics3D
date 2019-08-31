#include "texture.h"

#include "GL\glew.h"
#include "GLFW\glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb\stb_image.h"

#include "../util/log.h"
#include "../debug.h"

#pragma region Texture

//! Texture

Texture* load(const std::string& name) {
	int width;
	int height;
	int channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(name.c_str(), &width, &height, &channels, 0);

	Texture* texture = nullptr;

	if (data) {
		int format;

		switch (channels) {
			case 1:
				format = GL_RED;
				break;
			case 3:
				format = GL_RGB;
				break;
			case 4:
				format = GL_RGBA;
				break;
			default:
				Log::setSubject(name);
				Log::warn("Unknown amount of channels: %d, choosing RGB", channels);
				Log::resetSubject();
				format = GL_RGB;
		}

		texture = new Texture(width, height, data, format);
	} else {
		Log::setSubject(name);
		Log::error("Failed to load texture");
		Log::resetSubject();
	}

	stbi_image_free(data);
	return texture;
}

Texture::Texture(unsigned int width, unsigned int height) : Texture(width, height, nullptr, GL_RGBA) {};

Texture::Texture(unsigned int width, unsigned int height, const void* buffer, int format) : width(width), height(height), unit(0), format(format) {
	glGenTextures(1, &id);

	switch (format) {
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
			channels = 1;
			break;
		case GL_RGB:
			channels = 3;
			break;
		case GL_RGBA:
			channels = 4;
			break;
		default:
			channels = 4;
	}

	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	unbind();
}

Texture::~Texture() {
	close();
}

Texture::Texture(Texture&& other) {
	id = other.id;
	other.id = 0;
}

Texture& Texture::operator=(Texture&& other) {
	if (this != &other) {
		close();
		std::swap(id, other.id);
	}

	return *this;
}

void Texture::loadFrameBufferTexture(unsigned int width, unsigned int height) {
	bind();
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);
	unbind();
}

void Texture::resize(unsigned int width, unsigned int height, const void* buffer) {
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, buffer);
	unbind();
}

Texture* Texture::colored(Vec3 color) {
	return colored(Vec4(color.x, color.y, color.z, 1));
}

Texture* Texture::colored(Vec4 color) {
	bind();
	unsigned char* buffer = (unsigned char*) malloc(width * height * channels);

	glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, buffer);
	
	for (unsigned int j = 0; j < height; j++) {
		for (unsigned int i = 0; i < width; i++) {
			for (int k = 0; k < channels; k++) {

				int index = (i + height * j) * channels + k;
				unsigned char value = unsigned char (buffer[index] * color[k]);
				buffer[index] = value;
			}
		}
	}

	Texture* texture = new Texture(width, height, buffer, format);

	free(buffer);

	return texture;
}

void Texture::resize(unsigned int width, unsigned int height) {
	resize(width, height, nullptr);
}

void Texture::bind(int unit) {
	this->unit = unit;
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::bind() {
	bind(unit);
}

void Texture::unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::close() {
	glDeleteTextures(1, &id);
	id = 0;
}

#pragma endregion

#pragma region HDRTexture

//! HDRTexture

HDRTexture::HDRTexture(unsigned int width, unsigned int height) : HDRTexture(width, height, nullptr) {

};

HDRTexture::HDRTexture(unsigned int width, unsigned int height, const void* buffer) : width(width), height(height), unit(0) {
	glGenTextures(1, &id);

	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	unbind();
}

HDRTexture::~HDRTexture() {
	close();
}

HDRTexture::HDRTexture(HDRTexture&& other) {
	id = other.id;
	other.id = 0;
}

HDRTexture& HDRTexture::operator=(HDRTexture&& other) {
	if (this != &other) {
		close();
		std::swap(id, other.id);
	}

	return *this;
}

void HDRTexture::resize(unsigned int width, unsigned int height, const void* buffer) {
	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, buffer);
	unbind();
}

void HDRTexture::resize(unsigned int width, unsigned int height) {
	resize(width, height, nullptr);
}

void HDRTexture::bind(int unit) {
	this->unit = unit;
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, id);
}

void HDRTexture::bind() {
	bind(unit);
}

void HDRTexture::unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void HDRTexture::close() {
	glDeleteTextures(1, &id);
	id = 0;
}

#pragma endregion

#pragma region TextureMultisample

//! TextureMultisample

MultisampleTexture::MultisampleTexture(unsigned int width, unsigned int height, unsigned int samples) : width(width), height(height), samples(samples), unit(0) {
	glGenTextures(1, &id);
	bind();
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, width, height, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	unbind();
}

MultisampleTexture::~MultisampleTexture() {
	close();
}

MultisampleTexture::MultisampleTexture(MultisampleTexture&& other) {
	id = other.id;
	other.id = 0;
}

MultisampleTexture& MultisampleTexture::operator=(MultisampleTexture&& other) {
	if (this != &other) {
		close();
		std::swap(id, other.id);
	}

	return *this;
}

void MultisampleTexture::resize(unsigned int width, unsigned int height) {
	bind();
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
	unbind();
}

void MultisampleTexture::bind() {
	bind(unit);
}

void MultisampleTexture::bind(int unit) {
	this->unit = unit;
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, id);
}

void MultisampleTexture::unbind() {
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void MultisampleTexture::close() {
	glDeleteTextures(1, &id);
	id = 0;
}

#pragma endregion

#pragma region CubeMap

//! CubeMap

CubeMap::CubeMap(const std::string& right, const std::string& left, const std::string& top, const std::string& bottom, const std::string& front, const std::string& back) : unit(0) {
	glGenTextures(1, &id);
	bind();
	
	load(right, left, top, bottom, front, back);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	bind();
}

CubeMap::~CubeMap() {
	close();
}

CubeMap::CubeMap(CubeMap&& other) {
	id = other.id;
	other.id = 0;
}

CubeMap& CubeMap::operator=(CubeMap&& other) {
	if (this != &other) {
		close();
		std::swap(id, other.id);
	}

	return *this;
}

void CubeMap::bind() {
	bind(unit);
}

void CubeMap::bind(int unit) {
	this->unit = unit;
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void CubeMap::unbind() {
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void CubeMap::load(const std::string& right, const std::string& left, const std::string& top, const std::string& bottom, const std::string& front, const std::string& back) {
	int width;
	int height;
	int channels;
	unsigned char* data;
	std::string faces[6] = { right, left, top, bottom, front, back };

	stbi_set_flip_vertically_on_load(false);

	for (int i = 0; i < 6; i++) {
		data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
		if (data)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, (channels == 4)? GL_RGBA : GL_RGB, width, height, 0, (channels == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
		else {
			Log::setSubject(faces[i]);
			Log::error("Failed to load texture");
			Log::resetSubject();
		}

		stbi_image_free(data);	
	}
}

void CubeMap::close() {
	glDeleteTextures(1, &id);
	id = 0;
}

#pragma endregion

#pragma region DepthTexture

//! DepthTexture

DepthTexture::DepthTexture(unsigned int width, unsigned int height): width(width), height(height), unit(0) {
	glGenTextures(1, &id);

	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	unbind();
}

DepthTexture::~DepthTexture() {
	close();
}

DepthTexture::DepthTexture(DepthTexture&& other) {
	id = other.id;
	other.id = 0;
}

DepthTexture& DepthTexture::operator=(DepthTexture&& other) {
	if (this != &other) {
		close();
		std::swap(id, other.id);
	}

	return *this;
}

void DepthTexture::bind() {
	bind(unit);
}

void DepthTexture::bind(int unit) {
	this->unit = unit;
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, id);
}

void DepthTexture::unbind() {
	glBindTexture(GL_TEXTURE_2D, id);
}

void DepthTexture::close() {
	glDeleteTextures(1, &id);
	id = 0;
}

#pragma endregion