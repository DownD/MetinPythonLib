#pragma once
#include "stdafx.h"
#include "DetoursHook.h"
#include "../common/Patterns.h"
#include "ReturnHook.h"
#include "defines.h"
#include "Singleton.h"
#include "MemoryPatch.h"



class CMemory : public CSingleton<CMemory>
{
public:
	CMemory();
	~CMemory();

	bool setupPatterns(HMODULE hDll);
	bool setupHooks();

	bool setupProcessHook();

	inline ClassPointer getPythonNetwork() { return *pythonNetwork; };
	inline ClassPointer getPythonChrMgr() { return *pythonChrMgr; };
	inline ClassPointer getPythonPlayer() { return *pythonPlayer; };
	inline ClassPointer getNetworkStream() { return networkStream; };
	inline void setNetworkStream(ClassPointer val) {networkStream=val; };

	//Hooked original functions
	inline bool callBackgroundCheckAdv(ClassPointer p,void* instanceBase) { return backgroundCheckAdvHook->originalFunction(p, instanceBase);}
	inline bool callIntsanceBaseCheckAdv(ClassPointer p) { return instanceBaseCheckAdvHook->originalFunction(p); }
	inline bool callSendSequence() { return sendSequenceHook->originalFunction(getNetworkStream()); }
	inline bool callSendPacket(int size, void* buffer, ClassPointer classPointer = 0) { if (getNetworkStream() && classPointer == 0) return sendHook->originalFunction(getNetworkStream(), size, buffer); return false; }//If network stream is not set, the function will not be called
	inline bool callSendStatePacket(fPoint& pos, float rot, BYTE eFunc, BYTE uArg) { return sendStateHook->originalFunction(getPythonNetwork(),pos,rot,eFunc,uArg); }
	inline bool callMoveToDestPosition(ClassPointer p, fPoint& pos) { return setMoveToDestPositionHook->originalFunction(p, pos); }
	inline bool callMoveToDirection(ClassPointer p, float rot) { return setMoveToDirectionHook->originalFunction(p, rot); }
	inline bool callProcess(ClassPointer p) { return processHook->originalFunction(p); }
	inline bool callGet(ClassPointer cp, CMappedFile& file, const char* fileName, void** buffer) { return getEtherPacketHook->originalFunction(cp, file, fileName, (LPCVOID*)buffer); }
	inline bool callCheckPacket(BYTE * header) { return checkPacketHook->originalFunction(getPythonNetwork(),header); }
	inline bool callRecvPacket(int size, void* buffer) { return recvHook->originalFunction(getNetworkStream(), size, buffer); }

	//Client functions
	inline bool callSendAttackPacket(BYTE type, DWORD vid) { return sendAttackPacketHook->originalFunction(getPythonNetwork(), type, vid); }
	inline bool callGlobalToLocalPosition(long& lx, long& ly){return globalToLocalFunc(getPythonNetwork(),lx,ly);}
	inline bool callLocalToGlobalPosition(long& lx, long& ly) { return localToGlobalFunc(getPythonNetwork(), lx, ly); }
	inline void* callGetInstancePointer(DWORD vid) { return getInstanceFunc(getInstanceClassPtr,vid); }
	inline void callSendUseSkillBySlot(DWORD dwSkillSlotIndex, DWORD dwTargetVID) { return sendUseSkillBySlotFunc(getPythonPlayer(), dwSkillSlotIndex, dwTargetVID); }
	inline bool callPeek(int len, void*buffer) { return peekFunc(getNetworkStream(),len,buffer); }

	void setSkipRenderer();
	void unsetSkipRenderer();

	DetoursHook<tTracef>* traceFHook;
	DetoursHook<tTracef>* tracenFHook;

private:
	inline ClassPointer* SetClassPointer(DWORD** pointer) { if(pointer)return (ClassPointer*)*pointer; };

	

private:

	//Hooks
	DetoursHook<tRecvPacket>* recvHook;

	DetoursHook<tGet> * getEtherPacketHook;
	DetoursHook<tBackground_CheckAdvancing>* backgroundCheckAdvHook;
	DetoursHook<tInstanceBase_CheckAdvancing>* instanceBaseCheckAdvHook;
	DetoursHook<tSendSequencePacket>* sendSequenceHook;
	DetoursHook<tSendPacket>* sendHook;
	DetoursHook<tSendStatePacket>* sendStateHook;
	DetoursHook<tMoveToDestPosition>* setMoveToDestPositionHook;
	DetoursHook<tMoveToDirection>* setMoveToDirectionHook;
	DetoursHook<tProcess>* processHook;
	DetoursHook<tCheckPacket>* checkPacketHook;
	DetoursHook<tSendAttackPacket>* sendAttackPacketHook;

	//Memory patches
	CMemoryPatch* graphicPatch;

	//Pointers
	void* recvAddr;
	void* sendAddr;
	void* sendSequenceAddr;
	void* getEtherPackAddr;
	void* statePacketAddr;
	void** netClassPointer;
	void** pythonPlayerPointer;
	void** chrMgrClassPointer;
	void* moveToDestAddr;
	void* backgroundCheckAdvancingAddr;
	void* instanceCheckAdvancingAddr;
	void* traceFFuncAddr;
	void* tracenFFuncAddr;
	void* moveToDirectionAddr;
	void* processAddr;
	void* checkPacketAddr;
	void* sendAttackPacketAddr;
	void* skipGraphicsAddr;

	tLocalToGlobalPosition	localToGlobalFunc;
	tGlobalToLocalPosition	globalToLocalFunc;
	//tSendAttackPacket		sendAttackPacketFunc;
	tGetInstancePointer		getInstanceFunc;
	tSendUseSkillBySlot		sendUseSkillBySlotFunc;
	tPeek					peekFunc;

	ClassPointer* pythonNetwork;
	ClassPointer* pythonChrMgr;
	ClassPointer* pythonPlayer;
	ClassPointer networkStream; //Used by Send
	ClassPointer getInstanceClassPtr;


	HMODULE hDll;
};

