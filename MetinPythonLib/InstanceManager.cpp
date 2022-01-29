#include "stdafx.h"
#include "InstanceManager.h"
#include "NetworkStream.h"
#include "Memory.h"
#include "Background.h"
#include "Player.h"

#define YANG_ID  1

CInstanceManager::CInstanceManager()
{
	pyVIDList = PyDict_New();
	chrMod = 0;
	pickItemFirst = false;
	pickOnFilter = false;
	pickRange = 300;
	ignoreBlockedPath = false;
}

CInstanceManager::~CInstanceManager()
{
}

void CInstanceManager::importPython()
{
	chrMod = PyImport_ImportModule("chr");
}


void CInstanceManager::changeInstancePosition(SRcv_CharacterMovePacket& packet_move)
{
	if (instances.find(packet_move.dwVID) == instances.end()) {
		DEBUG_INFO_LEVEL_3("No instance vid %d found, creating new one", packet_move.dwVID);
		SRcv_PlayerCreatePacket player = { 0 };
		player.dwVID = packet_move.dwVID;
		player.x = packet_move.lX;
		player.y = packet_move.lY;
		appendNewInstance(player);
		return;
	}
	return;
	DEBUG_INFO_LEVEL_4("VID %d-> X:%d Y:%d", packet_move.dwVID, packet_move.lX, packet_move.lY);
	auto& instance = instances[packet_move.dwVID];
	instance.x = packet_move.lX;
	instance.y = packet_move.lY;
	//DEBUG_INFO("VID %d-> X:%d Y:%d", packet_move.dwVID, packet_move.lX, packet_move.lY);
}

void CInstanceManager::appendNewInstance(SRcv_PlayerCreatePacket& player)
{
	if (instances.find(player.dwVID) != instances.end()) {
		DEBUG_INFO_LEVEL_4("On adding instance with vid=%d, already exists, ignoring packet", player.dwVID);
		return;
	}
	DEBUG_INFO_LEVEL_4("Success Adding instance vid=%d", player.dwVID);

	SInstance i = { 0 };
	i.vid = player.dwVID;
	i.angle = player.angle;
	i.x = player.x;
	i.y = player.y;
	i.bAttackSpeed = player.bAttackSpeed;
	i.bMovingSpeed = player.bMovingSpeed;
	i.wRaceNum = player.wRaceNum;
	i.bStateFlag = player.bStateFlag;

	if (i.wRaceNum >= MIN_RACE_SHOP && i.wRaceNum <= MAX_RACE_SHOP) {
		CNetworkStream& net = CNetworkStream::Instance();
		net.callNewInstanceShop(player.dwVID);
	}

	instances[player.dwVID] = i;

	PyObject* pVid = PyLong_FromLong(player.dwVID);
	PyDict_SetItem(pyVIDList, pVid, pVid);
}

void CInstanceManager::deleteInstance(DWORD vid)
{
	if (instances.find(vid) == instances.end()) {
		DEBUG_INFO_LEVEL_3("On deleting instance with vid=%d doesn't exists, ignoring packet!", vid);
		return;
	}
	PyObject* pVid = PyLong_FromLong(vid);
	PyDict_DelItem(pyVIDList, pVid);
	instances.erase(vid);
}

void CInstanceManager::changeInstanceIsDead(DWORD vid, BYTE isDead)
{
	if (instances.find(vid) != instances.end()) {
		instances[vid].isDead = 1;
	}
}

void CInstanceManager::addItemGround(SRcv_GroundItemAddPacket& itemPacket)
{
	SGroundItem item;
	item.index = itemPacket.itemIndex;
	item.can_pick = true;
	item.x = itemPacket.x;
	item.y = itemPacket.y;
	item.vid = itemPacket.VID;

	if (item.vid <= 0) {
		return;
	}

	DEBUG_INFO_LEVEL_3("Adding item ground with index=%d vid=%d at position x=%d,y=%d, ownerVID=%d", item.index, item.vid, item.x, item.y, item.can_pick);

	groundItems.insert(std::pair<DWORD, SGroundItem>(item.vid, item));
}

void CInstanceManager::delItemGround(SRcv_GroundItemDeletePacket& item)
{
	if (groundItems.find(item.vid) == groundItems.end()) {
		DEBUG_INFO_LEVEL_3("On deleting item with vid=%d doesn't exists, ignoring packet!", item.vid);
		return;
	}
	DEBUG_INFO_LEVEL_4("Deleting item ground with vid=%d", item.vid);
	groundItems.erase(item.vid);
}

void CInstanceManager::clearInstances()
{
	DEBUG_INFO_LEVEL_2("Instances Cleared");
	instances.clear();
	groundItems.clear();
	PyDict_Clear(pyVIDList);
}

void CInstanceManager::changeItemOwnership(SRcv_PacketOwnership& ownership)
{
	auto iter = groundItems.find(ownership.dwVID);
	if (iter != groundItems.end()) {
		DEBUG_INFO_LEVEL_4("Setting ground Item %d ownership to %s!", ownership.dwVID, ownership.szName);
		CPlayer& player = CPlayer::Instance();
		std::string mainPlayerName = player.getPlayerName();
		auto& item = iter->second;
		if (mainPlayerName.compare(ownership.szName) == 0) {
			item.can_pick = true;
		}
		else if (mainPlayerName.compare("") == 0) {
			item.can_pick = true;
		}
		else {
			item.can_pick = false;
		}
		
	}
	else {
		DEBUG_INFO_LEVEL_3("Ground Item %d doesn't exist on list while trying to set ownership!", ownership.dwVID);
	}
}

bool CInstanceManager::getCharacterPosition(DWORD vid, fPoint3D* pos)
{
	void* instanceBase = getInstancePtr(vid);
	if (instanceBase) {
		fPoint3D* iPos = (fPoint3D*)(reinterpret_cast<DWORD>(instanceBase) + OFFSET_CLIENT_CHARACTER_POS);
		pos->x = iPos->x;
		pos->y = -iPos->y;
		pos->z = iPos->z;
		return true;
	}
	return false;
}

void* CInstanceManager::getInstancePtr(DWORD vid)
{
	CMemory& mem = CMemory::Instance();
	return mem.callGetInstancePointer(vid);
}

bool CInstanceManager::isInstanceDead(DWORD vid)
{
	if (instances.find(vid) != instances.end()) {
		return instances.at(vid).isDead;
	}
	return true;
}

bool CInstanceManager::getCloseItemGround(int x, int y, SGroundItem* buffer)
{
	DEBUG_INFO_LEVEL_4("Number of items on ground=%d", groundItems.size());


	DWORD selItemVID = 0;
	float minItemDist = (std::numeric_limits<float>::max)();

	DWORD selYangVID = 0;
	float minYangDist = (std::numeric_limits<float>::max)();

	CBackground& background = CBackground::Instance();


	for (auto iter = groundItems.begin(); iter != groundItems.end(); iter++) {
		DWORD vid = iter->first;
		SGroundItem& item = iter->second;
		if (vid == 0)
			continue;
		if (item.can_pick == false) {
			continue;
		}
		float dist = distance(x, y, item.x, item.y);
		if (dist > pickRange) {
			continue;
		}

		if (ignoreBlockedPath) {
			if (background.isPathBlocked(x, y, item.x, item.y)) {
				continue;
			}
		}
		DEBUG_INFO_LEVEL_4("Item Arround itemVID=%d ID=%d can_pick=%d | X:%d  Y:%d", item.vid, item.index, item.can_pick, item.x, item.y);
		bool is_in = pickupFilter.find(item.index) != pickupFilter.end();

		if (pickOnFilter && is_in) {
			if (item.index == YANG_ID) {
				if (dist < minYangDist) {
					minYangDist = dist;
					selYangVID = vid;
				}
			}
			else {
				if (dist < minItemDist) {
					minItemDist = dist;
					selItemVID = vid;
				}
			}
		}
		else if (!pickOnFilter && !is_in) {
			if (item.index == YANG_ID) {
				if (dist < minYangDist) {
					minYangDist = dist;
					selYangVID = vid;
				}
			}
			else {
				if (dist < minItemDist) {
					minItemDist = dist;
					selItemVID = vid;
				}
			}
		}
	}
	if (pickItemFirst && selItemVID) {
		SGroundItem& selItem = groundItems.at(selItemVID);
		DEBUG_INFO_LEVEL_4("Close Item itemVID=%d ID=%d can_pick=%d | X:%d  Y:%d", selItem.vid, selItem.index, selItem.can_pick, selItem.x, selItem.y);
		*buffer = selItem;
		return true;
	}

	if (selItemVID || selYangVID) {
		if (minItemDist < minYangDist) {
			SGroundItem& selItem = groundItems.at(selItemVID);
			DEBUG_INFO_LEVEL_4("Close Item itemVID=%d ID=%d can_pick =%d  | X:%d  Y:%d", selItem.vid, selItem.index, selItem.can_pick, selItem.x, selItem.y);
			*buffer = selItem;
			return true;
		}
		else {
			SGroundItem& selItem = groundItems.at(selYangVID);
			DEBUG_INFO_LEVEL_4("Close Yang yangVID=%d ID=%d can_pick=%d  | X:%d  Y:%d", selItem.vid, selItem.index, selItem.can_pick, selItem.x, selItem.y);
			*buffer = selItem;
			return true;
		}
	}
	else {
		return false;
	}
}

DWORD CInstanceManager::getItemGrndID(DWORD vid)
{
	if (groundItems.find(vid) == groundItems.end())
		return 0;

	return groundItems.at(vid).index;
}

void CInstanceManager::setPickItemFirst(bool val)
{
	pickItemFirst = val;
}

void CInstanceManager::setPickupRange(float range)
{
	pickRange = range;
}
