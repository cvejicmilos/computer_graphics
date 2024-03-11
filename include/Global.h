#pragma once

#include "AppWindow.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

extern bool g_ShouldDrawDepthMaps;
extern int g_DrawDepthMapIndex;

void SetMainWindow(AppWindow* window);
AppWindow* GetMainWindow();