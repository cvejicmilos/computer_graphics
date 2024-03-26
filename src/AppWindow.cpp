#include <assert.h>
#include <iostream>

#include "AppWindow.h"

bool g_GLFWInitialized = false;

AppWindow::AppWindow(const char* title, int width, int height) {
	m_Valid = false;

	std::cout << "Creating a Window...\n";

	// Initialize GLFW if it hasn't been initilized yet
	if (!g_GLFWInitialized) {
		auto result = glfwInit();
		assert(result == GLFW_TRUE);
		g_GLFWInitialized = true;
	}

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	//glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	m_WindowHandle = glfwCreateWindow(width, height, title, NULL, NULL);

	// Check for error in window creation
	if (!m_WindowHandle) {
		m_Valid = false;
		m_Running = false;
		std::cout << "Failed creating window\n";
		return;
	}

	glfwMakeContextCurrent(m_WindowHandle);

	m_Valid = true;
	m_Running = true;

	std::cout << "Window OK!\n";
}

AppWindow::~AppWindow() {
	glfwDestroyWindow(m_WindowHandle);
}

bool AppWindow::IsRunning() const {
	return m_Running && !glfwWindowShouldClose(m_WindowHandle);
}

bool AppWindow::IsValid() const {
	return m_Valid;
}

bool AppWindow::IsKeyDown(int key) {
	return glfwGetKey(m_WindowHandle, key) == GLFW_PRESS;
}

int AppWindow::GetWidth() {
	int x, y;
	glfwGetWindowSize(m_WindowHandle, &x, &y);
	return x;
}
int AppWindow::GetHeight() {
	int x, y;
	glfwGetWindowSize(m_WindowHandle, &x, &y);
	return y;
}

void AppWindow::PollEvents() {
	glfwPollEvents();
}

void AppWindow::SwapBuffers() {
	glfwSwapBuffers(m_WindowHandle);
}
