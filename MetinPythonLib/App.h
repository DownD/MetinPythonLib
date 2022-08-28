#pragma once
#include "stdafx.h"
#include "Singleton.h"
#include "defines.h"

#define METIN_GF


extern HMODULE hDll;



class CApp : public CSingleton<CApp>{
public:
	CApp();
	~CApp();

	void init();
	void exit();
	void setSkipRenderer();
	void unsetSkipRenderer();

	bool __AppProcess(ClassPointer p);

private:
	void initMainThread();
	void initPythonModules();
	void SetupDebugFile();
	void SetupConsole();

private:

	bool mainScriptExec;
	bool passed;
};
