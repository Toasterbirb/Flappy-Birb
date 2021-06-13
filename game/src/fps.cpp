#include "../include/FPS.hpp"

FPS::FPS(int p_refreshrate)
:refresh_rate(p_refreshrate)
{
	frametimes = {};
}

int FPS::calculateFPS(float p_frametime)
{
	if (frametimes.size() <= refresh_rate / 2)
	{
		frametimes.push_back(p_frametime);
	}
	else
	{
		if (frametimeAvgIndex >= frametimes.size())
			frametimeAvgIndex = 0;

		frametimes[frametimeAvgIndex] = p_frametime;
		frametimeAvgIndex++;
	}

	float averageFrametime = 0;
	for (int i = 0; i < frametimes.size(); i++)
	{
		averageFrametime += frametimes[i];
	}
	averageFrametime /= frametimes.size();

	return round(1 / averageFrametime);
}
