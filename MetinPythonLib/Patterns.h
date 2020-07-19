#pragma once
#include <Windows.h>
#include "utils.h"
#include <Psapi.h>
#include <iostream>
#include <Shlwapi.h>
#include <tlhelp32.h>


class Patterns {
public:
	Patterns(HMODULE hMod);
	~Patterns();

	DWORD* GetPatternAddress(const char *pattern, const char *mask, int offset);



private:
	bool Init();
	bool setModuleInfo();

	int getModuleSize(void* baseAddress);
	void printModules();

	//Finds Pattern in memory
	DWORD FindPattern(const char *pattern, const char *mask);

private:
	HMODULE hMod;
	MODULEINFO mInfo;

};

