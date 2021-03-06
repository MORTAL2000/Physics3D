#pragma once

#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "keyboard.h"

class Event;

class InputHandler {
protected:
	GLFWwindow* window;

private:
	bool keys[GLFW_KEY_LAST + 1];
	double timestamp[GLFW_KEY_LAST + 1];
	void keyCallback(int key, int action, int mods);
	void cursorCallback(double x, double y);
	void cursorEnterCallback(int entered);
	void scrollCallback(double xOffset, double yOffset);
	void mouseButtonCallback(int button, int action, int mods);
	void windowSizeCallback(int width, int height);
	void framebufferSizeCallback(int width, int height);

	double keyInterval = 0.2;
public:
	char anyKey = 0;

	bool rightDragging = false;
	bool middleDragging = false;
	bool leftDragging = false;

	Vec2 mousePosition;

	InputHandler(GLFWwindow* window);

	bool getKey(Keyboard::Key key);
	Vec2 getMousePosition();

	virtual void onEvent(Event& event) {};
};

