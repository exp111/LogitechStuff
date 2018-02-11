#include <LogitechGArxControlLib.h>
#pragma comment(lib, "LogitechGArxControlLib.lib")

#define NOMINMAX
#include <Windows.h>
#include <string>
#include <iostream>

#include "include/json.hpp"
using json = nlohmann::json;

int main()
{
	if (!LogiArxInit(_wcsdup(L"exp.test.radar"), _wcsdup(L"Simple ARX Radar Test"), NULL))
	{
		std::cout << "Failed Init." << std::endl;
		system("PAUSE");
		return 0;
	}

	const wchar_t* index = L"<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1, target-densityDpi=device-dpi, user-scalable=no' /></head><style>* {-webkit-touch-callout: none;-webkit-user-select: none;}canvas {border: 1px solid #d3d3d3;position: absolute;top: 25%;left: 0%;height: 50%;width: 95%;margin: 0 0 0 2.5%;}#json {visibility: hidden;}</style><body><canvas id='myCanvas' width=500 height=500></canvas><div id='json'>{}</div><script>function check() {//get canvasvar c = document.getElementById('myCanvas');var ctx = c.getContext('2d');//clearctx.clearRect(0, 0, 500, 500);//Json parsevar json = document.getElementById('json').innerHTML;var jsonParsed = JSON.parse(json);if (jsonParsed.hasOwnProperty('data')) {//draw the playersfor (i = 0; i < jsonParsed.data.length; i++) {ctx.fillStyle = jsonParsed.data[i].enemy ? 'red' : 'green';ctx.font = '20px Comic Sans MS';ctx.fillText(jsonParsed.data[i].name, 250 + jsonParsed.data[i].x, 250 + jsonParsed.data[i].y);ctx.fillRect(250 + jsonParsed.data[i].x, 250 + jsonParsed.data[i].y, 25, 25);}}//Radar indicatorctx.moveTo(250, 0); //top midctx.lineTo(250, 500); //down midctx.stroke();ctx.moveTo(0, 250); //left midctx.lineTo(500, 250); //right midctx.stroke();}setInterval(function () { check(); }, 100);</script></body></html>";
	const wchar_t* style = L"* { -webkit-touch-callout: none; -webkit-user-select: none;}canvas { border: 1px solid #d3d3d3; position: absolute; top: 25%; left: 0%; height: 50%; width: 95%; margin: 0 0 0 2.5%;}#json { visibility: hidden;}";
	const wchar_t* script = L"function check() { //get canvas var c = document.getElementById('myCanvas'); var ctx = c.getContext('2d'); //clear ctx.clearRect(0, 0, 500, 500); //Json parse var json = document.getElementById('json').innerHTML; var jsonParsed = JSON.parse(json); if (jsonParsed.hasOwnProperty('data')) { //draw the players for (i = 0; i < jsonParsed.data.length; i++) { ctx.fillStyle = jsonParsed.data[i].enemy ? 'red' : 'green'; ctx.font = '20px Comic Sans MS'; ctx.fillText(jsonParsed.data[i].name, 250 + jsonParsed.data[i].x, 250 + jsonParsed.data[i].y); ctx.fillRect(250 + jsonParsed.data[i].x, 250 + jsonParsed.data[i].y, 25, 25); } } //Radar indicator ctx.moveTo(250, 0); //top mid ctx.lineTo(250, 500); //down mid ctx.stroke(); ctx.moveTo(0, 250); //left mid ctx.lineTo(500, 250); //right mid ctx.stroke();}setInterval(function () { check(); }, 100);";

	if (!LogiArxAddUTF8StringAs(_wcsdup(index), _wcsdup(L"index.html"), _wcsdup(L"text/html")))
	{
		std::cout << "Could not load index.html." << std::endl;
		system("PAUSE");
		return 0;
	}
	if (!LogiArxAddUTF8StringAs(_wcsdup(style), _wcsdup(L"style.css"), _wcsdup(L"text/css")))
	{
		std::cout << "Could not load style.css." << std::endl;
		system("PAUSE");
		return 0;
	}
	if (!LogiArxAddUTF8StringAs(_wcsdup(script), _wcsdup(L"script.js"), _wcsdup(L"text/javascript")))
	{
		std::cout << "Could not load script.js." << std::endl;
		system("PAUSE");
		return 0;
	}

	/*if (!LogiArxAddFileAs(_wcsdup(L"index.html"), _wcsdup(L"index.html"), _wcsdup(L"")))
	{
		std::cout << "Did not found index.html. Make sure you have it in the same directory." << std::endl;
		system("PAUSE");
		return 0;
	}*/
	if (!LogiArxSetIndex(_wcsdup(L"index.html")))
	{
		std::cout << "Failed setting index." << std::endl;
		system("PAUSE");
		return 0;
	}

	json config;
	config["data"] = {};

	json player;
	player["name"] = "hans";
	player["x"] = 20;
	player["y"] = -20;
	player["enemy"] = true;
	config["data"].push_back(player);

	std::cout << config.dump() << std::endl;
	system("PAUSE");

	while (!(GetKeyState(VK_ESCAPE) & 0x8000))
	{
		std::system("cls");
		std::cout << "Press Tab to enter JSON Data." << std::endl;
		if ((GetKeyState(VK_TAB) & 0x8000))
		{
			std::string text;
			std::cout << "Enter JSON Data (with escape chars): ";
			std::cin >> text;
			LogiArxSetTagContentById(_wcsdup(L"json"), std::wstring(text.begin(), text.end()).c_str());
		}
		Sleep(50);
	}

	LogiArxShutdown();
	return 0;
}