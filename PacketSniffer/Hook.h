#pragma once
#include <Windows.h>
#include <unordered_map>
#include "Assembler.h"


typedef void(*tFunction)(void);

class Hook
{
public:
	virtual bool HookFunction() = 0;
	virtual bool UnHookFunction() = 0;

	virtual ~Hook() {};


protected:

	//copy src to target, and changes rpotection if needed
	bool changeProtectedMemory(void* target, void* src, int size);
	DWORD getJMPOffset(void * jmpInstruction, void * function);

	//Patch memory and override with nops
	bool patchMemory(void* target, void* src, int src_size, int patchSize);

	//Generates a trampoline function that saves every register
	//The function needs to be freed using VirtualFree
	//ATTENTION, the bytes overriten cannot be relative offsets (Example: JMP instruciton)
	void* generateStandartTrampoline(void* hookStart, int hookSize, void* redirection, Context * ctx,AssemblerX86* buffer = nullptr);
};

