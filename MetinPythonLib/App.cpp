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
	//https://gyazo.com/6696a0db5abb62a59203d7282bc4f90a //From RecvCharMovePacket
	//\xe8\x00\x00\x00\x00\x8d\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x4d x????xx?x????xxx

	Pattern globalToLocalPositionFunction = Pattern("GlobalToLocal Function", 0, "\xe8\x00\x00\x00\x00\x8d\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x4d", "x????xx?x????xxx");

	//SendExchangeStartPacket 
    Pattern globalPat = Pattern("Global Pattern",0,"\x55\x8b\xec\x83\xec\x00\x89\x4d\x00\xc6\x45\x00\x00\x8d\x45\x00\x50\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xc9\x75\x00\x32\xc0\xe9\x00\x00\x00\x00\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xd2\x75\x00\xb0\x00\xeb\x00\x8d\x45\x00\x89\x45\x00\x8b\x4d\x00\xc6\x01\x00\xba\x00\x00\x00\x00\x8b\x45\x00\x66\x89\x50\x00\x6a\x00\x6a\x00\x8d\x4d\x00\x51\xe8\x00\x00\x00\x00\x83\xc4\x00\xc6\x45\x00\x00\xc6\x45\x00\x00\x8b\x55", "xxxxx?xx?xx??xx?xxx?x????xx?xxx?xxx????xx?x????xx?xxx?x?x?xx?xx?xx?xx?x????xx?xxx?x?x?xx?xx????xx?xx??xx??xx");

	//wrapper
	//https://gyazo.com/0b5f3bc5a90938b7574db9b9a7020d50
	//https://gyazo.com/e95329d978c3918cca30910f05df26f1 GF
	//Pattern sendFunction = Pattern(1, "\xe8\x00\x00\x00\x00\x84\xc0\x75\x00\x68\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x83\xc4\x00\x32\xc0\x5e\x8b\xe5\x5d\xc2\x00\x00\xcc", "x????xxx?x????x????xx?xxxxxxx??x");
	//Pattern sendFunction = Pattern("Send Function",0, "\x55\x8b\xec\x56\x8b\xf1\x57\x8b\x7d\x00\x8b\x56", "xxxxxxxxx?xx");
	Pattern sendFunction = Pattern("Send Function", 0, "\x56\x8b\xf1\x8b\x46\x00\x8b\x4e\x00\x57\x8b\x7c\x24", "xxxxx?xx?xxxx");
	Pattern recvFunction = Pattern("Recv Function", 0, "\x8b\x44\x24\x00\x56\x57\x8b\x7c\x24\x00\x50", "xxx?xxxxx?x");

	//Pattern recvFunction = Pattern("Recv Function", 0, "\x55\x8b\xec\x56\x57\xff\x75\x00\x8b\x7d", "xxxxxxx?xx");

	//https://gyazo.com/509c7cc703d48fb8e31bea3150687c07
	Pattern getEtherPackFunction = Pattern("GetEther Function", 0, "\x56\x8b\xf1\x83\x7e\x00\x00\x57\x75\x00\x8b\x7c\x24", "xxxxx??xx?xxx");
	//Pattern getEtherPackFunction = Pattern("GetEther Function", 0, "\x55\x8b\xec\x56\x8b\x75\x00\x57\xff\x75\x00\x8b\xf9\x56", "xxxxxx?xxx?xxx");

	//Pattern from Send On_Click Packet caller
	Pattern NetworkClassPointer = Pattern("NetworkClass Pointer", 4, "\x6a\x00\x8b\x0d\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x8b\x55\x00\x83\xc2","x?xx????x????xx?xx");
	//Pattern NetworkClassPointer = Pattern("NetworkClass Pointer", 2, "\x8b\x35\x00\x00\x00\x00\xa3", "xx????x");

	//Pattern sendAttackPacket = Pattern("SendAttack Function", 0, "\x55\x8b\xec\x83\xec\x00\x53\x56\x57\x89\x4d\x00\xeb", "xxxxx?xxxxx?x");
	Pattern sendAttackPacket = Pattern("SendAttack Function", 0,"\x55\x8b\xec\x83\xec\x00\x89\x4d\x00\xc6\x45\x00\x00\x8d\x45\x00\x50\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xc9\x75\x00\x32\xc0\xeb\x00\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xd2\x75\x00\xb0\x00\xeb\x00\xc6\x45\x00\x00\x8a\x45\x00\x88\x45\x00\x8b\x4d", "xxxxx?xx?xx??xx?xxx?x????xx?xxx?xxx?xx?x????xx?xxx?x?x?xx??xx?xx?xx");


	//https://gyazo.com/2b8f2dcde97423b42470b343649ae0f6
	Pattern sendCharacterStatePacket = Pattern("SendCharacterState Function", 0, "\x55\x8b\xec\x83\xec\x00\x89\x4d\x00\xc6\x45\x00\x00\x8d\x45\x00\x50\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xc9\x75\x00\x32\xc0\xe9\x00\x00\x00\x00\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xd2\x75\x00\xb0\x00\xe9\x00\x00\x00\x00\xd9\x45", "xxxxx?xx?xx??xx?xxx?x????xx?xxx?xxx????xx?x????xx?xxx?x?x????xx");
	//Pattern sendCharacterStatePacket = Pattern("SendCharacterState Function",0, "\x55\x8b\xec\x83\xec\x00\xa1\x00\x00\x00\x00\x33\xc5\x89\x45\x00\x53\x56\x57\x89\x4d", "xxxxx?x????xxxx?xxxxx");
}

//THREADING MITGH BE A PROBLEM
void __declspec(naked) GetEter(CMappedFile& file, const char* fileName, void* buffer) {
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
		PUSH edx //Return Address
		JMP __RecvPacket
	}
}

void __declspec(naked) __SendPacketJMP(int size, void* buffer) {
	//pushes the return address
	__asm {
		PUSH [ESP] //Return Address
		JMP __SendPacket
	}
}


void init() {
	auto send = memory_patterns::sendFunction;
	auto recv = memory_patterns::recvFunction;
	auto getEther = memory_patterns::getEtherPackFunction;

	Patterns *patternFinder = 0;
	try {
		patternFinder = new Patterns(hDll, &memory_patterns::globalPat);
	}catch (std::exception &e) {
		MessageBox(NULL, e.what(), "Critical Error", MB_OK);
		exit();
	}

	void* recvAddr = patternFinder->GetPatternAddress(&recv);
	void* sendAddr = patternFinder->GetPatternAddress(&send);
	void* getEtherPackAddr = patternFinder->GetPatternAddress(&getEther);
	void* attackPacketAddr = patternFinder->GetPatternAddress(&memory_patterns::sendAttackPacket);
	void* statePacketAddr = patternFinder->GetPatternAddress(&memory_patterns::sendCharacterStatePacket);
	void** netClassPointer = (void**)patternFinder->GetPatternAddress(&memory_patterns::NetworkClassPointer);
	void* globalToLocalPositionPointer = patternFinder->GetPatternAddress(&memory_patterns::globalToLocalPositionFunction); //getRelativeCallAddress
	globalToLocalPositionPointer = getRelativeCallAddress(globalToLocalPositionPointer);
	printf("%#x\n", globalToLocalPositionPointer);


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
	SetGlobalToLocalPacket(globalToLocalPositionPointer);


	//Hooks
	getEtherPacketHook = new ReturnHook(getEtherPackAddr, GetEter, 7, 3);
	recvHook = new ReturnHook(recvAddr, __RecvPacketJMP, 5, 2);
	sendHook = new JMPStartFuncHook(sendAddr, __SendPacket, 6, THIS_CALL);

	recvHook->HookFunction();
	getEtherPacketHook->HookFunction();
	sendHook->HookFunction();

	initModule();
	delete patternFinder;
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