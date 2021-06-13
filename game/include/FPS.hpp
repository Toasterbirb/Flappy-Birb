#include <iostream>
#include <vector>

#include "../../engine/include/RenderWindow.hpp"
#include "../../engine/include/Entity.hpp"

class FPS
{
public:
	FPS(int p_refreshrate);
	int calculateFPS(float frametime);

private:
	std::vector<float> frametimes;
	int frametimeAvgIndex;
	int refresh_rate;
};
