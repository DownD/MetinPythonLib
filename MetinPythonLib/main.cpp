#pragma once
#include <Windows.h>
#include <stdio.h>
#include "App.h"
#include "utils.h"

HANDLE threadID;

//std::map<DWORD,DWORD> instances;

//Supposed to be a std::vector
struct DLLArgs {
	int size;
	int reserved;
	wchar_t path[256];
};

void SetupConsole()
{
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
	freopen("CONOUT$", "wb", stderr);
	freopen("CONIN$", "rb", stdin);
	SetConsoleTitle("Debug Console");
}


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
#ifdef _DEBUG
	SetupConsole();
	DEBUG_INFO("Dll Loaded From %s", getDllPath());
#endif
	init();
	return true;
}


BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{


	//  Perform global initialization.
	char test[256] = { 0 };

	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:
		if (Reserved) {
			DLLArgs* dl = (DLLArgs*)Reserved;
			sprintf(test, "%ws", dl->path);
			setDllPath(test);
		}
		else {
			GetModuleFileNameA(hDllHandle, test, 256);
			setDllPath(test);
		}
		hDll = (HMODULE)hDllHandle;
		threadID = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
		break;

	case DLL_PROCESS_DETACH:
		exit();
		break;
	}

	return true;
}

