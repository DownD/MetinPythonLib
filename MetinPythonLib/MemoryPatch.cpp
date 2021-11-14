#include "stdafx.h"
#include "MemoryPatch.h"
#include <exception>

CMemoryPatch::CMemoryPatch(BYTE* bufferToPatch, int sizeBuffer, void* location, int sizeLocation)
{
	if (sizeBuffer > sizeLocation) {
		throw std::exception("Error on CMemoryPatch trying to write buffer bigger then excpected");
	}
	
	this->location = location;
	patchInstructions = (BYTE*)malloc(sizeLocation);
	originalInstructions = (BYTE*)malloc(sizeLocation);
	memcpy(patchInstructions, bufferToPatch, sizeBuffer);
	memcpy(originalInstructions, location, sizeLocation);
	for (int i = sizeBuffer; i < sizeLocation; i++) {
		patchInstructions[i] = 0x90; //NOP
	}
	sizePatch = sizeLocation;
	m_isPatched = false;
}

CMemoryPatch::CMemoryPatch(BYTE* bufferToPatch,const char* mask, int sizeBuffer, void* location)
{

	this->location = location;
	patchInstructions = (BYTE*)malloc(sizeBuffer);
	originalInstructions = (BYTE*)malloc(sizeBuffer);
	memcpy(patchInstructions, bufferToPatch, sizeBuffer);
	memcpy(originalInstructions, location, sizeBuffer);
	for (int i = 0; i < sizeBuffer; i++) {
		if (mask[i] == '?') {
			patchInstructions[i] = originalInstructions[i];
		}
	}
	sizePatch = sizeBuffer;
	m_isPatched = false;
}

CMemoryPatch::~CMemoryPatch()
{
	unPatchMemory();
	free(patchInstructions);
	free(originalInstructions);
}


bool CMemoryPatch::patchMemory()
{
	if (m_isPatched)
		return true;

	m_isPatched = true;
	return changeProtectedMemory(location, patchInstructions, sizePatch);
}

bool CMemoryPatch::unPatchMemory()
{
	if (!m_isPatched)
		return true;
	m_isPatched = false;
	return changeProtectedMemory(location, originalInstructions, sizePatch);
}

bool CMemoryPatch::changeProtectedMemory(void* target, void* src, int size)
{
	DWORD oldProtection = 0;
	if (!VirtualProtect(target, size, PAGE_READWRITE, &oldProtection)) {
#ifdef _DEBUG
		printf("Fail Changing protection of Hooked Function - Error %d\n!", GetLastError());
#endif
		return 0;
	}

	memcpy(target, src, size);

	if (!VirtualProtect(target, size, oldProtection, &oldProtection)) {
#ifdef _DEBUG
		printf("Fail Changing to old protection of Hooked Function %d\n!", GetLastError());
#endif
		return 0;
	}
}