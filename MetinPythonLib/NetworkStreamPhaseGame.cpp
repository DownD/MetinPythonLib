#include "stdafx.h"
#include "NetworkStream.h"
#include "InstanceManager.h"
#include "Memory.h"


bool CNetworkStream::RecvDeadPacket()
{
	SRcvDeadPacket dead;
	if (peekNetworkStream(sizeof(SRcvDeadPacket), &dead)) {
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.changeInstanceIsDead(dead.vid, 1);
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse dead packet!");
	}
	return true;
}


bool CNetworkStream::RecvFishPacket()
{
	SRecvHeaderFishingPacket instancePacket;
	if (peekNetworkStream(sizeof(SRecvHeaderFishingPacket), &instancePacket)) {
		if (blockFishingPackets && instancePacket.fishingHeader != 5) {

			CMemory& mem = CMemory::Instance();
			BYTE tmp[0x21];
			if (!mem.callRecvPacket(getFishingPacketSize(instancePacket.fishingHeader), tmp)) {
				DEBUG_INFO_LEVEL_2("Could not block fishing packet!");
			}

			if (instancePacket.fishingHeader == 2) {
				callStartFishCallback();
			}
			return false;
		}
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse fishing packet!");
	}
	return true;
}

bool CNetworkStream::RecvChatPacket()
{
	SRcv_ChatPacket chatPacket;
	if (peekNetworkStream(sizeof(SRcv_ChatPacket), &chatPacket)) {
		handleChatPacket(chatPacket);
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse chat packet!");
	}
	return true;
}

bool CNetworkStream::RecvAddItemGrnd()
{
	SRcv_GroundItemAddPacket instance;
	if (peekNetworkStream(sizeof(SRcv_GroundItemAddPacket), &instance)) {
		GlobalToLocalPosition(instance.x, instance.y);
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.addItemGround(instance);
		callRecvAddGrndItemCallback(instance.VID, instance.itemIndex, instance.x, instance.y);
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse item ground add packet!");
	}
	return true;
}

bool CNetworkStream::RecvDelItemGrnd()
{
	SRcv_GroundItemDeletePacket instance;
	if (peekNetworkStream(sizeof(SRcv_GroundItemDeletePacket), &instance)) {
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.delItemGround(instance);
		callRecvDelGrndItemCallback(instance.vid);
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse item ground delete packet!");
	}
	return true;
}

bool CNetworkStream::RecvAddCharacter()
{
	SRcv_PlayerCreatePacket instance;
	if (peekNetworkStream(sizeof(SRcv_PlayerCreatePacket), &instance)) {
		GlobalToLocalPosition(instance.x, instance.y);
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.appendNewInstance(instance);
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse character add packet!");
	}
	return true;
}

bool CNetworkStream::RecvDelCharacter()
{
	SRcv_DeletePlayerPacket instance_;
	if (peekNetworkStream(sizeof(SRcv_DeletePlayerPacket), &instance_)) {
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.deleteInstance(instance_.dwVID);
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse character delete packet!");
	}
	return true;
}

bool CNetworkStream::RecvMoveCharacter()
{

	SRcv_CharacterMovePacket instance;
	if (peekNetworkStream(sizeof(SRcv_CharacterMovePacket), &instance)) {
		GlobalToLocalPosition(instance.lX, instance.lY);
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.changeInstancePosition(instance);
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse character move packet!");
	}
	return true;
}

bool CNetworkStream::RecvChangeOwnership()
{
	SRcv_PacketOwnership instance;
	if (peekNetworkStream(sizeof(SRcv_PacketOwnership), &instance)) {
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.changeItemOwnership(instance);
		callRecvChangeOwnershipGrndItemCallback(instance.dwVID, instance.szName);
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse item ownership packet!");
	}
	return true;
}

bool CNetworkStream::RecvPhaseChange()
{
	SRcv_ChangePhasePacket phase;
	if (peekNetworkStream(sizeof(SRcv_ChangePhasePacket), &phase)) {
		if (phase.phase != 1 && phase.phase != 2)
			setPhase(phase);
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse phase packet!");
	}
	return true;
}

bool CNetworkStream::RecvDigMotion()
{
	SRcv_DigMotionPacket instance_;
	if (peekNetworkStream(sizeof(SRcv_DigMotionPacket), &instance_)) {
		callDigMotionCallback(instance_.vid, instance_.target_vid, instance_.count);
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse dig motion packet!");
	}
	return true;
}