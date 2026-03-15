#include "RenderCycle.h"

#include <chrono>

RenderCycle::RenderCycle(std::shared_ptr<WindCanvas> cnv)
{
    canvas = std::move(cnv);

    processing = std::make_shared<MatrixProcessing>(canvas);
    fps_font = std::make_shared<Font>(canvas->GetWriteFactory(), 12);
}

int RenderCycle::render()
{
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (canvas->isRunning()) {
        canvas->ProcessMessages();

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;
        m_deltaTime = elapsed.count();

        lastTime = currentTime;
        if (m_showFps)
            m_fps = 1.0 / m_deltaTime;

        canvas->BeginDraw();
        canvas->Clear(canvas->MakeColor(10, 12, 12));

        //rawframe();
        //canvas->Present();

        frame();

        canvas->EndDraw(); // RENDER
    }
    return 0;
}

void RenderCycle::rawframe()
{
    //canvas->DrawPixel(canvas->GetW() / 2, canvas->GetH() / 2, canvas->MakeColor(255, 255, 255));
}

void RenderCycle::frame()
{
    if (!canvas->isInitialized)
        return;

    processing->Draw(m_deltaTime);

    if (m_showFps)
        canvas->DrawTextStr(0, 0, std::format("fps: {:.1f}", m_fps), *fps_font, canvas->MakeColor(255, 255, 255));
}