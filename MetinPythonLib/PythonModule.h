#pragma once
#include "SleepFunctionHook.h"
#include <stdio.h>
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#include <string>
#include "shlwapi.h"
#include "PythonUtils.h"
#include "Network.h"


//Deprecated
//#define OFFSET_CLIENT_ALIVE_MAP 0x38
#define OFFSET_CLIENT_INSTANCE_PTR_1 0x4
#define OFFSET_CLIENT_INSTANCE_PTR_2 0x8
#define OFFSET_CLIENT_CHARACTER_POS 0x7BC

//Private Shops Race 
#define MIN_RACE_SHOP 30000
#define MAX_RACE_SHOP 30008


/*SELECT THE METHOD USED TO INJECT THE PYTHON FILE, UNCOMMENT ONLY ONE*/
//#define USE_INJECTION_SLEEP_HOOK
#define USE_INJECTION_RECV_HOOK



extern PyObject* packet_mod;
extern PyObject* pyVIDList; //Instances


struct EterFile {
	void* data;
	int size;
	std::string name;
};

#pragma pack(push, 1)
struct CMappedFile {
	void *v_table;
	//int				m_mode;
	//char			m_filename[MAX_PATH + 1];
	//HANDLE			m_hFile;
	BYTE		uknown[280];
	DWORD		m_dwSize;//0x11C needs fix

	BYTE*		m_pbBufLinkData;
	DWORD		m_dwBufLinkSize;

	BYTE*		m_pbAppendResultDataBlock;
	DWORD		m_dwAppendResultDataSize;

	DWORD		m_seekPosition;
	HANDLE		m_hFM;
	DWORD		m_dataOffset;
	DWORD		m_mapSize;
	LPVOID		m_lpMapData;
	LPVOID		m_lpData;

	void *	m_pLZObj;
};
#pragma pack(pop)

bool executePythonFile(char* fileName);


//SET OLD FUNCTION
PyObject* GetPixelPosition(PyObject* poSelf, PyObject* poArgs);
PyObject* moveToDestPosition(PyObject* poSelf, PyObject* poArgs);


PyObject* GetEterPacket(PyObject * poSelf, PyObject * poArgs);
PyObject* IsPositionBlocked(PyObject * poSelf, PyObject * poArgs);
//PyObject* GetCurrentPhase(PyObject * poSelf, PyObject * poArgs);
PyObject* GetAttrByte(PyObject * poSelf, PyObject * poArgs); //Debug purposes
PyObject* pySendAttackPacket(PyObject * poSelf, PyObject * poArgs);
PyObject* pySendStatePacket(PyObject * poSelf, PyObject * poArgs);
PyObject* pySendPacket(PyObject * poSelf, PyObject * poArgs);
PyObject* pyIsDead(PyObject * poSelf, PyObject * poArgs);
PyObject* pySendStartFishing(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendStopFishing(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendAddFlyTarget(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendShoot(PyObject* poSelf, PyObject* poArgs);
PyObject* pyBlockFishingPackets(PyObject* poSelf, PyObject* poArgs);
PyObject* pyUnblockFishingPackets(PyObject* poSelf, PyObject* poArgs);
PyObject* pyDisableCollisions(PyObject* poSelf, PyObject* poArgs);
PyObject* pyEnableCollisions(PyObject* poSelf, PyObject* poArgs);
PyObject* pyRegisterNewShopCallback(PyObject* poSelf, PyObject* poArgs);

PyObject* pyItemGrndFilterClear(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndNotOnFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndOnFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndAddFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndDelFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyGetCloseItemGround(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendPickupItem(PyObject* poSelf, PyObject* poArgs);

//PyObject* pySetKeyState(PyObject* poSelf, PyObject* poArgs); //There is a similar function, OnKeyUp or OnKeyDown


//PACKET FILTER
PyObject* launchPacketFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* closePacketFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* startPacketFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* stopPacketFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* skipInHeader(PyObject* poSelf, PyObject* poArgs);
PyObject* skipOutHeader(PyObject* poSelf, PyObject* poArgs);
PyObject* doNotSkipInHeader(PyObject* poSelf, PyObject* poArgs);
PyObject* doNotSkipOutHeader(PyObject* poSelf, PyObject* poArgs);
PyObject* clearOutput(PyObject* poSelf, PyObject* poArgs);
PyObject* clearInFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* clearOutFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* setInFilterMode(PyObject* poSelf, PyObject* poArgs);
PyObject* setOutFilterMode(PyObject* poSelf, PyObject* poArgs);

//Client stuff
void* getInstancePtr(DWORD vid);
bool  moveToDestPosition(DWORD vid,fPoint& pos);



//Hooked function
DWORD __stdcall _GetEter(DWORD return_value, CMappedFile* file, const char* fileName, void** buffer);
void _RecvRoutine();

//Decrypt files
EterFile* CGetEter(const char* name);

//Instances
void changeInstancePosition(CharacterMovePacket & packet_move);
void appendNewInstance(PlayerCreatePacket & player);
//void registerNewInstanceShop(DWORD player);
void deleteInstance(DWORD vid);
void changeInstanceIsDead(DWORD vid, BYTE isDead);
void addItemGround(GroundItemAddPacket& item);
void delItemGround(GroundItemDeletePacket& item);
void clearInstances();

bool getCharacterPosition(DWORD vid, fPoint3D* pos);

//Intialize stuf
void initModule();
void SetChrMngrAndInstanceMap(void* classPointer);
void SetMoveToDistPositionFunc(void* func);