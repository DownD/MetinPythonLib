#include "stdafx.h"
#include "Hook.h"

bool Hook::changeProtectedMemory(void * target, void * src, int size)
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
		UnHookFunction();
		return 0;
	}
}

DWORD Hook::getJMPOffset(void * jmpInstruction, void * function)
{
	return (DWORD)function - (DWORD)jmpInstruction - 5;
}

bool Hook::patchMemory(void * target, void * src, int src_size, int patchSize)
{
	BYTE* buf = (BYTE*)malloc(patchSize);
	if (!buf) {
		MessageBox(NULL, "Critical error alocating memory for Return Hook", "CRITICAL ERROR", MB_OK);
		return 0;
	}
	memcpy(buf, src, patchSize);
	for (int i = src_size; i < patchSize; i++) {
		buf[i] = 0xCC;
	}
	bool result = changeProtectedMemory(target, buf, patchSize);
	free(buf);

	return result;
}

void* Hook::generateStandartTrampoline(void * hookStart, int hookSize, void * redirection, Context * ctx, AssemblerX86 * buffer)
{
	bool del = false;
	if (buffer == nullptr) {
		del = true;
		buffer = new AssemblerX86();
	}
	buffer->saveAllRegister(ctx);
	//CALL redirection
	int redirIndex = buffer->getNextInstructionIndex();
	buffer->callNearRelative(0);

	buffer->loadAllRegister(ctx);

	//Replaced Code
	buffer->appendInstructions((BYTE*)hookStart, hookSize);

	//JMP
	int jmpIndex = buffer->getNextInstructionIndex();
	buffer->jmpNearRelative(0);

	//Alloc
	int size = buffer->getCurrentInstructions()->getSize();
	BYTE* trampolineFunction = (BYTE*)VirtualAlloc(NULL, buffer->getCurrentInstructions()->getSize(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	//patch jumps
	buffer->patchRelativeInstruction(redirIndex, redirection, trampolineFunction);
	buffer->patchRelativeInstruction(jmpIndex, (void*)((DWORD)hookStart + hookSize), trampolineFunction);

	buffer->getCurrentInstructions()->copy(trampolineFunction, size);

	if (del)
		delete buffer;

	return trampolineFunction;
}
