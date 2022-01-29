#pragma once
#include "../common/utils.h"
#define MAX_TELEPORT_DIST 2400

#define MOVE_WALK 1
#define MOVE_POSITION 0

#define OFFSET_CLIENT_INSTANCE_PTR_1 0x4
#define OFFSET_CLIENT_INSTANCE_PTR_2 0x8
#define OFFSET_CLIENT_CHARACTER_POS 0x7C4

//Private Shops Race 
#define MIN_RACE_SHOP 30000
#define MAX_RACE_SHOP 30008

#define CHARACTER_NAME_MAX_LEN 24

//Pattern IDS
#define PYTHONAPP_PROCESS 1
#define BACKGROUND_CHECKADVANCING 2
#define INSTANCE_CHECKADVANCING 3
#define CHRACTERMANAGER_POINTER 4
#define INSTANCEBASE_MOVETODEST 5
#define MOVETODIRECTION_FUNCTION 6
#define PYTHONPLAYER_SENDUSESKILL 7
#define TRACEF_POINTER 8
#define TRACENF_POINTER 9
#define GLOBALTOLOCAL_FUNCTION 10
#define LOCALTOGLOBAL_FUNCTION 11
#define SEND_FUNCTION 13
#define SENDSEQUENCE_FUNCTION 14
#define GETETHER_FUNCTION 15
#define NETWORKCLASS_POINTER 16
#define PYTHONPLAYER_POINTER 17
#define SENDATTACK_FUNCTION 18
#define SENDSHOOT_FUNCTION 19
#define SENDCHARACTERSTATE_FUNCTION 20
#define RECV_FUNCTION 21
#define CHECK_PACKET_FUNCTION 22
#define PEEK_FUNCTION 23
#define RENDER_MID_FUNCTION 24


#define GLOBAL_PATTERN "\x55\x8b\xec\x83\xec\x00\x89\x4d\x00\xc6\x45\x00\x00\x8d\x45\x00\x50\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xc9\x75\x00\x32\xc0\xe9\x00\x00\x00\x00\x8b\x4d\x00\xe8\x00\x00\x00\x00\x0f\xb6\x00\x85\xd2\x75\x00\xb0\x00\xeb\x00\x8d\x45\x00\x89\x45\x00\x8b\x4d\x00\xc6\x01\x00\xba\x00\x00\x00\x00\x8b\x45\x00\x66\x89\x50\x00\x6a\x00\x6a\x00\x8d\x4d\x00\x51\xe8\x00\x00\x00\x00\x83\xc4\x00\xc6\x45\x00\x00\xc6\x45\x00\x00\x8b\x55"
#define GLOBAL_PATTERN_MASK "xxxxx?xx?xx??xx?xxx?x????xx?xxx?xxx????xx?x????xx?xxx?x?x?xx?xx?xx?xx?x????xx?xxx?x?x?xx?xx????xx?xx??xx??xx"
#define GLOBAL_PATTERN_OFFSET 0


typedef DWORD ClassPointer;
struct CMappedFile;

//Functions
typedef void(__cdecl* tTracef)(const char* c_szFormat, ...);
typedef bool(__thiscall* tRecvPacket)(ClassPointer classPointer, int size, void* buffer);
typedef bool(__thiscall* tSendPacket)(ClassPointer classPointer, int size, void* buffer);
typedef bool(__thiscall* tSendSequencePacket)(ClassPointer classPointer);
typedef bool(__thiscall* tSendStatePacket)(ClassPointer classPointer, fPoint& pos, float rot, BYTE eFunc, BYTE uArg);
typedef bool(__thiscall* tBackground_CheckAdvancing)(ClassPointer classPointer, void* instanceBase);
typedef bool(__thiscall* tInstanceBase_CheckAdvancing)(ClassPointer classPointer);
typedef bool(__thiscall* tMoveToDirection)(ClassPointer classPointer, float rot);
typedef bool(__thiscall* tMoveToDestPosition)(ClassPointer classPointer, fPoint& pos);
typedef void(__thiscall* tSendUseSkillBySlot)(ClassPointer classPointer, DWORD dwSkillSlotIndex, DWORD dwTargetVID);
typedef bool(__thiscall* tCheckPacket)(ClassPointer classPointer, BYTE* packetHeader);
typedef bool(__thiscall* tPeek)(ClassPointer classPointer, int len,void* buffer);

typedef bool(__thiscall* tGlobalToLocalPosition)(ClassPointer classPointer, long& lx, long& ly);
typedef bool(__thiscall* tLocalToGlobalPosition)(ClassPointer classPointer, LONG& rLocalX, LONG& rLocalY);
typedef bool(__thiscall* tSendAttackPacket)(ClassPointer classPointer, BYTE type, DWORD vid);

typedef void* (__thiscall* tGetInstancePointer)(ClassPointer classPointer, DWORD vid);
typedef bool (__thiscall* tProcess)(ClassPointer classPointer);

typedef bool(__thiscall* tGet)(ClassPointer classPointer, CMappedFile& rMappedFile, const char* c_szFileName, LPCVOID* pData);



#pragma pack(push, 1)
struct CMappedFile {
	void* v_table;
	//int				m_mode;
	//char			m_filename[MAX_PATH + 1];
	//HANDLE			m_hFile;
	BYTE		uknown[280];
	DWORD		m_dwSize;//0x11C needs fix

	BYTE* m_pbBufLinkData;
	DWORD		m_dwBufLinkSize;

	BYTE* m_pbAppendResultDataBlock;
	DWORD		m_dwAppendResultDataSize;

	DWORD		m_seekPosition;
	HANDLE		m_hFM;
	DWORD		m_dataOffset;
	DWORD		m_mapSize;
	LPVOID		m_lpMapData;
	LPVOID		m_lpData;

	void* m_pLZObj;
};
#pragma pack(pop)

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

struct InstanceLocalPosition {
	DWORD vid;
	float x, y;
};


enum {
	PHASE_LOADING = 4,
	PHASE_CHARACTER_CHOSE = 2,
	PHASE_GAME = 5
};

enum {
	CHAR_STATE_FUNC_STOP = 0,
	CHAR_STATE_FUNC_WALK = 1,
	CHAR_STATE_FUNC_ATTACK = 3,

	CHAR_STATE_ARG_HORSE_ATTACK1 = 14,
	CHAR_STATE_ARG_HORSE_ATTACK2 = 15,
	CHAR_STATE_ARG_HORSE_ATTACK3 = 16,

	CHAR_STATE_ARG_COMBO_ATTACK1 = 14,
	CHAR_STATE_ARG_COMBO_ATTACK2 = 15,
	CHAR_STATE_ARG_COMBO_ATTACK3 = 16,
	CHAR_STATE_ARG_COMBO_ATTACK4 = 17,

	COMBO_SKILL_ARCH = 0
};

enum {
	SUCESS_ON_FISHING = 3,
	UNSUCESS_ON_FISHING = 0
};

//Packet Headers
	enum {
		//FROM SERVER
		HEADER_GC_CHARACTER_ADD = 125,
		//HEADER_GC_MAIN_CHARACTER_MOVE = 11,
		HEADER_GC_CHARACTER_DEL = 84,
		HEADER_GC_DEAD = 124,
		HEADER_GC_PHASE = 255,
		HEADER_GC_CHARACTER_MOVE = 85,
		HEADER_GC_MAIN_CHARACTER = 75,
		HEADER_GC_CHAT = 0x56,
		HEADER_GC_FISHING = 42,
		HEADER_GC_ITEM_GROUND_ADD = 117,
		HEADER_GC_ITEM_GROUND_DEL = 129,
		HEADER_GC_SHOP_SIGN = 26,
		HEADER_CG_DIG_MOTION = 7,
		HEADER_GC_ITEM_OWNERSHIP = 98,
		HEADER_CG_SYNC_POSITION = 13,


		//TO SERVER
		HEADER_CG_ITEM_PICKUP = 20,
		HEADER_CG_SEND_LOGIN = 68,
		HEADER_CG_SEND_SEQUENCE = 0,
		HEADER_CG_SEND_CHAT = 3,
		HEADER_CG_CHARACTER_MOVE = 11,//19, 
		HEADER_CG_FISHING = 14,
		HEADER_CG_SHOOT = 4,
		HEADER_CG_USE_SKILL = 76,
		HEADER_CG_SELECT_TARGET = 82,
		HEADER_CG_BATTLE_ATTACK = 85,
		HEADER_CG_ADD_FLY_TARGETING = 77
	};



//Packet structs
#pragma pack(push, 1)
struct SRcvDeadPacket
{
	BYTE	header;
	DWORD	vid;
};

struct SRcv_GroundItemAddPacket {
	BYTE header;
	long x;
	long y;
	long z;

	DWORD VID;
	DWORD itemIndex;
};

struct SRcv_GroundItemDeletePacket {
	BYTE header;
	DWORD vid;
};

struct SSend_PickupPacket {
	BYTE header = HEADER_CG_ITEM_PICKUP;
	DWORD vid;
};

struct SRcv_DigMotionPacket
{
	BYTE header;
	DWORD vid;
	DWORD target_vid;
	BYTE count;
};

struct SSend_CharacterStatePacket
{
	BYTE		header;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
};

struct SRcv_RegisterShopPacket
{
	BYTE		header;
	DWORD		dwVID;
};

struct SSend_AddFlyTargetingPacket
{
	BYTE		header = HEADER_CG_ADD_FLY_TARGETING;
	DWORD		dwTargetVID;
	long		lX;
	long		lY;
};

struct SSend_ShootPacket
{
	BYTE		header = HEADER_CG_SHOOT;
	BYTE		type;
};

struct SSend_UseSkillPacket {
	BYTE		header = HEADER_CG_USE_SKILL;
	DWORD		dwSkillIndex;
	DWORD		vid;
};

struct SRcv_MainCharacterPacket
{
	BYTE		header;
	DWORD       dwVID;
	WORD		wRaceNum;
	char        szName[25];
	long        lX, lY, lZ;
	BYTE		bySkillGroup;
};

struct SSend_StartFishingPacket
{
	BYTE		header = HEADER_CG_FISHING;
	WORD		u1 = 6;   //6
	BYTE		u2 = 6;	  //6
	WORD		direction;
};

struct SSend_StopFishingPacket
{
	BYTE		header = HEADER_CG_FISHING;
	WORD        u1 = 12;	  //12
	BYTE		u2 = 8;   //8
	DWORD		type;	  //0 - unsuccess, 3 sucess
	float		timeLeft; //seconds
};

struct SSend_SyncPosition
{
	BYTE        header = HEADER_CG_SYNC_POSITION;
	WORD		wSize;
};

struct SSend_SyncPositionElement
{
	DWORD       dwVID;
	long        lX;
	long        lY;
};

struct SRecvHeaderFishingPacket
{
	BYTE		header = HEADER_CG_FISHING;
	BYTE        fishingHeader;
};


struct SRcv_ChangePhasePacket
{
	BYTE header;
	BYTE phase;
};

struct SRcv_CharacterMovePacket
{
	BYTE		header;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	DWORD		dwVID;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
	DWORD		dwDuration;
};

struct SRcv_ChatPacket
{
	BYTE	header = HEADER_GC_CHAT;
	WORD	size;
	BYTE	type;
	DWORD	dwVID;
	BYTE	bEmpire;
};

struct SRcv_DeletePlayerPacket
{
	BYTE	header;
	DWORD	dwVID;
};

struct SRcv_PacketOwnership
{
	BYTE        bHeader;
	DWORD       dwVID;
	char        szName[CHARACTER_NAME_MAX_LEN + 1];
};


struct SRcv_PlayerCreatePacket {

	BYTE	header;
	DWORD	dwVID;
	float	angle;
	long	x;
	long	y;
	long	z;
	DWORD	dwLevel;
	WORD	uknown;
	BYTE	bType;
	WORD	wRaceNum;
	BYTE	bMovingSpeed;
	BYTE	bAttackSpeed;
	BYTE	bStateFlag;
	DWORD	dwAffectFlag[2];
	/*DWORD dwVID;
	DWORD uknown1;
	DWORD uknown2;
	DWORD uknown3;
	CHAR uknown4;
	DWORD uknown5;
	DWORD uknown6;
	BYTE uknown7;
	BYTE uknown8;
	CHAR uknown9;
	CHAR uknown10;
	DWORD uknown11;
	DWORD uknown12;*/

};
#pragma pack(pop)





struct SInstance {
	DWORD	vid;
	BYTE	isDead;
	float	angle;
	long	x, y;
	BYTE	bType;
	WORD	wRaceNum;
	BYTE	bMovingSpeed;
	BYTE	bAttackSpeed;
	BYTE	bStateFlag;
};

struct SGroundItem {
	long x, y;
	DWORD index;
	DWORD vid;
	bool can_pick;
	std::string owner;
};