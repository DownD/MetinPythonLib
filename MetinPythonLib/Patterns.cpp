#include "Patterns.h"

Patterns::Patterns(HMODULE hMod) : hMod(hMod){
	if(!Init())
		throw std::exception("Fail to Initialize Patterns Class");
}

Patterns::~Patterns(){
}
bool Patterns::Init() {

	if (!setModuleInfo())
		throw std::runtime_error("Error setting module");

	DEBUG_INFO("Module start address: %x\nModule Size:%x", mInfo.lpBaseOfDll, mInfo.SizeOfImage);


	return true;
}

DWORD Patterns::FindPattern(const char *pattern, const char *mask)
{
	/*int len = strlen(mask);
	printf("Pattern: ");
	for (int i = 0; i < len; i++) {
		printf("%#x ", (BYTE)pattern[i]);
	}
	printf("\n\n");*/



	DWORD patternLength = (DWORD)strlen(mask);
	MEMORY_BASIC_INFORMATION PermInfo;


		DWORD startModule = (DWORD)mInfo.lpBaseOfDll;
		DWORD endModule = startModule + (DWORD)mInfo.SizeOfImage;
		int pageEndAddr = startModule;

		for (DWORD indexAddr = startModule; indexAddr + patternLength <= endModule; indexAddr++)
		{
			if (pageEndAddr <= indexAddr)
			{
				if (VirtualQuery((LPCVOID*)(indexAddr), &PermInfo, sizeof(PermInfo)))
				{
					if (PermInfo.Protect != PAGE_NOACCESS) {
						indexAddr = (DWORD)PermInfo.BaseAddress;
						pageEndAddr = indexAddr + PermInfo.RegionSize;
					}
					else {
						pageEndAddr += PermInfo.RegionSize;
						indexAddr = pageEndAddr;
						continue;
					}
				}
				else {
					DEBUG_INFO("Error Querying memory at %#x", indexAddr);
					continue;
				}
			}

			if (indexAddr + patternLength > pageEndAddr) {
				indexAddr = pageEndAddr;
				continue;
			}
			bool found = true;
			for (DWORD j = 0; j < patternLength && found; j++)
			{
				//if we have a ? in our mask then we have true by default, 
				//or if the bytes match then we keep searching until finding it or not
				found &= mask[j] == '?' || pattern[j] == *(char*)(indexAddr + j);
			}

			//found = true, our entire pattern was found
			//return the memory addy so we can write to it
			if (found) {
				return indexAddr;
			}
		}
	return NULL;
}




DWORD* Patterns::GetPatternAddress(const char *pattern, const char *mask, int offset) {
	
	DWORD* addr = (DWORD*)FindPattern(pattern, mask);
	if (addr) { return (DWORD*)((int)addr + offset); }
	else {
		std::string msg("Pattern code: ");
		msg += mask;
		MessageBox(NULL, msg.c_str(), "ERROR FINDING PATTERN", MB_ICONWARNING | MB_YESNO);	
	}

	return addr;
}

bool Patterns::setModuleInfo()
{
	HANDLE psHandle = GetCurrentProcess();

	TCHAR buffer[MAX_PATH];
	int path_size = GetProcessImageFileName(psHandle, buffer, MAX_PATH);


	if (!path_size) {
		DEBUG_INFO("Fail to Get Module FileName");
		return false;
	}


	PathStripPath(buffer);
	HMODULE hModule = GetModuleHandle(buffer);
	if (hModule == 0)
		return false;

	printf("Scanning Module: %S\n", buffer);
	GetModuleInformation(psHandle, hModule, &mInfo, sizeof(MODULEINFO));
	mInfo.SizeOfImage = getModuleSize(mInfo.lpBaseOfDll);
	return true;
}

int Patterns::getModuleSize(void * baseAddress)
{
	PIMAGE_DOS_HEADER     pDosH = (PIMAGE_DOS_HEADER)baseAddress;
	if (pDosH->e_magic == (WORD)0x5A4D) {
		PIMAGE_NT_HEADERS     pNtH = (PIMAGE_NT_HEADERS)((int)baseAddress + pDosH->e_lfanew);
		if (pNtH->Signature == (DWORD)0x4550) {
			if (pNtH->OptionalHeader.Magic == (WORD)0x010B) {
				return pNtH->OptionalHeader.SizeOfImage;
				
			}
		}

	}
	return 0;
}

void Patterns::printModules()
{
	HANDLE psHandle = GetCurrentProcess();
	HMODULE modules[1000] = { 0 };
	DWORD numModules = 0;

	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
	if (Snapshot == INVALID_HANDLE_VALUE) {
		DEBUG_INFO("Error CreatingToolhelp32, code: %#x", GetLastError());
		return;
	}

	MODULEENTRY32 module;
	module.dwSize = sizeof(MODULEENTRY32);

	if (!Module32First(Snapshot, &module)) {
		DEBUG_INFO("Error on Module32First, code: %#x", GetLastError());
		return;
	}

	do
	{
		printf("\n\n     MODULE NAME:     %S", module.szModule);
		printf("\n     executable     = %S", module.szExePath);
		printf("\n     process ID     = 0x%08X", module.th32ProcessID);
		printf("\n     ref count (g)  =     0x%04X", module.GlblcntUsage);
		printf("\n     ref count (p)  =     0x%04X", module.ProccntUsage);
		printf("\n     base address   = 0x%08X", (DWORD)module.modBaseAddr);
		printf("\n     base size      = 0x%10X", module.modBaseSize);

	} while (Module32Next(Snapshot, &module));
}
