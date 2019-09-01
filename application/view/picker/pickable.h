#pragma once

#include "ray.h"

class Screen;

struct Pickable {
	virtual void init() {};
	virtual void render(Screen& screen) {};
	virtual void close() {};

	virtual float intersect(Screen&, const Ray& ray) = 0;

	virtual void press(Screen& screen) {};
	virtual void release(Screen& screen) {};
	virtual void drag(Screen& screen) {};
};