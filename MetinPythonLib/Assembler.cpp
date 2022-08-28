#include "stdafx.h"
#include "Assembler.h"

Stack::Stack(int Capacity)
{
	arr.reserve(Capacity);
}


Stack::~Stack()
{
}

void Stack::printDebug()
{
	for (BYTE b : arr) {
		printf("%#02x ", b);
	}
}

int Stack::getSize()
{
	return arr.size();
}

void Stack::pushBYTE(BYTE value)
{
	arr.push_back(value);
}

void Stack::pushDWORD(DWORD value)
{
	BYTE* bytes = (BYTE*)&value;
	arr.push_back(bytes[0]);
	arr.push_back(bytes[1]);
	arr.push_back(bytes[2]);
	arr.push_back(bytes[3]);
}

void Stack::pushPOINTER(void* value)
{
	pushDWORD((DWORD)value);
}

void Stack::pushWORD(WORD value)
{
	BYTE* bytes = (BYTE*)value;
	arr.push_back(bytes[0]);
	arr.push_back(bytes[1]);
}

void Stack::pushARRAY(BYTE* arr, int size)
{
	for (int i = 0; i < size; i++) {
		this->arr.push_back(arr[i]);
	}
}

bool Stack::copy(BYTE* buffer, int copySize)
{
	if (copySize >= arr.size()) {
		memcpy(buffer, arr.data(), arr.size());
		return true;
	}

	return false;
}


AssemblerX86::AssemblerX86(Stack* stack)
{
	if (stack == nullptr) {
		freeOnDestruction = true;
		this->stack = new Stack();
	}
	else {
		this->stack = stack;
		freeOnDestruction = false;
	}
}

AssemblerX86::~AssemblerX86()
{
	if (freeOnDestruction)
		delete stack;
}


void AssemblerX86::saveECX(DWORD* addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x0D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveEAX(DWORD* addr)
{
	stack->pushBYTE(0xA3);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveEDX(DWORD* addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x15);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveEDI(DWORD* addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x3D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveESI(DWORD* addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x35);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveEBP(DWORD* addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x2D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveEBX(DWORD* addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x1D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadECX(DWORD* addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x0D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadEAX(DWORD* addr)
{
	stack->pushBYTE(0xA1);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadEDX(DWORD* addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x15);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadEDI(DWORD* addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x3D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadESI(DWORD* addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x35);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadEBP(DWORD* addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x2D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadEBX(DWORD* addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x1D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveAllRegister(Context* loc)
{
	saveECX(&(loc->ECX));
	saveEAX(&(loc->EAX));
	saveEDX(&(loc->EDX));
	saveEDI(&(loc->EDI));
	saveESI(&(loc->ESI));
	saveEBP(&(loc->EBP));
	saveEBX(&(loc->EBX));
}

void AssemblerX86::loadAllRegister(Context* loc)
{
	loadECX(&(loc->ECX));
	loadEAX(&(loc->EAX));
	loadEDX(&(loc->EDX));
	loadEDI(&(loc->EDI));
	loadESI(&(loc->ESI));
	loadEBP(&(loc->EBP));
	loadEBX(&(loc->EBX));
}

void AssemblerX86::pushAllRegister()
{
	stack->pushBYTE(0x60);
}

void AssemblerX86::popAllRegister()
{
	stack->pushBYTE(0x61);
}

void AssemblerX86::pushRelativeToESP(BYTE offset)
{
	if (offset <= 0 || offset > 128) {
		throw "Error offset cannot be null or bigger then 128";
	}
	stack->pushBYTE(0xFF);
	stack->pushBYTE(0x74);
	stack->pushBYTE(0x24);
	stack->pushBYTE(offset);
}

void AssemblerX86::pushAddress(DWORD* valueAddr)
{
	stack->pushBYTE(0xFF);
	stack->pushBYTE(0x35);
	stack->pushPOINTER(valueAddr);
}

void AssemblerX86::popToAddress(DWORD* addr)
{
	stack->pushBYTE(0x8F);
	stack->pushBYTE(0x05);
	stack->pushPOINTER(addr);
}

void AssemblerX86::callNearRelative(DWORD offset)
{
	stack->pushBYTE(0xE8);
	stack->pushDWORD(offset);
}

void AssemblerX86::jmpNearRelative(DWORD offset)
{
	stack->pushBYTE(0xE9);
	stack->pushDWORD(offset);
}

void AssemblerX86::appendInstructions(BYTE* bytes, int size)
{
	stack->pushARRAY(bytes, size);
}

bool AssemblerX86::patchRelativeInstruction(int index, void* target, void* baseAlloc)
{
	BYTE* arr = stack->getData();
	BYTE instruction = arr[index];
	DWORD* offset = (DWORD*)(arr + index + 1);

	switch (instruction) {
	case 0xE9:
	case 0xE8: {
		*offset = (DWORD)target - ((DWORD)baseAlloc + index + 5);
		break;
	}
	default:
		return false;
	}
	return true;
}