#pragma once
#include <chrono>

#include <LogitechLCDLib.h>
#include <LogitechLEDLib.h>

double GetEpochTimeInMilliseconds();
double GetEpochTime();
void SetRainbowColor(float speed);