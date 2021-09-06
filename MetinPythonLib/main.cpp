#pragma once
#include "stdafx.h"
#include "App.h"
#include "../common/utils.h"
#include <io.h>

HANDLE threadID;

//std::map<DWORD,DWORD> instances;

//Supposed to be a std::vector
struct DLLArgs {
	int size;
	int reserved;
	char path[256];
};

/*
void SetupDebugFile()
{
	std::string log_path(getDllPath());
	log_path += "ex_log.txt";

	HANDLE new_stdout = CreateFileA(log_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetStdHandle(STD_OUTPUT_HANDLE, new_stdout);
	int fd = _open_osfhandle((intptr_t)new_stdout, O_WRONLY | O_TEXT);
	_dup2(fd, 1);
}
void SetupConsole()
{
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
	freopen("CONOUT$", "wb", stderr);
	freopen("CONIN$", "rb", stdin);
	SetConsoleTitle("Debug Console");

}*/


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	static CApp app = CApp();
	app.init();
	//MessageBox(NULL, "Success Loading", "SUCCESS", MB_OK);
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
			setDllPath(dl->path);
		}
		else {
			GetModuleFileNameA(hDllHandle, test, 256);
			setDllPath(test);
		}
		hDll = (HMODULE)hDllHandle;
		threadID = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
		break;

	case DLL_PROCESS_DETACH:
		CApp & app = CApp::Instance();
		DEBUG_INFO_LEVEL_1("DETACHED CALLED");
		//app.exit();
		break;
	}

	return true;
}

