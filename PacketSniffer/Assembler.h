#pragma once
#include "Windows.h"
#include <vector>

struct Context {
public:
	DWORD ECX, EAX, EDI, ESI, EBP, EDX, EBX;
};

class Stack {
public:
	Stack(int Capacity = 0);
	~Stack();
	int getSize();

	void pushBYTE(BYTE value);
	void pushDWORD(DWORD value);
#ifdef _M_IX86
	void pushPOINTER(void* value);
#endif
	void pushWORD(WORD value);
	void pushARRAY(BYTE* arr, int size);

	bool copy(BYTE* buffer, int copySize);

	inline BYTE* getData() { return arr.data(); }

	void printDebug();

private:
	std::vector<BYTE> arr;
};



class AssemblerX86 {
public:

	AssemblerX86(Stack* stack = nullptr);
	~AssemblerX86();

	void saveECX(DWORD* addr);
	void saveEAX(DWORD* addr);
	void saveEDX(DWORD* addr);
	void saveEDI(DWORD* addr);
	void saveESI(DWORD* addr);
	void saveEBP(DWORD* addr);
	void saveEBX(DWORD* addr);

	void loadECX(DWORD* addr);
	void loadEAX(DWORD* addr);
	void loadEDX(DWORD* addr);
	void loadEDI(DWORD* addr);
	void loadESI(DWORD* addr);
	void loadEBP(DWORD* addr);
	void loadEBX(DWORD* addr);

	void saveAllRegister(Context* loc);
	void loadAllRegister(Context* loc);

	void pushAllRegister();
	void popAllRegister();

	//use positive offset to reference the stack
	void pushRelativeToESP(BYTE offset);

	//Push value pointed by the address
	void pushAddress(DWORD* valueAddr);
	void popToAddress(DWORD* addr);

	void callNearRelative(DWORD offset);
	void jmpNearRelative(DWORD offset);

	void appendInstructions(BYTE* bytes, int size);

	//TO BE TESTED
	//if index out of bounds, a crash will ocurr
	//target is the absolute address to be jumped
	//baseAlloc is the start address where the current code will be allocated
	//index, is the index of the instruction (use getNextInstructionIndex, before doing a jmp or a call to know the index
	bool patchRelativeInstruction(int index, void* target, void* baseAlloc);
	inline int getNextInstructionIndex() { return stack->getSize(); };

	inline Stack* getCurrentInstructions() { return stack; };

private:
	Stack* stack;
	bool freeOnDestruction;

};


