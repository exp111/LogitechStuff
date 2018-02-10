#include <LogitechGArxControlLib.h>
#pragma comment(lib, "LogitechGArxControlLib.lib")

#include <Windows.h>
#include <string>

typedef struct arxAppCallbackMessage
{
	unsigned __int32 eventType;
	unsigned __int32 eventValue;
	wchar_t eventArg[128]; //Max length for this callback param (see documentation for more details)
}arxAppCallbackMessage;

static void __cdecl onCallback(int eventType, int eventValue, wchar_t * eventArg, void *context) {
	switch (eventType)
	{
	case LOGI_ARX_EVENT_FOCUS_ACTIVE:
		printf("got focus \n");
		break;
	case LOGI_ARX_EVENT_FOCUS_INACTIVE:
		printf("lost focus \n");
		break;
	case  LOGI_ARX_EVENT_TAP_ON_TAG:
	{
		printf("click on tag with id : ");
		if (eventArg)
		{
			printf("User Tapped on tag with id :%ls", eventArg);
			printf("%ls", eventArg);
			printf("\n");

		}
		else
		{
			OutputDebugStringW(L"NULL\n");
		}
		break;
	}
	case LOGI_ARX_EVENT_MOBILEDEVICE_ARRIVAL:
		printf("device arrived \n");
		break;
	case LOGI_ARX_EVENT_MOBILEDEVICE_REMOVAL:
		printf("device removed \n");
		break;
	default:
		printf("unknown message %d:%d \n", eventType, eventValue);
		break;
	}
}

int main()
{
	logiArxCbContext callbackStruct;
	callbackStruct.arxContext = GetActiveWindow();
	callbackStruct.arxCallBack = (logiArxCb)onCallback;

	if (!LogiArxInit(_wcsdup(L"exp.test.callback"), _wcsdup(L"Callback Test"), &callbackStruct))
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

	return 0;
}