#include "PythonModule.h"
#include "App.h"
#include "Network.h"
#include "MapCollision.h"
#include <unordered_map>
#include <map>

static std::string pythonAddSearchPath;
static std::string path;




//Client Functions
typedef bool(__thiscall* tMoveToDestPosition)(void* classPointer, fPoint& pos);

tMoveToDestPosition fMoveToDestPosition;

//Client characterManager stuff
std::map<DWORD, void*>* clientInstanceMap;
DWORD* characterManagerClassPointer;


//SYNCRONIZATION
static bool pass = true;
bool error = false;
bool executeFile = false;

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


bool pythonExecuteFile(char* filePath) {
	char* arr = new char[strlen(filePath)];
	strcpy(arr, filePath);
	PathStripPathA(arr);

	PyObject* PyFileObject = PyFile_FromString(filePath, (char*)"r");
	if (PyFileObject == NULL) {
		printf("Error Not a File\n");
		return 0;
	}
	PyObject *sys = PyImport_ImportModule("sys");
	PyObject *path = PyObject_GetAttrString(sys, "path");
	PyList_Append(path, PyString_FromString(pythonAddSearchPath.c_str()));
int result = PyRun_SimpleFileEx(PyFile_AsFile(PyFileObject), "MyFile", 1);
if (result == -1)
return false;
else
return true;
}


//Execute from main thread
//Add the current path to the search path
void functionHook() {
	if (pass)
		return;
	if (pythonExecuteFile((char*)path.c_str())) {
		error = false;
		pass = true;
		return;
	}
	else {
		error = true;
	}
	pass = true;
	return;
}



//setup to run form main thread
void executeScript(const char* name, char*_path) {
	path = std::string(_path);
	path.append(name);
	pythonAddSearchPath = std::string(_path);
	pythonAddSearchPath[pythonAddSearchPath.size()] = 0;
#ifdef _DEBUG
	printf("Trying to load %s on %s\n", name, _path);
#endif
	//MessageBox(NULL, pythonAddSearchPath.c_str(), "Error", MB_ICONWARNING | MB_YESNO);

	pass = false;

#ifdef USE_INJECTION_SLEEP_HOOK
	//hook = SleepFunctionHook::setupHook(functionHook);
	//hook->HookFunction();
#endif
#ifdef USE_INJECTION_RECV_HOOK
	executeFile = true;
#endif

	while (!pass) {

	}
	while (error) {
		int message = MessageBoxA(NULL, "Fail To Inject Script!\nDo you want to try again?", "Error", MB_ICONWARNING | MB_YESNO);
		switch (message)
		{
		case IDYES:
			error = false;
			pass = false;
			executeFile = true;
			while (!pass) {

			}
			break;
		case IDNO:
			error = false;
			break;
		default:
			error = false;
		}
	}

#ifdef USE_INJECTION_SLEEP_HOOK
	hook->UnHookFunction();
	delete hook;
#endif
	DEBUG_INFO("Python script execution complete!");
}

PyObject* GetPixelPosition(PyObject* poSelf, PyObject* poArgs)
{
	int vid = 0;

	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	if (instances.find(vid) != instances.end()) {
		auto &instance = instances[vid];
		printf("%d\n", instance.x);

		return Py_BuildValue("fff", (float)instance.x, (float)instance.y, (float)0);
	}
	return Py_BuildException();
}

PyObject* moveToDestPosition(PyObject* poSelf, PyObject* poArgs)
{
	int vid, x, y;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &x))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 2, &y))
		return Py_BuildException();

	fPoint pos(x,y);

	moveToDestPosition(vid, pos);
	return Py_BuildNone();
}

DWORD __stdcall _GetEter(DWORD return_value, CMappedFile* file, const char* fileName, void** buffer) {
#ifdef _DEBUG 
	//printf("Loading %s, uknown_1 = %s, uknown_2 = %d, return = %d, length=%d\n", fileName, uknown, uknown_2, return_value, file->m_dwSize);

#endif

	if (getTrigger && strcmp(eterFile.name.c_str(), fileName) == 0) {

#ifdef _DEBUG 
		//printf("File address %#x, File size address %#x, buffer address = %#x\n",file, &(file->m_dwSize), buffer);
#endif
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

void _RecvRoutine()
{
	if (executeFile) {
		executeFile = 0;
		functionHook();
	}
}

//C Wrapper
EterFile* CGetEter(const char* name) {
	PyObject* obj = Py_BuildValue("(si)", name, 1);
	GetEterPacket(0, obj);
	Py_DECREF(obj);
	return &eterFile;
}

void changeInstancePosition(CharacterMovePacket& packet_move)
{
	if (instances.find(packet_move.dwVID) != instances.end()) {
		return;
	}
	auto& instance = instances[packet_move.dwVID];
	instance.x = packet_move.lX;
	instance.y = packet_move.lY;
}

void appendNewInstance(PlayerCreatePacket & player)
{
	if (instances.find(player.dwVID) != instances.end()){
#ifdef _DEBUG
		DEBUG_INFO("On adding instance with vid=%d, already exists, ignoring packet!\n", player.dwVID);
#endif
		return;
	}
#ifdef _DEBUG
	DEBUG_INFO("Success Adding instance vid=%d!\n", player.dwVID);
#endif
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
#ifdef _DEBUG
	DEBUG_INFO("On deleting instance with vid=%d doesn't exists, ignoring packet!\n", vid);
#endif
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
#ifdef _DEBUG
	DEBUG_INFO("Instances Cleared\n");
#endif
	instances.clear();
	PyDict_Clear(pyVIDList);
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
	auto itor = clientInstanceMap->find(vid);

	if (clientInstanceMap->end() == itor)
		return NULL;

	return itor->second;
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
	{ "SetOutFilterMode",		setInFilterMode,	METH_VARARGS },
	{ "SetInFilterMode",		setInFilterMode,	METH_VARARGS },
	{ "GetPixelPosition",		GetPixelPosition,	METH_VARARGS },
	{ "MoveToDestPosition",     moveToDestPosition, METH_VARARGS},
	{ NULL, NULL }
};

void initModule() {

	//char dllPath[MAX_PATH] = { 0 };
	//getCurrentPath(hDll, dllPath, MAX_PATH);
	//printf("%#x\n", hDll);
#ifdef _DEBUG
	printf("Executable current path %s\n", getDllPath());
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


	executeScript("script.py", (char *)getDllPath());

}

void SetChrMngrClassPointer(void* classPointer)
{
	characterManagerClassPointer = (DWORD*)classPointer;
	printf("%#x\n", (void*)characterManagerClassPointer);
	int finalAddr = *characterManagerClassPointer + OFFSET_CLIENT_ALIVE_MAP;
	clientInstanceMap = (std::map<DWORD, void*>*)finalAddr;
	printf("%#x\n", (void*)clientInstanceMap);
}

void SetMoveToDistPositionFunc(void* func)
{
	fMoveToDestPosition = (tMoveToDestPosition)func;
}

bool moveToDestPosition(DWORD vid,fPoint& pos) {
	void* p = getInstancePtr(vid);
	if (p)
		return fMoveToDestPosition(p, pos);
	else
		return 0;
}
