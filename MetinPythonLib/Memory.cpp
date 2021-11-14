#include "stdafx.h"
#include "Memory.h"
#include "NetworkStream.h"
#include "Player.h"
#include "App.h"
#include "CAddressLoader.h"



bool __fastcall __GetEter(ClassPointer cp,DWORD edx,CMappedFile & file, const char* fileName, void** buffer) {
	CPlayer& instance = CPlayer::Instance();
	CMemory& mem = CMemory::Instance();
	bool val = mem.callGet(cp, file, fileName, buffer);
	instance.__GetEter(file, fileName, buffer);
	return val;
}

bool __fastcall __Process(ClassPointer p) {
	CApp& instance = CApp::Instance();
	return instance.__AppProcess(p);
}

bool __fastcall __RecvPacket(ClassPointer p, DWORD edx,int size, void* buffer) {
	CNetworkStream& net = CNetworkStream::Instance();
	CMemory& mem = CMemory::Instance();
	mem.setNetworkStream(p);
	return net.__RecvPacket(size, buffer);
}
bool __fastcall __SendPacket(ClassPointer classPointer,DWORD edx, int size, void* buffer) {
	CNetworkStream& net = CNetworkStream::Instance();
	CMemory& mem = CMemory::Instance();
	mem.setNetworkStream(classPointer);
	return net.__SendPacket(size, buffer);
}
/*bool __declspec(naked) __SendPacketJMP(int size, void* buffer) {
	__asm {
		PUSH ECX		//this_call
		PUSH[ESP + 8] //Return address
		JMP __SendPacket
	}
}*/

bool __fastcall __SendAttackPacket(ClassPointer classPointer, DWORD EDX, BYTE type, DWORD vid) {
	CNetworkStream& net = CNetworkStream::Instance();
	return net.__SendAttackPacket(type, vid);
}



bool __fastcall __SendSequencePacket(DWORD classPointer)
{
	CNetworkStream& net = CNetworkStream::Instance();
	return net.__SendSequencePacket();
}


//return 0 to ignore collisions
bool __fastcall __BackgroundCheckAdvanced(ClassPointer classPointer, DWORD EDX, void* instanceBase)
{
	CPlayer & instance = CPlayer::Instance();
	return instance.__BackgroundCheckAdvanced(classPointer,instanceBase);
}

//return 0 to ignore collisions
bool __fastcall __InstanceBaseCheckAdvanced(ClassPointer classPointer)
{
	CPlayer & instance = CPlayer::Instance();
	return instance.__InstanceBaseCheckAdvanced(classPointer);
}

bool __fastcall __SendStatePacket(ClassPointer classPointer, DWORD EDX, fPoint& pos, float rot, BYTE eFunc, BYTE uArg)
{
	CNetworkStream& net = CNetworkStream::Instance();
	return net.__SendStatePacket(pos,rot,eFunc,uArg);
}

bool __fastcall __MoveToDestPosition(ClassPointer classPointer, DWORD EDX, fPoint& pos)
{
	CPlayer & instance = CPlayer::Instance();
	return instance.__MoveToDestPosition(classPointer, pos);
}

bool __fastcall __MoveToDirection(ClassPointer classPointer, DWORD EDX, float rot)
{
	CPlayer& instance = CPlayer::Instance();
	return instance.__MoveToDirection(classPointer, rot);
}

bool __fastcall __CheckPacket(ClassPointer classPointer, DWORD EDX, BYTE * header)
{
	CNetworkStream& instance = CNetworkStream::Instance();
	CMemory& mem = CMemory::Instance();
	return instance.__CheckPacket(header);
}

void __Tracef(const char* c_szFormat, ...) {
	va_list args;
	va_start(args, c_szFormat);
	Tracef(1, c_szFormat, args);
	CMemory & instance = CMemory::Instance();
	instance.traceFHook->originalFunction(c_szFormat, args);
	va_end(args);
}

void __Tracenf(const char* c_szFormat, ...) {
	va_list args;
	va_start(args, c_szFormat);
	Tracef(0, c_szFormat, args);
	CMemory & memory = CMemory::Instance();

	memory.tracenFHook->originalFunction(c_szFormat, args);
	va_end(args);
}

CMemory::CMemory()
{
	networkStream = 0;
}

CMemory::~CMemory()
{

#ifdef _DEBUG
	delete traceFHook;
	delete tracenFHook;
#endif // _DEBUG

	delete checkPacketHook;
	delete backgroundCheckAdvHook;
	delete instanceBaseCheckAdvHook;
	delete sendHook;
	delete recvHook;
	delete getEtherPacketHook;
	delete sendSequenceHook;
	delete sendStateHook;
	delete setMoveToDestPositionHook;
	delete setMoveToDirectionHook;
	delete getEtherPacketHook;
	delete processHook;
	delete graphicPatch;
}

bool CMemory::setupPatterns(HMODULE hDll)
{
	this->hDll = hDll;
	CAddressLoader addrLoader;
	bool value = addrLoader.setAddress(hDll);
	if (!value) {
		DEBUG_INFO_LEVEL_1("Error Setting Addresses");
		return value;
	}

	recvAddr = addrLoader.GetAddress(RECV_FUNCTION);//patternFinder->GetPatternAddress(&memory_patterns::recvFunction);
	sendAddr = addrLoader.GetAddress(SEND_FUNCTION);
	sendSequenceAddr = addrLoader.GetAddress(SENDSEQUENCE_FUNCTION);
	getEtherPackAddr = addrLoader.GetAddress(GETETHER_FUNCTION);//patternFinder->GetPatternAddress(&memory_patterns::getEtherPackFunction);
	statePacketAddr = addrLoader.GetAddress(SENDCHARACTERSTATE_FUNCTION);
	netClassPointer = (void**)addrLoader.GetAddress(NETWORKCLASS_POINTER);
	pythonPlayerPointer = (void**)addrLoader.GetAddress(PYTHONPLAYER_POINTER);
	chrMgrClassPointer = (void**)addrLoader.GetAddress(CHRACTERMANAGER_POINTER);
	moveToDestAddr = addrLoader.GetAddress(INSTANCEBASE_MOVETODEST);
	backgroundCheckAdvancingAddr = addrLoader.GetAddress(BACKGROUND_CHECKADVANCING);
	instanceCheckAdvancingAddr = addrLoader.GetAddress(INSTANCE_CHECKADVANCING);
	traceFFuncAddr = addrLoader.GetAddress(TRACEF_POINTER);
	tracenFFuncAddr = addrLoader.GetAddress(TRACENF_POINTER);
	moveToDirectionAddr = addrLoader.GetAddress(MOVETODIRECTION_FUNCTION);
	processAddr = addrLoader.GetAddress(PYTHONAPP_PROCESS);
	checkPacketAddr = addrLoader.GetAddress(CHECK_PACKET_FUNCTION);
	sendAttackPacketAddr = addrLoader.GetAddress(SENDATTACK_FUNCTION);
	skipGraphicsAddr = addrLoader.GetAddress(RENDER_MID_FUNCTION);

	//sendAttackPacketFunc = (tSendAttackPacket)addrLoader.GetAddress(SENDATTACK_FUNCTION);
	sendUseSkillBySlotFunc = (tSendUseSkillBySlot)addrLoader.GetAddress(PYTHONPLAYER_SENDUSESKILL);
	localToGlobalFunc = (tLocalToGlobalPosition)addrLoader.GetAddress(LOCALTOGLOBAL_FUNCTION);
	globalToLocalFunc = (tGlobalToLocalPosition)addrLoader.GetAddress(GLOBALTOLOCAL_FUNCTION);
	peekFunc = (tPeek)addrLoader.GetAddress(PEEK_FUNCTION);

	peekFunc = (tPeek)getRelativeCallAddress((void*)peekFunc);
	globalToLocalFunc = (tGlobalToLocalPosition)getRelativeCallAddress((void*)globalToLocalFunc);
	recvAddr = getRelativeCallAddress(recvAddr);

	DEBUG_INFO_LEVEL_1("Peek relative final address: %#x", peekFunc);
	DEBUG_INFO_LEVEL_1("globalToLocal relative final address: %#x", globalToLocalFunc);
	DEBUG_INFO_LEVEL_1("recv relative final address: %#x", recvAddr);

	pythonNetwork = SetClassPointer((DWORD**)netClassPointer);
	pythonPlayer = SetClassPointer((DWORD**)pythonPlayerPointer);
	pythonChrMgr = SetClassPointer((DWORD**)chrMgrClassPointer);

	DEBUG_INFO_LEVEL_1("pythonNetwork->%#x", pythonNetwork);
	DEBUG_INFO_LEVEL_1("pythonPlayer->%#x", pythonPlayer);
	DEBUG_INFO_LEVEL_1("pythonChrMgr->%#x", pythonChrMgr);

	getInstanceClassPtr = (ClassPointer)((DWORD)*pythonChrMgr + OFFSET_CLIENT_INSTANCE_PTR_1);

	getInstanceFunc = reinterpret_cast<tGetInstancePointer>(*(DWORD*)(*(DWORD*)getInstanceClassPtr + OFFSET_CLIENT_INSTANCE_PTR_2));
	return true;
}

bool CMemory::setupHooks()
{
	//Hooks
	getEtherPacketHook = new DetoursHook<tGet>((tGet)getEtherPackAddr, __GetEter);
	recvHook = new DetoursHook<tRecvPacket>((tRecvPacket)recvAddr, __RecvPacket);
	sendHook = new DetoursHook<tSendPacket>((tSendPacket)sendAddr, __SendPacket);//new JMPStartFuncHook(sendAddr, __SendPacketJMP, 6, THIS_CALL);
	sendSequenceHook = new DetoursHook<tSendSequencePacket>((tSendSequencePacket)sendSequenceAddr, __SendSequencePacket);
	backgroundCheckAdvHook = new DetoursHook<tBackground_CheckAdvancing>((tBackground_CheckAdvancing)backgroundCheckAdvancingAddr, __BackgroundCheckAdvanced);
	instanceBaseCheckAdvHook = new DetoursHook<tInstanceBase_CheckAdvancing>((tInstanceBase_CheckAdvancing)instanceCheckAdvancingAddr, __InstanceBaseCheckAdvanced);
	sendStateHook = new DetoursHook<tSendStatePacket>((tSendStatePacket)statePacketAddr, __SendStatePacket);
	setMoveToDestPositionHook = new DetoursHook<tMoveToDestPosition>((tMoveToDestPosition)moveToDestAddr, __MoveToDestPosition);
	setMoveToDirectionHook = new DetoursHook<tMoveToDirection>((tMoveToDirection)moveToDirectionAddr, __MoveToDirection);
	checkPacketHook = new DetoursHook<tCheckPacket>((tCheckPacket)checkPacketAddr, __CheckPacket);
	sendAttackPacketHook = new DetoursHook<tSendAttackPacket>((tSendAttackPacket)sendAttackPacketAddr, __SendAttackPacket);

	traceFHook = new DetoursHook<tTracef>((tTracef)traceFFuncAddr, __Tracef);
	tracenFHook = new DetoursHook<tTracef>((tTracef)tracenFFuncAddr, __Tracenf);

	BYTE patch[] = { 0x90,0x90,0x0,0x0,0x0,0x0,0x0,0x0,0x1 }; //Patch for skip graphics
	graphicPatch = new CMemoryPatch(patch, "xx??????x", 9, skipGraphicsAddr);
#ifdef _DEBUG
	traceFHook->HookFunction();
	tracenFHook->HookFunction();
#endif // DEBUG
#ifdef _DEBUG_FILE

	traceFHook->HookFunction();
	tracenFHook->HookFunction();
#endif

	getEtherPacketHook->HookFunction();
	recvHook->HookFunction();
	sendHook->HookFunction();
	sendSequenceHook->HookFunction();
	backgroundCheckAdvHook->HookFunction();
	instanceBaseCheckAdvHook->HookFunction();
	sendStateHook->HookFunction();
	setMoveToDestPositionHook->HookFunction();
	setMoveToDirectionHook->HookFunction();
	checkPacketHook->HookFunction();
	sendAttackPacketHook->HookFunction();
	return true;
}

bool CMemory::setupProcessHook()
{
	processHook = new DetoursHook<tProcess>((tProcess)processAddr, __Process);
	return processHook->HookFunction();
}

void CMemory::setSkipRenderer()
{
	graphicPatch->patchMemory();
}

void CMemory::unsetSkipRenderer()
{
	graphicPatch->unPatchMemory();
}
