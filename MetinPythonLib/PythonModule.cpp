#include "PythonModule.h"
#include "App.h"
#include "Network.h"
#include "MapCollision.h"


static std::string pythonAddSearchPath;
static std::string path;
static bool pass = true;
bool error = false;
Hook* hook;
PyObject* packet_mod;
PyObject* instanceList;

bool getTrigger = false;
EterFile eterFile = { 0 };

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


	std::vector<Point> path;
	bool val = findPath(x_start, y_start, x_end, y_end, path);
	PyObject* pList = PyList_New(0);
	if (!val) {
		return pList;
	}
	int i = 0;
	for (Point& p : path) {
		PyObject* obj = Py_BuildValue("ii", p.x, p.y);
		PyList_Append(pList, obj);
	}

	return pList;


}

PyObject * GetCurrentPhase(PyObject * poSelf, PyObject * poArgs)
{
	int phase = getCurrentPhase();
	return Py_BuildValue("i", phase);
}

static PyMethodDef s_methods[] =
{
	{ "Get",					GetEterPacket,		METH_VARARGS },
	{ "IsPositionBlocked",		IsPositionBlocked,	METH_VARARGS },
	{ "GetCurrentPhase",		GetCurrentPhase,	METH_VARARGS },
	{ "FindPath",				FindPath,			METH_VARARGS },
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
	instanceList = PyDict_New();
	PyModule_AddObject(packet_mod, "InstancesList", instanceList);
	PyModule_AddStringConstant(packet_mod, "PATH", dllPath);

	executeScript("script.py", dllPath);

}
