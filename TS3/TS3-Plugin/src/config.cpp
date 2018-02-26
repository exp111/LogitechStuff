#include "config.h"
#include "plugin.h"

LCDScreen* screen = new LCDScreen();

void LCDScreen::AddMessage(const char * msg, anyID sender, const char* senderName)
{
	gotMessage = true;
	newestMessage = std::string(msg);
	messageSender = sender;
	messageSenderName = std::string(senderName);
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
		messageSenderName = "";
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

	if (channelClientList.size() - changed < 7) //if we hit the limit don't
		return;

	cursorPosition = changed;

	//Refresh
	Update();
}

void LCDScreen::SelectActiveItem()
{
	hasSelected = false;

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
		clientCursorPosition = 0;
		adminMenuCursorPosition = 0;
		currentMode = ADMIN;
		Update();
		break;
	case CLIENT_INFO:
		clientCursorPosition = 0;
		adminMenuCursorPosition = 0;
		currentMode = CLIENT_INFO;
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
		changed += MAX_MENU_ITEMS;

	if (changed >= MAX_MENU_ITEMS) //if we hit the limit go back to first
		changed -= MAX_MENU_ITEMS;

	menuCursorPosition = changed;

	//Refresh
	Update();
}

void LCDScreen::ChangeChannelCursorPosition(int changeValue)
{
	int changed = channelCursorPosition + changeValue;

	uint64* channelList;
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	if (ts3Functions.getChannelList(serverConnectionHandlerID, &channelList) != ERROR_ok)
		return;

	unsigned count = 0;
	for (unsigned i = 0; channelList[i]; i++)
	{
		count++;
	}

	if (changed < 0)
		changed += count;

	if (changed >= count) //if we hit the limit go back to first
		changed -= count;

	channelCursorPosition = changed;

	//Refresh
	Update();
}

void LCDScreen::SwitchChannel()
{
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	char* path = new char[512];
	char* pw = new char[512];
	if (hasSelected)
	{
		uint64 channelID;
		ts3Functions.getChannelOfClient(serverConnectionHandlerID, selectedClient, &channelID);
		if (channelID == selectedChannel) //don't move if already in channel
			return;
		//get channel pw
		ts3Functions.getChannelConnectInfo(serverConnectionHandlerID, channelID, path, pw, 512);
		ts3Functions.requestClientMove(serverConnectionHandlerID, selectedClient, selectedChannel, pw, NULL);
	}
	else
	{
		anyID mClientID;
		uint64 mChannelID;
		ts3Functions.getClientID(serverConnectionHandlerID, &mClientID);
		ts3Functions.getChannelOfClient(serverConnectionHandlerID, mClientID, &mChannelID);
		if (mChannelID == selectedChannel) //don't move if already in channel
			return;
		//get channel pw
		ts3Functions.getChannelConnectInfo(serverConnectionHandlerID, mChannelID, path, pw, 512);
		ts3Functions.requestClientMove(serverConnectionHandlerID, mClientID, selectedChannel, pw, NULL);
	}
}

void LCDScreen::SelectClient()
{
	if (!hasSelected)
	{
		hasSelected = true;
		adminMenuCursorPosition = 0;
	}
	else
	{
		uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
		switch (adminMenuCursorPosition)
		{
		case KICK_CHANNEL:
			ts3Functions.requestClientKickFromChannel(serverConnectionHandlerID, selectedClient, "Just cause", NULL);
			break;
		case KICK_SERVER:
			ts3Functions.requestClientKickFromServer(serverConnectionHandlerID, selectedClient, "Just cause", NULL);
			break;
		case MOVE_TO_CHANNEL:
			currentMode = CHANNELS;
			channelCursorPosition = 0; //Reset Cursor Position
			Update();
			return;
			break;
		case MOVE_HERE:
		{
			anyID mClientID;
			uint64 channelID, mChannelID;
			char* path = new char[512];
			char* password = new char[512];
			ts3Functions.getClientID(serverConnectionHandlerID, &mClientID);
			ts3Functions.getChannelOfClient(serverConnectionHandlerID, selectedClient, &channelID);
			ts3Functions.getChannelOfClient(serverConnectionHandlerID, mClientID, &mChannelID);
			
			if (channelID == mChannelID) //don't move if same channel
				return;
			//get pw of my channel
			ts3Functions.getChannelConnectInfo(serverConnectionHandlerID, mChannelID, path, password, 512); 

			ts3Functions.requestClientMove(serverConnectionHandlerID, selectedClient, channelID, password, NULL);
			break;
		}
		case BAN_TEMP:
			ts3Functions.banclient(serverConnectionHandlerID, selectedClient, 10, "Just cause", NULL);
			break;
		case BAN_PERM:
			ts3Functions.banclient(serverConnectionHandlerID, selectedClient, 0, "Just cause", NULL);
			break;
		case MUTE:
		{
			int isMuted;
			ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, selectedClient, CLIENT_IS_MUTED, &isMuted);
			
			anyID clientArray[1] = { selectedClient };
			
			if (isMuted)
				ts3Functions.requestUnmuteClients(serverConnectionHandlerID, clientArray, NULL);
			else
				ts3Functions.requestMuteClients(serverConnectionHandlerID, clientArray, NULL);

			break;
		}
		default:
			break;
		}

		hasSelected = false;
	}

	//Refresh
	Update();
}

void LCDScreen::ChangeClientCursorPosition(int changeValue)
{
	int changed = clientCursorPosition + changeValue;

	anyID* clientList;
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	if (ts3Functions.getClientList(serverConnectionHandlerID, &clientList) != ERROR_ok)
		return;

	unsigned count = 0;
	for (unsigned i = 0; clientList[i]; i++)
	{
		count++;
	}

	if (changed < 0)
		changed += count;

	if (changed >= count) //if we hit the limit go back to first
		changed -= count;

	clientCursorPosition = changed;

	//Refresh
	Update();
}

void LCDScreen::ChangeAdminMenuCursorPosition(int changeValue)
{
	int changed = adminMenuCursorPosition + changeValue;

	if (changed < 0)
		changed += MAX_ADMIN_MENU_ITEMS;

	if (changed >= MAX_ADMIN_MENU_ITEMS) //if we hit the limit go back to first
		changed -= MAX_ADMIN_MENU_ITEMS;

	adminMenuCursorPosition = changed;

	//Refresh
	Update();
}

void LCDScreen::ChangeHelpSite(int changeValue)
{
	int changed = helpSite + changeValue;
	if (changed < 0)
		changed += HELP_MAX_SITES;

	if (changed >= HELP_MAX_SITES)
		changed = 0;

	helpSite = changed;

	//Refresh
	Update();
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
	case ADMIN:
		//ClientCursor
		if (hasSelected)
			ChangeAdminMenuCursorPosition(-1);
		else
			ChangeClientCursorPosition(-1);
		break;
	case CLIENT_INFO:
		if (!hasSelected)
			ChangeClientCursorPosition(-1);
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
	case ADMIN:
		//ClientCursor
		if (hasSelected)
			ChangeAdminMenuCursorPosition(1);
		else
			ChangeClientCursorPosition(1);
		break;
	case CLIENT_INFO:
		if (!hasSelected)
			ChangeClientCursorPosition(1);
		break;
	default:
		break;
	}
}

void LCDScreen::ButtonLeftEvent()
{
	switch (currentMode)
	{
	case NORMAL:
		if (gotMessage) //Message Cursor
			ChangeMessageCursorPosition(-1);
		else //Mute
		{
			MuteInput();
		}
		break;
	case HELP:
		ChangeHelpSite(-1);
		break;
	default:
		break;
	}
}

void LCDScreen::ButtonRightEvent()
{
	switch (currentMode)
	{
	case NORMAL:
		if (gotMessage) //Message Cursor
			ChangeMessageCursorPosition(1);
		else //Mute
		{
			MuteOutput();
		}
		break;
	case HELP:
		ChangeHelpSite(1);
		break;
	default:
		break;
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
	case ADMIN:
		SelectClient();
		break;
	case CLIENT_INFO:
		if (!hasSelected)
		{
			hasSelected = true;
			adminMenuCursorPosition = 0;
		}
		Update();
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
		if (hasSelected)
		{
			menuCursorPosition = 0;
			lastMode = ADMIN;
			currentMode = MENU;
			hasSelected = false;
		}
		else
		{
			menuCursorPosition = 0;
			lastMode = currentMode;
			currentMode = MENU;
		}
		break;
	case ADMIN:
		menuCursorPosition = 0;
		lastMode = currentMode;
		currentMode = MENU;
		break;
	case CLIENT_INFO:
		menuCursorPosition = 0;
		lastMode = currentMode;
		currentMode = MENU;
		break;
	case HELP:
		menuCursorPosition = 0;
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
	if (hasSelected) 
	{
		if (currentMode == ADMIN || currentMode == CLIENT_INFO) //return to admin menu from selection menu
			hasSelected = false;
		else if (currentMode == CHANNELS)
		{
			currentMode = ADMIN;
		}
	}
	else
	{
		currentMode = NORMAL;
		lastMode = NORMAL;
	}

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
	TerminateThread(controlThread, 0);

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
			title = "From: " + messageSenderName + " (" + std::to_string(messageSender) + ")";
		}
		else
		{
			unsigned channelClientCount = channelClientList.size();
			int serverClientCount = 0;
			anyID* clientList;
			if (ts3Functions.getClientList(serverConnectionHandlerID, &clientList) == ERROR_ok)
			{
				for (int i = 0; clientList[i]; i++)
				{
					serverClientCount++;
				}
			}
			//ts3Functions.requestServerVariables(serverConnectionHandlerID); //we need to request for client count
			//ts3Functions.getServerVariableAsInt(serverConnectionHandlerID, VIRTUALSERVER_CLIENTS_ONLINE, &serverClientCount);
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
			int talking, inputMuted, outputMuted, clientType = 0, isMuted = 0;
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
				ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, channelClientList[i], CLIENT_IS_MUTED, &isMuted);
			}

			int red = clientType ? 0 : talking ? 255 : outputMuted ? 160 : inputMuted || isMuted ? 0 : 255;
			int green = clientType ? 255 : talking ? 0 : outputMuted ? 160 : inputMuted || isMuted ? 0 : 255;
			int blue = clientType ? 0 : talking ? 0 : outputMuted ? 160 : inputMuted || isMuted ? 204 : 255;
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
		if (!hasSelected)
		{
			LogiLcdColorSetTitle(_wcsdup(L"Channel Switcher"), red, green, blue);
		}
		else
		{
			std::string text = "Move: ";
			char* clientName = new char[64];
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, selectedClient, CLIENT_NICKNAME, &clientName);
			text += std::string(clientName) + " (" + std::to_string(selectedClient) + ")";
			LogiLcdColorSetTitle(_wcsdup(std::wstring(text.begin(), text.end()).c_str()), red, green, blue);
		}

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
			unsigned start = ((int)channelCursorPosition) - 3 < 0 ? 0 : channelCursorPosition + 5 > count ? ((int)count) - 8 < 0 ? 0 : count - 8 : channelCursorPosition - 3; //so we don't go under 0 and don't go to far
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
				text += " (" + std::to_string(channelList[i]) + ")";
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
	case ADMIN:
	{
		LogiLcdColorSetTitle(_wcsdup(L"Admin Menu"), red, green, blue);
		if (!hasSelected)
		{
			anyID clientID = 0;
			uint64 channelID = 0;
			ts3Functions.getClientID(serverConnectionHandlerID, &clientID);

			anyID* clientList;
			unsigned count = 0;
			if (ts3Functions.getClientList(serverConnectionHandlerID, &clientList) == ERROR_ok)
			{
				for (unsigned i = 0; clientList[i]; i++)
				{
					count++;
				}

				unsigned clientCount = 0; //as we can't print at i we need a seperate counter
				unsigned start = ((int)clientCursorPosition) - 3 < 0 ? 0 : clientCursorPosition + 5 > count ? ((int)count) - 8 < 0 ? 0 : count - 8 : clientCursorPosition - 3; //so we don't go under 0 and don't go to far
				unsigned end = clientCursorPosition + 5 > count ? count : clientCursorPosition + 5 < 8 ? 8 : clientCursorPosition + 5; //don't go over the limit and don't stay to small
				for (unsigned i = start; i < count && i < end; i++)
				{
					anyID current = clientList[i];

					bool active = clientCursorPosition == i;
					if (active)
						selectedClient = clientList[i];
					bool isMe = current == clientID;

					int red = active ? 255 : isMe ? 255 : 255;
					int green = active ? 255 : isMe ? 0 : 255;
					int blue = active ? 0 : isMe ? 0 : 255;

					char* clientName = new char[64];
					ts3Functions.getClientVariableAsString(serverConnectionHandlerID, current, CLIENT_NICKNAME, &clientName);

					std::string text = std::string(clientName) + " (" + std::to_string(clientList[i]) + ")";
					LogiLcdColorSetText(clientCount, _wcsdup(std::wstring(text.begin(), text.end()).c_str()), red, green, blue);
					clientCount++;
				}
			}

			for (unsigned i = count; i < 8; i++) //Empty the other lines
			{
				LogiLcdColorSetText(i, _wcsdup(L""));
			}
		}
		else
		{
			std::string text = "Selected: ";
			char* clientName = new char[64];
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, selectedClient, CLIENT_NICKNAME, &clientName);
			text += std::string(clientName) + " (" + std::to_string(selectedClient) + ")";
			LogiLcdColorSetText(0, _wcsdup(std::wstring(text.begin(), text.end()).c_str()));

			for (unsigned i = 0; i < MAX_ADMIN_MENU_ITEMS; i++)
			{
				bool active = adminMenuCursorPosition == i;
				int red = active ? 255 : 255;
				int green = active ? 255 : 255;
				int blue = active ? 0 : 255;

				std::string text = adminMenuItems[i];
				
				if (i == MUTE)
				{
					int isMuted;
					ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, selectedClient, CLIENT_IS_MUTED, &isMuted);
					if (isMuted)
						text = "Unmute";
				}

				LogiLcdColorSetText(i + 1, _wcsdup(std::wstring(text.begin(), text.end()).c_str()), red, green, blue);
			}

			for (unsigned i = MAX_ADMIN_MENU_ITEMS; i < 8; i++) //Empty the other lines
			{
				LogiLcdColorSetText(i + 1, _wcsdup(L""));
			}
		}
		break;
	}
	case CLIENT_INFO:
	{
		LogiLcdColorSetTitle(_wcsdup(L"Client Info"), red, green, blue);
		if (!hasSelected)
		{
			anyID clientID = 0;
			uint64 channelID = 0;
			ts3Functions.getClientID(serverConnectionHandlerID, &clientID);

			anyID* clientList;
			unsigned count = 0;
			if (ts3Functions.getClientList(serverConnectionHandlerID, &clientList) == ERROR_ok)
			{
				for (unsigned i = 0; clientList[i]; i++)
				{
					count++;
				}

				unsigned clientCount = 0; //as we can't print at i we need a seperate counter
				unsigned start = ((int)clientCursorPosition) - 3 < 0 ? 0 : clientCursorPosition + 5 > count ? ((int)count) - 8 < 0 ? 0 : count - 8 : clientCursorPosition - 3; //so we don't go under 0 and don't go to far
				unsigned end = clientCursorPosition + 5 > count ? count : clientCursorPosition + 5 < 8 ? 8 : clientCursorPosition + 5; //don't go over the limit and don't stay to small
				for (unsigned i = start; i < count && i < end; i++)
				{
					anyID current = clientList[i];

					bool active = clientCursorPosition == i;
					if (active)
						selectedClient = clientList[i];
					bool isMe = current == clientID;

					int red = active ? 255 : isMe ? 255 : 255;
					int green = active ? 255 : isMe ? 0 : 255;
					int blue = active ? 0 : isMe ? 0 : 255;

					char* clientName = new char[64];
					ts3Functions.getClientVariableAsString(serverConnectionHandlerID, current, CLIENT_NICKNAME, &clientName);

					std::string text = std::string(clientName) + " (" + std::to_string(clientList[i]) + ")";
					LogiLcdColorSetText(clientCount, _wcsdup(std::wstring(text.begin(), text.end()).c_str()), red, green, blue);
					clientCount++;
				}
			}

			for (unsigned i = count; i < 8; i++) //Empty the other lines
			{
				LogiLcdColorSetText(i, _wcsdup(L""));
			}
		}
		else
		{
			for (unsigned i = 0; i < 8; i++) //Empty the other lines
			{
				LogiLcdColorSetText(i, _wcsdup(L""));
			}

			std::string text = "Selected: ";
			char* clientName = new char[64];
			ts3Functions.getClientVariableAsString(serverConnectionHandlerID, selectedClient, CLIENT_NICKNAME, &clientName);
			text += std::string(clientName) + " (" + std::to_string(selectedClient) + ")";
			LogiLcdColorSetText(0, _wcsdup(std::wstring(text.begin(), text.end()).c_str()));

			int type = 0;
			ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, selectedClient, ClientPropertiesRare::CLIENT_TYPE, &type);
			if (type == ClientType::ClientType_SERVERQUERY)
			{
				LogiLcdColorSetText(1, _wcsdup(L"This is a Server Query."));
			}
			else
			{
				LogiLcdColorSetText(7, _wcsdup(L"Press OK to refresh"));

				//Request Connection Variables
				ts3Functions.requestConnectionInfo(serverConnectionHandlerID, selectedClient, NULL);

				//Connected Time
				uint64 connectedTime = 0;
				ts3Functions.getConnectionVariableAsUInt64(serverConnectionHandlerID, selectedClient, CONNECTION_CONNECTED_TIME, &connectedTime);
				if (connectedTime > 0) //Format
				{
					int years = connectedTime / 1000 / 60 / 60 / 24 / 365;
					int days = connectedTime / 1000 / 60 / 60 / 24 % 365;
					int hours = connectedTime / 1000 / 60 / 60 % 24;
					int minutes = connectedTime / 1000 / 60 % 60;
					int seconds = connectedTime / 1000 % 60;

					std::string connectionText = "";

					if (years > 0)
						connectionText += std::to_string(years) + "a ";
					if (days > 0)
						connectionText += std::to_string(days) + "d ";
					if (hours > 0)
						connectionText += std::to_string(hours) + "h ";
					if (minutes > 0)
						connectionText += std::to_string(minutes) + "m ";
					connectionText += std::to_string(seconds) + "s";

					std::string text = "Connected since: " + connectionText;

					LogiLcdColorSetText(1, _wcsdup(std::wstring(text.begin(), text.end()).c_str()));
				}

				//Idle Time
				uint64 idleTime = 0;
				ts3Functions.getConnectionVariableAsUInt64(serverConnectionHandlerID, selectedClient, CONNECTION_IDLE_TIME, &idleTime);

				if (idleTime > 0) //Format
				{
					int years = idleTime / 1000 / 60 / 60 / 24 / 365;
					int days = idleTime / 1000 / 60 / 60 / 24 % 365;
					int hours = idleTime / 1000 / 60 / 60 % 24;
					int minutes = idleTime / 1000 / 60 % 60;
					int seconds = idleTime / 1000 % 60;

					std::string idleText = "";

					if (years > 0)
						idleText += std::to_string(years) + "a ";
					if (days > 0)
						idleText += std::to_string(days) + "d ";
					if (hours > 0)
						idleText += std::to_string(hours) + "h ";
					if (minutes > 0)
						idleText += std::to_string(minutes) + "m ";
					idleText += std::to_string(seconds) + "s";

					std::string text = "Idle Time: " + idleText;

					LogiLcdColorSetText(2, _wcsdup(std::wstring(text.begin(), text.end()).c_str()));
				}

				//Ping
				uint64 ping = 0;
				double pingDeviation = 0;
				ts3Functions.getConnectionVariableAsUInt64(serverConnectionHandlerID, selectedClient, CONNECTION_PING, &ping);
				ts3Functions.getConnectionVariableAsDouble(serverConnectionHandlerID, selectedClient, CONNECTION_PING_DEVIATION, &pingDeviation);

				if (ping > 0)
				{
					std::string text = "Ping: ";

					std::ostringstream pingD;
					pingD << ping << " +- " << std::fixed << std::setprecision(1) << pingDeviation;

					text += pingD.str();

					LogiLcdColorSetText(3, _wcsdup(std::wstring(text.begin(), text.end()).c_str()));
				}

				//IP
				char* ip = new char[64];
				if (ts3Functions.getConnectionVariableAsString(serverConnectionHandlerID, selectedClient, CONNECTION_CLIENT_IP, &ip) == ERROR_ok)
				{
					std::string text = "IP: ";

					text += ip;

					LogiLcdColorSetText(4, _wcsdup(std::wstring(text.begin(), text.end()).c_str()));
				}
			}
		}
		break;
	}
	case HELP:
	{
		switch (helpSite)
		{
		case HELP_HELP:
			LogiLcdColorSetTitle(_wcsdup(L"Help - Help Menu"), red, green, blue);
			LogiLcdColorSetText(0, _wcsdup(L"Press LEFT/RIGHT to navigate"), 0, 255, 0);
			LogiLcdColorSetText(1, _wcsdup(L"through the help pages."), 0, 255, 0);
			LogiLcdColorSetText(2, _wcsdup(L""));
			LogiLcdColorSetText(3, _wcsdup(L"Press MENU to open the Menu."), 0, 0, 255);
			LogiLcdColorSetText(4, _wcsdup(L""));
			LogiLcdColorSetText(5, _wcsdup(L""));
			LogiLcdColorSetText(6, _wcsdup(L""));
			LogiLcdColorSetText(7, _wcsdup(L"Press CANCEL to go back."), 255, 255, 255);
			break;
		case HELP_GENERAL:
			LogiLcdColorSetTitle(_wcsdup(L"Help - General"), red, green, blue);
			LogiLcdColorSetText(0, _wcsdup(L"Press anywhere MENU to open"), 0, 255, 0);
			LogiLcdColorSetText(1, _wcsdup(L"the menu."), 0, 255, 0);
			LogiLcdColorSetText(2, _wcsdup(L"Title bar shows talking and"), 255, 255, 255);
			LogiLcdColorSetText(3, _wcsdup(L"mute indicator:"), 255, 255, 255);
			LogiLcdColorSetText(4, _wcsdup(L"Red = Talking"), 255, 0, 0);
			LogiLcdColorSetText(5, _wcsdup(L"Blue = Input Muted"), 0, 0, 255);
			LogiLcdColorSetText(6, _wcsdup(L"Grey = Output Muted"), 160, 160, 160);
			LogiLcdColorSetText(7, _wcsdup(L"Press CANCEL to return."), 255, 255, 255);
			break;
		case HELP_NORMAL:
			LogiLcdColorSetTitle(_wcsdup(L"Help - Normal View"), red, green, blue);
			LogiLcdColorSetText(0, _wcsdup(L"Green Clients are queries."), 0, 255, 0);
			LogiLcdColorSetText(1, _wcsdup(L"Press UP/DOWN to scroll thro"), 0, 0, 255);
			LogiLcdColorSetText(2, _wcsdup(L"ugh the clients."), 0, 0, 255);
			LogiLcdColorSetText(3, _wcsdup(L"Press LEFT/RIGHT to scroll"), 255, 0, 0);
			LogiLcdColorSetText(4, _wcsdup(L"through messages or mute"), 255, 0, 0);
			LogiLcdColorSetText(5, _wcsdup(L"Input/Output."), 255, 0, 0);
			LogiLcdColorSetText(6, _wcsdup(L"Clients use same color"), 255, 255, 0);
			LogiLcdColorSetText(7, _wcsdup(L"format as the title bar."), 255, 255, 0);
			break;
		case HELP_MENU:
			LogiLcdColorSetTitle(_wcsdup(L"Help - Menu"), red, green, blue);
			LogiLcdColorSetText(0, _wcsdup(L"Press MENU to go back to"), 0, 255, 0);
			LogiLcdColorSetText(1, _wcsdup(L"last page."), 0, 255, 0);
			LogiLcdColorSetText(2, _wcsdup(L""));
			LogiLcdColorSetText(3, _wcsdup(L"Press UP/DOWN to navigate"), 0, 0, 255);
			LogiLcdColorSetText(4, _wcsdup(L"through the menu."), 0, 0, 255);
			LogiLcdColorSetText(5, _wcsdup(L"Press OK to select a item."), 255, 0, 0);
			LogiLcdColorSetText(6, _wcsdup(L""));
			LogiLcdColorSetText(7, _wcsdup(L"Press CANCEL to go back."), 255, 255, 255);
			break;
		case HELP_CHANNEL:
			LogiLcdColorSetTitle(_wcsdup(L"Help - Channel Switcher"), red, green, blue);
			LogiLcdColorSetText(0, _wcsdup(L"Press UP/DOWN to navigate"), 0, 0, 255);
			LogiLcdColorSetText(1, _wcsdup(L"through the channels."), 0, 0, 255);
			LogiLcdColorSetText(2, _wcsdup(L"Press OK to switch to"), 255, 0, 0);
			LogiLcdColorSetText(3, _wcsdup(L"a channel."), 255, 0, 0);
			LogiLcdColorSetText(4, _wcsdup(L""));
			LogiLcdColorSetText(5, _wcsdup(L""));
			LogiLcdColorSetText(6, _wcsdup(L""));
			LogiLcdColorSetText(7, _wcsdup(L"Press CANCEL to go back."), 255, 255, 255);
			break;
		case HELP_ADMIN:
			LogiLcdColorSetTitle(_wcsdup(L"Help - Admin Menu"), red, green, blue);
			LogiLcdColorSetText(0, _wcsdup(L"Press UP/DOWN to navigate"), 0, 0, 255);
			LogiLcdColorSetText(1, _wcsdup(L"through the clients."), 0, 0, 255);
			LogiLcdColorSetText(2, _wcsdup(L"Press OK to select a client."), 255, 0, 0);
			LogiLcdColorSetText(3, _wcsdup(L""));
			LogiLcdColorSetText(4, _wcsdup(L""));
			LogiLcdColorSetText(5, _wcsdup(L""));
			LogiLcdColorSetText(6, _wcsdup(L""));
			LogiLcdColorSetText(7, _wcsdup(L"Press CANCEL to go back."), 255, 255, 255);
			break;
		default:
			break;
		}
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