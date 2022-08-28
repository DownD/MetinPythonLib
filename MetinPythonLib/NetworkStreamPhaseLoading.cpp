#include "stdafx.h"
#include "NetworkStream.h"


bool CNetworkStream::RecvMainCharacter()
{
	SRcv_MainCharacterPacket m;
	if (peekNetworkStream(sizeof(SRcv_MainCharacterPacket), &m)) {
		DEBUG_INFO_LEVEL_2("MAIN VID: %d", m.dwVID);
		mainCharacterVID = m.dwVID;
	}
	else {
		DEBUG_INFO_LEVEL_2("Could not parse main character packet!");
	}
	return true;
}