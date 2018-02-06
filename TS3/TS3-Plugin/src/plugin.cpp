/*
* TeamSpeak 3 demo plugin
*
* Copyright (c) 2008-2017 TeamSpeak Systems GmbH
*/

#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"
#include <iostream>
#include <string>
//#include "config.h"
#include "QtConfig.h"
#include "config.h"

#include <LogitechLCDLib.h>
#pragma comment(lib, "LogitechLCDLib.lib")

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 22

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

//Global Variables
static char* pluginID = NULL;

#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if (WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif

/*********************************** Required functions ************************************/


const char* ts3plugin_name() {
#ifdef _WIN32
	static char* result = NULL;  /* Static variable so it's allocated only once */
	if (!result) {
		const wchar_t* name = L"Logitech G19 Plugin";
		if (wcharToUtf8(name, &result) == -1) {  /* Convert serverName into UTF-8 encoded result */
			result = "Logitech G19 Plugin";  /* Conversion failed, fallback here */
		}
	}
	return result;
#else
	return "Logitech G19 Plugin";
#endif
}

const char* ts3plugin_version() {
	return "1.0";
}

int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

const char* ts3plugin_author() {
	return "Exp";
}

const char* ts3plugin_description() {
	return "Unofficial Plugin for the Logitech G19 keyboard.";
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
	ts3Functions = funcs;
}

int ts3plugin_init() {
	char appPath[PATH_BUFSIZE];
	char resourcesPath[PATH_BUFSIZE];
	char configPath[PATH_BUFSIZE];
	char pluginPath[PATH_BUFSIZE];

	ts3Functions.getAppPath(appPath, PATH_BUFSIZE);
	ts3Functions.getResourcesPath(resourcesPath, PATH_BUFSIZE);
	ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);
	ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE, pluginID);

	LogiLcdInit(_wcsdup(L"TeamSpeak 3"), LOGI_LCD_TYPE_MONO | LOGI_LCD_TYPE_COLOR);
	LogiLcdUpdate();

	return 0;
}

void ts3plugin_shutdown() {
	LogiLcdShutdown();
	if (pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

int ts3plugin_offersConfigure() {
	return PLUGIN_OFFERS_CONFIGURE_QT_THREAD;
}

void ts3plugin_configure(void* handle, void* qParentWidget)
{
	Q_UNUSED(handle);
	QtConfig* cfg = new QtConfig((QWidget*)qParentWidget);
	cfg->setAttribute(Qt::WA_DeleteOnClose);
	cfg->show();
}

void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);
}

const char* ts3plugin_commandKeyword() {
	//return "test";
	return ""; //Command Suffix for chat; example: /test
}

const char* ts3plugin_infoTitle() {
	return "Logitech G19 Plugin";
}

void ts3plugin_freeMemory(void* data) {
	free(data);
}
int ts3plugin_requestAutoload() {
	return 0;
}

//Helper Functions

/* Helper function to create a menu item */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
	struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
	menuItem->type = type;
	menuItem->id = id;
	_strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
	_strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
	return menuItem;
}

/* Some makros to make the code to create menu items a bit more readable */
#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *menuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*menuItems)[n++] = NULL; assert(n == sz);


enum {
	MENU_ID_GLOBAL_1 = 1,
	MENU_ID_GLOBAL_2,
	MENU_ID_GLOBAL_3,
	MENU_ID_GLOBAL_4,
	MENU_ID_MAX
};

void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {

	BEGIN_CREATE_MENUS(MENU_ID_MAX - 1); //Needs to be correct
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_1, "Print Test Bool", "");
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_2, "Force Update", "");
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_3, "Pos -1", "");
	CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_4, "Pos +1", "");
	END_CREATE_MENUS;

	*menuIcon = (char*)malloc(PLUGIN_MENU_BUFSZ * sizeof(char));
	_strcpy(*menuIcon, PLUGIN_MENU_BUFSZ, ""); //PLUGIN MENU IMAGE
}

bool isClientInList(anyID* clientList, anyID* clientID) 
{
	for (int i = 0; clientList[i]; i++)
	{
		//ts3Functions.printMessageToCurrentTab(std::string(std::to_string(clientList[i]) + ";" +std::to_string(*clientID) + "\n").c_str());
		if (clientList[i] == *clientID) 
			return true;
	}

	return false;
}

std::vector<anyID> GetChannelContent(uint64 serverConnectionHandlerID, uint64 channelID)
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

unsigned GetChannelContentCount(uint64 serverConnectionHandlerID, uint64 channelID)
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
}

void Update()
{
	uint64 serverConnectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	anyID clientID = 0;
	uint64 channelID = 0;
	ts3Functions.getClientID(serverConnectionHandlerID, &clientID);
	ts3Functions.getChannelOfClient(serverConnectionHandlerID, clientID, &channelID);

	std::vector<anyID> channelClientList = GetChannelContent(serverConnectionHandlerID, channelID);

	//Title (ex: TeamSpeak 3 - 3/15)
	//unsigned channelClientCount = GetChannelContentCount(serverConnectionHandlerID, channelID);
	unsigned channelClientCount = channelClientList.size();
	int serverClientCount = 0;
	ts3Functions.requestServerVariables(serverConnectionHandlerID); //we need to request for client count
	ts3Functions.getServerVariableAsInt(serverConnectionHandlerID, VIRTUALSERVER_CLIENTS_ONLINE, &serverClientCount);
	std::string title = "TS3 - " + std::to_string(channelClientCount) + "/" + std::to_string(serverClientCount);
	LogiLcdColorSetTitle(_wcsdup(std::wstring(title.begin(), title.end()).c_str()));

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
	for (unsigned i = config->pos; i < channelClientList.size() && i < config->pos + 7; i++)
	{
		char* clientName = new char[64];
		ts3Functions.getClientDisplayName(serverConnectionHandlerID, channelClientList[i], clientName, 64);
		std::string sClientName(clientName); //queries get faulty name
		LogiLcdColorSetText(i + 1 - config->pos, _wcsdup(std::wstring(sClientName.begin(), sClientName.end()).c_str()));
	}

	for (unsigned i = channelClientList.size(); i < 7; i++) //Empty the other lines
	{
		LogiLcdColorSetText(i + 1, _wcsdup(L""));
	}

	//Refresh Screen
	LogiLcdUpdate();
}

void ChangePosition(int changeValue)
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

/************************** TeamSpeak callbacks ***************************/

void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {
	switch (type) {
	case PLUGIN_MENU_TYPE_GLOBAL:
		/* Channel contextmenu item was triggered. selectedItemID is the channelID of the selected channel */
		switch (menuItemID) {
		case MENU_ID_GLOBAL_1:
			ts3Functions.printMessageToCurrentTab(std::to_string(config->testBool).c_str());
			break;
		case MENU_ID_GLOBAL_2:
			Update();
			break;
		case MENU_ID_GLOBAL_3:
			ChangePosition(-1);
			break;
		case MENU_ID_GLOBAL_4:
			ChangePosition(1);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber)
{
	Update();
}

void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char * moveMessage)
{
	Update();
}

const char* ts3plugin_keyDeviceName(const char* keyIdentifier) {
	return NULL;
}

const char* ts3plugin_displayKeyText(const char* keyIdentifier) {
	return NULL;
}

const char* ts3plugin_keyPrefix() {
	return NULL;
}