#include "stdafx.h"
#include "NetworkStream.h"
#include "Memory.h"
#include "Player.h"
#include "InstanceManager.h"
#include "Background.h"

CNetworkStream::CNetworkStream() : lastPoint(0,0)
{
	m_blockAttackPackets = false;
	currentPhase = 0;
	lastPointIsStored = false;
	speed_Multiplier = 1.0;

	mainCharacterVID = 0;

	blockFishingPackets = 0;
	block_next_sequence = 0;

	recvDigMotionCallback = 0;
	shopRegisterCallback = 0;
	recvStartFishCallback = 0;
	chatCallback = 0;
	recvAddGrndItemCallback = 0;
	recvChangeOwnershipGrndItemCallback = 0;
	recvDelGrndItemCallback = 0;

	filterInboundOnlyIncluded = false;
	filterOutboundOnlyIncluded = false;
	printToConsole = false;

	netMod = 0;
}

CNetworkStream::~CNetworkStream()
{
}

void CNetworkStream::importPython()
{
	netMod = PyImport_ImportModule("net");

	if (!netMod) {
		DEBUG_INFO_LEVEL_1("Could not import net module! Maybe init.py was not executed");
	}
	if (netMod) {
		DEBUG_INFO_LEVEL_1("Net module loaded");
	}
}

bool CNetworkStream::SendBattlePacket(DWORD vid, BYTE type)
{
	DEBUG_INFO_LEVEL_5("Sending AttackPacket vid=%d",vid);
	CMemory& mem = CMemory::Instance();
	return mem.callSendAttackPacket(type, vid);
}

bool CNetworkStream::SendStatePacket(fPoint& pos, float rot, BYTE eFunc, BYTE uArg)
{
	CMemory& mem = CMemory::Instance();
	DEBUG_INFO_LEVEL_5("Sending StatePacket to x=%f,y=%f,rot=%f,eFunc=%d,uArg=%d", pos.x,pos.y,rot,eFunc,uArg);
	return mem.callSendStatePacket(pos, rot, eFunc, uArg);
}

bool CNetworkStream::SendPacket(int size, void* buffer)
{
	CMemory& mem = CMemory::Instance();
	return mem.callSendPacket(size, buffer);
}

bool CNetworkStream::SendSequencePacket()
{
	CMemory& mem = CMemory::Instance();
	return mem.callSendSequence();
}

bool CNetworkStream::SendAddFlyTargetingPacket(DWORD dwTargetVID, float x, float y)
{
	SSend_AddFlyTargetingPacket packet;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = (long)x;
	packet.lY = (long)y;
	LocalToGlobalPosition(packet.lX, packet.lY);

	if (SendPacket(sizeof(SSend_AddFlyTargetingPacket), &packet))
		return SendSequencePacket();
	return false;
}

bool CNetworkStream::SendShootPacket(BYTE uSkill)
{
	SSend_ShootPacket packet;
	packet.type = uSkill;

	if (SendPacket(sizeof(SSend_ShootPacket), &packet))
		return SendSequencePacket();
	return false;
}

bool CNetworkStream::SendStartFishing(WORD direction)
{
	SSend_StartFishingPacket packet;
	packet.direction = direction;
	if (SendPacket(sizeof(SSend_StartFishingPacket), &packet))
		return SendSequencePacket();

	return false;
}

bool CNetworkStream::SendStopFishing(BYTE type, float timeLeft)
{
	SSend_StopFishingPacket packet;
	packet.type = type;
	packet.timeLeft = timeLeft;
	if (SendPacket(sizeof(SSend_StopFishingPacket), &packet))
		return SendSequencePacket();

	return false;
}

bool CNetworkStream::SendPickupItemPacket(DWORD vid)
{
	SSend_PickupPacket packet;
	packet.vid = vid;
	DEBUG_INFO_LEVEL_4("Send Pickup Item vid=%d", vid);
	if (SendPacket(sizeof(SSend_PickupPacket), &packet))
		return SendSequencePacket();

	return false;
}

bool CNetworkStream::SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID)
{
	SSend_UseSkillPacket packet;
	packet.vid = dwTargetVID;
	packet.dwSkillIndex = dwSkillIndex;
	DEBUG_INFO_LEVEL_3("Sending Skill Packet index=%d, vid =%d", dwSkillIndex, dwTargetVID);
	if (SendPacket(sizeof(SSend_UseSkillPacket), &packet))
		return SendSequencePacket();

	return false;
}

void CNetworkStream::SendUseSkillBySlot(DWORD dwSkillSlotIndex, DWORD dwTargetVID)
{
	CMemory& mem = CMemory::Instance();
	return mem.callSendUseSkillBySlot(dwSkillSlotIndex,dwTargetVID);
}

bool CNetworkStream::SendSyncPacket(std::vector<InstanceLocalPosition>& targetPositions)
{
	SSend_SyncPosition kPacketSync;
	kPacketSync.wSize = sizeof(kPacketSync) + sizeof(SSend_SyncPositionElement) * targetPositions.size();

	DEBUG_INFO_LEVEL_3("Sending sync packet size=%d", kPacketSync.wSize);
	if (!SendPacket(sizeof(SSend_SyncPosition), &kPacketSync)) {
		DEBUG_INFO_LEVEL_2("Fail to send Sync Packet");
		return false;
	}
	for(InstanceLocalPosition & pos : targetPositions){
		SSend_SyncPositionElement kSyncPos;
		kSyncPos.dwVID = pos.vid;
		kSyncPos.lX = (long)pos.x;
		kSyncPos.lY = (long)pos.y;
		DEBUG_INFO_LEVEL_3("SendSyncPacketElement: Local vid=%d, x=%d, y=%d", kSyncPos.dwVID, kSyncPos.lX, kSyncPos.lY);
		if (!LocalToGlobalPosition(kSyncPos.lX, kSyncPos.lY)) {
			DEBUG_INFO_LEVEL_2("Fail to transform local to global on Sync Packet");
			return false;
		}
		DEBUG_INFO_LEVEL_3("SendSyncPacketElement: Global vid=%d, x=%d, y=%d", kSyncPos.dwVID, kSyncPos.lX, kSyncPos.lY);
		if (!SendPacket(sizeof(SSend_SyncPositionElement), &kSyncPos))
		{
			DEBUG_INFO_LEVEL_2("Fail to send Sync Element Packet");
			return false;
		}
	}

	return SendSequencePacket();
}


bool CNetworkStream::__RecvPacket(int size, void* buffer)
{
	CMemory& mem = CMemory::Instance();
	bool val = mem.callRecvPacket(size, buffer);
	if (size > 0 && printToConsole && val) {
		BYTE* byte_buffer = (BYTE*)buffer;
		if (byte_buffer[0] != 0) {
			printPacket(0, byte_buffer, size, INBOUND);

		}
	}

	return val;
}

bool CNetworkStream::__SendPacket(int size, void* buffer)
{
	BYTE header = getPacketHeader(buffer, size);
	DEBUG_INFO_LEVEL_4("Hook SendPacket called header = %d return size=%d", header, size);
	//PacketFilter
	if (size > 0 && printToConsole) {
		BYTE* byte_buffer = (BYTE*)buffer;
		if (byte_buffer[0] != 0) {
			printPacket(0, byte_buffer, size, OUTBOUND);

		}
	}

	CMemory& mem = CMemory::Instance();
	return mem.callSendPacket(size, buffer);
}

bool CNetworkStream::__SendStatePacket(fPoint& pos, float rot, BYTE eFunc, BYTE uArg)
{
	DEBUG_INFO_LEVEL_3("__SendStatePacket StartPos X->%f, Y->%f, EndPos X->%f, Y->%f", lastPoint.x, lastPoint.y, pos.x, pos.y);
	CMemory& mem = CMemory::Instance();

	if (eFunc == CHAR_STATE_FUNC_WALK && speed_Multiplier > 1) {
		if (pos.x < 0 || pos.y < 0) {
			return false;
		}
		if (!lastPointIsStored) {
			lastPoint = pos;
			lastPointIsStored = true;
		}
		else {
			//DEBUG_INFO_LEVEL_3("starting X->%f, Y->%f, EndPos X->%f, Y->%f", lastPoint.x, lastPoint.y, pos.x, pos.y);
			if (distance(lastPoint.x, lastPoint.y, pos.x, pos.y) > 1) {

				//DEBUG_INFO_LEVEL_3("BOSTING");
				fPoint currPosition = { 0,0 };
				CPlayer& player = CPlayer::Instance();
				if (player.getLastMovementType() == MOVE_POSITION)
					currPosition = getPointAtDistanceTimes(lastPoint.x, lastPoint.y, pos.x, pos.y, speed_Multiplier / 2);
				else
					currPosition = getPointAtDistanceTimes(lastPoint.x, lastPoint.y, pos.x, pos.y, speed_Multiplier);


				//Check if it is destination position and if it is in between the points
				if (player.getLastMovementType() == MOVE_POSITION) {

					fPoint destPos = player.getLastDestPosition();
					if (checkPointBetween(lastPoint.x, lastPoint.y, destPos.x, destPos.y, currPosition.x, currPosition.y)) {
						//DEBUG_INFO_LEVEL_3("CharacterState Point is in between");
						currPosition.x = destPos.x;
						currPosition.y = destPos.y;
					}
				}
				float dist = distance(lastPoint.x, lastPoint.y, currPosition.x, currPosition.y);
				int steps = dist / MAX_TELEPORT_DIST;


				//DEBUG_INFO_LEVEL_3("StartPos X->%f, Y->%f, EndPos X->%f, Y->%f, dist %f", lastPoint.x, lastPoint.y, currPosition.x, currPosition.y, dist);

				//Fix large movement speed
				for (int step = 0; step < steps; step++) {
					fPoint this_pos = getPointAtDistanceTimes(lastPoint.x, lastPoint.y, currPosition.x, currPosition.y, (float)(step + 1) / (float)(steps + 1));
					pos.x = this_pos.x;
					pos.y = this_pos.y;

					//DEBUG_INFO_LEVEL_3("CharStateBoostedSteps X->%f, Y->%f, eFunc %d, rot %f, uArg %d, step %d, dist %f steps %f", pos.x, pos.y, 0, rot, uArg, step, distance(lastPoint.x, lastPoint.y, pos.x, pos.y), (float)(step + 1) / (float)(steps + 1));

					mem.callSendStatePacket(pos, rot, CHAR_STATE_FUNC_STOP, 0);
				}


				pos.x = currPosition.x;
				pos.y = currPosition.y;
				//DEBUG_INFO_LEVEL_3("CharStateBoosted X->%f, Y->%f, eFunc %d, rot %f, uArg %d, dist %f", pos.x, pos.y, 0, rot, uArg, distance(lastPoint.x, lastPoint.y, pos.x, pos.y));

				mem.callSendStatePacket(pos, rot, CHAR_STATE_FUNC_STOP, 0);

				//Reload Mobs
				pos.x += 200;
				pos.y -= 200;
				mem.callSendStatePacket(pos, rot, eFunc, uArg);

				pos.x -= 200;
				pos.y += 200;
				mem.callSendStatePacket(pos, rot, eFunc, uArg);

				pos.x = currPosition.x;
				pos.y = currPosition.y;

				player.setPixelPosition(pos);
				lastPoint = pos;
			}
		}


	}
	else {
		lastPointIsStored = false;
	}
	DEBUG_INFO_LEVEL_3("End __SendStatePacket X->%f, Y->%f, eFunc %d, rot %f, uArg %d", pos.x, pos.y, eFunc, rot, uArg);
	return mem.callSendStatePacket(pos, rot, eFunc, uArg);
}

bool CNetworkStream::__CheckPacket(BYTE * header)
{
	CMemory& mem = CMemory::Instance();
	bool val = mem.callCheckPacket(header);
	if (!val || header == 0)
		return val;

	DEBUG_INFO_LEVEL_5("Hook CheckPacket header=%d", *header);

	switch (currentPhase) {
	case PHASE_GAME:
		val = RecvGamePhase(header);
		break;

	case PHASE_LOADING:
		val = RecvLoadingPhase(header);
		break;
	default: {
		switch (*header) {
		case HEADER_GC_PHASE: {
			SRcv_ChangePhasePacket phase;
			if (peekNetworkStream(sizeof(SRcv_ChangePhasePacket), &phase)) {
				if (phase.phase != 1 && phase.phase != 2)
					setPhase(phase);
			}
			else {
				DEBUG_INFO_LEVEL_2("Could not parse phase packet!");
			}
			break;
		}
		case HEADER_GC_CHARACTER_ADD: {
			SRcv_PlayerCreatePacket instance;
			if (peekNetworkStream(sizeof(SRcv_PlayerCreatePacket), &instance)) {
				GlobalToLocalPosition(instance.x, instance.y);
				CInstanceManager& mgr = CInstanceManager::Instance();
				mgr.appendNewInstance(instance);
			}
			else {
				DEBUG_INFO_LEVEL_2("Could not parse character add packet!");
			}
			break;
		}
		}

		}
	}

	return val;
}

bool CNetworkStream::__SendAttackPacket(BYTE type, DWORD vid)
{
	DEBUG_INFO_LEVEL_5("__SendAttackPacket called ");
	CMemory& mem = CMemory::Instance();
	if (m_blockAttackPackets)
		return true;
	else
		return mem.callSendAttackPacket(type, vid);
}

bool CNetworkStream::__SendSequencePacket()
{
	DEBUG_INFO_LEVEL_5("__SendSequence called ");
	if (block_next_sequence) {
		block_next_sequence = 0;
		DEBUG_INFO_LEVEL_3("Hook SendSequence called return val=%d", 1);
		return 1;
	}
	CMemory& mem = CMemory::Instance();
	return mem.callSendSequence();
}


int CNetworkStream::GetCurrentPhase()
{
    return currentPhase;
}

DWORD CNetworkStream::GetMainCharacterVID()
{
	if (netMod == 0) {
		DEBUG_INFO_LEVEL_3("Net Module has not been loaded!");
	}
	//return mainCharacterVID;
	PyObject* poArgs = Py_BuildValue("()");
	long ret = 0;

	if (netMod == 0) {
		return 0;
	}

	if (PyCallClassMemberFunc(netMod, "GetMainActorVID", poArgs, &ret)) {
		Py_DECREF(poArgs);
		return ret;
	}

	Py_DECREF(poArgs);
	return ret;
}

bool CNetworkStream::GlobalToLocalPosition(long& lx, long& ly)
{
	CMemory& mem = CMemory::Instance();
	return mem.callGlobalToLocalPosition(lx,ly);
}

bool CNetworkStream::LocalToGlobalPosition(LONG& rLocalX, LONG& rLocalY)
{
	CMemory& mem = CMemory::Instance();
	return mem.callLocalToGlobalPosition(rLocalX, rLocalY);
}

void CNetworkStream::callDigMotionCallback(DWORD target_player, DWORD target_vein, DWORD n)
{
	//call python callback
	DEBUG_INFO_LEVEL_3("Mining packet recived");
	if (recvDigMotionCallback && PyCallable_Check(recvDigMotionCallback)) {
		DEBUG_INFO_LEVEL_3("Calling python DigMotionCallback");
		PyObject* val = Py_BuildValue("(iii)", target_player, target_vein, n);
		PyObject* result = PyObject_CallObject(recvDigMotionCallback, val);
		Py_DECREF(val);
		Py_XDECREF(result);
	}
}

void CNetworkStream::callStartFishCallback()
{
	//call python callback
	DEBUG_INFO_LEVEL_3("Start Fish recived");
	if (recvStartFishCallback && PyCallable_Check(recvStartFishCallback)) {
		DEBUG_INFO_LEVEL_3("Calling python StartFishCallback");
		PyObject* val = Py_BuildValue("()");
		PyObject* result = PyObject_CallObject(recvStartFishCallback, val);
		Py_DECREF(val);
		Py_XDECREF(result);
	}
}

void CNetworkStream::callRecvAddGrndItemCallback(DWORD vid, DWORD index, long x, long y, std::string owner)
{
	if (recvAddGrndItemCallback) {
		if (PyCallable_Check(recvAddGrndItemCallback)) {
			DEBUG_INFO_LEVEL_3("Calling python recvAddGrndItemCallback vid=%d,index=%d,x=%d,y=%d,owner=%s",vid, index, x,y,owner.data());
			PyObject* val = Py_BuildValue("(iiiis)", vid, index, x, y, owner.data());
			PyObject* result = PyObject_CallObject(recvAddGrndItemCallback, val);
			Py_DECREF(val);
			Py_XDECREF(result);
		}
	}
}

void CNetworkStream::callRecvChangeOwnershipGrndItemCallback(DWORD vid, std::string owner)
{
	if (recvChangeOwnershipGrndItemCallback && PyCallable_Check(recvChangeOwnershipGrndItemCallback)) {
		DEBUG_INFO_LEVEL_3("Calling python recvChangeOwnershipGrndItemCallback vid=%d,owner=%s", vid, owner.data());
		PyObject* val = Py_BuildValue("(is)", vid, owner.data());
		PyObject* result = PyObject_CallObject(recvChangeOwnershipGrndItemCallback, val);
		Py_DECREF(val);
		Py_XDECREF(result);
	}
}

void CNetworkStream::callRecvDelGrndItemCallback(DWORD vid)
{
	if (recvDelGrndItemCallback && PyCallable_Check(recvDelGrndItemCallback)) {
		DEBUG_INFO_LEVEL_3("Calling python recvChangeOwnershipGrndItemCallback vid=%d", vid);
		PyObject* val = Py_BuildValue("(i)", vid);
		PyObject* result = PyObject_CallObject(recvDelGrndItemCallback, val);
		Py_DECREF(val);
		Py_XDECREF(result);
	}
}

//THIS FUNCTION WILL CRASH IF IT HAS SELF ARGUMENT
bool CNetworkStream::setDigMotionCallback(PyObject* func)
{
	if (PyCallable_Check(func)) {
		DEBUG_INFO_LEVEL_2("RecvDigMotionCallback function set sucessfully");
		if(recvDigMotionCallback)
			Py_XDECREF(recvDigMotionCallback);
		recvDigMotionCallback = func;
		return true;
	}
	else {
		DEBUG_INFO_LEVEL_1("RegisterNewDigMotionCallback argument is not a function");
		recvDigMotionCallback = 0;
		return false;
	}
}

void CNetworkStream::SetSpeedMultiplier(float val)
{
	speed_Multiplier = val;
}

void CNetworkStream::SetFishingPacketsBlock(bool val)
{
	blockFishingPackets = val;
}

bool CNetworkStream::setStartFishCallback(PyObject* func)
{
	if (PyCallable_Check(func)) {
		DEBUG_INFO_LEVEL_2("StartFishCallback function set sucessfully");
		if (recvStartFishCallback)
			Py_DECREF(recvStartFishCallback);
		recvStartFishCallback = func;
		return true;
	}
	else {
		DEBUG_INFO_LEVEL_1("RegisterStartFishCallback argument is not a function");
		recvStartFishCallback = 0;
		return false;
	}
}

bool CNetworkStream::setNewShopCallback(PyObject* func)
{
	if (!PyCallable_Check(func)) {
		DEBUG_INFO_LEVEL_1("RegisterNewShopCallback argument is not a function");
		return false;
	}

	if (shopRegisterCallback)
		Py_DECREF(shopRegisterCallback);
	shopRegisterCallback = func;

	DEBUG_INFO_LEVEL_2("RegisterNewShopCallback function set sucessfully");

	return true;
}

void CNetworkStream::callNewInstanceShop(DWORD player)
{
	if (shopRegisterCallback && PyCallable_Check(shopRegisterCallback)) {
		DEBUG_INFO_LEVEL_3("Calling python RegisterShopCallback");
		PyObject* val = Py_BuildValue("(i)", player);
		PyObject* result = PyObject_CallObject(shopRegisterCallback, val);
		Py_DECREF(val);
		Py_DECREF(result);
	}
}

bool CNetworkStream::setChatCallback(PyObject* func)
{
	if (!PyCallable_Check(func)) {
		DEBUG_INFO_LEVEL_1("ChatCallback argument is not a function");
		return false;
	}

	if (chatCallback)
		Py_DECREF(chatCallback);
	chatCallback = func;

	DEBUG_INFO_LEVEL_2("ChatCallback function set sucessfully");

	return true;
}

void CNetworkStream::callRecvChatCallback(DWORD vid, const char* msg, BYTE type, BYTE empire, const char* locale)
{
	if (chatCallback) {
		if (PyCallable_Check(chatCallback)) {
			DEBUG_INFO_LEVEL_3("Calling python chatCallback message=%s,locale=%s", msg, locale);
			PyObject* val = Py_BuildValue("iiiss", vid, type, empire, msg, locale);
			PyObject* result = PyObject_CallObject(chatCallback, val);
			Py_DECREF(val);
			Py_DECREF(result);
		}
		else {
			Py_XDECREF(chatCallback);
			chatCallback = 0;
		}
	}
}

void CNetworkStream::openConsole()
{
	filterInboundOnlyIncluded = false;
	filterOutboundOnlyIncluded = false;
	clearPacketFilter(OUTBOUND);
	clearPacketFilter(INBOUND);
	AllocConsole();
#ifdef _DEBUG
	setDebugOff();
	return;
#endif
	system("cls");
	SetConsoleTitle("PACKET SNIFFER");
}

void CNetworkStream::closeConsole()
{
	filterInboundOnlyIncluded = false;
	filterOutboundOnlyIncluded = false;
	stopFilterPacket();
	clearPacketFilter(OUTBOUND);
	clearPacketFilter(INBOUND);
#ifdef _DEBUG
	setDebugOn();
	return;
#endif
	FreeConsole();
}

void CNetworkStream::startFilterPacket()
{
	printToConsole = true;
}

void CNetworkStream::stopFilterPacket()
{
	printToConsole = false;
}

void CNetworkStream::removeHeaderFilter(BYTE header, PACKET_TYPE t)
{
	if (t == INBOUND)
		inbound_header_filter.erase(header);
	else
		outbound_header_filter.erase(header);
}

void CNetworkStream::addHeaderFilter(BYTE header, PACKET_TYPE t)
{
	if (t == INBOUND)
		inbound_header_filter.insert(header);
	else
		outbound_header_filter.insert(header);
}

void CNetworkStream::clearPacketFilter(PACKET_TYPE t)
{
	if (t == INBOUND)
		inbound_header_filter.clear();
	else
		outbound_header_filter.clear();
}

void CNetworkStream::setFilterMode(PACKET_TYPE t, bool val)
{
	if (t == INBOUND)
		filterInboundOnlyIncluded = val;
	else
		filterOutboundOnlyIncluded = val;
}

void CNetworkStream::printPacket(DWORD calling_function, BYTE* buffer, int size, bool type)
{
	if (size < 1) {
		return;
	}

	if (outbound_header_filter.find(buffer[0]) == outbound_header_filter.end() && filterOutboundOnlyIncluded && type == OUTBOUND) {
		return;
	}
	if (inbound_header_filter.find(buffer[0]) == inbound_header_filter.end() && filterInboundOnlyIncluded && type == INBOUND) {
		return;
	}

	if (outbound_header_filter.find(buffer[0]) != outbound_header_filter.end() && !filterOutboundOnlyIncluded && type == OUTBOUND) {
		return;
	}
	if (inbound_header_filter.find(buffer[0]) != inbound_header_filter.end() && !filterInboundOnlyIncluded && type == INBOUND) {
		return;
	}



	if (INBOUND == type) {
		printf("[INBOUND]\n");
	}
	else {
		printf("[OUTBOUND]\n");
	}
	printf("Header: %d\n", buffer[0]);
	printf("Size: %d\n", size);
	printf("Calling Address: %#x\n", calling_function);
	printf("Content-Bytes: ");
	for (int i = 1; i < size; i++) {
		printf("%#x ", buffer[i]);
	}
	printf("\n\n");
	fflush(stdout);
}

std::set<BYTE>* CNetworkStream::getPacketFilter(PACKET_TYPE t)
{
	if (t == INBOUND)
		return &inbound_header_filter;
	else
		return &outbound_header_filter;
}

bool CNetworkStream::setRecvAddGrndItemCallback(PyObject* func)
{
	if (!PyCallable_Check(func)) {
		DEBUG_INFO_LEVEL_1("RecvAddGrndItemCallback argument is not a function");
		return false;
	}

	if (recvAddGrndItemCallback)
		Py_DECREF(recvAddGrndItemCallback);
	recvAddGrndItemCallback = func;

	DEBUG_INFO_LEVEL_2("RecvAddGrndItemCallback function set sucessfully");

	return true;
}

bool CNetworkStream::setRecvChangeOwnershipGrndItemCallback(PyObject* func)
{
	if (!PyCallable_Check(func)) {
		DEBUG_INFO_LEVEL_1("RecvChangeOwnershipGrndItemCallback argument is not a function");
		return false;
	}

	if (recvChangeOwnershipGrndItemCallback)
		Py_DECREF(recvChangeOwnershipGrndItemCallback);
	recvChangeOwnershipGrndItemCallback = func;

	DEBUG_INFO_LEVEL_2("RecvChangeOwnershipGrndItemCallback function set sucessfully");

	return true;
}

bool CNetworkStream::setRecvDelGrndItemCallback(PyObject* func)
{
	if (!PyCallable_Check(func)) {
		DEBUG_INFO_LEVEL_1("RecvDelGrndItemCallback argument is not a function");
		return false;
	}

	if (recvDelGrndItemCallback)
		Py_DECREF(recvDelGrndItemCallback);
	recvDelGrndItemCallback = func;

	DEBUG_INFO_LEVEL_2("RecvDelGrndItemCallback function set sucessfully");

	return true;
}

void CNetworkStream::blockAttackPackets()
{
	m_blockAttackPackets = true;
}

void CNetworkStream::unblockAttackPackets()
{
	m_blockAttackPackets = false;
}

void CNetworkStream::setPhase(SRcv_ChangePhasePacket& phase) {
	if (phase.phase > 5)
		return;
	//printf("Phase Packet %d\n",phase.phase);
	//if (phase.phase == 1 || phase.phase == 10 || phase.phase == 2) //The server sends 2 strange packets values(1,10), that disrupt the fase check
	//	return;
	currentPhase = phase.phase;
	lastPointIsStored = false;
	DEBUG_INFO_LEVEL_1("Phased changed to: %d", currentPhase);
	CBackground& background = CBackground::Instance();
	CInstanceManager& mgr = CInstanceManager::Instance();
	//CPlayer& player = CPlayer::Instance
	
	if (phase.phase == PHASE_LOADING) {
		mgr.clearInstances();
		background.freeCurrentMap();
	}
	else if (phase.phase == PHASE_GAME) {
		background.setCurrentCollisionMap();
	}
}

void CNetworkStream::handleChatPacket(SRcv_ChatPacket& packet)
{
	if (packet.size > 1025) {
		DEBUG_INFO_LEVEL_1("Error chat packet recived is too large dwSize=%d",packet.size);
		return;
	}
	int startChatMsgIndex = sizeof(SRcv_ChatPacket);
	char line[1024 + 1 + sizeof(SRcv_ChatPacket)] = { 0 };
	char* locale = (char*)((int)line + sizeof(SRcv_ChatPacket) + 1);
	char* msg = (char*)((int)line + sizeof(SRcv_ChatPacket) + 4);
	if (!peekNetworkStream(packet.size, line)) {
		DEBUG_INFO_LEVEL_2("Could not parse chat message packet!");
	}

	callRecvChatCallback(packet.dwVID, msg, packet.type, packet.bEmpire,locale);

}


bool CNetworkStream::peekNetworkStream(int len, void* buffer)
{
	CMemory& mem = CMemory::Instance();
	return mem.callPeek(len, buffer);
}

int CNetworkStream::getFishingPacketSize(BYTE header, bool isRecive)
{
	switch (header) {
		case 0x0:
		case 0x1:
		case 0x3:
		case 0x4:
		case 0x5:
			return 8;
		case 0x2:
			return 0x14;
	}
	DEBUG_INFO_LEVEL_3("Fishing header unknown: %d", header);
	return 0;
}


BYTE CNetworkStream::getPacketHeader(void* buffer, int size)
{
	if (size >= 1) {
		return *(BYTE*)buffer;
	}
	return 0;
}

bool CNetworkStream::RecvGamePhase(BYTE* header)
{
	switch (*header) {
	case HEADER_GC_FISHING:
		return RecvFishPacket();

	case HEADER_GC_CHAT:
		return RecvChatPacket();

	case HEADER_GC_ITEM_GROUND_ADD:
		return RecvAddItemGrnd();

	case HEADER_GC_ITEM_GROUND_DEL:
		return RecvDelItemGrnd();

	case HEADER_GC_CHARACTER_ADD:
		return RecvAddCharacter();

	case HEADER_GC_CHARACTER_MOVE:
		return RecvMoveCharacter();

	case HEADER_GC_CHARACTER_DEL:
		return RecvDelCharacter();

	case HEADER_GC_ITEM_OWNERSHIP:
		return RecvChangeOwnership();

	case HEADER_GC_PHASE:
		return RecvPhaseChange();

	case HEADER_CG_DIG_MOTION:
		return RecvDigMotion();

	case HEADER_GC_DEAD:
		return RecvDeadPacket();
	}
	
	return true;
}


bool CNetworkStream::RecvLoadingPhase( BYTE* header)
{
	switch (*header) {
	case HEADER_GC_CHARACTER_ADD:
		return RecvAddCharacter();

	case HEADER_GC_MAIN_CHARACTER:
		return RecvMainCharacter();

	case HEADER_GC_ITEM_OWNERSHIP:
		return RecvChangeOwnership();

	case HEADER_GC_ITEM_GROUND_ADD:
		return RecvAddItemGrnd();

	case HEADER_GC_PHASE:
		return RecvPhaseChange();
	}
	

	return true;
}
