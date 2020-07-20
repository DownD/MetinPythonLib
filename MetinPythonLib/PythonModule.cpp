#include "PythonModule.h"
#include "App.h"


static std::string pythonAddSearchPath;
static std::string path;
static bool pass = true;
bool error = false;
Hook* hook;
PyObject* packet_mod;
PyObject* instanceList;

struct EterFile {
	void* data;
	int size;
	std::string name;
};

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
	printf("Loading %s, uknown_1 = %s, uknown_2 = %d, return = %d, length=%d\n", fileName, uknown, uknown_2, return_value, file->m_dwSize);

#endif

	if (getTrigger) {

#ifdef _DEBUG 
		//printf("File address %#x, File size address %#x, buffer address = %#x\n",file, &(file->m_dwSize), buffer);
#endif
		eterFile.data = *buffer;
		eterFile.name = std::string(fileName);
		eterFile.size = file->m_dwSize;
	}

	return return_value;
}



//NEEDS TO BE CALLED AFTER SCRIPT EXECUTION
//TEST FOR MEMORY LEAKS
PyObject* GetEterPacket(PyObject * poSelf, PyObject * poArgs) {
	//char * szFileName;
//	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		//return Py_BuildException();

	PyObject * mod = PyImport_ImportModule("app");
	getTrigger = true;


	PyCallClassMemberFunc(mod, "OpenTextFile", poArgs);
	getTrigger = false;

	//PyObject * obj = PyString_FromStringAndSize((const char*)eterFile.data, eterFile.size);

	PyObject* buffer = PyBuffer_FromMemory(eterFile.data, eterFile.size);
	return Py_BuildValue("O", buffer);
}

static PyMethodDef s_methods[] =
{
	{ "Get",		GetEterPacket,		METH_VARARGS },
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
