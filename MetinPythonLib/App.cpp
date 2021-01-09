#include "App.h"
#include "JMPStartFuncHook.h"
#include <stdio.h>
#include <intrin.h>
#include "ReturnHook.h"
#include "Patterns.h"
#include "PythonModule.h"
#include "Network.h"
#include "MapCollision.h"
#include "DetoursHook.h"


DetoursHook<tBackground_CheckAdvancing>* background_CheckAdvHook = 0;
DetoursHook<tInstanceBase_CheckAdvancing>* instanceBase_CheckAdvHook = 0;
DetoursHook<tSendSequencePacket>* sendSequenceHook = 0;
DetoursHook<tSendPacket>* sendHook = 0;
Hook* recvHook = 0;
Hook* getEtherPacketHook = 0;
HMODULE hDll = 0;


namespace memory_patterns {

	//From CPythonBackground::CheckAdvancing
	Pattern Background_CheckAdvancingFunc= Pattern("BackGround::CheckAdvancing", 0, "\x55\x8b\xec\x6a\x00\x68\x00\x00\x00\x00\x64\xa1\x00\x00\x00\x00\x50\x83\xec\x00\xa1\x00\x00\x00\x00\x33\xc5\x50\x8d\x45\x00\x64\xa3\x00\x00\x00\x00\x89\x4d\x00\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xc0\x75\x00\xb0", "xxxx?x????xx????xxx?x????xxxxx?xx????xx?xx?x????xx?xxx?x");

	//From CInstanceBase::CheckAdvancing
	Pattern Instance_CheckAdvancingFunc = Pattern("Instance::CheckAdvancing", 0, "\x55\x8b\xec\x81\xec\x00\x00\x00\x00\x89\x8d\x00\x00\x00\x00\x8b\x8d\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xc0\x0f\x85", "xxxxx????xx????xx????x????xx?xxxx");

	//From CPythonCharacterManager::Instance()
	//https://gyazo.com/7fee4a91c3432b3ba859627612e21dce
	Pattern characterManagerClassPointer = Pattern("ChracterManager Pointer", 4, "\x89\x55\x00\xa1\x00\x00\x00\x00\x89\x45\x00\x8b\x4d\x00\xe8\x00\x00\x00\x00\x89\x45\x00\x83\x7d", "xx?x????xx?xx?x????xx?xx");

	//Search for string CPythonPlayer::__OnPressItem, go to caller of that function and find the __OnPressGround function, and a reference will be inside
	//https://gyazo.com/46f70061fc47d132496d61e92af78bc5	
	Pattern moveToDest = Pattern("MoveToDest Pointer", 0, "\x55\x8b\xec\x83\xec\x00\x89\x4d\x00\x8d\x45\x00\x50\x8b\x4d\x00\xe8\x00\x00\x00\x00\x8b\x4d", "xxxxx?xx?xx?xxx?x????xx");

	//https://gyazo.com/6696a0db5abb62a59203d7282bc4f90a //From RecvCharMovePacket
	//\xe8\x00\x00\x00\x00\x8d\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x4d x????xx?x????xxx
	Pattern globalToLocalPositionFunction = Pattern("GlobalToLocal Function", 0, "\xe8\x00\x00\x00\x00\x8d\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x4d", "x????xx?x????xxx");
	Pattern localToGlobalPositionFunction = Pattern("LocalToGlobal Function", 0, "\x55\x8b\xec\x83\xec\x00\x89\x4d\x00\xa1\x00\x00\x00\x00\x89\x45\x00\x8b\x4d\x00\x51\x8b\x55\x00\x52\x8b\x4d\x00\xe8\x00\x00\x00\x00\x8b\xe5\x5d\xc2\x00\x00\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\x55\x8b\xec\x83\xec\x00\x89\x4d\x00\xa1\x00\x00\x00\x00\x89\x45\x00\x8b\x4d\x00\xe8", "xxxxx?xx?x????xx?xx?xxx?xxx?x????xxxx??xxxxxxxxxxxxxx?xx?x????xx?xx?x");
	//GetInstancePtr on source optional pattern
	//Pattern globalToLocalPositionFunction = Pattern("GetInstancePtr Function", 0, "\x55\x8b\xec\x83\xec\x00\x89\x4d\x00\x8d\x45\x00\x50\x8d\x4d\x00\x51\x8b\x4d\x00\x83\xc1\x00\xe8\x00\x00\x00\x00\x8b\x55\x00\x83\xc2\x00\x89\x55\x00\x8b\x45\x00\x8b\x48\x00\x89\x4d\x00\xc7\x45\x00\x00\x00\x00\x00\x8b\x55\x00\x89\x55\x00\x83\x7d\x00\x00\x75\x00\xe8\x00\x00\x00\x00\x33\xc0\x75\x00\x8b\x4d\x00\x8b\x11\x89\x55\x00\x33\xc0\x83\x7d\x00\x00\x0f\x95\x00\x0f\xb6\x00\x85\xc9\x74\x00\x8b\x55\x00\x33\xc0\x3b\x55\x00\x0f\x94\x00\x0f\xb6\x00\x85\xc9\x75\x00\xe8\x00\x00\x00\x00\x33\xd2\x75\x00\x8b\x45\x00\x33\xc9\x3b\x45\x00\x0f\x94\x00\x0f\xb6\x00\x85\xd2\x74\x00\x33\xc0\xeb\x00\x8d\x4d\x00\xe8\x00\x00\x00\x00\x8b\x40\x00\x8b\xe5\x5d\xc2\x00\x00\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\x55\x8b\xec\x51" "xxxxx?xx?xx?xxx?xxx?xx?x????xx?xx?xx?xx?xx?xx?xx?????xx?xx?xx??x?x????xxx?xx?xxxx?xxxx??xx?xx?xxx?xx?xxxx?xx?xx?xxx?x????xxx?xx?xxxx?xx?xx?xxx?xxx?xx?x????xx?xxxx??xxxxxxxxxxxxxxxx"

	//SendExchangeStartPacket 
    Pattern globalPat = Pattern("Global Pattern",0,"\x55\x8b\xec\x83\xec\x00\x89\x4d\x00\xc6\x45\x00\x00\x8d\x45\x00\x50\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xc9\x75\x00\x32\xc0\xe9\x00\x00\x00\x00\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xd2\x75\x00\xb0\x00\xeb\x00\x8d\x45\x00\x89\x45\x00\x8b\x4d\x00\xc6\x01\x00\xba\x00\x00\x00\x00\x8b\x45\x00\x66\x89\x50\x00\x6a\x00\x6a\x00\x8d\x4d\x00\x51\xe8\x00\x00\x00\x00\x83\xc4\x00\xc6\x45\x00\x00\xc6\x45\x00\x00\x8b\x55", "xxxxx?xx?xx??xx?xxx?x????xx?xxx?xxx????xx?x????xx?xxx?x?x?xx?xx?xx?xx?x????xx?xxx?x?x?xx?xx????xx?xx??xx??xx");

	//wrapper
	//https://gyazo.com/0b5f3bc5a90938b7574db9b9a7020d50
	//https://gyazo.com/e95329d978c3918cca30910f05df26f1 GF
	//Pattern sendFunction = Pattern(1, "\xe8\x00\x00\x00\x00\x84\xc0\x75\x00\x68\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x83\xc4\x00\x32\xc0\x5e\x8b\xe5\x5d\xc2\x00\x00\xcc", "x????xxx?x????x????xx?xxxxxxx??x");
	//Pattern sendFunction = Pattern("Send Function",0, "\x55\x8b\xec\x56\x8b\xf1\x57\x8b\x7d\x00\x8b\x56", "xxxxxxxxx?xx");
	Pattern sendFunction = Pattern("Send Function", 0, "\x56\x8b\xf1\x8b\x46\x00\x8b\x4e\x00\x57\x8b\x7c\x24", "xxxxx?xx?xxxx");
	Pattern sendSequenceFunction = Pattern("SendSequence Function", 0, "\x51\x56\x8b\xf1\x80\xbe\x00\x00\x00\x00\x00\x75", "xxxxxx?????x");
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

	//Migh not be needed
	Pattern sendShootPacket = Pattern("SendShoot Function", 0, "\x55\x8b\xec\x83\xec\x00\x89\x4d\x00\xc6\x45\x00\x00\x8d\x45\x00\x50\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xc9\x75\x00\x32\xc0\xeb\x00\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xd2\x74\x00\xb0\x00\xeb\x00\xc6\x45\x00\x00\x8a\x45\x00\x88\x45\x00\x8d\x4d", "xxxxx?xx?xx??xx?xxx?x????xx?xxx?xxx?xx?x????xx?xxx?x?x?xx??xx?xx?xx");



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


bool __declspec(naked) __SendPacketJMP(int size,void* buffer) {
	__asm {
		PUSH sendHook
		PUSH ECX		//this_call
		PUSH [ESP+8] //Return address
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
	void* sendSequenceAddr = patternFinder->GetPatternAddress(&memory_patterns::sendSequenceFunction);
	void* getEtherPackAddr = patternFinder->GetPatternAddress(&getEther);
	void* attackPacketAddr = patternFinder->GetPatternAddress(&memory_patterns::sendAttackPacket);
	void* statePacketAddr = patternFinder->GetPatternAddress(&memory_patterns::sendCharacterStatePacket);
	void** netClassPointer = (void**)patternFinder->GetPatternAddress(&memory_patterns::NetworkClassPointer);
	void** chrMgrClassPointer = (void**)patternFinder->GetPatternAddress(&memory_patterns::characterManagerClassPointer);
	void* moveToDestAddr = patternFinder->GetPatternAddress(&memory_patterns::moveToDest);
	void* localToGlobalPositionPointer = patternFinder->GetPatternAddress(&memory_patterns::localToGlobalPositionFunction);
	void* globalToLocalPositionPointer = patternFinder->GetPatternAddress(&memory_patterns::globalToLocalPositionFunction);
	void* Background_CheckAdvancingFuncPointer = patternFinder->GetPatternAddress(&memory_patterns::Background_CheckAdvancingFunc);
	void* Instance_CheckAdvancingFuncPointer = patternFinder->GetPatternAddress(&memory_patterns::Instance_CheckAdvancingFunc);
	globalToLocalPositionPointer = getRelativeCallAddress(globalToLocalPositionPointer);


#ifdef _DEBUG
	system("pause");
#endif

	SetNetClassPointer(*netClassPointer);
	SetSendFunctionPointer(sendAddr);
	SetSendBattlePacket(attackPacketAddr);
	SetSendStatePacket(statePacketAddr);
	SetGlobalToLocalFunction(globalToLocalPositionPointer);
	SetChrMngrAndInstanceMap(*chrMgrClassPointer);
	SetMoveToDistPositionFunc(moveToDestAddr);
	SetSendSequenceFunction(sendSequenceAddr);
	SetLocalToGlobalFunction(localToGlobalPositionPointer);


	//Hooks
	getEtherPacketHook = new ReturnHook(getEtherPackAddr, GetEter, 7, 3);
	recvHook = new ReturnHook(recvAddr, __RecvPacketJMP, 5, 2);
	sendHook = new DetoursHook<tSendPacket>((tSendPacket)sendAddr, __SendPacketJMP);//new JMPStartFuncHook(sendAddr, __SendPacketJMP, 6, THIS_CALL);
	sendSequenceHook = new DetoursHook<tSendSequencePacket>((tSendSequencePacket)sendSequenceAddr, __SendSequencePacket);
	background_CheckAdvHook = new DetoursHook<tBackground_CheckAdvancing>((tBackground_CheckAdvancing)Background_CheckAdvancingFuncPointer, __BackgroundCheckAdvanced);
	instanceBase_CheckAdvHook = new DetoursHook<tInstanceBase_CheckAdvancing>((tInstanceBase_CheckAdvancing)Instance_CheckAdvancingFuncPointer, __InstanceBaseCheckAdvanced);


	recvHook->HookFunction();
	getEtherPacketHook->HookFunction();
	sendHook->HookFunction();
	sendSequenceHook->HookFunction();

	//This hooks are initialize in the respective function
	SetBCheckAdvanceFunction(background_CheckAdvHook);
	SetICheckAdvanceFunction(instanceBase_CheckAdvHook);

	//THIS MAY CRASH
	SetSendFunctionPointer(sendHook->originalFunction);
	SetSendSequenceFunction(sendSequenceHook->originalFunction);


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

	delete background_CheckAdvHook;
	delete instanceBase_CheckAdvHook;
	delete sendHook;
	delete recvHook;
	delete getEtherPacketHook;
	delete sendSequenceHook;

	freeCurrentMap();

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	FreeConsole();
}