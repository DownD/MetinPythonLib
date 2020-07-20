#pragma once
#include <Windows.h>
#include "utils.h"
#include <Psapi.h>
#include <iostream>
#include <Shlwapi.h>
#include <tlhelp32.h>


struct Pattern {
	Pattern(int ofset, const char* pat, const char*masc) { offset = ofset; pattern = pat; mask = masc; }
	int offset;
	const char* pattern;
	const char* mask;
};


class Patterns {
public:
	Patterns(HMODULE hMod);
	~Patterns();

	DWORD* GetPatternAddress(Pattern* pat);



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

