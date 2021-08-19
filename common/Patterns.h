#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include <Shlwapi.h>
#include <tlhelp32.h>


struct Pattern {
	Pattern(const char* name,int ofset, const char* pat, const char* masc) { this->name = name; offset = ofset; pattern = pat; mask = masc;}
	int offset;
	const char* pattern;
	const char* mask;
	const char* name;
};


//If module pattern is provided, it will scan that pattern to find the memory location to search for next patterns
class Patterns {
public:
	Patterns(HMODULE hMod,Pattern* modulePattern = 0);
	~Patterns();

	DWORD* GetPatternAddress(Pattern* pat);
	void* GetStartModuleAddress();



private:
	bool Init(Pattern* modulePattern = 0);
	bool setModuleInfo();

	int getModuleSize(void* baseAddress);
	void printModules();

	//Finds Pattern in memory
	DWORD FindPattern(const char *pattern, const char *mask);

private:
	HMODULE hMod;
	MODULEINFO mInfo;

};

