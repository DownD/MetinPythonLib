#pragma once
#include "../common/Patterns.h"
#include <map>
class CAddressLoader
{
public:
	CAddressLoader();
	~CAddressLoader();

	bool setAddress(HMODULE hDll);
	void* GetAddress(int id);

private:
	bool setAddressByPatterns(Patterns* p);
	bool setAddressByFile(const char* path, void* baseDllAddress);
	bool setAddressByServer( void* baseDllAddress);

	void parseFileBuffer(const char* buffer, int size, void* baseDllAddress);

private:
	
	std::map<int, DWORD> memoryAddress;
};

