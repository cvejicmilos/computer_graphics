#include <assert.h>

#include "Global.h"

bool g_ShouldDrawDepthMaps = false;
int g_DrawDepthMapIndex = 0;

AppWindow* g_MainWindow;

void SetMainWindow(AppWindow* window) {
    g_MainWindow = window;
}
AppWindow* GetMainWindow() {
    assert(g_MainWindow && "Main window not set in Global");
    return g_MainWindow;
}