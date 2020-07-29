#include "App.h"
#include "JMPStartFuncHook.h"
#include <stdio.h>
#include <intrin.h>
#include "ReturnHook.h"
#include "Patterns.h"
#include "PythonModule.h"
#include "Network.h"
#include "MapCollision.h"


Hook* sendHook = 0;
Hook* recvHook = 0;
Hook* getEtherPacketHook = 0;
HMODULE hDll = 0;


namespace memory_patterns {

	//wrapper
	//https://gyazo.com/0b5f3bc5a90938b7574db9b9a7020d50
	//Pattern sendFunction = Pattern(1, "\xe8\x00\x00\x00\x00\x84\xc0\x75\x00\x68\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x83\xc4\x00\x32\xc0\x5e\x8b\xe5\x5d\xc2\x00\x00\xcc", "x????xxx?x????x????xx?xxxxxxx??x");
	Pattern sendFunction = Pattern(0, "\x55\x8b\xec\x56\x8b\xf1\x57\x8b\x7d\x00\x8b\x56", "xxxxxxxxx?xx");
	Pattern recvFunction = Pattern(0, "\x55\x8b\xec\x56\x57\xff\x75\x00\x8b\x7d", "xxxxxxx?xx");

	//https://gyazo.com/509c7cc703d48fb8e31bea3150687c07
	Pattern getEtherPackFunction = Pattern(0, "\x55\x8b\xec\x56\x8b\x75\x00\x57\xff\x75\x00\x8b\xf9\x56", "xxxxxx?xxx?xxx");

	//Pattern from Send On_Click Packet caller
	Pattern NetworkClassPointer = Pattern(2, "\x8b\x35\x00\x00\x00\x00\xa3", "xx????x");

	Pattern sendAttackPacket = Pattern(0, "\x55\x8b\xec\x83\xec\x00\x53\x56\x57\x89\x4d\x00\xeb", "xxxxx?xxxxx?x");


	//https://gyazo.com/2b8f2dcde97423b42470b343649ae0f6
	Pattern sendCharacterStatePacket = Pattern(0, "\x55\x8b\xec\x83\xec\x00\xa1\x00\x00\x00\x00\x33\xc5\x89\x45\x00\x53\x56\x57\x89\x4d", "xxxxx?x????xxxx?xxxxx");
}

//THREADING MITGH BE A PROBLEM
void __declspec(naked) GetEter(CMappedFile& file, const char* fileName, void* buffer, const char* uknown, bool uknown_2) {
	//pushes the return address
	__asm {
		POP EDX
		PUSH eax
		PUSH edx
		JMP _GetEter
	}
}

void __declspec(naked) __RecvPacketJMP(int size, void* buffer) {
	//pushes the return address
	__asm {
		POP edx
		PUSH eax
		PUSH edx
		JMP __RecvPacket
	}
}


void init() {
	auto send = memory_patterns::sendFunction;
	auto recv = memory_patterns::recvFunction;
	auto getEther = memory_patterns::getEtherPackFunction;
	Patterns patternFinder(hDll);

	void* recvAddr = patternFinder.GetPatternAddress(&recv);
	void* sendAddr = patternFinder.GetPatternAddress(&send);
	void* getEtherPackAddr = patternFinder.GetPatternAddress(&getEther);
	void* attackPacketAddr = patternFinder.GetPatternAddress(&memory_patterns::sendAttackPacket);
	void* statePacketAddr = patternFinder.GetPatternAddress(&memory_patterns::sendCharacterStatePacket);
	void** netClassPointer = (void**)patternFinder.GetPatternAddress(&memory_patterns::NetworkClassPointer);

	if (!netClassPointer) {
		MessageBox(NULL, "Network Class Pointer not found", "Critical Error", MB_OK);
		exit();
	}

	if (!sendAddr) {
		MessageBox(NULL, "Network Send Packet Function not found", "Critical Error", MB_OK);
		exit();
	}

	if (!attackPacketAddr) {
		MessageBox(NULL, "Network Send Attack Packet Function not found", "Critical Error", MB_OK);
		exit();
	}
	if (!statePacketAddr) {
		MessageBox(NULL, "Network Send State Packet Function not found", "Critical Error", MB_OK);
		exit();
	}

#ifdef _DEBUG
	printf("Send Addr: %#x\n", sendAddr);
	printf("Recv Addr: %#x\n", recvAddr);
	printf("GetEterFunction Addr: %#x\n", getEtherPackAddr);
	system("pause");
#endif

	SetNetClassPointer(*netClassPointer);
	SetSendFunctionPointer(sendAddr);
	SetSendBattlePacket(attackPacketAddr);
	SetSendStatePacket(statePacketAddr);


	//Hooks
	getEtherPacketHook = new ReturnHook(getEtherPackAddr, GetEter, 7, 5);
	recvHook = new ReturnHook(recvAddr, __RecvPacketJMP, 5, 2);
	//sendHook = new JMPStartFuncHook(sendAddr, __SendPacket, 6, THIS_CALL);

	recvHook->HookFunction();
	getEtherPacketHook->HookFunction();
	//sendHook->HookFunction();

	initModule();
}

void exit() {
	if (getEtherPacketHook)
		getEtherPacketHook->UnHookFunction();
	if (sendHook)
		sendHook->UnHookFunction();
	if (recvHook)
		recvHook->UnHookFunction();

	delete sendHook;
	delete recvHook;
	delete getEtherPacketHook;

	freeCurrentMap();

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	FreeConsole();
}