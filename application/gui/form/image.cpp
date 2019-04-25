#include "image.h"

#include "gui.h"

Image::Image(double x, double y, Texture* texture) : Component(x, y) {
	this->texture = texture;
};

Image::Image(double x, double y, double width, double height, Texture* texture) : Component(x, y, width, height) {
	this->texture = texture;
}

void Image::render() {
	if (texture) GUI::defaultShader->update(texture);
	else GUI::defaultShader->update(GUI::COLOR::BLACK);
	GUI::defaultQuad->resize(position, dimension);
	GUI::defaultQuad->render();
}

Vec2 Image::resize() {
	if (resizing)
		dimension = Vec2(texture->width, texture->height);
	return dimension;
}