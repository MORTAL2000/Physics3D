#pragma once

#include "../engine/layer/layer.h"

namespace Application {

class Screen;

class ModelLayer : public Layer {
public:
	ModelLayer() : Layer() {};
	ModelLayer(Screen* screen, char flags = None) : Layer("Model", screen, flags) {};

	void onInit() override;
	void onUpdate() override;
	void onEvent(Event& event) override;
	void onRender() override;
	void onRender2();
	void onClose() override;
};

};