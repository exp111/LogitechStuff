#include "config.h"
#include "plugin.h"

Config* config = new Config;

LCDScreen* screen = new LCDScreen();

void LCDScreen::AddMessage(const char * msg, anyID sender)
{
	gotMessage = true;
	newestMessage = std::string(msg);
	messageSender = sender;
	messageCursorPosition = 0;

	//Refresh
	Update();
}

void LCDScreen::RemoveMessage()
{
	if (gotMessage)
	{
		newestMessage = "";
		messageSender = 0;
		gotMessage = false;
		messageCursorPosition = 0;
	}

	//Refresh
	Update();
}

void LCDScreen::ChangeMessageCursorPosition(int changeValue)
{
	int changed = messageCursorPosition + changeValue;
	if (changed < 0)
		return;

	std::string message(newestMessage);
	
	if (message.size() < 29) //29 is the limit we can show; if the message is shorter we don't need to change
		return;

	if (message.size() - changed < 29) //if we're over the limit fuck off
		return;

	messageCursorPosition = changed;

	//Refresh
	Update();
}

void LCDScreen::ChangeCursorPosition(int changeValue)
{
	int changed = cursorPosition + changeValue;
	if (changed < 0)
		return;

	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	anyID clientID = 0;
	uint64 channelID = 0;
	ts3Functions.getClientID(serverConnectionHandlerID, &clientID);
	ts3Functions.getChannelOfClient(serverConnectionHandlerID, clientID, &channelID);

	std::vector<anyID> channelClientList = GetChannelContent(serverConnectionHandlerID, channelID);

	if (channelClientList.size() - changed <= 7) //if we hit the limit don't
		return;

	cursorPosition = changed;

	//Refresh
	Update();
}

void LCDScreen::SelectActiveItem()
{
	switch (menuCursorPosition)
	{
	case MUTE_INPUT:
		MuteInput();
		break;
	case MUTE_OUTPUT:
		MuteOutput();
		break;
	case SWITCH_CHANNEL:
		currentMode = CHANNELS;
		channelCursorPosition = 0;
		Update();
		break;
	case ADMIN_MENU:
		currentMode = ADMIN;
		Update();
		break;
	case HELP:
		currentMode = HELP;
		Update();
		break;
	default:
		break;
	}
}

void LCDScreen::ChangeMenuCursorPosition(int changeValue)
{
	int changed = menuCursorPosition + changeValue;
	if (changed < 0)
		return;

	if (MAX_MENU_ITEMS - changed <= 0) //if we hit the limit don't;
		return;

	menuCursorPosition = changed;

	//Refresh
	Update();
}

void LCDScreen::ChangeChannelCursorPosition(int changeValue)
{
	int changed = channelCursorPosition + changeValue;
	if (changed < 0)
		return;

	uint64* channelList;
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	if (ts3Functions.getChannelList(serverConnectionHandlerID, &channelList) != ERROR_ok)
		return;

	unsigned count = 0;
	for (unsigned i = 0; channelList[i]; i++)
	{
		count++;
	}

	if (count - changed <= 0) //if we hit the limit don't;
		return;

	channelCursorPosition = changed;

	//Refresh
	Update();
}

void LCDScreen::SwitchChannel()
{
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	anyID mClientID;
	uint64 mChannelID;
	ts3Functions.getClientID(serverConnectionHandlerID, &mClientID);
	ts3Functions.getChannelOfClient(serverConnectionHandlerID, mClientID, &mChannelID);
	if (mChannelID == selectedChannel)
		return;
	ts3Functions.requestClientMove(serverConnectionHandlerID, mClientID, selectedChannel, "", NULL);
}

DWORD WINAPI ControlThread(void * data)
{
	while (screen->IsActive())
	{
		if (LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_UP))
			screen->ButtonUpEvent();
		else if (LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_DOWN))
			screen->ButtonDownEvent();
		else if (LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_LEFT))
			screen->ButtonLeftEvent();
		else if (LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_RIGHT))
			screen->ButtonRightEvent();
		else if (LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_OK))
			screen->ButtonOKEvent();
		else if (LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_MENU))
			screen->ButtonMenuEvent();
		else if (LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_CANCEL))
			screen->ButtonCancelEvent();

		Sleep(150);
	}

	return 0;
}

void LCDScreen::StartControlThread()
{
	controlThread = CreateThread(NULL, 0, ControlThread, NULL, 0, NULL);
}

void LCDScreen::ButtonUpEvent()
{
	switch (currentMode)
	{
	case NORMAL:
		//ClientCursor
		ChangeCursorPosition(-1);
		break;
	case MENU:
		//MenuCursor
		ChangeMenuCursorPosition(-1);
		break;
	case CHANNELS:
		//ChannelCursor
		ChangeChannelCursorPosition(-1);
		break;
	default:
		break;
	}
}

void LCDScreen::ButtonDownEvent()
{
	switch (currentMode)
	{
	case NORMAL:
		//ClientCursor
		ChangeCursorPosition(1);
		break;
	case MENU:
		//MenuCursor
		ChangeMenuCursorPosition(1);
		break;
	case CHANNELS:
		//ChannelCursor
		ChangeChannelCursorPosition(1);
		break;
	default:
		break;
	}
}

void LCDScreen::ButtonLeftEvent()
{
	if (currentMode == NORMAL)
	{
		if (gotMessage) //Message Cursor
			ChangeMessageCursorPosition(-1);
		else //Mute
		{
			MuteInput();
		}
	}
}

void LCDScreen::ButtonRightEvent()
{
	if (currentMode == NORMAL)
	{
		if (gotMessage) //Message Cursor
			ChangeMessageCursorPosition(1);
		else //Mute
		{
			MuteOutput();
		}
	}
}

void LCDScreen::ButtonOKEvent()
{
	switch (currentMode)
	{
	case NORMAL:
		RemoveMessage();
		break;
	case MENU:
		SelectActiveItem();
		break;
	case CHANNELS:
		SwitchChannel();
		break;
	default:
		break;
	}
}

void LCDScreen::ButtonMenuEvent()
{
	switch (currentMode)
	{
	case NORMAL:
		menuCursorPosition = 0;
		lastMode = currentMode;
		currentMode = MENU;
		break;
	case MENU:
		currentMode = lastMode;
		break;
	case CHANNELS:
		menuCursorPosition = 0;
		lastMode = currentMode;
		currentMode = MENU;
		break;
	case ADMIN:
		menuCursorPosition = 0;
		lastMode = currentMode;
		currentMode = MENU;
		break;
	case HELP:
		lastMode = currentMode;
		currentMode = MENU;
	default:
		break;
	}

	//Refresh
	Update();
}

void LCDScreen::ButtonCancelEvent()
{
	currentMode = NORMAL;
	lastMode = NORMAL;

	//Refresh
	Update();
}

void LCDScreen::MuteInput()
{
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	int inputMuted;
	if (ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_INPUT_MUTED, &inputMuted) == ERROR_ok)
	{
		ts3Functions.setClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_INPUT_MUTED, !inputMuted);
		ts3Functions.flushClientSelfUpdates(serverConnectionHandlerID, NULL); //send update to server
	}
}

void LCDScreen::MuteOutput()
{
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	int outputMuted;
	if (ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_OUTPUT_MUTED, &outputMuted) == ERROR_ok)
	{
		ts3Functions.setClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_OUTPUT_MUTED, !outputMuted);
		ts3Functions.flushClientSelfUpdates(serverConnectionHandlerID, NULL); //send update to server
	}
}

void LCDScreen::Init()
{
	LogiLcdInit(_wcsdup(L"TeamSpeak 3"), LOGI_LCD_TYPE_MONO | LOGI_LCD_TYPE_COLOR);

	isActive = true;

	StartControlThread();

	//Refresh
	Update();
}

void LCDScreen::Shutdown()
{
	LogiLcdShutdown();

	isActive = false;
}

void LCDScreen::SetTS3Functions(const TS3Functions funcs)
{
	ts3Functions = funcs;
}

void LCDScreen::Update()
{
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();

	//Talking & Muted Status + title bar colors
	int talking, inputMuted, outputMuted;
	ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_FLAG_TALKING, &talking);
	ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_INPUT_MUTED, &inputMuted);
	ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_OUTPUT_MUTED, &outputMuted);

	int red = talking ? 255 : outputMuted ? 160 : inputMuted ? 0 : 255;
	int green = talking ? 0 : outputMuted ? 160 : inputMuted ? 0 : 255;
	int blue = talking ? 0 : outputMuted ? 160 : inputMuted ? 204 : 255;

	switch (currentMode)
	{
	case NORMAL:
	{
		anyID clientID = 0;
		uint64 channelID = 0;
		ts3Functions.getClientID(serverConnectionHandlerID, &clientID);
		ts3Functions.getChannelOfClient(serverConnectionHandlerID, clientID, &channelID);

		std::vector<anyID> channelClientList = GetChannelContent(serverConnectionHandlerID, channelID);

		//Title (ex: TeamSpeak 3 - 3/15)
		std::string title;
		if (gotMessage)
		{
			char* messageSenderName = new char[64];
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, messageSender, CLIENT_NICKNAME, &messageSenderName);
			title = "From: " + std::string(messageSenderName);
		}
		else
		{
			unsigned channelClientCount = channelClientList.size();
			int serverClientCount = 0;
			ts3Functions.requestServerVariables(serverConnectionHandlerID); //we need to request for client count
			ts3Functions.getServerVariableAsInt(serverConnectionHandlerID, VIRTUALSERVER_CLIENTS_ONLINE, &serverClientCount);
			title = "TS3 - " + std::to_string(channelClientCount) + "/" + std::to_string(serverClientCount);
		}
		LogiLcdColorSetTitle(_wcsdup(std::wstring(title.begin(), title.end()).c_str()), red, green, blue);

		//ServerName (ex: MaCoGa - Dj's Kotstube)
		std::string fLineText = "";
		if (gotMessage)
		{
			std::string message(newestMessage);
			message = message.substr(messageCursorPosition); //cut the first messageCursorPosition characters off
			fLineText += message;
		}
		else
		{
			bool connected = true;
			char* serverName = new char[128];
			char* channelName = new char[128];
			if (ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_NAME, &serverName) == ERROR_not_connected)
				connected = false;
			if (ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, channelID, CHANNEL_NAME, &channelName) == ERROR_not_connected)
				connected = false;
			std::string sServerName{ serverName };
			if (sServerName.size() > 13) //30 is the maximum so resize it to have the '-' in the middle
				sServerName.resize(13);
			std::string sChannelName{ channelName };
			fLineText += connected ? sServerName + " - " + sChannelName : "Not Connected.";
		}
		LogiLcdColorSetText(0, _wcsdup(std::wstring(fLineText.begin(), fLineText.end()).c_str()));

		if (channelClientList.size() - cursorPosition < 7) //if people have left and we have space move the pos so the screen is full
		{
			cursorPosition -= (7 - (channelClientList.size() - cursorPosition));
		}

		if (cursorPosition > channelClientList.size())
			cursorPosition = 0; //Better reset it if I fucked up

							 //Clients	//start at pos ; go till we hit the limit (no more clients/no more space)
							 //int position = 0;
		for (unsigned i = cursorPosition; i < channelClientList.size() && i < cursorPosition + 7; i++)
		{
			//if (channelClientList[i] == clientID) //TODO
			//	continue;
			//position++;
			char* clientName = new char[64];
			int talking, inputMuted, outputMuted, clientType = 0;
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, channelClientList[i], CLIENT_NICKNAME, &clientName);

			if (channelClientList[i] == clientID)
			{
				ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_FLAG_TALKING, &talking);
				ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_INPUT_MUTED, &inputMuted);
				ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_OUTPUT_MUTED, &outputMuted);
			}
			else
			{
				ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, channelClientList[i], CLIENT_FLAG_TALKING, &talking);
				ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, channelClientList[i], CLIENT_INPUT_MUTED, &inputMuted);
				ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, channelClientList[i], CLIENT_OUTPUT_MUTED, &outputMuted);
				ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, channelClientList[i], CLIENT_TYPE, &clientType);
			}

			int red = clientType ? 0 : talking ? 255 : outputMuted ? 160 : inputMuted ? 0 : 255;
			int green = clientType ? 255 : talking ? 0 : outputMuted ? 160 : inputMuted ? 0 : 255;
			int blue = clientType ? 0 : talking ? 0 : outputMuted ? 160 : inputMuted ? 204 : 255;
			//std::string status = inputMuted && outputMuted ? "<IO> " : inputMuted ? "<I> " : outputMuted ? "<O> " : "";
			std::string text = std::string(clientName);
			LogiLcdColorSetText(i + 1 - cursorPosition, _wcsdup(std::wstring(text.begin(), text.end()).c_str()), red, green, blue);
		}

		for (unsigned i = channelClientList.size(); i < 7; i++) //Empty the other lines
		{
			LogiLcdColorSetText(i + 1, _wcsdup(L""));
		}
		break;
	}
	case MENU:
	{
		LogiLcdColorSetTitle(_wcsdup(L"Menu"), red, green, blue);

		for (unsigned i = 0; i < MAX_MENU_ITEMS; i++)
		{
			bool active = menuCursorPosition == i;
			int red = active ? 255 : 255;
			int green = active ? 255 : 255;
			int blue = active ? 0 : 255;

			LogiLcdColorSetText(i, _wcsdup(std::wstring(menuItems[i].begin(), menuItems[i].end()).c_str()), red, green, blue);
		}

		for (unsigned i = MAX_MENU_ITEMS; i < 8; i++) //Empty the other lines
		{
			LogiLcdColorSetText(i, _wcsdup(L""));
		}
		break;
	}
	case CHANNELS:
	{
		LogiLcdColorSetTitle(_wcsdup(L"Channel Switcher"), red, green, blue);

		anyID clientID = 0;
		uint64 channelID = 0;
		ts3Functions.getClientID(serverConnectionHandlerID, &clientID);
		ts3Functions.getChannelOfClient(serverConnectionHandlerID, clientID, &channelID);

		uint64* channelList;
		unsigned count = 0;
		if (ts3Functions.getChannelList(serverConnectionHandlerID, &channelList) == ERROR_ok)
		{
			for (unsigned i = 0; channelList[i]; i++)
			{
				count++;
			}

			unsigned channelCount = 0; //as we can't print at i we need a seperate counter
			unsigned start = ((int)channelCursorPosition) - 3 < 0 ? 0 : channelCursorPosition + 5 > count ? count - 8 : channelCursorPosition - 3; //so we don't go under 0 and don't go to far
			unsigned end = channelCursorPosition + 5 > count ? count : channelCursorPosition + 5 < 8 ? 8 : channelCursorPosition + 5; //don't go over the limit and don't stay to small
			for (unsigned i = start; i < count && i < end; i++)
			{
				uint64 current = channelList[i];

				bool active = channelCursorPosition == i;
				if (active)
					selectedChannel = channelList[i];
				bool isMyChannel = current == channelID;

				int red = active ? 255 : isMyChannel ? 255 : 255;
				int green = active ? 255 : isMyChannel ? 0 : 255;
				int blue = active ? 0 : isMyChannel ? 0 : 255;

				char* channelName = new char[64];
				ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, current, CHANNEL_NAME, &channelName);

				std::string text(channelName);
				LogiLcdColorSetText(channelCount, _wcsdup(std::wstring(text.begin(), text.end()).c_str()), red, green, blue);
				channelCount++;
			}
		}

		for (unsigned i = count; i < 8; i++) //Empty the other lines
		{
			LogiLcdColorSetText(i, _wcsdup(L""));
		}
		break;
	}
	case ADMIN: //TODO
	{
		LogiLcdColorSetTitle(_wcsdup(L"Admin Menu"), red, green, blue);


		break;
	}
	case HELP:
	{
		LogiLcdColorSetTitle(_wcsdup(L"Help"), red, green, blue);
		LogiLcdColorSetText(0, _wcsdup(L"Normal: L/R for In/O Mute &"), 0, 255, 0);
		LogiLcdColorSetText(1, _wcsdup(L"MSG Scroll; U/D for Client"), 0, 255, 0);
		LogiLcdColorSetText(2, _wcsdup(L"Scroll; OK to hide messages."), 0, 255, 0);

		LogiLcdColorSetText(3, _wcsdup(L"Channel:U/D for Channel Scro"), 0, 0, 255);
		LogiLcdColorSetText(4, _wcsdup(L"ll; OK to go to the channel."), 0, 0, 255);

		LogiLcdColorSetText(5, _wcsdup(L"Admin:U/D for Client Scroll"), 255, 0, 0);
		LogiLcdColorSetText(6, _wcsdup(L"OK to do something."), 255, 0, 0);

		LogiLcdColorSetText(7, _wcsdup(L"Cancel to go back to normal."), 255, 255, 255);
		break;
	}
	default:
		break;
	}

	//Refresh Screen
	LogiLcdUpdate();
}

bool LCDScreen::isClientInList(anyID* clientList, anyID* clientID)
{
	for (int i = 0; clientList[i]; i++)
	{
		//ts3Functions.printMessageToCurrentTab(std::string(std::to_string(clientList[i]) + ";" +std::to_string(*clientID) + "\n").c_str());
		if (clientList[i] == *clientID)
			return true;
	}

	return false;
}

std::vector<anyID> LCDScreen::GetChannelContent(uint64 serverConnectionHandlerID, uint64 channelID)
{
	std::vector<anyID> channelClientVector = std::vector<anyID>();

	anyID* channelClientList;
	ts3Functions.getChannelClientList(serverConnectionHandlerID, channelID, &channelClientList);

	anyID* clientList;
	if (ts3Functions.getClientList(serverConnectionHandlerID, &clientList) != ERROR_ok)
		return channelClientVector;

	int clientType;
	for (int i = 0; clientList[i]; i++)
	{
		if (ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, clientList[i], CLIENT_TYPE, &clientType) != ERROR_ok)
			continue;

		//if (clientType == 1) //Query
		//	continue;

		if (!isClientInList(channelClientList, &clientList[i]))
			continue;

		channelClientVector.push_back(clientList[i]);
	}

	return channelClientVector;
}

/*unsigned LCDScreen::GetChannelContentCount(uint64 serverConnectionHandlerID, uint64 channelID)
{
	unsigned count = 0;

	anyID* channelClientList;
	ts3Functions.getChannelClientList(serverConnectionHandlerID, channelID, &channelClientList);

	anyID* clientList;
	ts3Functions.getClientList(serverConnectionHandlerID, &clientList);

	int clientType;
	for (int i = 0; clientList[i]; i++)
	{
		if (ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, clientList[i], CLIENT_TYPE, &clientType) != ERROR_ok)
			continue;

		//if (clientType == 1) //Query
		//	continue;

		if (!isClientInList(channelClientList, &clientList[i]))
			continue;

		count++;
	}

	return count;
}*/