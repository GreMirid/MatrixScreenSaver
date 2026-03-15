#include "MatrixProcessing.h"

#include <string>

MatrixProcessing::MatrixProcessing(std::shared_ptr<WindCanvas> canvs)
{
	canvas = canvs;
	neo_font = std::make_shared<Font>(canvas->GetWriteFactory(), 20);
}

void MatrixProcessing::Draw(double delta)
{
	ProcessTrails(delta);

	for (auto& trail : trails)
	{
		if (!trail.CanBeDrawn())
			continue;

		canvas->DrawTextStr(trail.x, trail.y, GetRandomChar(), *neo_font, canvas->MakeColor(150, 210, 150));

		// draw its trial
		for (int tr_i = 1; tr_i < trail.trial_len; tr_i++)
		{
			int tailY = trail.y - (tr_i * g_StepTime);
			if (tailY < 0)
				continue;

			uint8_t alpha = (uint8_t)(255 * (1.0f - (float)tr_i / trail.trial_len));

			canvas->DrawTextStr(trail.x, tailY, GetRandomChar(), *neo_font,
				canvas->MakeColor(0, 180, 35, alpha));
		}
	}
}

std::string MatrixProcessing::GetRandomChar()
{
	char c = (char)(33 + rand() % 94);
	return std::string(1, c);
}

void MatrixProcessing::ProcessTrails(double delta)
{
	int columns = trails.size();

	if (latest_x != canvas->GetH() || latest_y != canvas->GetW())
	{
		int columns = canvas->GetW() / g_StepTime;

		trails.clear();
		trails.resize(columns); // clear x

		for (int i = 0; i < trails.size(); i++)
		{
			// create it 
			Trail trail;
			trail.x = (x_start + (i * g_StepTime));
			trail.y = (int)(rand() % 1000);
			trail.speed = (int)(100 + rand() % 300);
			trail.trail_len_max = (int)(rand() % 30);

			trails[i] = std::move(trail);
		}

		latest_x = canvas->GetH();
		latest_y = canvas->GetW();
	}

	if (trails.empty())
		return;

	for (int i = 0; i < trails.size(); i++)
	{
		if (trails[i].x == (x_start + (i * g_StepTime)))
		{
			// already have trail on column
			// process it
			if (trails[i].CanBeDrawn())
				trails[i].Step(delta);

			//  clear by y
			if (trails[i].y > canvas->GetH() - y_start)
				trails[i].clear();
		}
		else
		{
			// recreate it
			trails[i].x = (x_start + (i * g_StepTime));
			trails[i].y = (int)(rand() % 1000);
			trails[i].speed = (int)(100 + rand() % 300);
			trails[i].trail_len_max = (int)(rand() % 30);
		}
	}
}
