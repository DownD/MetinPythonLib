#include "stdafx.h"
#include "PythonModule.h"
#include "App.h"
#include "NetworkStream.h"
#include "InstanceManager.h"
#include "Background.h"
#include "Player.h"
#include "Communication.h"

/* PyObject* playerModule;
PyObject* getMainPlayerPosition;*/

PyObject* packet_mod;





PyObject* GetPixelPosition(PyObject* poSelf, PyObject* poArgs)
{
	int vid = 0;

	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	fPoint3D pos = { 0 };
	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.getCharacterPosition(vid, &pos);
	return Py_BuildValue("fff", (float)pos.x, (float)pos.y, (float)pos.z);
}

PyObject* moveToDestPosition(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	float x, y;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();
	if (!PyTuple_GetFloat(poArgs, 1, &x))
		return Py_BuildException();
	if (!PyTuple_GetFloat(poArgs, 2, &y))
		return Py_BuildException();

	fPoint pos(x, y);

	CPlayer& player = CPlayer::Instance();
	player.moveToDestPosition(vid, pos);
	return Py_BuildNone();
}



PyObject* pySetMoveSpeed(PyObject* poSelf, PyObject* poArgs)
{
	float speed;
	if (!PyTuple_GetFloat(poArgs, 0, &speed))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	net.SetSpeedMultiplier(speed);
	return Py_BuildNone();
}


PyObject* GetEterPacket(PyObject* poSelf, PyObject* poArgs) {
	CPlayer& player = CPlayer::Instance();
	return player.GetEterPacket(poSelf, poArgs);
}

PyObject* IsPositionBlocked(PyObject* poSelf, PyObject* poArgs)
{
	int x, y;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	x /= 100;
	y /= 100;
	CBackground& bck = CBackground::Instance();

	return Py_BuildValue("i", bck.isBlockedPosition(x, y));
}

PyObject* pyIsPathBlocked(PyObject* poSelf, PyObject* poArgs)
{
	int x1, y1,x2,y2;
	if (!PyTuple_GetInteger(poArgs, 0, &x1))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &y1))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 2, &x2))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 3, &y2))
		return Py_BuildException();

	CBackground& bck = CBackground::Instance();

	return Py_BuildValue("i", bck.isPathBlocked(x1, y1,x2,y2));
}


PyObject* FindPath(PyObject* poSelf, PyObject* poArgs)
{
	int x_start, y_start, x_end, y_end;
	if (!PyTuple_GetInteger(poArgs, 0, &x_start))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &y_start))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 2, &x_end))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 3, &y_end))
		return Py_BuildException();

	x_start /= 100;
	y_start /= 100;
	x_end /= 100;
	y_end /= 100;

	int x_start_unblocked = -1;
	int y_start_unblocked = -1;

	CBackground& bck = CBackground::Instance();
	if (bck.isBlockedPosition(x_start, y_start)) {
		Point a = { 0,0 };
		if (bck.getClosestUnblocked(x_start, y_start, &a)) {
			x_start_unblocked = a.x;
			y_start_unblocked = a.y;
			DEBUG_INFO_LEVEL_3("[PATH-FIDING] Start Position blocked, new position X:%d  Y:%d", x_start_unblocked, y_start_unblocked);
		}
		else {
			DEBUG_INFO_LEVEL_3("[PATH-FIDING] Cannot get closest unblocked position");
			return PyList_New(0);
		}
	}

	if (bck.isBlockedPosition(x_end, y_end)) {
		Point a = { 0,0 };
		if (bck.getClosestUnblocked(x_end, y_end, &a)) {
			x_end = a.x;
			y_end = a.y;
			DEBUG_INFO_LEVEL_3("[PATH-FIDING] End Position blocked, new position X:%d  Y:%d", x_end, y_end);
		}
		else {
			DEBUG_INFO_LEVEL_3("[PATH-FIDING] Cannot get closest unblocked position");
			return PyList_New(0);
		}
	}



	std::vector<Point> path;
	PyObject* pList = PyList_New(0);
	bool val = false;
	if (x_start_unblocked != -1) {
		val = bck.findPath(x_start_unblocked, y_start_unblocked, x_end, y_end, path);
		if (val)
			PyList_Append(pList, Py_BuildValue("ii", x_start_unblocked * 100, y_start_unblocked * 100));
		else
			return pList;
	}
	else {
		val = bck.findPath(x_start, y_start, x_end, y_end, path);
	}


	if (!val) {
		return pList;
	}
	int i = 0;
	for (Point& p : path) {
		PyObject* obj = Py_BuildValue("ii", p.x * 100, p.y * 100);
		PyList_Append(pList, obj);
	}

	return pList;
}


PyObject* pyGetCurrentPhase(PyObject* poSelf, PyObject* poArgs)
{
	CNetworkStream& net = CNetworkStream::Instance();
	int phase = net.GetCurrentPhase();
	return Py_BuildValue("i", phase);
}

PyObject* GetAttrByte(PyObject* poSelf, PyObject* poArgs)
{
	int x, y;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();
	x /= 100;
	y /= 100;

	CBackground& bck = CBackground::Instance();
	BYTE b = bck.getAttrByte(x, y);
	return Py_BuildValue("i", b);
}

PyObject* pySendAttackPacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	BYTE type;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();
	if (!PyTuple_GetByte(poArgs, 1, &type))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	net.SendBattlePacket(vid, type);

	return Py_BuildNone();
}

PyObject* pySendStatePacket(PyObject* poSelf, PyObject* poArgs)
{
	float x, y;
	float rot;
	BYTE eFunc;
	BYTE uArg;
	if (!PyTuple_GetFloat(poArgs, 0, &x))
		return Py_BuildException();
	if (!PyTuple_GetFloat(poArgs, 1, &y))
		return Py_BuildException();
	if (!PyTuple_GetFloat(poArgs, 2, &rot))
		return Py_BuildException();
	if (!PyTuple_GetByte(poArgs, 3, &eFunc))
		return Py_BuildException();
	if (!PyTuple_GetByte(poArgs, 4, &uArg))
		return Py_BuildException();

	fPoint p(x, y);
	CNetworkStream& net = CNetworkStream::Instance();
	net.SendStatePacket(p, rot, eFunc, uArg);

	return Py_BuildNone();
}

PyObject* pySendPacket(PyObject* poSelf, PyObject* poArgs)
{
	int size;
	BYTE* arr;
	if (!PyTuple_GetInteger(poArgs, 0, &size))
		return Py_BuildException();
	if (!PyTuple_GetByteArray(poArgs, 1, &arr))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	net.SendPacket(size, arr);
	return Py_BuildNone();
}

PyObject* launchPacketFilter(PyObject* poSelf, PyObject* poArgs) {
	CNetworkStream& net = CNetworkStream::Instance();
	net.openConsole();
	return Py_BuildNone();
}
PyObject* closePacketFilter(PyObject* poSelf, PyObject* poArgs) {
	CNetworkStream& net = CNetworkStream::Instance();
	net.closeConsole();
	return Py_BuildNone();
}
PyObject* startPacketFilter(PyObject* poSelf, PyObject* poArgs) {
	CNetworkStream& net = CNetworkStream::Instance();
	net.startFilterPacket();
	return Py_BuildNone();
}
PyObject* stopPacketFilter(PyObject* poSelf, PyObject* poArgs) {
	CNetworkStream& net = CNetworkStream::Instance();
	net.stopFilterPacket();
	return Py_BuildNone();
}
PyObject* skipInHeader(PyObject* poSelf, PyObject* poArgs) {
	int header;
	if (!PyTuple_GetInteger(poArgs, 0, &header))
		return Py_BuildException();

	if (header <= 256 && header >= 0) {
		CNetworkStream& net = CNetworkStream::Instance();
		net.addHeaderFilter((BYTE)header, INBOUND);
	}
	return Py_BuildNone();
}

PyObject* skipOutHeader(PyObject* poSelf, PyObject* poArgs) {
	int header;
	if (!PyTuple_GetInteger(poArgs, 0, &header))
		return Py_BuildException();

	if (header <= 256 && header >= 0) {
		CNetworkStream& net = CNetworkStream::Instance();
		net.addHeaderFilter((BYTE)header, OUTBOUND);
	}
	return Py_BuildNone();
}
PyObject* doNotSkipInHeader(PyObject* poSelf, PyObject* poArgs) {
	int header;
	if (!PyTuple_GetInteger(poArgs, 0, &header))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	net.removeHeaderFilter((BYTE)header, INBOUND);
	return Py_BuildNone();
}
PyObject* doNotSkipOutHeader(PyObject* poSelf, PyObject* poArgs) {
	int header;
	if (!PyTuple_GetInteger(poArgs, 0, &header))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	net.addHeaderFilter((BYTE)header, OUTBOUND);
	return Py_BuildNone();
}

PyObject* clearOutput(PyObject* poSelf, PyObject* poArgs) {
	system("cls");
	return Py_BuildNone();
}

PyObject* clearInFilter(PyObject* poSelf, PyObject* poArgs) {
	CNetworkStream& net = CNetworkStream::Instance();
	net.clearPacketFilter(INBOUND);
	return Py_BuildNone();
}

PyObject* clearOutFilter(PyObject* poSelf, PyObject* poArgs) {
	CNetworkStream& net = CNetworkStream::Instance();
	net.clearPacketFilter(OUTBOUND);
	return Py_BuildNone();
}

PyObject* setInFilterMode(PyObject* poSelf, PyObject* poArgs)
{
	int mode;
	if (!PyTuple_GetInteger(poArgs, 0, &mode))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	net.setFilterMode(INBOUND, (bool)mode);
	return Py_BuildNone();
}

PyObject* setOutFilterMode(PyObject* poSelf, PyObject* poArgs)
{
	int mode;
	if (!PyTuple_GetInteger(poArgs, 0, &mode))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	net.setFilterMode(OUTBOUND, (bool)mode);
	return Py_BuildNone();
}

PyObject* pyIsDead(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CInstanceManager& mgr = CInstanceManager::Instance();
	return Py_BuildValue("i", mgr.isInstanceDead(vid));
}


PyObject* pySendStartFishing(PyObject* poSelf, PyObject* poArgs) {
	int direction;
	if (!PyTuple_GetInteger(poArgs, 0, &direction))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	bool val = net.SendStartFishing((WORD)direction);

	return Py_BuildValue("i", val);
}
PyObject* pySendStopFishing(PyObject* poSelf, PyObject* poArgs) {
	int type;
	float timeLeft;
	if (!PyTuple_GetInteger(poArgs, 0, &type))
		return Py_BuildException();

	if (!PyTuple_GetFloat(poArgs, 1, &timeLeft))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	bool val = net.SendStopFishing(type, timeLeft);
	return Py_BuildValue("i", val);
}


PyObject* pySendAddFlyTarget(PyObject* poSelf, PyObject* poArgs) {
	int vid;
	float x, y;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	if (!PyTuple_GetFloat(poArgs, 1, &x))
		return Py_BuildException();

	if (!PyTuple_GetFloat(poArgs, 2, &y))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	bool val = net.SendAddFlyTargetingPacket(vid, x, y);

	return Py_BuildValue("i", val);
}
PyObject* pySendShoot(PyObject* poSelf, PyObject* poArgs) {
	int type;
	if (!PyTuple_GetInteger(poArgs, 0, &type))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	bool val = net.SendShootPacket(type);
	return Py_BuildValue("i", val);

}

PyObject* pyBlockFishingPackets(PyObject* poSelf, PyObject* poArgs)
{
	CNetworkStream& net = CNetworkStream::Instance();
	net.SetFishingPacketsBlock(1);
	return Py_BuildNone();
}

PyObject* pyUnblockFishingPackets(PyObject* poSelf, PyObject* poArgs)
{
	CNetworkStream& net = CNetworkStream::Instance();
	net.SetFishingPacketsBlock(0);
	return Py_BuildNone();
}

PyObject* pyDisableCollisions(PyObject* poSelf, PyObject* poArgs)
{
	CPlayer& player = CPlayer::Instance();
	player.SetBuildingWallHack(1);
	player.SetMonsterTerrainWallHack(1);
	return Py_BuildNone();
}

PyObject* pyEnableCollisions(PyObject* poSelf, PyObject* poArgs)
{
	CPlayer& player = CPlayer::Instance();
	player.SetBuildingWallHack(0);
	player.SetMonsterTerrainWallHack(0);
	return Py_BuildNone();
}

PyObject* pyRegisterNewShopCallback(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* obj;
	if (!PyTuple_GetObject(poArgs, 0, &obj)) {
		return Py_BuildException();
	}

	CNetworkStream& net = CNetworkStream::Instance();
	bool val = net.setNewShopCallback(obj);
	if(val)
		return Py_BuildNone();
	else
		return Py_BuildException();


	return Py_BuildNone();
}

PyObject* pyRecvDigMotionCallback(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* obj;
	if (!PyTuple_GetObject(poArgs, 0, &obj)) {
		return Py_BuildException();
	}

	CNetworkStream& net = CNetworkStream::Instance();
	bool val = net.setDigMotionCallback(obj);
	if (val)
		return Py_BuildNone();
	else
		return Py_BuildException();


	return Py_BuildNone();
}

PyObject* pyRecvStartFishCallback(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* obj;
	if (!PyTuple_GetObject(poArgs, 0, &obj)) {
		return Py_BuildException();
	}

	CNetworkStream& net = CNetworkStream::Instance();
	bool val = net.setStartFishCallback(obj);
	if (val)
		return Py_BuildNone();
	else
		return Py_BuildException();
}

PyObject* pyBlockAttackPackets(PyObject* poSelf, PyObject* poArgs)
{
	CNetworkStream& net = CNetworkStream::Instance();
	net.blockAttackPackets();
	return Py_BuildNone();
}

PyObject* pyUnblockAttackPackets(PyObject* poSelf, PyObject* poArgs)
{
	CNetworkStream& net = CNetworkStream::Instance();
	net.unblockAttackPackets();
	return Py_BuildNone();
}



PyObject* pyItemGrndFilterClear(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.clearFilter();
	return Py_BuildNone();
}

//PICKUP STUFF
PyObject* pyItemGrndNotOnFilter(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.setModeFilter(false);
	return Py_BuildNone();
}

PyObject* pyItemGrndOnFilter(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.setModeFilter(true);
	return Py_BuildNone();
}

PyObject* pyItemGrndAddFilter(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.addItemFilter(index);
	return Py_BuildNone();
}

PyObject* pyItemGrndItemFirst(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.setPickItemFirst(true);
	return Py_BuildNone();
}

PyObject* pyItemGrndNoItemFirst(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.setPickItemFirst(false);
	return Py_BuildNone();
}

PyObject* pyItemGrndDelFilter(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.deleteItemFilter(index);
	return Py_BuildNone();
}

PyObject* pyItemGrndSelectRange(PyObject* poSelf, PyObject* poArgs)
{
	float range = 0;
	if (!PyTuple_GetFloat(poArgs, 0, &range))
		return Py_BuildException();

	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.setPickupRange(range);
	return Py_BuildNone();
}

PyObject* pyItemGrndIgnoreBlockedPath(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.setIgnoreBlockedPath(true);
	return Py_BuildNone();
}

PyObject* pyItemGrndNoIgnoreBlockedPath(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceManager& mgr = CInstanceManager::Instance();
	mgr.setIgnoreBlockedPath(false);
	return Py_BuildNone();
}

PyObject* pyGetCloseItemGround(PyObject* poSelf, PyObject* poArgs)
{
	int x, y;
	SGroundItem item;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	CInstanceManager& mgr = CInstanceManager::Instance();

	if (mgr.getCloseItemGround(x, y, &item)) {
		return Py_BuildValue("(iii)", item.vid, item.x, item.y);
	}
	else {
		return Py_BuildValue("(iii)", 0, 0, 0);
	}
}

PyObject* pySendPickupItem(PyObject* poSelf, PyObject* poArgs)
{
	int vid = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();
	if (vid == 0)
		return Py_BuildNone();

	CNetworkStream& net = CNetworkStream::Instance();
	net.SendPickupItemPacket(vid);
	return Py_BuildNone();
}

PyObject* pyGetItemGrndID(PyObject* poSelf, PyObject* poArgs)
{
	int vnum;
	if (!PyTuple_GetInteger(poArgs, 0, &vnum))
		return Py_BuildException();

	CInstanceManager& mgr = CInstanceManager::Instance();

	int id = mgr.getItemGrndID(vnum);

	return Py_BuildValue("i", id);
}

PyObject* pySkipRenderer(PyObject* poSelf, PyObject* poArgs)
{
	CApp& app = CApp::Instance();
	app.setSkipRenderer();
	return Py_BuildNone();
}

PyObject* pyUnSkipRenderer(PyObject* poSelf, PyObject* poArgs)
{
	CApp& app = CApp::Instance();
	app.unsetSkipRenderer();
	return Py_BuildNone();
}

PyObject* pySyncPlayerPosition(PyObject* poSelf, PyObject* poArgs)
{
	CNetworkStream& net = CNetworkStream::Instance();
	std::vector<InstanceLocalPosition> resultList;

	PyObject* InstanceLst = 0;
	if (!PyTuple_GetObject(poArgs, 0, &InstanceLst))
		return Py_BuildException();

	PyObject *iterator = PyObject_GetIter(InstanceLst);
	PyObject* item;

	if (iterator == NULL) {
		Py_DECREF(InstanceLst);
		return Py_BuildNone();
		//return Py_BuildException("Argument provided is not an list");
	}

	int size = PyList_Size(InstanceLst);
	resultList.reserve(size);

	//Loop through all lists
	while ((item = PyIter_Next(iterator))) {
		InstanceLocalPosition pos;
		if (!PyList_Check(item)) {
			Py_DECREF(InstanceLst);
			Py_DECREF(iterator);
			Py_DECREF(item);
			DEBUG_INFO_LEVEL_2("pySyncPlayerPosition:Argument provided must be a list of lists");
			return Py_BuildNone();
		}
		if (PyList_Size(item) < 3) {
			Py_DECREF(InstanceLst);
			Py_DECREF(iterator);
			Py_DECREF(item);
			DEBUG_INFO_LEVEL_2("pySyncPlayerPosition: To few values on each row, the values of each row must be vid,x,y");
			return Py_BuildNone();
		}
		PyObject* vid_py = PyList_GetItem(item, 0);
		PyObject* x_py = PyList_GetItem(item, 1);
		PyObject* y_py = PyList_GetItem(item, 2);
		pos.vid = PyLong_AsLong(vid_py);
		pos.x = PyFloat_AsDouble(x_py);
		pos.y = PyFloat_AsDouble(y_py);
		DEBUG_INFO_LEVEL_3("pySyncPlayerPosition: vid=%d, x=%f, y=%f", pos.vid,pos.x,pos.y);

		resultList.push_back(pos);
		Py_DECREF(item);
	}
	Py_DECREF(iterator);
	Py_DECREF(InstanceLst);

	DEBUG_INFO_LEVEL_3("Calling SyncPacket");
	int result = net.SendSyncPacket(resultList);
	return Py_BuildValue("(i)", result);
}

PyObject* pySetRecvChatCallback(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* obj;
	if (!PyTuple_GetObject(poArgs, 0, &obj)) {
		return Py_BuildException();
	}

	CNetworkStream& net = CNetworkStream::Instance();
	bool val = net.setChatCallback(obj);
	if (val)
		return Py_BuildNone();
	else
		return Py_BuildException("Fail to set chat callback");


	return Py_BuildNone();
}

PyObject* pySendUseSkillPacket(PyObject* poSelf, PyObject* poArgs) {
	int vid = 0;
	int dwSkillIndex = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &dwSkillIndex))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &vid))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	net.SendUseSkillPacket(dwSkillIndex, vid);
	return Py_BuildNone();

}

PyObject* pySendUseSkillPacketBySlot(PyObject* poSelf, PyObject* poArgs)
{
	int vid = 0;
	int dwSkillSlotIndex = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &dwSkillSlotIndex))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &vid))
		return Py_BuildException();

	CNetworkStream& net = CNetworkStream::Instance();
	net.SendUseSkillBySlot(dwSkillSlotIndex, vid);
	return Py_BuildNone();
}

PyObject* pyGetRequest(PyObject* poSelf, PyObject* poArgs)
{
	char* url = 0;
	PyObject* callback = 0;
	if (!PyTuple_GetString(poArgs, 0, &url))
		return Py_BuildException();
	if (!PyTuple_GetObject(poArgs, 1, &callback)) {
		return Py_BuildException();
	}

	if (PyCallable_Check(callback)) {
		CCommunication& c = CCommunication::Instance();
		int id = c.GetRequest(url, ComCallbackFunction(callback));
		return Py_BuildValue("(i)", id);
	}

	return Py_BuildValue("(i)", -1);
}

PyObject* pyOpenWebsocket(PyObject* poSelf, PyObject* poArgs)
{
	char* url = 0;
	PyObject* callback = 0;
	if (!PyTuple_GetString(poArgs, 0, &url))
		return Py_BuildException();
	if (!PyTuple_GetObject(poArgs, 1, &callback)) {
		return Py_BuildException();
	}

	if (PyCallable_Check(callback)) {
		CCommunication& c = CCommunication::Instance();
		int id = c.OpenWebsocket(url, ComCallbackFunction(callback));
		if (id != -1) {
			return Py_BuildValue("i", id);
		}
	}
	else {
		Py_XDECREF(callback);
	}

	return Py_BuildValue("i", -1);
}

PyObject* pySendWebsocket(PyObject* poSelf, PyObject* poArgs)
{
	int id = 0;
	char* message;
	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();
	if (!PyTuple_GetString(poArgs, 1, &message)) {
		return Py_BuildException();
	}

	
	CCommunication& c = CCommunication::Instance();
	bool val = c.WebsocketSend(id, message);
	if (val) {
		DEBUG_INFO_LEVEL_3("Websocket message sent: %s", message);
	}
	return Py_BuildValue("i", c.WebsocketSend(id, message));
}

PyObject* pyCloseWebsocket(PyObject* poSelf, PyObject* poArgs)
{
	int id = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();



	CCommunication& c = CCommunication::Instance();
	return Py_BuildValue("i", c.CloseWebsocket(id));
}



//This methods must be the last ones on the s_methods variable
static std::set<std::string> premium_methods = 
{
	std::string("GetRequest"),
	std::string("OpenWebsocket"),
	std::string("SendWebsocket"),
	std::string("CloseWebsocket"),
	std::string("SkipRenderer"),
	std::string("UnskipRenderer")
};

static PyMethodDef s_methods[] =
{
	{ "Get",					GetEterPacket,		METH_VARARGS },
	{ "IsPathBlocked",			pyIsPathBlocked,	METH_VARARGS },
	{ "IsPositionBlocked",		IsPositionBlocked,	METH_VARARGS },
	{ "GetAttrByte",			GetAttrByte,		METH_VARARGS },
	{ "GetCurrentPhase",		pyGetCurrentPhase,	METH_VARARGS },
	{ "FindPath",				FindPath,			METH_VARARGS },
	{ "SendPacket",				pySendPacket,		METH_VARARGS },
	{ "SendAttackPacket",		pySendAttackPacket,	METH_VARARGS },
	{ "SendStatePacket",		pySendStatePacket,	METH_VARARGS },
	{ "IsDead",					pyIsDead,			METH_VARARGS },

#ifdef _DEBUG
	{ "LaunchPacketFilter",		launchPacketFilter,	METH_VARARGS },
	{ "ClosePacketFilter",		closePacketFilter,	METH_VARARGS },
	{ "StartPacketFilter",		startPacketFilter,	METH_VARARGS },
	{ "StopPacketFilter",		stopPacketFilter,	METH_VARARGS },
	{ "SkipInHeader",			skipInHeader,		METH_VARARGS },
	{ "SkipOutHeader",			skipOutHeader,		METH_VARARGS },
	{ "DoNotSkipInHeader",		doNotSkipInHeader,	METH_VARARGS },
	{ "DoNotSkipOutHeader",		doNotSkipOutHeader,	METH_VARARGS },
	{ "ClearOutput",			clearOutput,		METH_VARARGS },
	{ "ClearInFilter",			clearInFilter,		METH_VARARGS },
	{ "ClearOutFilter",			clearOutFilter,		METH_VARARGS },
	{ "SetOutFilterMode",		setOutFilterMode,	METH_VARARGS },
	{ "SetInFilterMode",		setInFilterMode,	METH_VARARGS },
#endif
	{ "SendAddFlyTarget",		pySendAddFlyTarget,	METH_VARARGS },
	{ "SendShoot",				pySendShoot,		METH_VARARGS },
	{ "EnableCollisions",		pyEnableCollisions,	METH_VARARGS },
	{ "DisableCollisions",		pyDisableCollisions,METH_VARARGS },
	{ "RegisterNewShopCallback",pyRegisterNewShopCallback,METH_VARARGS },
	{ "SendUseSkillPacket",		pySendUseSkillPacket,METH_VARARGS },
	{ "SendUseSkillPacketBySlot",pySendUseSkillPacketBySlot,METH_VARARGS },

	//PICKUP
	{ "ItemGrndFilterClear",	pyItemGrndFilterClear,	METH_VARARGS },
	{ "ItemGrndNotOnFilter",	pyItemGrndNotOnFilter,	METH_VARARGS },
	{ "ItemGrndOnFilter",		pyItemGrndOnFilter,		METH_VARARGS },
	{ "ItemGrndAddFilter",		pyItemGrndAddFilter,	METH_VARARGS },
	{ "ItemGrndDelFilter",		pyItemGrndDelFilter,	METH_VARARGS },
	{ "GetCloseItemGround",		pyGetCloseItemGround,	METH_VARARGS },
	{ "SendPickupItem",			pySendPickupItem,		METH_VARARGS },
	{ "GetItemGrndID",			pyGetItemGrndID,		METH_VARARGS },
	{ "ItemGrndSelectRange",	pyItemGrndSelectRange,	METH_VARARGS },
	{ "ItemGrndNoItemFirst",	pyItemGrndNoItemFirst,	METH_VARARGS},
	{ "ItemGrndItemFirst",		pyItemGrndItemFirst,	METH_VARARGS},
	{ "ItemGrndInBlockedPath",		pyItemGrndIgnoreBlockedPath,	METH_VARARGS},
	{ "ItemGrndNotInBlockedPath",		pyItemGrndNoIgnoreBlockedPath,	METH_VARARGS},

	{ "RegisterDigMotionCallback",	pyRecvDigMotionCallback,METH_VARARGS },

	{ "BlockAttackPackets",		pyBlockAttackPackets,		METH_VARARGS},
	{ "UnblockAttackPackets",	pyUnblockAttackPackets,		METH_VARARGS},

//#ifdef METIN_GF
	{ "SendStartFishing",		pySendStartFishing,	METH_VARARGS },
	{ "SendStopFishing",		pySendStopFishing,	METH_VARARGS },
	{ "BlockFishingPackets",	pyBlockFishingPackets,	METH_VARARGS },
	{ "UnblockFishingPackets",	pyUnblockFishingPackets,METH_VARARGS },
	{ "RecvStartFishCallback",	pyRecvStartFishCallback,METH_VARARGS},

	//{ "SetKeyState",			pySetKeyState,		METH_VARARGS },
	//{ "SetAttackKeyState",		pySetKeyState,		METH_VARARGS },
	{ "GetPixelPosition",		GetPixelPosition,	METH_VARARGS},
	{ "MoveToDestPosition",     moveToDestPosition, METH_VARARGS},
	{ "SetMoveSpeedMultiplier",	pySetMoveSpeed,		METH_VARARGS},
//#endif

	{ "SyncPlayerPosition", pySyncPlayerPosition ,	METH_VARARGS},
	{ "SetRecvChatCallback", pySetRecvChatCallback ,	METH_VARARGS},

	//Premium
	{ "GetRequest",			pyGetRequest,			METH_VARARGS},
	{ "OpenWebsocket",		pyOpenWebsocket,		METH_VARARGS},
	{ "SendWebsocket",		pySendWebsocket,		METH_VARARGS},
	{ "CloseWebsocket",		pyCloseWebsocket,		METH_VARARGS},
	{ "SkipRenderer",		pySkipRenderer ,		METH_VARARGS},
	{ "UnskipRenderer",		pyUnSkipRenderer ,		METH_VARARGS},


	{ NULL, NULL }
};

void initModule() {
	
	CCommunication& c = CCommunication::Instance();
	bool is_premium = c.IsPremiumUser();
	if (!is_premium) {
		for (int i = 0; s_methods[i].ml_name != 0; i++) {
			if (premium_methods.find(std::string(s_methods[i].ml_name)) != premium_methods.end()) { //is premium function, remove
				DEBUG_INFO_LEVEL_1("Removing premium function %s", s_methods[i].ml_name);
				s_methods[i] = { NULL, NULL };
			}
		}
	}
	DEBUG_INFO_LEVEL_1("Premium setup ended");
	packet_mod = Py_InitModule("eXLib", s_methods);
	DEBUG_INFO_LEVEL_1("eXLib module created");

	PyModule_AddObject(packet_mod, "InstancesList", CInstanceManager::Instance().getVIDList());
	PyModule_AddStringConstant(packet_mod, "PATH", getDllPath());
#ifdef _DEBUG
	PyModule_AddIntConstant(packet_mod, "IS_DEBUG", 1);
#else
	PyModule_AddIntConstant(packet_mod, "IS_DEBUG", 0);
#endif

	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_ATTACK", CHAR_STATE_FUNC_ATTACK);
	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_STOP", CHAR_STATE_FUNC_STOP);
	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_WALK", CHAR_STATE_FUNC_WALK);


	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_ARG_NONE", 0);
	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_ARG_HORSE_ATTACK1", CHAR_STATE_ARG_HORSE_ATTACK1);
	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_ARG_HORSE_ATTACK2", CHAR_STATE_ARG_HORSE_ATTACK2);
	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_ARG_HORSE_ATTACK3", CHAR_STATE_ARG_HORSE_ATTACK3);

	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_ARG_COMBO_ATTACK1", CHAR_STATE_ARG_COMBO_ATTACK1);
	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_ARG_COMBO_ATTACK2", CHAR_STATE_ARG_COMBO_ATTACK2);
	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_ARG_COMBO_ATTACK3", CHAR_STATE_ARG_COMBO_ATTACK3);
	PyModule_AddIntConstant(packet_mod, "CHAR_STATE_ARG_COMBO_ATTACK4", CHAR_STATE_ARG_COMBO_ATTACK4);


	PyModule_AddIntConstant(packet_mod, "COMBO_SKILL_ARCH", COMBO_SKILL_ARCH);

	//FISHING
	PyModule_AddIntConstant(packet_mod, "SUCCESS_FISHING", SUCESS_ON_FISHING);
	PyModule_AddIntConstant(packet_mod, "UNSUCCESS_FISHING", UNSUCESS_ON_FISHING);
	if (!addPathToInterpreter(getDllPath())) {
		DEBUG_INFO_LEVEL_1("Error adding current path to intepreter!");
		MessageBox(NULL, "Error adding current path to intepreter!", "Error", MB_OK);
		CApp & i = CApp::Instance();
		i.exit();
		return;
	}

	executePythonFile("init.py");
}

bool addPathToInterpreter(const char* path) {
	PyObject* sys = PyImport_ImportModule("sys");
	if (!sys) {
		return false;
	}
	PyObject* py_path = PyObject_GetAttrString(sys, "path");
	if (!py_path) {
		Py_DECREF(sys);
		return false;
	}
	PyList_Append(py_path, PyString_FromString(path));
	Py_DECREF(sys);
	Py_DECREF(py_path);
	return true;
}

bool executePythonFile(const char* file) {
	int result = 0;
	char path[256] = { 0 };
	strcpy(path, getDllPath());
	//char del = '\';
	//strncat(path, &del, 1);
	strcat(path, file);
	DEBUG_INFO_LEVEL_1("Executing Python file: %s", path);
	PyObject* PyFileObject = PyFile_FromString(path, (char*)"r");
	if (PyFileObject == NULL) {
		DEBUG_INFO_LEVEL_1("%s  is not a File!", path);
		goto error_code;
	}
	result = PyRun_SimpleFileEx(PyFile_AsFile(PyFileObject), "MyFile", 1);
	if (result == -1) {
		DEBUG_INFO_LEVEL_1("Error executing python script!");
		goto error_code;
	}
	else {
		DEBUG_INFO_LEVEL_1("Python script execution complete!");
		Py_DECREF(PyFileObject);
		return true;
	}

error_code:
	Py_DECREF(PyFileObject);
	return 0;
}
