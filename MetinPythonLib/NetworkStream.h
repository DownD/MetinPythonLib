#pragma once
#include "stdafx.h"
#include "Singleton.h"
#include "../common/utils.h"
#include "defines.h"
#include "PythonUtils.h"

typedef bool PACKET_TYPE;
#define INBOUND 1
#define OUTBOUND 0

#define ONLY_INCLUDED_FILTER 1
#define ONLY_EXCLUDED_FILTER 0

class CNetworkStream : public CSingleton<CNetworkStream>
{
public:
	CNetworkStream();
	~CNetworkStream();

	void importPython();

	//Packet Send
	bool SendBattlePacket(DWORD vid, BYTE type);
	bool SendStatePacket(fPoint& pos, float rot, BYTE eFunc, BYTE uArg);
	bool SendPacket(int size, void* buffer);
	bool SendSequencePacket();
	bool SendAddFlyTargetingPacket(DWORD dwTargetVID, float x, float y);
	bool SendShootPacket(BYTE uSkill);
	bool SendStartFishing(WORD direction);
	bool SendStopFishing(BYTE type, float timeLeft);
	bool SendPickupItemPacket(DWORD vid);
	bool SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID);
	void SendUseSkillBySlot(DWORD dwSkillSlotIndex, DWORD dwTargetVID);

	//Hooks callbacks
	bool __RecvPacket(int size, void* buffer);
	bool __SendPacket(int size, void* buffer);
	bool __SendSequencePacket();
	bool __SendStatePacket(fPoint& pos, float rot, BYTE eFunc, BYTE uArg);
	bool __CheckPacket(BYTE * header);


	int GetCurrentPhase();
	DWORD GetMainCharacterVID();
	bool GlobalToLocalPosition(long& lx, long& ly);
	bool LocalToGlobalPosition(LONG& rLocalX, LONG& rLocalY);

	//DigMotion
	void callDigMotionCallback(DWORD target_player, DWORD target_vein, DWORD n);
	bool setDigMotionCallback(PyObject* func);

	//Fishing and speed hooks
	void SetSpeedMultiplier(float val);
	void SetFishingPacketsBlock(bool val); //1 block packets, 0 do not block packets
	bool setStartFishCallback(PyObject* func);

	//New Shop callback
	bool setNewShopCallback(PyObject* func);
	void callNewInstanceShop(DWORD player);

	//Packet filter
	void openConsole();
	void closeConsole();
	void startFilterPacket();
	void stopFilterPacket();
	void removeHeaderFilter(BYTE header, PACKET_TYPE t);
	void addHeaderFilter(BYTE header, PACKET_TYPE t);
	void clearPacketFilter(PACKET_TYPE t);
	void setFilterMode(PACKET_TYPE t, bool val);
	void printPacket(DWORD calling_function, BYTE* buffer , int size, bool type);
	std::set<BYTE>* getPacketFilter(PACKET_TYPE t);


private:

	void callStartFishCallback();

	BYTE getPacketHeader(void* buffer, int size);
	bool RecvGamePhase(BYTE* header);
	bool RecvLoadingPhase(BYTE* header);
	void setPhase(SRcv_ChangePhasePacket& phase);

	bool peekNetworkStream(int len,void * buffer);

	//0x2 fishing header is responsible for telling client when it can be fished
	int getFishingPacketSize(BYTE header, bool isRecive= true);


private:

	//Related to speedhack
	fPoint lastPoint;
	bool lastPointIsStored;
	float speed_Multiplier;

	//Fishing
	bool blockFishingPackets;
	bool block_next_sequence;

	//DigMotion callback
	PyObject* recvDigMotionCallback;
	PyObject* shopRegisterCallback;
	PyObject* recvStartFishCallback;

	//Packet filter
	bool filterInboundOnlyIncluded;
	bool filterOutboundOnlyIncluded;
	bool printToConsole;
	std::set<BYTE> inbound_header_filter;
	std::set<BYTE> outbound_header_filter;

	DWORD mainCharacterVID;
	BYTE currentPhase;
	PyObject* netMod;
};

