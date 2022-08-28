#pragma once
#include "Patterns.h"
#include "../MetinPythonLib/defines.h"
#include <map>


#define STR(x) #x


std::map<int, Pattern> memPatterns = {
	{PYTHONAPP_PROCESS,Pattern(STR(PYTHONAPP_PROCESS), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{BACKGROUND_CHECKADVANCING,Pattern(STR(BACKGROUND_CHECKADVANCING), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{INSTANCE_CHECKADVANCING,Pattern(STR(INSTANCE_CHECKADVANCING), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},

	//From CPythonCharacterManager::Instance()
	{CHRACTERMANAGER_POINTER,Pattern(STR(CHRACTERMANAGER_POINTER), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},

	//Search for string CPythonPlayer::__OnPressItem, go to caller of that functionand find the __OnPressGround function,and a reference will be inside
	{INSTANCEBASE_MOVETODEST,Pattern(STR(INSTANCEBASE_MOVETODEST), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{PYTHONPLAYER_SENDUSESKILL,Pattern(STR(PYTHONPLAYER_SENDUSESKILL), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{TRACENF_POINTER,Pattern(STR(TRACENF_POINTER), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{TRACEF_POINTER,Pattern(STR(TRACEF_POINTER), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{GLOBALTOLOCAL_FUNCTION,Pattern(STR(GLOBALTOLOCAL_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{LOCALTOGLOBAL_FUNCTION,Pattern(STR(LOCALTOGLOBAL_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},

	{MOVETODIRECTION_FUNCTION,Pattern(STR(MOVETODIRECTION_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	//SendExchangeStartPacket 
	//{GLOBAL_PATTERN,Pattern(STR(GLOBAL_PATTERN), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},

	{SEND_FUNCTION,Pattern(STR(SEND_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{SENDSEQUENCE_FUNCTION,Pattern(STR(SENDSEQUENCE_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	//{RECV_FUNCTION,Pattern(STR(RECV_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},

	//RELATIVE CPYTHONNETWORK
	{RECV_FUNCTION,Pattern(STR(RECV_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},

	{GETETHER_FUNCTION,Pattern(STR(GETETHER_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},

	//Pattern from Send On_Click Packet caller
	{NETWORKCLASS_POINTER,Pattern(STR(NETWORKCLASS_POINTER), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},

	//CPythonPlayer Gathered from playerRegisterCacheEffect
	{PYTHONPLAYER_POINTER,Pattern(STR(PYTHONPLAYER_POINTER), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{SENDATTACK_FUNCTION,Pattern(STR(SENDATTACK_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{SENDSHOOT_FUNCTION,Pattern(STR(SENDSHOOT_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{SENDCHARACTERSTATE_FUNCTION,Pattern(STR(SENDCHARACTERSTATE_FUNCTION), 0, "\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	{RENDER_MID_FUNCTION,Pattern(STR(RENDER_MID_FUNCTION),0,"\xe8\x00\x00\x00\x00\x0f","x??x?x")},

	//CPythonNetwork
	{CHECK_PACKET_FUNCTION,Pattern(STR(CHECK_PACKET_FUNCTION),0,"\xe8\x00\x00\x00\x00\x0f","x??x?x")},
	
	//CPythonNetwork - relative
	{PEEK_FUNCTION,Pattern(STR(PEEK_FUNCTION),0,"\xe8\x00\x00\x00\x00\x0f","x??x?x")},
};

