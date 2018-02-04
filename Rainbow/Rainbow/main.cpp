#include <Windows.h>
#include <cmath>
#include <chrono>

#include <LogitechLEDLib.h>
#pragma comment(lib, "LogitechLEDLib.lib")

double GetEpochTime()
{
	double now = (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	return now;
}

void SetRainbowColor(float speed)
{
	double now = GetEpochTime();
	LogiLedSetLighting( //127 as the center of the color; 128 as the max width it can go; / 2.55f so we have the percentage
		((int)(std::sin(speed*now + 0) * 127) + 128) / 2.55f,
		((int)(std::sin(speed*now + 2) * 127) + 128) / 2.55f,
		((int)(std::sin(speed*now + 4) * 127) + 128) / 2.55f);
}

int main()
{
	LogiLedInit();
	while (!(GetKeyState(VK_ESCAPE) & 0x8000)) //Press Escape to stop
	{
		SetRainbowColor(0.001f);
	}
	LogiLedShutdown();
	return 0;
}