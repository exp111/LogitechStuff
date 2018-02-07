#pragma once

#include <string>
#include <vector>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include <LogitechLCDLib.h>

class Config
{
public:
	Config() { };
	~Config() { };

	bool testBool = false;
	unsigned pos = 0;
};

extern Config* config;


class LCDScreen
{
private:
	bool gotMessage = false;
	std::string newestMessage = "";
	anyID messageSender = 0;
	bool isActive = false;
	unsigned cursorPosition = 0;
	HANDLE controlThread = 0;
	struct TS3Functions ts3Functions;

public:
	LCDScreen() {};
	~LCDScreen()
	{
		if (isActive)
			Shutdown();
	}
	
	void Init();
	void Shutdown();

	void AddMessage(const char* msg, anyID sender);

	unsigned GetPosition() const;
	void ChangePosition(int changeValue);

	bool IsActive() const { return isActive; }

	void SetTS3Functions(const struct TS3Functions funcs);

	void Update();

	void StartControlThread(); //Check controls in another thread

	//Helper
	bool isClientInList(anyID* clientList, anyID* clientID);
	std::vector<anyID> GetChannelContent(uint64 serverConnectionHandlerID, uint64 channelID);
};

extern LCDScreen* screen;