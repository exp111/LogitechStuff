#include "hooks.h"

#include <LogitechGArxControlLib.h>
#pragma comment(lib, "LogitechGArxControlLib.lib")

#define NOMINMAX
#include <Windows.h>
#include <string>
#include <iostream>

//cs related
#include "sdk/sdk.h"
#include "helpers\findpattern.h"
#include "helpers\VMTHooks.h"
#include "sdk\CNetVars.h"
#include "hooks.h"

//Json
#include "include/json.hpp"
using json = nlohmann::json;

void __stdcall Init()
{
	if (!LogiArxInit(_wcsdup(L"exp.csgo.radar"), _wcsdup(L"CSGO ARX Radar"), NULL))
	{
		printf("Failed Init.");
		return;
	}

	const wchar_t* index = L"<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1, target-densityDpi=device-dpi, user-scalable=no' /><link rel='stylesheet' type='text/css' href='style.css'></head><body><canvas id='myCanvas' width=500 height=500></canvas><div id='json'>{}</div><script src='script.js'></script></body></html>";
	const wchar_t* style = L"* { -webkit-touch-callout: none; -webkit-user-select: none;}canvas { border: 1px solid #d3d3d3; position: absolute; top: 25%; left: 0%; height: 50%; width: 95%; margin: 0 0 0 2.5%;}#json { visibility: hidden; color: white; }";
	const wchar_t* script = L"function check() { var c = document.getElementById('myCanvas'); var ctx = c.getContext('2d'); ctx.clearRect(0, 0, 500, 500); var json = document.getElementById('json').innerHTML; var jsonParsed = JSON.parse(json); if (jsonParsed.hasOwnProperty('data')) { for (i = 0; i < jsonParsed.data.length; i++) { ctx.fillStyle = jsonParsed.data[i].enemy ? 'red' : 'green'; ctx.font = '20px Comic Sans MS'; ctx.fillText(jsonParsed.data[i].name, 250 + jsonParsed.data[i].x, 250 + jsonParsed.data[i].y); ctx.fillRect(250 + jsonParsed.data[i].x, 250 + jsonParsed.data[i].y, 25, 25); } } ctx.moveTo(250, 0); ctx.lineTo(250, 500); ctx.stroke(); ctx.moveTo(0, 250); ctx.lineTo(500, 250); ctx.stroke();}setInterval(function () { check(); }, 10);";

	if (!LogiArxAddUTF8StringAs(_wcsdup(index), _wcsdup(L"index.html"), _wcsdup(L"text/html")))
	{
		printf("Could not load index.html.");
		return;
	}
	if (!LogiArxAddUTF8StringAs(_wcsdup(style), _wcsdup(L"style.css"), _wcsdup(L"text/css")))
	{
		printf("Could not load style.css.");
		return;
	}
	if (!LogiArxAddUTF8StringAs(_wcsdup(script), _wcsdup(L"script.js"), _wcsdup(L"text/javascript")))
	{
		printf("Could not load script.js.");
		return;
	}

	/*if (!LogiArxAddFileAs(_wcsdup(L"index.html"), _wcsdup(L"index.html"), _wcsdup(L"")))
	{
		printf("Did not found index.html. Make sure you have it in the same directory.");
		return;
	}*/
	if (!LogiArxSetIndex(_wcsdup(L"index.html")))
	{
		printf("Failed setting index.");
		system("PAUSE");
		return;
	}

	//Init Game Stuff

	toolkit::VMTBaseManager* clientModeHook = new toolkit::VMTBaseManager();

	CreateInterface_t ClientFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleW(L"client.dll"), "CreateInterface");
	CreateInterface_t EngineFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleW(L"engine.dll"), "CreateInterface");

	Client = (ICHLClient*)ClientFactory("VClient018", NULL);
	EntityList = (IEntityList*)ClientFactory("VClientEntityList003", NULL);
	EngineClient = (IEngineClient*)EngineFactory("VEngineClient014", NULL);
	ClientMode = *(IClientModeShared**)(FindPattern("client.dll", "A1 ? ? ? ? 8B 80 ? ? ? ? 5D") + 1);

	NetVars.Initialize();

	clientModeHook->Init(ClientMode);
	clientModeHook->HookMethod(&hkCreateMove, 24);
	clientModeHook->Rehook();
}

bool __fastcall hkCreateMove(void * ClientMode, int edx, float input_sample_frametime, void * cmd)
{
	toolkit::VMTManager& hook = toolkit::VMTManager::GetHook(ClientMode); //Get a pointer to the instance of your VMTManager with the function GetHook.
	bool oCreateMove = hook.GetMethod<bool(__thiscall*)(void*, float, void*)>(24)(ClientMode, input_sample_frametime, cmd); //Call the original.

	if (!cmd)
		return false;

	auto g_LocalPlayer = EntityList->GetClientEntity(EngineClient->GetLocalPlayer());

	if (!g_LocalPlayer || !EngineClient->IsInGame())
		return oCreateMove;

	
	//do stuff
	json config;
	config["data"] = {};

	for (int i = 1; i < EntityList->GetHighestEntityIndex() && i <= 64; i++)
	{
		CBaseEntity* entity = EntityList->GetClientEntity(i);

		if (!entity)
			continue;

		Vector pos = entity->GetOrigin() - g_LocalPlayer->GetOrigin(); //TODO

		if (entity == g_LocalPlayer)
			continue;

		if (entity->IsDormant())
			continue;

		if (!entity->IsAlive())
			continue;

		//player_info_t info = entity->GetPlayerInfo();

		//json shit
		json player;
		//player["name"] = info.szName;
		player["name"] = "a"; //TODO
		player["x"] = (int)pos.x;
		player["y"] = (int)pos.y;
		player["enemy"] = entity->GetTeam() != g_LocalPlayer->GetTeam();
		config["data"].push_back(player);
	}
	std::string oText = config.dump();
	std::string text = "";
	for (unsigned i = 0; i < oText.size(); i++)
	{
		if (oText[i] == '"')
			text += "\\\"";
		else
			text += oText[i];
	}
	LogiArxSetTagContentById(_wcsdup(L"json"), std::wstring(text.begin(), text.end()).c_str());

	return false;
}
