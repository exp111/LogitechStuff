#pragma once

class Config
{
public:
	Config() { };
	~Config() { };

	bool testBool = false;
	unsigned pos = 0;
};

extern Config* config;