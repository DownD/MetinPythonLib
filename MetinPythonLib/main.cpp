#pragma once
#include <Windows.h>
#include <stdio.h>
#include "App.h"

HANDLE threadID;

//std::map<DWORD,DWORD> instances;


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
#endif
	init();
	return true;
}

BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{


	//  Perform global initialization.

	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:

		hDll = (HMODULE)hDllHandle;
		threadID = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
		break;

	case DLL_PROCESS_DETACH:
		exit();
		break;
	}

	return true;
}

