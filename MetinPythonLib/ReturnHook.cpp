#include "stdafx.h"
#include "ReturnHook.h"


bool ReturnHook::HookFunction()
{
	if (isHooked)
		return false;

	BYTE arr[5];
	arr[0] = 0xE9;
	DWORD* jmp = (DWORD*)((DWORD)arr + 1);
	*jmp = getJMPOffset(addressToHook, stackReplaceFunction);// -(sizeHook - 5);

	patchMemory(addressToHook, arr, 5, sizeHook);

	isHooked = true;
	return true;
}

bool ReturnHook::UnHookFunction()
{
	if (!isHooked)
		return false;

	patchMemory(addressToHook, oldCode, sizeHook, sizeHook);

	isHooked = false;
	return true;
}

ReturnHook::ReturnHook(void * addressToHook, void * redirection, BYTE size, int numArguments) : numArguments(numArguments), addressToHook(addressToHook), redirection(redirection), sizeHook(size)
{
	stackReplaceFunction = generateStackReplaceFunction();
	oldCode = malloc(size);
	if (!oldCode) {
		MessageBox(NULL, "Critical error alocating memory for Return Hook", "CRITICAL ERROR", MB_OK);
		throw new std::exception("Error");
	}
	oldCode = memcpy(oldCode, addressToHook, sizeHook);
}

ReturnHook::~ReturnHook()
{
	VirtualFree(stackReplaceFunction, 0, MEM_RELEASE);
}

void * ReturnHook::generateStackReplaceFunction()
{
	AssemblerX86 assembler;

	// PUSH arguments
	for (int i = 0; i < numArguments; i++) {
		assembler.pushRelativeToESP(numArguments * 4);
	}

	// PUSH return
	assembler.pushAddress((DWORD*)&redirection);

	//Replaced Code
	assembler.appendInstructions((BYTE*)addressToHook, sizeHook);

	//Jump
	int jmpIndex = assembler.getNextInstructionIndex();
	assembler.jmpNearRelative(0);

	//Alloc
	int size = assembler.getCurrentInstructions()->getSize();
	BYTE* trampolineFunction = (BYTE*)VirtualAlloc(NULL, assembler.getCurrentInstructions()->getSize(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	//Patch jumps
	assembler.patchRelativeInstruction(jmpIndex, (void*)((DWORD)addressToHook + sizeHook), trampolineFunction);

	bool val = assembler.getCurrentInstructions()->copy(trampolineFunction, size);
	return trampolineFunction;
}
