#include "stdafx.h"
#include "Player.h"
#include "NetworkStream.h"
#include "Memory.h"
#include "InstanceManager.h"

EterFile* CPlayer::CGetEter(const char* name)
{
	PyObject* obj = Py_BuildValue("(si)", name, 1);
	auto obj2 = GetEterPacket(0, obj);
	Py_DECREF(obj);
	Py_DECREF(obj2);
	return &eterFile;
}

bool CPlayer::moveToDestPosition(DWORD vid, fPoint& pos)
{
	CInstanceManager& mgr = CInstanceManager::Instance();
	CMemory& mem = CMemory::Instance();
	void* p = mgr.getInstancePtr(vid);
	if (p) {
		DEBUG_INFO_LEVEL_3("Moving VID %d to position X:%f y:%f", vid, pos.x, pos.y);
		lastMovement = MOVE_POSITION;
		lastDestPos = pos;
		return mem.callMoveToDestPosition((DWORD)p, pos);
	}
	else {
		return 0;
	}
}

void CPlayer::setPixelPosition(fPoint fPos)
{
	CNetworkStream& net = CNetworkStream::Instance();
	DWORD actor = net.GetMainCharacterVID();
	PyObject* poArgs_select = Py_BuildValue("(i)", actor);
	PyObject* poArgs = Py_BuildValue("(iii)", (int)fPos.x, (int)fPos.y, actor);
	long ret = 0;

	DEBUG_INFO_LEVEL_3("SetPixelPosition x->%d, y->%d vid->%d", (int)fPos.x, (int)fPos.y, (int)actor);
	PyCallClassMemberFunc(chr_mod, "SelectInstance", poArgs_select, &ret);

	PyCallClassMemberFunc(chr_mod, "SetPixelPosition", poArgs, &ret);
	Py_DECREF(poArgs_select);
	Py_DECREF(poArgs);
}

BYTE CPlayer::getLastMovementType()
{
	return lastMovement;
}

fPoint CPlayer::getLastDestPosition()
{
	return lastDestPos;
}

std::string CPlayer::getPlayerName()
{
	std::string result;
	if (!PyCallClassMemberFunc(player_mod, "GetName", Py_BuildValue("()"),result)) {
		return "";
	}
	return result;
}

PyObject* CPlayer::GetEterPacket(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	int val = 0;

	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	PyTuple_GetInteger(poArgs, 1, &val);

	PyObject* mod = PyImport_ImportModule("app");
	eterFile.name = std::string(szFileName);

	getTrigger = true;
	PyCallClassMemberFunc(mod, "OpenTextFile", poArgs);
	getTrigger = false;

	Py_DECREF(poArgs);
	Py_DECREF(mod);
	Py_DECREF(szFileName);

	//PyObject * obj = PyString_FromStringAndSize((const char*)eterFile.data, eterFile.size);
	if (val == 0) {
		PyObject* buffer = PyBuffer_FromMemory(eterFile.data, eterFile.size);
		return Py_BuildValue("O", buffer);
	}
	return Py_BuildValue("()");
}

CPlayer::CPlayer() : lastDestPos(0,0)
{
	getTrigger = false;
	eterFile = { 0 };

	//If sets this variable according to the last type of movement
	lastMovement = MOVE_WALK;
	lastDestPos = { 0,0 };

	//Wallhack
	wallHackBuildings = 0;
	wallHackTerrainMonsters = 0;

	chr_mod = 0;
	player_mod = 0;
}

CPlayer::~CPlayer()
{
	Py_DECREF(chr_mod);
	Py_DECREF(player_mod);
}

void CPlayer::importPython()
{
	chr_mod = PyImport_ImportModule("chr");
	player_mod = PyImport_ImportModule("player");
}

void CPlayer::__GetEter(CMappedFile& file, const char* fileName, void** buffer)
{
	DEBUG_INFO_LEVEL_5("Hook Ether_Get called, name=%s", fileName);
	if (getTrigger && strcmp(eterFile.name.c_str(), fileName) == 0) {

		if (eterFile.data != 0) {
			free(eterFile.data);
		}
		eterFile.data = malloc(file.m_dwSize);
		memcpy(eterFile.data, *buffer, file.m_dwSize);
		eterFile.name = std::string(fileName);
		eterFile.size = file.m_dwSize;
	}
	//DEBUG_INFO_LEVEL_4("Hook Ether_Get Returning");
}

bool CPlayer::__MoveToDestPosition(ClassPointer p ,fPoint& pos)
{
	DEBUG_INFO_LEVEL_4("__MoveToDestPosition Called x->%f y->%f", pos.x, pos.y);
	lastMovement = MOVE_POSITION;
	lastDestPos = pos;
	CMemory& mem = CMemory::Instance();
	return mem.callMoveToDestPosition(p, pos);
}

bool CPlayer::__MoveToDirection(ClassPointer p, float rot)
{
	DEBUG_INFO_LEVEL_4("__MoveToDirection Called rot=%f", rot);
	lastMovement = MOVE_WALK;
	CMemory& mem = CMemory::Instance();
	return mem.callMoveToDirection(p, rot);
}

bool CPlayer::__BackgroundCheckAdvanced(ClassPointer classPointer, void* instanceBase)
{
	//DEBUG_INFO_LEVEL_4("Hook __BackgroundCheckAdvanced called");
	if (wallHackBuildings)
		return false;
	else {
		CMemory& mem = CMemory::Instance();
		return mem.callBackgroundCheckAdv(classPointer, instanceBase);
	}
}

bool CPlayer::__InstanceBaseCheckAdvanced(ClassPointer classPointer)
{
	//DEBUG_INFO_LEVEL_4("Hook __InstanceBaseCheckAdvanced called");
	if (wallHackTerrainMonsters)
		return false;
	else {
		CMemory& mem = CMemory::Instance();
		return mem.callIntsanceBaseCheckAdv(classPointer);
	}
}
