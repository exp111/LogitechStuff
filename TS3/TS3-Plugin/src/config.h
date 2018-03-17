#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include <LogitechLCDLib.h>

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

	unsigned sendCursorPosition = 0;
public:
	std::vector<std::string> sendItems = {  };
private:
	enum MENU_ITEMS
	{
		MUTE_INPUT = 0,
		MUTE_OUTPUT = 1,
		SWITCH_CHANNEL,
		ADMIN_MENU,
		CLIENT_INFO,
		SEND_CHANNEL,
		HELP,
		MAX_MENU_ITEMS
	};
	const std::vector<std::string> menuItems = { "Mute Input", "Mute Output", "Switch Channel", "Admin Menu", "Client Info", "Send Channel Messages","Help" };
	unsigned menuCursorPosition = 0;

	uint64 selectedChannel = 0;
	unsigned channelCursorPosition = 0;

	enum HELP_SITES
	{
		HELP_HELP = 0,
		HELP_GENERAL = 1,
		HELP_NORMAL,
		HELP_MENU,
		HELP_CHANNEL,
		HELP_ADMIN,
		HELP_MAX_SITES
	};
	unsigned helpSite = 0;

	enum ADMIN_MENU_ITEMS
	{
		KICK_CHANNEL = 0,
		KICK_SERVER = 1,
		MOVE_TO_CHANNEL,
		MOVE_HERE,
		BAN_TEMP,
		BAN_PERM,
		MUTE,
		MAX_ADMIN_MENU_ITEMS
	};
	const std::vector<std::string> adminMenuItems = { "Kick from Channel", "Kick from Server", "Move to Channel", "Move to my Channel", "Ban Temporary", "Ban Permanent", "Mute (Buggy)" };
	anyID selectedClient = 0;
	unsigned clientCursorPosition = 0;
	bool hasSelected = false;
	unsigned adminMenuCursorPosition = 0;

	const unsigned refreshRate = 50;

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

	//Admin Menu -> Clients
	void SelectClient();
	void ChangeClientCursorPosition(int changeValue);
	void ChangeAdminMenuCursorPosition(int changeValue);

	//Send Channel
	void SendChannelMessage();
	void ChangeSendCursorPosition(int changeValue);

	//Help
	void ChangeHelpSite(int changeValue);

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