#include <assert.h>

#include "AppWindow.h"

bool g_GLFWInitialized = false;

AppWindow::AppWindow(const char* title, int width, int height) {
	m_Valid = false;

	// Initialize GLFW if it hasn't been initilized yet
	if (!g_GLFWInitialized) {
		assert(glfwInit() == GLFW_TRUE);
		g_GLFWInitialized = true;
	}

	m_WindowHandle = glfwCreateWindow(width, height, title, NULL, NULL);

	// Check for error in window creation
	if (!m_WindowHandle) {
		m_Valid = false;
		m_Running = false;
		return;
	}

	glfwMakeContextCurrent(m_WindowHandle);

	m_Valid = true;
	m_Running = true;
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

void AppWindow::PollEvents() {
	glfwPollEvents();
}

void AppWindow::SwapBuffers() {
	glfwSwapBuffers(m_WindowHandle);
}
