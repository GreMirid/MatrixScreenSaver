// Simple Matrix screen saver by GreMirid what tooks to write 2 days
// Writen on Direct2D
// March 15 2026

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include "WindCanvas.h"
#include "RenderCycle.h"

int main()
{
    auto window = std::make_shared<WindCanvas>(800, 600);
    window->SetName("Matrix");
    window->SetFullscreen(true);

    RenderCycle cycle(window);
    cycle.SetShowFPS(false);

    return cycle.render();
}