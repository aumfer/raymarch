#include "Timer.hpp"
#include "MovingAverage.hpp"
#include "Font.hpp"
#include "Quad.hpp"
#include "Raymarch.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cassert>

static void OnGlfwError(int error, const char* description) {
	puts(description);
	assert(0);
}

static void TextFps(char *text, size_t len, float avg) {
	float ms = glm::clamp(avg * 1000, 0.0f, 1000000.0f);
	float fps = glm::clamp(1 / avg, 0.0f, 1000000.0f);
	sprintf_s(text, len, "%.0ffps %.1fms", fps, ms);
}

int main(int argc, char **argv) {
	glfwSetErrorCallback(OnGlfwError);
	auto glfw_init_result = glfwInit();
	assert(glfw_init_result == GL_TRUE);

	auto window = glfwCreateWindow(640, 480, "raymarch", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glewExperimental = GL_TRUE;
	auto glew_init_result = glewInit();
	if (glew_init_result != GLEW_OK) {
		const GLubyte *glew_init_error = glewGetErrorString(glew_init_result);
		puts((const char *)glew_init_error);
		assert(false);
	}

	Timer timer;
	MovingAverage<8> fps;
	Font font(FontStyle::CONSOLAS_16);
	Quad quad;
	Raymarch raymarch;

	do {
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		int width, height;
		glfwGetWindowSize(window, &width, &height);

		raymarch.Begin();
		quad.Begin();

		raymarch.Draw(width, height);
		quad.Draw();

		quad.End();
		raymarch.End();

		fps.Update(timer.Delta());
		char fps_text[32];
		TextFps(fps_text, 32, fps.Average());

		font.Begin();
		font.Print(fps_text, glm::vec2(20), glm::uvec4(255));
		font.End();
		font.Draw(width, height);

		glfwSwapBuffers(window);
		timer.Update();
	} while (!glfwWindowShouldClose(window));

	return 0;
}