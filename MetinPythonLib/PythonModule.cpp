#include "PythonModule.h"
#include "App.h"
#include "Network.h"
#include "MapCollision.h"
#include <unordered_map>
#include <map>

static std::string fileName;

/* PyObject* playerModule;
PyObject* getMainPlayerPosition;*/

//Client Functions
typedef bool(__thiscall* tMoveToDestPosition)(void* classPointer, fPoint& pos);
typedef void*(__thiscall* tGetInstancePointer)(DWORD classPointer, DWORD vid);

tMoveToDestPosition fMoveToDestPosition;
tGetInstancePointer fGetInstancePointer;

//Client characterManager stuff
std::map<DWORD, void*>* clientInstanceMap; //Not working
DWORD* characterManagerClassPointer;
DWORD characterManagerSubClass;


//SYNCRONIZATION
static bool executeFile = false;

Hook* hook;
PyObject* packet_mod;
PyObject* pyVIDList;

//File Related
bool getTrigger = false;
EterFile eterFile = { 0 };

struct Instance {
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

std::unordered_map<DWORD, Instance> instances;


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

//The file name will be appended to the path of the dll
bool executePythonFile(char* file) {
	int result = 0;
	char path[256] = { 0 };
	strcpy(path,getDllPath());
	//char del = '\';
	//strncat(path, &del, 1);
	strcat(path, file);
	DEBUG_INFO_LEVEL_1("Executing Python file: %s", path);
	PyObject* PyFileObject = PyFile_FromString(path, (char*)"r");
	if (PyFileObject == NULL) {
		DEBUG_INFO_LEVEL_1("%s  is not a File!",path);
		goto error_code;
	}
	result = PyRun_SimpleFileEx(PyFile_AsFile(PyFileObject), "MyFile", 1);
	if (result == -1) {
		DEBUG_INFO_LEVEL_1("Error executing python script!");
		goto error_code;
	}
	else {
		DEBUG_INFO_LEVEL_1("Python script execution complete!");
		return true;
	}

error_code:
	int message = MessageBoxA(NULL, "Fail To Inject Script!\nEither script failed or does not exist.\nDo you want to try again?", "Error", MB_ICONWARNING | MB_YESNO);
	switch (message)
	{
	case IDYES:
		return executePythonFile(file);
		break;
	case IDNO:
		return 0;
		break;
	default:
		return 0;
	}
}



//setup to run from main thread
void executeScriptFromMainThread(const char* name) {
	fileName = std::string(name);


#ifdef USE_INJECTION_SLEEP_HOOK
	//hook = SleepFunctionHook::setupHook(functionHook);
	//hook->HookFunction();
#endif
#ifdef USE_INJECTION_RECV_HOOK
	executeFile = true;
#endif
	Sleep(50);
	while (executeFile) {
		
	}

#ifdef USE_INJECTION_SLEEP_HOOK
	hook->UnHookFunction();
	delete hook;
#endif
}

PyObject* GetPixelPosition(PyObject* poSelf, PyObject* poArgs)
{
	int vid = 0;

	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	fPoint3D pos = { 0 };
	getCharacterPosition(vid, &pos);
	return Py_BuildValue("fff", (float)pos.x, (float)pos.y, (float)pos.z);


	/*int vid = 0;
	int main_vid = getMainCharacterVID();

	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	if (vid == main_vid)
		return PyObject_CallObject(getMainPlayerPosition, NULL);



	if (instances.find(vid) != instances.end()) {
		auto &instance = instances[vid];

		return Py_BuildValue("fff", (float)instance.x, (float)instance.y, (float)0);
	}*/


	return Py_BuildValue("fff", 0, 0, 0);
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

	fPoint pos(x,y);

	moveToDestPosition(vid, pos);
	return Py_BuildNone();
}

DWORD __stdcall _GetEter(DWORD return_value, CMappedFile* file, const char* fileName, void** buffer) {


	if (getTrigger && strcmp(eterFile.name.c_str(), fileName) == 0) {

		if (eterFile.data != 0) {
			free(eterFile.data);
		}
		eterFile.data = malloc(file->m_dwSize);
		memcpy(eterFile.data, *buffer, file->m_dwSize);
		eterFile.name = std::string(fileName);
		eterFile.size = file->m_dwSize;
	}

	return return_value;
}

void _RecvRoutine(){
	if (executeFile) {
		DEBUG_INFO_LEVEL_1("Executing Python file from Recv Hook");
		executePythonFile((char*)fileName.c_str());
		executeFile = 0;
	}
}

//C Wrapper
EterFile* CGetEter(const char* name) {
	PyObject* obj = Py_BuildValue("(si)", name, 1);
	GetEterPacket(0, obj);
	Py_DECREF(obj);
	return &eterFile;
}

void changeInstancePosition(CharacterStatePacket& packet_move)
{
	/*DEBUG_INFO_LEVEL_1("MAIN Character X:%d Y:%d", packet_move.lX, packet_move.lY);
	if (instances.find(packet_move.dwVID) == instances.end()) {
		DEBUG_INFO_LEVEL_1("Error on changing instance position no vid found");
		return;
	}
	auto& instance = instances[packet_move.dwVID];
	instance.x = packet_move.lX;
	instance.y = packet_move.lY;
	//DEBUG_INFO("VID %d-> X:%d Y:%d", packet_move.dwVID, packet_move.lX, packet_move.lY);*/
}

void changeInstancePosition(CharacterMovePacket& packet_move)
{
	DEBUG_INFO_LEVEL_1("VID %d-> X:%d Y:%d", packet_move.dwVID, packet_move.lX, packet_move.lY);
	if (instances.find(packet_move.dwVID) == instances.end()) {
		DEBUG_INFO_LEVEL_1("Error on changing instance position no vid found");
		return;
	}
	auto& instance = instances[packet_move.dwVID];
	instance.x = packet_move.lX;
	instance.y = packet_move.lY;
	//DEBUG_INFO("VID %d-> X:%d Y:%d", packet_move.dwVID, packet_move.lX, packet_move.lY);
}

void appendNewInstance(PlayerCreatePacket & player)
{
	if (instances.find(player.dwVID) != instances.end()){
		DEBUG_INFO_LEVEL_4("On adding instance with vid=%d, already exists, ignoring packet!\n", player.dwVID);
		return;
	}
	DEBUG_INFO_LEVEL_4("Success Adding instance vid=%d!\n", player.dwVID);
	Instance i = { 0 };
	i.vid = player.dwVID;
	i.angle = player.angle;
	i.x = player.x;
	i.y = player.y;
	i.bAttackSpeed = player.bAttackSpeed;
	i.bMovingSpeed = player.bMovingSpeed;
	i.wRaceNum = player.wRaceNum;
	i.bStateFlag = player.bStateFlag;

	instances[player.dwVID] = i;

	PyObject* pVid = PyLong_FromLong(player.dwVID);
	PyDict_SetItem(pyVIDList, pVid, pVid);

}

void deleteInstance(DWORD vid)
{
	if (instances.find(vid) == instances.end()) {
	DEBUG_INFO_LEVEL_3("On deleting instance with vid=%d doesn't exists, ignoring packet!\n", vid);
		return;
	}
	PyObject* pVid = PyLong_FromLong(vid);
	PyDict_DelItem(pyVIDList, pVid);
	instances.erase(vid);
}

void changeInstanceIsDead(DWORD vid, BYTE isDead)
{
	if (instances.find(vid) != instances.end()) {
		instances[vid].isDead = 1;
	}
}

void clearInstances()
{
	DEBUG_INFO_LEVEL_2("Instances Cleared\n");
	instances.clear();
	PyDict_Clear(pyVIDList);
}

bool getCharacterPosition(DWORD vid, fPoint3D* pos)
{
	void* instanceBase = getInstancePtr(vid);
	if (instanceBase) {
		fPoint3D* iPos = (fPoint3D*)(reinterpret_cast<DWORD>(instanceBase) + OFFSET_CLIENT_CHARACTER_POS);
		pos->x = iPos->x;
		pos->y = -iPos->y;
		pos->z = iPos->z;
		return true;
	}
	return false;
}



//NEEDS TO BE CALLED AFTER SCRIPT EXECUTION
//TEST FOR MEMORY LEAKS
PyObject* GetEterPacket(PyObject * poSelf, PyObject * poArgs) {
	char * szFileName;
	int val = 0;

	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	PyTuple_GetInteger(poArgs, 1, &val);

	PyObject * mod = PyImport_ImportModule("app");
	eterFile.name = std::string(szFileName);

	getTrigger = true;
	PyCallClassMemberFunc(mod, "OpenTextFile", poArgs);
	getTrigger = false;

	Py_DECREF(mod);

	//PyObject * obj = PyString_FromStringAndSize((const char*)eterFile.data, eterFile.size);
	if (val == 0) {
		PyObject* buffer = PyBuffer_FromMemory(eterFile.data, eterFile.size);
		return Py_BuildValue("O", buffer);
	}
	return Py_BuildValue("");
}

PyObject * IsPositionBlocked(PyObject * poSelf, PyObject * poArgs)
{
	int x, y;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	x /= 100;
	y /= 100;

	return Py_BuildValue("i", isBlockedPosition(x, y));
}

bool getUnblockedAdjacentBlock(int x_start, int y_start , Point* adjPoint) {
	if (!isBlockedPosition(x_start + 1, y_start)) {
		adjPoint->x = x_start + 1;
		adjPoint->y = y_start;
		return true;
	}
	else if (!isBlockedPosition(x_start + 1, y_start + 1)) {
		adjPoint->x = x_start + 1;
		adjPoint->y = y_start + 1;
		return true;
	}

	else if (!isBlockedPosition(x_start + 1, y_start - 1)) {
		adjPoint->x = x_start + 1;
		adjPoint->y = y_start - 1;
		return true;
	}
	else if (!isBlockedPosition(x_start - 1, y_start + 1)) {
		adjPoint->x = x_start - 1;
		adjPoint->y = y_start + 1;
		return true;
	}
	else if (!isBlockedPosition(x_start - 1, y_start)) {
		adjPoint->x = x_start - 1;
		adjPoint->y = y_start;
		return true;
	}
	else if (!isBlockedPosition(x_start - 1, y_start - 1)) {
		adjPoint->x = x_start - 1;
		adjPoint->y = y_start - 1;
		return true;
	}
	else if (!isBlockedPosition(x_start, y_start + 1)) {
		adjPoint->x = x_start;
		adjPoint->y = y_start + 1;
		return true;
	}
	else if (!isBlockedPosition(x_start, y_start - 1)) {
		adjPoint->x = x_start;
		adjPoint->y = y_start - 1;
		return true;
	}

	return false;
}

PyObject * FindPath(PyObject * poSelf, PyObject * poArgs)
{
	int x_start, y_start, x_end,y_end;
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

	if (isBlockedPosition(x_start, y_start)) {
		Point a = { 0,0 };
		if (getUnblockedAdjacentBlock(x_start, y_start,&a)) {
			x_start_unblocked = a.x;
			y_start_unblocked = a.y;
		}
	}

	if (isBlockedPosition(x_end, y_end)) {
		Point a = { 0,0 };
		if (getUnblockedAdjacentBlock(x_end, y_end, &a)) {
			x_end = a.x;
			y_end = a.y;
		}
	}


	std::vector<Point> path;
	PyObject* pList = PyList_New(0);
	bool val = false;
	if (x_start_unblocked != -1) {
		val = findPath(x_start_unblocked, y_start_unblocked, x_end, y_end, path);
		if (val)
			PyList_Append(pList, Py_BuildValue("ii", x_start_unblocked * 100, y_start_unblocked * 100));
	}
	else {
		val = findPath(x_start, y_start, x_end, y_end, path);
	}


	if (!val) {
		return pList;
	}
	int i = 0;
	for (Point& p : path) {
		PyObject* obj = Py_BuildValue("ii", p.x*100, p.y*100);
		PyList_Append(pList, obj);
	}

	return pList;


}

PyObject * GetCurrentPhase(PyObject * poSelf, PyObject * poArgs)
{
	int phase = getCurrentPhase();
	return Py_BuildValue("i", phase);
}

PyObject * GetAttrByte(PyObject * poSelf, PyObject * poArgs)
{
	int x, y;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();
	x /= 100;
	y /= 100;

	BYTE b = getAttrByte(x, y);
	return Py_BuildValue("i", b);
}

PyObject * pySendAttackPacket(PyObject * poSelf, PyObject * poArgs)
{
	int vid;
	BYTE type;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();
	if (!PyTuple_GetByte(poArgs, 1, &type))
		return Py_BuildException();

	SendBattlePacket(vid, type);
	
	return Py_BuildNone();
}

PyObject * pySendStatePacket(PyObject * poSelf, PyObject * poArgs)
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
	SendStatePacket(p, rot, eFunc, uArg);

	return Py_BuildNone();
}

PyObject * pySendPacket(PyObject * poSelf, PyObject * poArgs)
{
	int size;
	BYTE* arr;
	if (!PyTuple_GetInteger(poArgs, 0, &size))
		return Py_BuildException();
	if (!PyTuple_GetByteArray(poArgs, 1, &arr))
		return Py_BuildException();

	SendPacket(size, arr);
	return Py_BuildNone();
}

PyObject* launchPacketFilter(PyObject* poSelf, PyObject* poArgs) {
	openConsole();
	return Py_BuildNone();
}
PyObject* closePacketFilter(PyObject* poSelf, PyObject* poArgs) {
	closeConsole();
	return Py_BuildNone();
}
PyObject* startPacketFilter(PyObject* poSelf, PyObject* poArgs) {
	startFilterPacket();
	return Py_BuildNone();
}
PyObject* stopPacketFilter(PyObject* poSelf, PyObject* poArgs) {
	stopFilterPacket();
	return Py_BuildNone();
}
PyObject* skipInHeader(PyObject* poSelf, PyObject* poArgs) {
	int header;
	if (!PyTuple_GetInteger(poArgs, 0, &header))
		return Py_BuildException();

	if (header <= 256 && header >= 0) {
		addHeaderFilter((BYTE)header, INBOUND);
	}
	return Py_BuildNone();
}

PyObject* skipOutHeader(PyObject* poSelf, PyObject* poArgs) {
	int header;
	if (!PyTuple_GetInteger(poArgs, 0, &header))
		return Py_BuildException();

	if (header <= 256 && header >= 0) {
		addHeaderFilter((BYTE)header, OUTBOUND);
	}
	return Py_BuildNone();
}
PyObject* doNotSkipInHeader(PyObject* poSelf, PyObject* poArgs) {
	int header;
	if (!PyTuple_GetInteger(poArgs, 0, &header))
		return Py_BuildException();

	removeHeaderFilter((BYTE)header, INBOUND);
	return Py_BuildNone();
}
PyObject* doNotSkipOutHeader(PyObject* poSelf, PyObject* poArgs) {
	int header;
	if (!PyTuple_GetInteger(poArgs, 0, &header))
		return Py_BuildException();

	addHeaderFilter((BYTE)header, OUTBOUND);
	return Py_BuildNone();
}

PyObject* clearOutput(PyObject* poSelf, PyObject* poArgs) {
	system("cls");
	return Py_BuildNone();
}

PyObject* clearInFilter(PyObject* poSelf, PyObject* poArgs) {
	clearPacketFilter(INBOUND);
	return Py_BuildNone();
}

PyObject* clearOutFilter(PyObject* poSelf, PyObject* poArgs) {
	clearPacketFilter(OUTBOUND);
	return Py_BuildNone();
}

PyObject* setInFilterMode(PyObject* poSelf, PyObject* poArgs)
{
	int mode;
	if (!PyTuple_GetInteger(poArgs, 0, &mode))
		return Py_BuildException();

	setFilterMode(INBOUND, (bool)mode);
	return Py_BuildNone();
}

PyObject* setOutFilterMode(PyObject* poSelf, PyObject* poArgs)
{
	int mode;
	if (!PyTuple_GetInteger(poArgs, 0, &mode))
		return Py_BuildException();

	setFilterMode(OUTBOUND, (bool)mode);
	return Py_BuildNone();
}

void* getInstancePtr(DWORD vid)
{


	return fGetInstancePointer(characterManagerSubClass,vid);

}

PyObject * pyIsDead(PyObject * poSelf, PyObject * poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	if (instances.find(vid) != instances.end()) {
		return Py_BuildValue("i",instances[vid].isDead);
	}
	return Py_BuildValue("i", 1);
}


PyObject* pySendStartFishing(PyObject* poSelf, PyObject* poArgs) {
	int direction;
	if (!PyTuple_GetInteger(poArgs, 0, &direction))
		return Py_BuildException();

	bool val = SendStartFishing((WORD)direction);

	return Py_BuildValue("i", val);
}
PyObject* pySendStopFishing(PyObject* poSelf, PyObject* poArgs) {
	int type;
	float timeLeft;
	if (!PyTuple_GetInteger(poArgs, 0, &type))
		return Py_BuildException();

	if (!PyTuple_GetFloat(poArgs, 1, &timeLeft))
		return Py_BuildException();


	bool val = SendStopFishing(type,timeLeft);
	return Py_BuildValue("i", val);
}


PyObject* pySendAddFlyTarget(PyObject* poSelf, PyObject* poArgs) {
	int vid;
	float x,y;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	if (!PyTuple_GetFloat(poArgs, 1, &x))
		return Py_BuildException();

	if (!PyTuple_GetFloat(poArgs, 2, &y))
		return Py_BuildException();

	bool val = SendAddFlyTargetingPacket(vid, x, y);
	return Py_BuildValue("i", val);
}
PyObject* pySendShoot(PyObject* poSelf, PyObject* poArgs) {
	int type;
	if (!PyTuple_GetInteger(poArgs, 0, &type))
		return Py_BuildException();

	bool val = SendShootPacket(type);
	return Py_BuildValue("i", val);

}




static PyMethodDef s_methods[] =
{
	{ "Get",					GetEterPacket,		METH_VARARGS },
	{ "IsPositionBlocked",		IsPositionBlocked,	METH_VARARGS },
	{ "GetAttrByte",			GetAttrByte,		METH_VARARGS },
	{ "GetCurrentPhase",		GetCurrentPhase,	METH_VARARGS },
	{ "FindPath",				FindPath,			METH_VARARGS },
	{ "SendPacket",				pySendPacket,		METH_VARARGS },
	{ "SendAttackPacket",		pySendAttackPacket,	METH_VARARGS },
	{ "SendStatePacket",		pySendStatePacket,	METH_VARARGS },
	{ "IsDead",					pyIsDead,			METH_VARARGS },
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
	{ "SendAddFlyTarget",		pySendAddFlyTarget,	METH_VARARGS },
	{ "SendShoot",				pySendShoot,		METH_VARARGS },
#ifdef METIN_GF
	{ "SendStartFishing",		pySendStartFishing,	METH_VARARGS },
	{ "SendStopFishing",		pySendStopFishing,	METH_VARARGS },

	{ "GetPixelPosition",		GetPixelPosition,	METH_VARARGS },
	{ "MoveToDestPosition",     moveToDestPosition, METH_VARARGS},
#endif
	{ NULL, NULL }
};

void initModule() {

	//char dllPath[MAX_PATH] = { 0 };
	//getCurrentPath(hDll, dllPath, MAX_PATH);
	//printf("%#x\n", hDll);

#ifdef METIN_GF
	/*playerModule =  PyImport_ImportModule("playerm2g2");
	if (!playerModule) {
		MessageBox(NULL, "Error Importing playerm2g2 module", "ERROR IMPOTRING MODULE", MB_OK);
		exit();
	}

	getMainPlayerPosition = PyObject_GetAttrString(playerModule, "GetMainCharacterPosition");
	if (!getMainPlayerPosition) {
		MessageBox(NULL, "Error GetMainPlayerPosition", "ERROR IMPORTING FUNCTION", MB_OK);
		exit();
	}*/
#endif

	packet_mod = Py_InitModule("net_packet", s_methods);
	pyVIDList = PyDict_New();

	PyModule_AddObject(packet_mod, "InstancesList", pyVIDList);
	PyModule_AddStringConstant(packet_mod, "PATH", getDllPath());

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

	//PHISHING
	PyModule_AddIntConstant(packet_mod, "SUCCESS_FISHING", SUCESS_ON_FISHING);
	PyModule_AddIntConstant(packet_mod, "UNSUCCESS_FISHING", UNSUCESS_ON_FISHING);

	if (!addPathToInterpreter(getDllPath())) {
		DEBUG_INFO_LEVEL_1("Error adding current path to intepreter!");
		MessageBox(NULL, "Error adding current path to intepreter!", "Error", MB_OK);
		exit();
	}
	executeScriptFromMainThread("init.py");
}

void SetChrMngrAndInstanceMap(void* classPointer)
{
	characterManagerClassPointer = (DWORD*)classPointer;


	//Uses fixed offsets to obtain GetInstancePtr
	characterManagerSubClass = (*characterManagerClassPointer + OFFSET_CLIENT_INSTANCE_PTR_1);
	fGetInstancePointer = reinterpret_cast<tGetInstancePointer>(*(DWORD*)(*(DWORD*)characterManagerSubClass + OFFSET_CLIENT_INSTANCE_PTR_2));


	//Not needed for now
	/*int finalAddr = *characterManagerClassPointer + OFFSET_CLIENT_ALIVE_MAP;
	clientInstanceMap = (std::map<DWORD, void*>*)finalAddr;
	DEBUG_INFO_LEVEL_1("InstanceMap Address %#x", clientInstanceMap);*/
	DEBUG_INFO_LEVEL_1("Character Manager %#x", *characterManagerClassPointer);
	DEBUG_INFO_LEVEL_1("GetInstancePointer %#x", fGetInstancePointer);
}

void SetMoveToDistPositionFunc(void* func)
{
	fMoveToDestPosition = (tMoveToDestPosition)func;
}

bool moveToDestPosition(DWORD vid,fPoint& pos) {
	void* p = getInstancePtr(vid);
	if (p) {
		DEBUG_INFO_LEVEL_3("Moving VID %d to posititon X:%d y:%d!",vid,pos.x,pos.y);
		return fMoveToDestPosition(p, pos);
	}
	else {
		return 0;
	}
}
