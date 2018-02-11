#include <Windows.h>
#include "hooks.h"

bool WINAPI DllMain(HINSTANCE dll_instance, DWORD call_reason, LPVOID reserved) {
	DisableThreadLibraryCalls(dll_instance);

	if (call_reason == DLL_PROCESS_ATTACH)
		CreateThread(0, 0, LPTHREAD_START_ROUTINE(Init), 0, 0, 0);

	return true;
}