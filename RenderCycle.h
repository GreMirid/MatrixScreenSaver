
#ifndef RENDERCYCLE
#define RENDERCYCLE

#include "WindCanvas.h"
#include "MatrixProcessing.h"

#include <memory>

class RenderCycle
{
private:
	std::shared_ptr<WindCanvas> canvas;
	double m_deltaTime = 0.;
	float m_fps = 0.f;
	bool m_showFps = false;
	std::shared_ptr<MatrixProcessing> processing;
	std::shared_ptr<Font> fps_font;

public:
	RenderCycle(std::shared_ptr<WindCanvas>);
	int render();

	double GetDelta() { return m_deltaTime; }
	void SetShowFPS(bool ye) { m_showFps = ye; }

private:
	void rawframe();
	void frame();
};

#endif