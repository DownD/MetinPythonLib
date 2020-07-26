#include "PythonModule.h"
#include "App.h"
#include "Network.h"
#include "MapCollision.h"
#include <unordered_map>


static std::string pythonAddSearchPath;
static std::string path;
static bool pass = true;
bool error = false;
Hook* hook;
PyObject* packet_mod;
PyObject* pyVIDList;

bool getTrigger = false;
EterFile eterFile = { 0 };

struct Instance {
	DWORD vid;
	BYTE isDead;
	//float x, y; not needed for now
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
	printf("Trying to load %s on %s folder\n", name, _path);
#endif
	//MessageBox(NULL, pythonAddSearchPath.c_str(), "Error", MB_ICONWARNING | MB_YESNO);

	pass = false;

	hook = SleepFunctionHook::setupHook(functionHook);
	hook->HookFunction();
	while (!pass) {

	}
	while (error) {
		int message = MessageBoxA(NULL, "Fail To Inject Script!\nDo you want to try again?", "Error", MB_ICONWARNING | MB_YESNO);
		switch (message)
		{
		case IDYES:
			error = false;
			pass = false;
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
	hook->UnHookFunction();
	delete hook;
}

DWORD __stdcall _GetEter(DWORD return_value, CMappedFile* file, const char* fileName, void** buffer, const char* uknown, bool uknown_2) {
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

//C Wrapper
EterFile* CGetEter(const char* name) {
	PyObject* obj = Py_BuildValue("(si)", name,1);
	GetEterPacket(0, obj);
	Py_DECREF(obj);
	return &eterFile;
}

void appendNewInstance(DWORD vid)
{
	PyObject* pVid = PyLong_FromLong(vid);
	PyDict_SetItem(pyVIDList, pVid, pVid);

	Instance i = { 0 };
	i.vid = vid;

	instances[vid] = i;
}

void deleteInstance(DWORD vid)
{
	PyObject* pVid = PyLong_FromLong(vid);
	PyDict_DelItem(pyVIDList, pVid);
	instances.erase(vid);
}

void changeInstanceIsDead(DWORD vid, BYTE isDead)
{
	PyObject* pVid = PyLong_FromLong(vid);
	PyDict_DelItem(pyVIDList, pVid);
	if (instances.find(vid) != instances.end()) {
		instances[vid].isDead = 1;
	}
}

void clearInstances()
{
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

	//Try to find adjacent unblocked
	if (isBlockedPosition(x_start, y_start)) {
		if (!isBlockedPosition(x_start + 1, y_start)) {
			x_start_unblocked = x_start+1;
			y_start_unblocked = y_start;
		}
		else if (!isBlockedPosition(x_start + 1, y_start+1)) {
			x_start_unblocked = x_start + 1;
			y_start_unblocked = y_start + 1;
		}

		else if (!isBlockedPosition(x_start + 1, y_start - 1)) {
			x_start_unblocked = x_start + 1;
			y_start_unblocked = y_start - 1;
		}
		else if (!isBlockedPosition(x_start - 1, y_start + 1)) {
			x_start_unblocked = x_start - 1;
			y_start_unblocked = y_start + 1;
		}
		else if (!isBlockedPosition(x_start - 1, y_start)) {
			x_start_unblocked = x_start - 1;
			y_start_unblocked = y_start;
		}
		else if (!isBlockedPosition(x_start - 1, y_start - 1)) {
			x_start_unblocked = x_start - 1;
			y_start_unblocked = y_start - 1;
		}
		else if (!isBlockedPosition(x_start, y_start + 1)) {
			x_start_unblocked = x_start;
			y_start_unblocked = y_start + 1;
		}else if (!isBlockedPosition(x_start, y_start - 1)) {
			x_start_unblocked = x_start;
			y_start_unblocked = y_start - 1;
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
	{ NULL, NULL }
};

void initModule() {

	char dllPath[MAX_PATH] = { 0 };
	getCurrentPath(hDll, dllPath, MAX_PATH);
	printf("%#x\n", hDll);
#ifdef _DEBUG
	printf("Executable current path %s\n",dllPath);
#endif

	packet_mod = Py_InitModule("net_packet", s_methods);
	pyVIDList = PyDict_New();
	PyModule_AddObject(packet_mod, "InstancesList", pyVIDList);
	PyModule_AddStringConstant(packet_mod, "PATH", dllPath);

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


	executeScript("script.py", dllPath);

}
