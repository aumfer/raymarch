#include "Timer.hpp"
#include <GLFW/glfw3.h>

Timer::Timer() :
	elapsed(0),
	time(float(glfwGetTime())),
	prev(time),
	dt(0) {
	
}

void Timer::Update() {
	prev = time;
	time = float(glfwGetTime());
	dt = time - prev;
	elapsed += dt;
}

float Timer::Delta() const {
	return dt;
}
float Timer::Time() const {
	return time;
}
float Timer::Elapsed() const {
	return elapsed;
}