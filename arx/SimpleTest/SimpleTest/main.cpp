#include <LogitechGArxControlLib.h>
#pragma comment(lib, "LogitechGArxControlLib.lib")

#include <Windows.h>
#include <string>

int main()
{
	if (!LogiArxInit(_wcsdup(L"exp.test.sample"), _wcsdup(L"Simple ARX Test"), NULL))
	{ 
		system("PAUSE");
	}

	LogiArxAddFileAs(_wcsdup(L"index.html"), _wcsdup(L"index.html"), _wcsdup(L""));
	LogiArxSetIndex(_wcsdup(L"index.html"));

	while (!(GetKeyState(VK_ESCAPE) & 0x8000))
	{
		std::string text = std::to_string(rand());
		LogiArxSetTagContentById(_wcsdup(L"health"), std::wstring(text.begin(), text.end()).c_str());

		Sleep(1000);
	}

	LogiArxShutdown();
}