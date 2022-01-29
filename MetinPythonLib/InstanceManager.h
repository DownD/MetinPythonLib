#pragma once
#include "stdafx.h"
#include "Singleton.h"
#include "defines.h"
#include "../common/utils.h"
#include "PythonUtils.h"
#include <map>
#include <unordered_map>

class CInstanceManager : public CSingleton<CInstanceManager>
{
public:
	CInstanceManager();
	~CInstanceManager();

	void importPython();
public:

	void changeInstancePosition(SRcv_CharacterMovePacket& packet_move);
	void appendNewInstance(SRcv_PlayerCreatePacket& player);
	void deleteInstance(DWORD vid);
	void changeInstanceIsDead(DWORD vid, BYTE isDead);
	void addItemGround(SRcv_GroundItemAddPacket& item);
	void delItemGround(SRcv_GroundItemDeletePacket& item);
	void clearInstances();

	//Sets ownerVID of items to -1 if it belongs to another person
	void changeItemOwnership(SRcv_PacketOwnership& ownership);


	bool getCharacterPosition(DWORD vid, fPoint3D* pos);
	void* getInstancePtr(DWORD vid);
	bool isInstanceDead(DWORD vid);
	inline PyObject* getVIDList() { return pyVIDList; };

	//Pickup filter
	inline void clearFilter() { pickupFilter.clear(); };
	inline void addItemFilter(DWORD index) {DEBUG_INFO_LEVEL_3("Pickup Filter Add=%d", index);pickupFilter.insert(index);};
	inline void deleteItemFilter(DWORD index) {DEBUG_INFO_LEVEL_3("Pickup Filter Remove=%d", index);pickupFilter.erase(index);};
	inline void setModeFilter(bool val) { pickOnFilter = val; };
	bool getCloseItemGround(int x, int y, SGroundItem* buffer);
	DWORD getItemGrndID(DWORD vid);
	void setPickItemFirst(bool val);
	void setPickupRange(float range);
	void setIgnoreBlockedPath(bool val){this->ignoreBlockedPath = val;}

private:
	PyObject* pyVIDList;
	PyObject* chrMod;

	bool pickOnFilter;
	bool pickItemFirst;
	bool ignoreBlockedPath;
	float pickRange;

	std::set<DWORD> pickupFilter;
	std::map<DWORD, SGroundItem> groundItems;
	std::unordered_map<DWORD, SInstance> instances;
};

