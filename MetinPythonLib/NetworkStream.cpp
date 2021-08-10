#include "NetworkStream.h"
#include "Memory.h"
#include "Player.h"
#include "InstanceManager.h"
#include "Background.h"

CNetworkStream::CNetworkStream() : lastPoint(0,0)
{
	currentPhase = 0;
	lastPointIsStored = false;
	speed_Multiplier = 1.0;

	mainCharacterVID = 0;

	blockFishingPackets = 0;
	block_next_sequence = 0;

	recvDigMotionCallback = 0;
	shopRegisterCallback = 0;

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
	CMemory& mem = CMemory::Instance();
	return mem.callSendAttackPacket(type, vid);
}

bool CNetworkStream::SendStatePacket(fPoint& pos, float rot, BYTE eFunc, BYTE uArg)
{
	CMemory& mem = CMemory::Instance();
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


bool CNetworkStream::__RecvPacket(DWORD returnFunction, bool return_value, int size, void* buffer)
{
	BYTE header = getPacketHeader(buffer, size);
	if (return_value != 0 && size > 0 && header != 0) {

		DEBUG_INFO_LEVEL_5("Hook RecvPacket header=%d", header);

		//PacketFilter
		if (printToConsole) {
			if (inbound_header_filter.find(header) == inbound_header_filter.end()) {
				if (!filterInboundOnlyIncluded)
					printPacket(returnFunction, (BYTE*)buffer,size, INBOUND);
			}
			else {
				if (filterInboundOnlyIncluded)
					printPacket(returnFunction, (BYTE*)buffer, size, INBOUND);
			}
		}


		switch (currentPhase) {
		case PHASE_GAME:
			return_value = RecvGamePhase(buffer, size,header);
			break;

		case PHASE_LOADING:
			return_value = RecvLoadingPhase(buffer, size,header);
			break;
		default: {
			switch (header) {
			case HEADER_GC_PHASE: {
				SRcv_ChangePhasePacket phase;
				if(fillPacket(buffer,size, &phase)>=2)
					setPhase(phase);
				break;
			}
			case HEADER_GC_CHARACTER_ADD: {
				SRcv_PlayerCreatePacket instance;
				fillPacket(buffer, size, &instance);
				GlobalToLocalPosition(instance.x, instance.y);
				CInstanceManager& mgr = CInstanceManager::Instance();
				mgr.appendNewInstance(instance);
				break;
			}
			}
		}

		}
	}

	return return_value;
}

bool CNetworkStream::__SendPacket(int size, void* buffer)
{
	BYTE header = getPacketHeader(buffer, size);
	DEBUG_INFO_LEVEL_5("Hook SendPacket called header = %d return size=%d", header, size);
	//PacketFilter
	if (printToConsole) {
		if (outbound_header_filter.find(header) == outbound_header_filter.end()) {
			if (!filterOutboundOnlyIncluded)
				printPacket(0, (BYTE*)buffer, size, OUTBOUND);
		}
		else {
			if (filterOutboundOnlyIncluded)
				printPacket(0, (BYTE*)buffer, size, OUTBOUND);
		}
	}

	switch (header) {
	case HEADER_CG_FISHING: { //SEQUENCE IS NOT SENT
		if (blockFishingPackets) {
			DEBUG_INFO_LEVEL_2("Blocking Fishing Packet");
			block_next_sequence = 1; //Also block sequence packet
			return 1;			//Block fishing packets
		}
		break;
	}
	}

	CMemory& mem = CMemory::Instance();
	return mem.callSendPacket(size, buffer);
}

bool CNetworkStream::__SendStatePacket(fPoint& pos, float rot, BYTE eFunc, BYTE uArg)
{
	DEBUG_INFO_LEVEL_3("__SendStatePacket StartPos X->%f, Y->%f, EndPos X->%f, Y->%f", lastPoint.x, lastPoint.y, pos.x, pos.y);
	CMemory& mem = CMemory::Instance();
	if (pos.x < 0 || pos.y < 0) {
		mem.callSendStatePacket(pos, rot, eFunc, uArg);
	}

	if (eFunc == CHAR_STATE_FUNC_WALK && speed_Multiplier > 1) {
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

bool CNetworkStream::__CheckPacket(BYTE& header)
{
	CMemory& mem = CMemory::Instance();
	bool val = mem.callCheckPacket(header);
	if (!val)
		return false;

	return val;
}

bool CNetworkStream::__SendSequencePacket()
{
	DEBUG_INFO_LEVEL_3("__SendSequence called ");
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
		PyObject_CallObject(recvDigMotionCallback, val);
		Py_DECREF(val);
	}
}

//THIS FUNCTION WILL CRASH IF IT HAS SELF ARGUMENT
bool CNetworkStream::setDigMotionCallback(PyObject* func)
{
	if (PyCallable_Check(func)) {
		DEBUG_INFO_LEVEL_2("RecvDigMotionCallback function set sucessfully");
		if(recvDigMotionCallback)
			Py_DECREF(recvDigMotionCallback);
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
		PyObject_CallObject(shopRegisterCallback, val);
		Py_XDECREF(val);
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
}

std::set<BYTE>* CNetworkStream::getPacketFilter(PACKET_TYPE t)
{
	if (t == INBOUND)
		return &inbound_header_filter;
	else
		return &outbound_header_filter;
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
	//CPlayer& player = CPlayer::Instance();
	
	if (phase.phase == PHASE_LOADING) {
		mgr.clearInstances();
		background.freeCurrentMap();
	}
	else if (phase.phase == PHASE_GAME) {
		background.setCurrentCollisionMap();
	}
}


bool CNetworkStream::interceptPackets(BYTE header, void* buffer, int size)
{
	/*BYTE header = getPacketHeader(buffer, size);
	if (return_value != 0 && size > 0 && header != 0) {

		DEBUG_INFO_LEVEL_5("Hook RecvPacket header=%d", header);

		//PacketFilter
		if (printToConsole) {
			if (inbound_header_filter.find(header) == inbound_header_filter.end()) {
				if (!filterInboundOnlyIncluded)
					printPacket(returnFunction, (BYTE*)buffer, size, INBOUND);
			}
			else {
				if (filterInboundOnlyIncluded)
					printPacket(returnFunction, (BYTE*)buffer, size, INBOUND);
			}
		}*/
	bool return_value = 0;
	switch (currentPhase) {
	case PHASE_GAME:
		return_value = RecvGamePhase(buffer, size, header);
		break;

	case PHASE_LOADING:
		return_value = RecvLoadingPhase(buffer, size, header);
		break;
	default: {
		switch (header) {
		case HEADER_GC_PHASE: {
			SRcv_ChangePhasePacket phase;
			if (fillPacket(buffer, size, &phase) >= 2)
				setPhase(phase);
			break;
		}
		case HEADER_GC_CHARACTER_ADD: {
			SRcv_PlayerCreatePacket instance;
			fillPacket(buffer, size, &instance);
			GlobalToLocalPosition(instance.x, instance.y);
			CInstanceManager& mgr = CInstanceManager::Instance();
			mgr.appendNewInstance(instance);
			break;
		}
		}
	}
		switch (currentPhase) {
		case PHASE_GAME:
			return_value = RecvGamePhase(buffer, size, header);
			break;

		case PHASE_LOADING:
			return_value = RecvLoadingPhase(buffer, size, header);
			break;
		default: {
			switch (header) {
			case HEADER_GC_PHASE: {
				SRcv_ChangePhasePacket phase;
				if (fillPacket(buffer, size, &phase) >= 2)
					setPhase(phase);
				break;
			}
			case HEADER_GC_CHARACTER_ADD: {
				SRcv_PlayerCreatePacket instance;
				fillPacket(buffer, size, &instance);
				GlobalToLocalPosition(instance.x, instance.y);
				CInstanceManager& mgr = CInstanceManager::Instance();
				mgr.appendNewInstance(instance);
				break;
			}
			}
		}

		}
	}

	return return_value;
}

BYTE CNetworkStream::getPacketHeader(void* buffer, int size)
{
	if (size >= 1) {
		return *(BYTE*)buffer;
	}
	return 0;
}

bool CNetworkStream::RecvGamePhase(void* buffer, int size, BYTE header)
{
	switch (header) {
	case HEADER_GC_FISHING: {
		if (blockFishingPackets) {
			if (size >= 2 && (((BYTE*)buffer)[1] == 0x0 || ((BYTE*)buffer)[1] == 0x4)) { //This is for refresh the inventory window
				return false;
			}
		}
		break;
	}
	case HEADER_GC_ITEM_GROUND_ADD: {
		SRcv_GroundItemAddPacket instance;
		fillPacket(buffer,size, &instance);
		GlobalToLocalPosition(instance.x, instance.y);
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.addItemGround(instance);
		break;
	}
	case HEADER_GC_ITEM_GROUND_DEL: {
		SRcv_GroundItemDeletePacket instance;
		fillPacket(buffer, size, &instance);
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.delItemGround(instance);
		break;
	}
	case HEADER_GC_CHARACTER_ADD: {
		SRcv_PlayerCreatePacket instance;
		fillPacket(buffer, size, &instance);
		GlobalToLocalPosition(instance.x, instance.y);
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.appendNewInstance(instance);
		break;
	}
	case HEADER_GC_CHARACTER_MOVE: {

		SRcv_CharacterMovePacket instance;
		fillPacket(buffer, size, &instance);
		GlobalToLocalPosition(instance.lX, instance.lY);
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.changeInstancePosition(instance);
		break;
	}
	case HEADER_GC_CHARACTER_DEL: {

		SRcv_PlayerCreatePacket instance_;
		fillPacket(buffer, size, &instance_);
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.deleteInstance(instance_.dwVID);
		break;
	}
	case HEADER_GC_PHASE: {
		SRcv_ChangePhasePacket phase;
		fillPacket(buffer, size, &phase);
		if (phase.phase != 1 && phase.phase != 2)
			setPhase(phase);
		break;
	}
	case HEADER_CG_DIG_MOTION: {
		SRcv_DigMotionPacket instance_;
		fillPacket(buffer, size, &instance_);
		callDigMotionCallback(instance_.vid, instance_.target_vid, instance_.count);
		break;
	}

	case HEADER_GC_DEAD: {
		SRcvDeadPacket dead;
		fillPacket(buffer, size, &dead);
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.changeInstanceIsDead(dead.vid, 1);
		break;
	}
	}

	return true;
}

bool CNetworkStream::RecvLoadingPhase(void* buffer, int size, BYTE header)
{
	switch (header) {
	case HEADER_GC_CHARACTER_ADD: {
		SRcv_PlayerCreatePacket instance;
		fillPacket(buffer, size, &instance);
		GlobalToLocalPosition(instance.x, instance.y);
		CInstanceManager& mgr = CInstanceManager::Instance();
		mgr.appendNewInstance(instance);
		break;
	}
	case HEADER_GC_MAIN_CHARACTER: {
		SRcv_MainCharacterPacket m;
		fillPacket(buffer, size, &m);
		DEBUG_INFO_LEVEL_2("MAIN VID: %d", m.dwVID);
		mainCharacterVID = m.dwVID;
		break;
	}
	case HEADER_GC_PHASE: {
		SRcv_ChangePhasePacket phase;
		fillPacket(buffer, size, &phase);
		if (phase.phase != 1 && phase.phase != 2)
			setPhase(phase);
		break;
	}
	}

	return true;
}
