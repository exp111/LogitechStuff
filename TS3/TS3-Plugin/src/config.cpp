#include "config.h"
#include "plugin.h"

Config* config = new Config;

LCDScreen* screen = new LCDScreen();

void LCDScreen::AddMessage(const char * msg, anyID sender)
{
	gotMessage = true;
	newestMessage = std::string(msg);
	messageSender = sender;
}

void LCDScreen::ChangePosition(int changeValue)
{
	int changed = config->pos + changeValue;
	if (changed < 0)
		return;

	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	anyID clientID = 0;
	uint64 channelID = 0;
	ts3Functions.getClientID(serverConnectionHandlerID, &clientID);
	ts3Functions.getChannelOfClient(serverConnectionHandlerID, clientID, &channelID);

	std::vector<anyID> channelClientList = GetChannelContent(serverConnectionHandlerID, channelID);

	if (channelClientList.size() - changed <= 7) //if we have less than 7 people no need to change the pos
		return;

	config->pos = changed;

	//Refresh
	Update();
}

DWORD WINAPI ControlThread(void * data)
{
	while (screen->IsActive())
	{
		if (LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_UP))
			screen->ChangePosition(-1);
		else if (LogiLcdIsButtonPressed(LOGI_LCD_COLOR_BUTTON_DOWN))
			screen->ChangePosition(1);

		Sleep(100);
	}

	return 0;
}

void LCDScreen::StartControlThread()
{
	controlThread = CreateThread(NULL, 0, ControlThread, NULL, 0, NULL);
}

void LCDScreen::Init()
{
	LogiLcdInit(_wcsdup(L"TeamSpeak 3"), LOGI_LCD_TYPE_MONO | LOGI_LCD_TYPE_COLOR);
	LogiLcdUpdate(); //Force Update

	isActive = true;

	StartControlThread();
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
	anyID clientID = 0;
	uint64 channelID = 0;
	ts3Functions.getClientID(serverConnectionHandlerID, &clientID);
	ts3Functions.getChannelOfClient(serverConnectionHandlerID, clientID, &channelID);

	std::vector<anyID> channelClientList = GetChannelContent(serverConnectionHandlerID, channelID);

	//Title (ex: TeamSpeak 3 - 3/15)
	int talking, inputMuted, outputMuted;
	ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_FLAG_TALKING, &talking);
	ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_INPUT_MUTED, &inputMuted);
	ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_OUTPUT_MUTED, &outputMuted);

	int red = talking ? 255 : outputMuted ? 160 : inputMuted ? 0 : 255;
	int green = talking ? 0 : outputMuted ? 160 : inputMuted ? 0 : 255;
	int blue = talking ? 0 : outputMuted ? 160 : inputMuted ? 204 : 255;

	unsigned channelClientCount = channelClientList.size();
	int serverClientCount = 0;
	ts3Functions.requestServerVariables(serverConnectionHandlerID); //we need to request for client count
	ts3Functions.getServerVariableAsInt(serverConnectionHandlerID, VIRTUALSERVER_CLIENTS_ONLINE, &serverClientCount);
	std::string title = "TS3 - " + std::to_string(channelClientCount) + "/" + std::to_string(serverClientCount);
	LogiLcdColorSetTitle(_wcsdup(std::wstring(title.begin(), title.end()).c_str()), red, green, blue);

	//ServerName (ex: MaCoGa - Dj's Kotstube)
	char* serverName = new char[128];
	char* channelName = new char[128];
	std::wstring serverChannelNames = L"";
	ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_NAME, &serverName);
	ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, channelID, CHANNEL_NAME, &channelName);
	std::string sServerName{ serverName };
	if (sServerName.size() > 13) //30 is the maximum so resize it to have the '-' in the middle
		sServerName.resize(13);
	std::string sChannelName{ channelName };
	serverChannelNames += std::wstring(sServerName.begin(), sServerName.end()) + L" - " + std::wstring(sChannelName.begin(), sChannelName.end());
	LogiLcdColorSetText(0, _wcsdup(serverChannelNames.c_str()));

	if (channelClientList.size() - config->pos < 7) //if people have left and we have space move the pos so the screen is full
	{
		config->pos -= (7 - (channelClientList.size() - config->pos));
	}

	if (config->pos > channelClientList.size())
		config->pos = 0; //Better reset it if I fucked up

						 //Clients	//start at pos ; go till we hit the limit (no more clients/no more space)
						 //int position = 0;
	for (unsigned i = config->pos; i < channelClientList.size() && i < config->pos + 7; i++)
	{
		//if (channelClientList[i] == clientID) //TODO
		//	continue;
		//position++;
		char* clientName = new char[64];
		int talking, inputMuted, outputMuted;
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
		}

		int red = talking ? 255 : outputMuted ? 160 : inputMuted ? 0 : 255;
		int green = talking ? 0 : outputMuted ? 160 : inputMuted ? 0 : 255;
		int blue = talking ? 0 : outputMuted ? 160 : inputMuted ? 204 : 255;
		//std::string status = inputMuted && outputMuted ? "<IO> " : inputMuted ? "<I> " : outputMuted ? "<O> " : "";
		std::string text = std::string(clientName);
		LogiLcdColorSetText(i + 1 - config->pos, _wcsdup(std::wstring(text.begin(), text.end()).c_str()), red, green, blue);
	}

	for (unsigned i = channelClientList.size(); i < 7; i++) //Empty the other lines
	{
		LogiLcdColorSetText(i + 1, _wcsdup(L""));
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