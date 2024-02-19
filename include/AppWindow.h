#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Class for opening and managing a GLFW Window
class AppWindow {
private:
	GLFWwindow* m_WindowHandle;
	bool m_Running = true;
	bool m_Valid = false;
public:
	AppWindow(const char* title, int width, int height);
	~AppWindow();

	// Check if window should still run
	bool IsRunning() const;
	// Check if window is valid and no errors
	bool IsValid() const;

	bool IsKeyDown(int key);

	// Poll Window Events
	void PollEvents();
	// Swap window buffers
	void SwapBuffers();
};
