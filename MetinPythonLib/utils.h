#pragma once
#include <windows.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream> 
#include <chrono>
#include <ctime>
#include <math.h>

bool isDebugEnable();
void setDebugOn();
void setDebugOff();

#define SUBPATH_MAPS "Resources\\Maps\\"

#define DEBUG_INFO_LEVEL_1(...); {if(isDebugEnable()){printf(__VA_ARGS__); printf("\n");}}
#define DEBUG_INFO_LEVEL_2(...); {DEBUG_INFO_LEVEL_1(__VA_ARGS__); }
#define DEBUG_INFO_LEVEL_3(...); {DEBUG_INFO_LEVEL_1(__VA_ARGS__); }
#define DEBUG_INFO_LEVEL_4(...); {}


typedef void (__stdcall *tTimerFunction)();
typedef std::chrono::time_point<std::chrono::system_clock> tTimePoint;

bool getCurrentPathFromModule(HMODULE hMod, char* dllPath, int size);
void stripFileFromPath(char* dllPath, int size);
const char* getDllPath();
const char* getMapsPath();
void setDllPath(char* file);


//There are bugs here that migh crash the process
void setTimerFunction(tTimerFunction func,float sec);
void executeTimerFunctions();


inline void* getRelativeCallAddress(void* startCallAddr) {
	DWORD addr = (DWORD)startCallAddr;
	DWORD* offset = (DWORD*)(addr + 1);
	void* _final = (void*)(addr + *offset + 5);
	return  _final;
}


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

struct Context {
public:
	DWORD ECX, EAX, EDI, ESI, EBP, EDX, EBX;
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

inline float distance(float x1, float y1, float x2, float y2) {
	return sqrt((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
}

struct Point {
	Point(int x=0, int y=0) : x(x), y(y){}
	int x, y;
};

struct fPoint {
	fPoint(float x, float y) : x(x), y(y) {}
	float x, y;
};

struct fPoint3D {
	float x, y, z;
};

