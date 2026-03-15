#include "WindCanvas.h"

WindCanvas::WindCanvas(int w, int h) : width(w), height(h) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"WindCanvasClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    RegisterClass(&wc);

    hwnd = CreateWindowEx(0, wc.lpszClassName, BaseUtil::ToWString(wind_name).c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, w + 16, h + 39, NULL, NULL, NULL, NULL);
    
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
    isInitialized = true;

    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(pDWriteFactory),
        reinterpret_cast<IUnknown**>(&pDWriteFactory));
}

void WindCanvas::Clear(uint32_t color)
{
    if (!isInitialized)
        return;

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    pRenderTarget->Clear(D2D1::ColorF(r, g, b));
}

void WindCanvas::DrawPixel(int x, int y, uint32_t color)
{
    uint32_t* currentBuffer = cpuPixels;
    if (!currentBuffer) return;

    if (x >= 0 && x < width && y >= 0 && y < height) {
        currentBuffer[y * width + x] = color;
    }
}

void WindCanvas::BeginDraw()
{
    if (!isInitialized)
        return;

    CreateResources();
    pRenderTarget->BeginDraw();
}

void WindCanvas::DrawRect(int x, int y, int w, int h, uint32_t color)
{
    if (!isInitialized)
        return;

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    pBrush->SetColor(D2D1::ColorF(r, g, b));
    pRenderTarget->FillRectangle(D2D1::RectF(x, y, x + w, y + h), pBrush);
}

void WindCanvas::DrawTextStr(int x, int y, const std::string& text, Font font, uint32_t color)
{
    if (!isInitialized)
        return;

    std::wstring wtext = BaseUtil::ToWString(text);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;
    pTextBrush->SetColor(D2D1::ColorF(r, g, b, a));

    D2D1_RECT_F layoutRect = D2D1::RectF(x, y, (float)width, (float)height);
    
    pRenderTarget->DrawText(
        wtext.c_str(),
        (uint32_t)wtext.length(),
        font.Get(),
        layoutRect,
        pTextBrush
    );
}

void WindCanvas::EndDraw()
{
    HRESULT hr = pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        DiscardResources();
    }
}

void WindCanvas::Present()
{
    if (!pBitmap)
        return;

    D2D1_RECT_U destRect = { 0, 0, (UINT32)width, (UINT32)height };
    pBitmap->CopyFromMemory(&destRect, cpuPixels, width * sizeof(uint32_t));

    D2D1_RECT_F screenRect = D2D1::RectF(0, 0, (float)width, (float)height);
    pRenderTarget->DrawBitmap(pBitmap, screenRect);
}

bool WindCanvas::ProcessMessages()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) return false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void WindCanvas::SetName(const std::string& name)
{
    wind_name = name;
    SetWindowTextW(hwnd, BaseUtil::ToWString(wind_name).c_str());
}

void WindCanvas::Resize(int newW, int newH)
{
    if (newW <= 0 || newH <= 0 || !pRenderTarget)
        return;

    uint32_t* oldPixels = cpuPixels;
    cpuPixels = nullptr;

    width = newW;
    height = newH;

    delete[] oldPixels;
    cpuPixels = new uint32_t[width * height];
    memset(cpuPixels, 0, width * height * sizeof(uint32_t));

    if (pRenderTarget) {
        pRenderTarget->Resize(D2D1::SizeU(width, height));

        if (pBitmap) {
            pBitmap->Release();
            pBitmap = nullptr;
        }
    }
}

LRESULT WindCanvas::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WindCanvas* pCanvas = (WindCanvas*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (pCanvas && pCanvas->isInitialized) {
        switch (uMsg) {
        case WM_CLOSE: {
            if (pCanvas) pCanvas->running = false;
            PostQuitMessage(0);
            return 0;
            }
        case WM_SIZE: {
            pCanvas->Resize(int(LOWORD(lParam)), int(HIWORD(lParam)));
            return 0;
            }
        case WM_GETMINMAXINFO: {
            LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
            mmi->ptMinTrackSize.x = 800;
            mmi->ptMinTrackSize.y = 800;
            return 0;
            }
        
        case WM_ERASEBKGND: {
            return 1;
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F11) {
                static bool isFull = false;
                isFull = !isFull;
                pCanvas->SetFullscreen(isFull);
            }
            // close if pressed
            if (wParam == VK_SPACE || wParam == VK_ESCAPE || wParam == VK_RETURN) {
                if (pCanvas) pCanvas->running = false;
                PostQuitMessage(0);
            }
            break;
        }
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

uint32_t WindCanvas::MakeColor(int r, int g, int b, int a)
{
    return (b << 0) | (g << 8) | (r << 16) | (a << 24);;
}

void WindCanvas::SetFullscreen(bool fullscreen)
{
    static WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

    if (fullscreen) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hwnd, &g_wpPrev) &&
            GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hwnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else {
        SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &g_wpPrev);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }

    // resize!
    RECT rc;
    GetClientRect(hwnd, &rc);
    width = rc.right - rc.left;
    height = rc.bottom - rc.top;
    this->Resize(width, height);
}

void WindCanvas::CreateResources()
{
    if (!pRenderTarget) {
        RECT rc;
        GetClientRect(hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

            D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
            D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(
                hwnd,
                size,
                D2D1_PRESENT_OPTIONS_IMMEDIATELY // vsync off
            );

            pFactory->CreateHwndRenderTarget(
                rtProps,
                hwndProps,
                &pRenderTarget
            );

            pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                &pBrush
            );

        HRESULT hr = pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            &pTextBrush
        );

        D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
        );

        pRenderTarget->CreateBitmap(D2D1::SizeU(width, height), props, &pBitmap);

        if (!cpuPixels)
            cpuPixels = new uint32_t[width * height];
    }

    if (!pBitmap && pRenderTarget && cpuPixels) {
        D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
        );

        pRenderTarget->CreateBitmap(D2D1::SizeU(width, height), props, &pBitmap);
    }
}

void WindCanvas::DiscardResources()
{
    if (pRenderTarget)
    {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }

    if (pBrush) {
        pBrush->Release();
        pBrush = nullptr;
    }
}

std::wstring BaseUtil::ToWString(const std::string& utf8Str)
{
    if (utf8Str.empty())
        return L"";

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), NULL, 0);
    std::wstring wstrTo(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), &wstrTo[0], sizeNeeded);

    return wstrTo;
}