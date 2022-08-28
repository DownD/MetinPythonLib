#include "stdafx.h"
#include "JMPMidFuncHook.h"



JMPMidFuncHook::JMPMidFuncHook(void* addressToHook, void* redirection, BYTE sizeHook) : hookLocation(addressToHook), redirection(redirection), hookSize(sizeHook)
{
	isHooked = false;
	if (sizeHook < 5)
		throw std::exception("JMPHook needs 5 or more bytes");
	
	oldCode = malloc(sizeHook);
}


JMPMidFuncHook::~JMPMidFuncHook()
{
	free(oldCode);
	free(trampoline);
}

bool JMPMidFuncHook::HookFunction()
{
	if (isHooked)
		return false;
	trampoline = generateStandartTrampoline(hookLocation, hookSize, redirection, &ctx);
	
	//old code copy
	memcpy(oldCode, hookLocation, hookSize);

	BYTE arr[5];
	arr[0] = 0xE9;
	DWORD* jmp = (DWORD*)((DWORD)arr + 1);
	*jmp = getJMPOffset(hookLocation, trampoline) - (hookSize - 5);

	patchMemory(hookLocation, arr, 5, hookSize);

	isHooked = true;
	return true;
}

bool JMPMidFuncHook::UnHookFunction()
{
	if (!isHooked)
		return false;

	patchMemory(hookLocation, oldCode, hookSize, hookSize);

	isHooked = false;
	return true;
}
