#ifndef WINDCANVAS
#define WINDCANVAS

#include <windows.h>
#include <vector>
#include <stdint.h>
#include <string>
#include <d2d1.h>
#include <dwrite.h>

#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")

//shame on microsoft
class BaseUtil
{
public:
    static std::wstring ToWString(const std::string& utf8Str);
};

class Font
{
private:
    IDWriteTextFormat* pTextFormat = nullptr;

public:
    Font(IDWriteFactory* pDWriteFactory, int size = 12, const std::string& font = "Consolas")
    {
        pDWriteFactory->CreateTextFormat(
            BaseUtil::ToWString(font).c_str(),
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            static_cast<float>(size),
            L"",
            &pTextFormat
        );
    }

    IDWriteTextFormat* Get() { return pTextFormat; };
};

class WindCanvas {
public:
    uint32_t* cpuPixels = nullptr;
    bool isInitialized = false;

private:
    int width = 800, height = 600;
    
    std::string wind_name = "Formal Window";
    bool running = true;

public:
    WindCanvas(int w, int h);

    void Clear(uint32_t color);

    void BeginDraw();

    void DrawRect(int x, int y, int w, int h, uint32_t color);
    void DrawPixel(int x, int y, uint32_t color);
    void DrawTextStr(int x, int y, const std::string& text, Font font, uint32_t color = 0xFFFFFF);

    void EndDraw();

    void Present();
    bool ProcessMessages();

    void SetName(const std::string& name);
    void Resize(int newW, int newH);

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    bool isRunning() { return running; }

    int GetW() { return width; }
    int GetH() { return height; }

    uint32_t MakeColor(int r, int g, int b, int a = 0xFF);

    IDWriteFactory* GetWriteFactory() { return pDWriteFactory; }

    void SetFullscreen(bool fullscreen);

private:
    HWND hwnd;
    ID2D1Factory* pFactory = nullptr;
    ID2D1HwndRenderTarget* pRenderTarget = nullptr;
    ID2D1SolidColorBrush* pBrush = nullptr;
    ID2D1Bitmap* pBitmap = nullptr;
    
    IDWriteFactory* pDWriteFactory = nullptr;
    ID2D1SolidColorBrush* pTextBrush = nullptr;

    void CreateResources();
    void DiscardResources();
};


#endif // !WINDCANVAS
