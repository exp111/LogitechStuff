#include <LogitechGArxControlLib.h>
#pragma comment(lib, "LogitechGArxControlLib.lib")

#include <Windows.h>
#include <string>
#include <iostream>

int main()
{
	if (!LogiArxInit(_wcsdup(L"exp.test.radar"), _wcsdup(L"Simple ARX Radar Test"), NULL))
	{
		std::cout << "Failed Init." << std::endl;
		system("PAUSE");
		return 0;
	}
	
	if (!LogiArxAddFileAs(_wcsdup(L"index.html"), _wcsdup(L"index.html"), _wcsdup(L"")))
	{
		std::cout << "Did not found index.html. Make sure you have it in the same directory." << std::endl;
		system("PAUSE");
		return 0;
	}
	if (!LogiArxSetIndex(_wcsdup(L"index.html")))
	{
		std::cout << "Failed setting index." << std::endl;
		system("PAUSE");
		return 0;
	}

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