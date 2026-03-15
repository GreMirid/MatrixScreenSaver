
#ifndef RENDER_MATRIX_PROCESSING
#define RENDER_MATRIX_PROCESSING

#include "WindCanvas.h"
#include <memory>
#include <vector>

const int g_StepTime = 18;

class MatrixProcessing
{
private:
	std::shared_ptr<Font> neo_font;
	std::shared_ptr<WindCanvas> canvas;

	int x_start = 10, y_start = 10;

	int latest_x = 0, latest_y = 0;

	struct Trail {
		int x = 0, y = 0;
		float speed = 0;

		int trail_len_max = 0, trial_len = 0;

		double timer = 0;
		double stepTime = 0.1;

		void Step(double delta)
		{
			double stepTime = g_StepTime / (speed > 0 ? speed : 1.0);

			timer += delta;

			while (timer >= stepTime) {
				y += g_StepTime;

				if (trial_len < trail_len_max)
					trial_len++;
				else
					trial_len = trail_len_max;

				timer -= stepTime;
			}
		}

		void clear()
		{
			x = y = trail_len_max = trial_len = 0;
			speed = 0;
		}

		bool CanBeDrawn()
		{
			return x != 0;
		}
	};

	std::vector<Trail> trails = {};

public:
	MatrixProcessing(std::shared_ptr<WindCanvas> canvas);
	void Draw(double delta);

private:
	std::string GetRandomChar();

	void ProcessTrails(double delta);
};

#endif // !RENDER_MATRIX_PROCESSINGS