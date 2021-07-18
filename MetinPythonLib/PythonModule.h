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


#define OFFSET_GRAPHIC_INSTANCE 0x234

//Deprecated
//#define OFFSET_CLIENT_ALIVE_MAP 0x38
#define OFFSET_CLIENT_INSTANCE_PTR_1 0x4
#define OFFSET_CLIENT_INSTANCE_PTR_2 0x8
#define OFFSET_CLIENT_CHARACTER_POS 0x7C0
#define OFFSET_CLIENT_CHARACTER_DST 0x08B8

//MOVEMENT SPEED
#define STATUS_MOVEMENT_SPEED 19

//Private Shops Race 
#define MIN_RACE_SHOP 30000
#define MAX_RACE_SHOP 30008

#define MOVE_WALK 1
#define MOVE_POSITION 0


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

struct SState
{
	TPixelPosition kPPosSelf;
	FLOAT fAdvRotSelf;
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


typedef bool	(__thiscall* tMoveToDirection)(void* classPointer, float rot);
typedef bool	(__thiscall* tMoveToDestPosition)(void* classPointer, fPoint& pos);
typedef void	(__thiscall* tSendUseSkill)(DWORD classPointer, DWORD dwSkillSlotIndex, DWORD dwTargetVID);


bool executePythonFile(char* fileName);


//SET OLD FUNCTION
PyObject* GetPixelPosition(PyObject* poSelf, PyObject* poArgs);
PyObject* moveToDestPosition(PyObject* poSelf, PyObject* poArgs);
PyObject* pySetMoveSpeed(PyObject* poSelf, PyObject* poArgs);


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
PyObject* pySendUseSkillPacket(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendUseSkillPacketBySlot(PyObject* poSelf, PyObject* poArgs);
PyObject* pyRecvDigMotionCallback(PyObject* poSelf, PyObject* poArgs);

PyObject* pyItemGrndFilterClear(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndNotOnFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndOnFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndAddFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyItemGrndDelFilter(PyObject* poSelf, PyObject* poArgs);
PyObject* pyGetCloseItemGround(PyObject* poSelf, PyObject* poArgs);
PyObject* pySendPickupItem(PyObject* poSelf, PyObject* poArgs);
PyObject* pyGetItemGrndID(PyObject* poSelf, PyObject* poArgs);

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


void _RecvRoutine();

//Hooked function
DWORD __stdcall _GetEter(DWORD return_value, CMappedFile* file, const char* fileName, void** buffer);
bool __fastcall __MoveToDestPosition(void* classPointer, DWORD EDX ,fPoint& pos);
bool __fastcall __MoveToDirection(void* classPointer, DWORD EDX, float rot);

//Decrypt files
EterFile* CGetEter(const char* name);

//Instances
void changeInstancePosition(CharacterMovePacket & packet_move);
void appendNewInstance(PlayerCreatePacket & player);
void deleteInstance(DWORD vid);
void changeInstanceIsDead(DWORD vid, BYTE isDead);
void addItemGround(GroundItemAddPacket& item);
void delItemGround(GroundItemDeletePacket& item);
void clearInstances();
void callDigMotionCallback(DWORD target_player, DWORD target_vein, DWORD n);
//long getCurrentSpeed();
void setPixelPosition(fPoint fPos);
void sendUseSkillBySlot(DWORD dwSkillSlotIndex, DWORD dwTargetVID); //By index and sets cooldown
BYTE getLastMovementType();
fPoint getLastDestPosition();

bool getCharacterPosition(DWORD vid, fPoint3D* pos);



//Intialize stuff
void initModule();
void SetChrMngrAndInstanceMap(void* classPointer);
void SetPythonPlayerPointer(void* classPointer);
void SetSendUseSkillFunc(void* func);
void SetMoveToDistPositionFunc(DetoursHook<tMoveToDestPosition>* hook);
void SetMoveToToDirectionFunc(DetoursHook<tMoveToDirection>* hook);