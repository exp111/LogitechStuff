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

enum LCD_MODES
{
	NORMAL = 0,
	MENU = 1,
	CHANNELS,
	ADMIN,
};

class LCDScreen
{
private:
	bool gotMessage = false;
	std::string newestMessage = "";
	anyID messageSender = 0;
	std::string messageSenderName = "";
	unsigned messageCursorPosition = 0;

	bool isActive = false;
	unsigned cursorPosition = 0;
	HANDLE controlThread = 0;
	struct TS3Functions ts3Functions;
	int currentMode = NORMAL;
	int lastMode = NORMAL;

	enum MENU_ITEMS
	{
		MUTE_INPUT = 0,
		MUTE_OUTPUT = 1,
		SWITCH_CHANNEL,
		ADMIN_MENU,
		HELP,
		MAX_MENU_ITEMS
	};
	const std::vector<std::string> menuItems = { "Mute Input", "Mute Output", "Switch Channel", "Admin Menu (WIP)", "Help" };
	unsigned menuCursorPosition = 0;

	uint64 selectedChannel = 0;
	unsigned channelCursorPosition = 0;

public:
	LCDScreen() {};
	~LCDScreen()
	{
		if (isActive)
			Shutdown();
	}
	
	void Init();
	void Shutdown();

	//Message related
	void AddMessage(const char* msg, anyID sender, const char* senderName);
	void RemoveMessage();
	void ChangeMessageCursorPosition(int changeValue);

	//Normal Cursor
	unsigned GetPosition() const;
	void ChangeCursorPosition(int changeValue);

	//Menu
	void SelectActiveItem();
	void ChangeMenuCursorPosition(int changeValue);

	//Channel
	void ChangeChannelCursorPosition(int changeValue);
	void SwitchChannel();

	bool IsActive() const { return isActive; }

	void SetTS3Functions(const struct TS3Functions funcs);

	void Update();

	void StartControlThread(); //Check controls in another thread

	//Button Events
	void ButtonUpEvent();
	void ButtonDownEvent();

	void ButtonLeftEvent();
	void ButtonRightEvent();

	void ButtonOKEvent();

	void ButtonMenuEvent();
	void ButtonCancelEvent();

	//Mute Helpers
	void MuteInput();
	void MuteOutput();

	//Helper
	bool isClientInList(anyID* clientList, anyID* clientID);
	std::vector<anyID> GetChannelContent(uint64 serverConnectionHandlerID, uint64 channelID);
};

extern LCDScreen* screen;