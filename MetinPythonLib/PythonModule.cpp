#include "PythonModule.h"
#include "App.h"
#include "Network.h"
#include "MapCollision.h"
#include <unordered_map>
#include <map>
#include <set>

static std::string fileName;

/* PyObject* playerModule;
PyObject* getMainPlayerPosition;*/

//Client Functions
typedef void*(__thiscall* tGetInstancePointer)(DWORD classPointer, DWORD vid);

tGetInstancePointer fGetInstancePointer;

//Hooks
DetoursHook<tMoveToDestPosition>* moveToDestPositionHook = 0;
DetoursHook<tMoveToDirection>* moveToDirectionHook = 0;

//If sets this variable according to the last type of movement
BYTE lastMovement = MOVE_WALK;
fPoint lastDestPos = { 0,0 };

//Movement speed stuff
float speedMultiplier = 1;

//Client characterManager stuff
std::map<DWORD, void*>* clientInstanceMap; //Not working
DWORD* characterManagerClassPointer;
DWORD characterManagerSubClass;

//CallBacks Functions
PyObject* shopRegisterCallback = 0;
PyObject* recvDigMotionCallback = 0;

//SYNCRONIZATION
static bool executeFile = false;

Hook* hook;
PyObject* packet_mod;
PyObject* pyVIDList;

//Python Modules
PyObject* chr_mod;
PyObject* player_mod;

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

struct GroundItem {
	long x, y;
	DWORD index;
	DWORD vid;
	DWORD ownerVID;

};

bool pickOnFilter = false;

std::set<DWORD> pickupFilter;
std::map<DWORD, GroundItem> groundItems;
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
	/*int message = MessageBoxA(NULL, "Fail To Inject Script!\nEither script failed or does not exist.\nDo you want to try again?", "Error", MB_ICONWARNING | MB_YESNO);
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
	}*/
	return 0;
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

long getCurrentSpeed() {
	PyObject* poArgs = Py_BuildValue("(i)", STATUS_MOVEMENT_SPEED);
	long ret = 0;

	if (PyCallClassMemberFunc(player_mod, "GetStatus", poArgs, &ret)) {
		Py_DECREF(poArgs);
		return ret / 100;
	}
	Py_DECREF(poArgs);
	return ret;
}

void setPixelPosition(fPoint fPos)
{

	PyObject* poArgs = Py_BuildValue("(ii)",(int)fPos.x, (int)fPos.y);
	long ret = 0;
	DEBUG_INFO_LEVEL_3("SetPixelPosition x->%d, y->%d", (int)fPos.x, (int)fPos.y);
	if (PyCallClassMemberFunc(chr_mod, "SetPixelPosition", poArgs, &ret)) {
		Py_DECREF(poArgs);
		return;
	}

	Py_DECREF(poArgs);
}

BYTE getLastMovementType()
{
	return lastMovement;
}

fPoint getLastDestPosition()
{
	return lastDestPos;
}

PyObject* pySetMoveSpeed(PyObject* poSelf, PyObject* poArgs)
{
	float speed;
	if (!PyTuple_GetFloat(poArgs, 0, &speed))
		return Py_BuildException();

	SetSpeedMultiplier(speed);
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

bool __fastcall __MoveToDestPosition(void* classPointer, DWORD EDX, fPoint& pos)
{
	lastMovement = MOVE_POSITION;
	lastDestPos = pos;
	//DEBUG_INFO_LEVEL_4("__MoveToDestPosition Called");
	return moveToDestPositionHook->originalFunction(classPointer, pos);
}

bool __fastcall __MoveToDirection(void* classPointer, DWORD EDX, float rot)
{
	lastMovement = MOVE_WALK;
	//DEBUG_INFO_LEVEL_4("__MoveToDirection Called");
	return moveToDirectionHook->originalFunction(classPointer, rot);
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
	if (instances.find(packet_move.dwVID) == instances.end()) {
		DEBUG_INFO_LEVEL_3("No instance vid %d found, creating new one",packet_move.dwVID);
		PlayerCreatePacket player = { 0 };
		player.dwVID = packet_move.dwVID;
		player.x = packet_move.lX;
		player.y = packet_move.lY;
		appendNewInstance(player);
		return;
	}
	DEBUG_INFO_LEVEL_4("VID %d-> X:%d Y:%d", packet_move.dwVID, packet_move.lX, packet_move.lY);
	auto& instance = instances[packet_move.dwVID];
	instance.x = packet_move.lX;
	instance.y = packet_move.lY;
	//DEBUG_INFO("VID %d-> X:%d Y:%d", packet_move.dwVID, packet_move.lX, packet_move.lY);
}

void registerNewInstanceShop(DWORD player)
{
	//call python callback
	DEBUG_INFO_LEVEL_3("Checking Callback VID->", player);
	if (shopRegisterCallback && PyCallable_Check(shopRegisterCallback)) {
		DEBUG_INFO_LEVEL_3("Calling python RegisterShopCallback");
		PyObject* val = Py_BuildValue("(i)", player);
		PyObject_CallObject(shopRegisterCallback, val);
		Py_XDECREF(val);
	}

	/* Appears that there is always an character append packet after shop creation
	* So append to instances is not needed
	if (instances.find(player) != instances.end()) {
		DEBUG_INFO_LEVEL_3("On adding instance shop with vid=%d, already exists, ignoring packet", player);
		return;
	}
	DEBUG_INFO_LEVEL_3("Success Adding instance shop vid=%d", player);

	Instance i = { 0 };
	i.vid = player;

	instances[player] = i;

	PyObject* pVid = PyLong_FromLong(player);
	PyDict_SetItem(pyVIDList, pVid, pVid);*/
}

void callDigMotionCallback(DWORD target_player,DWORD target_vein,DWORD n)
{
	//call python callback
	DEBUG_INFO_LEVEL_3("Mining packet recived");
	if (recvDigMotionCallback && PyCallable_Check(recvDigMotionCallback)) {
		DEBUG_INFO_LEVEL_3("Calling python DigMotionCallback");
		PyObject* val = Py_BuildValue("(iii)", target_player, target_vein, n);
		PyObject_CallObject(recvDigMotionCallback, val);
		Py_XDECREF(val);
	}

}


void appendNewInstance(PlayerCreatePacket & player)
{
	if (instances.find(player.dwVID) != instances.end()){
		DEBUG_INFO_LEVEL_4("On adding instance with vid=%d, already exists, ignoring packet", player.dwVID);
		return;
	}
	DEBUG_INFO_LEVEL_4("Success Adding instance vid=%d", player.dwVID);

	Instance i = { 0 };
	i.vid = player.dwVID;
	i.angle = player.angle;
	i.x = player.x;
	i.y = player.y;
	i.bAttackSpeed = player.bAttackSpeed;
	i.bMovingSpeed = player.bMovingSpeed;
	i.wRaceNum = player.wRaceNum;
	i.bStateFlag = player.bStateFlag;

	if (i.wRaceNum >= MIN_RACE_SHOP && i.wRaceNum <= MAX_RACE_SHOP) {
		registerNewInstanceShop(player.dwVID);
	}

	instances[player.dwVID] = i;

	PyObject* pVid = PyLong_FromLong(player.dwVID);
	PyDict_SetItem(pyVIDList, pVid, pVid);

}


void deleteInstance(DWORD vid)
{
	if (instances.find(vid) == instances.end()) {
		DEBUG_INFO_LEVEL_3("On deleting instance with vid=%d doesn't exists, ignoring packet!", vid);
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

void addItemGround(GroundItemAddPacket& itemPacket)
{
	GroundItem item;
	item.index = itemPacket.itemIndex;
	item.ownerVID = itemPacket.playerVID;
	item.x = itemPacket.x;
	item.y = itemPacket.y;
	item.vid = itemPacket.VID;

	DEBUG_INFO_LEVEL_3("Adding item ground with vid=%d at position x=%d,y=%d", item.vid, item.x, item.y);


	groundItems[item.vid] = item;
}

void delItemGround(GroundItemDeletePacket& item)
{
	if (groundItems.find(item.vid) == groundItems.end()) {
		DEBUG_INFO_LEVEL_3("On deleting item with vid=%d doesn't exists, ignoring packet!", item.vid);
		return;
	}
	DEBUG_INFO_LEVEL_4("Deleting item ground with vid=%d",item.vid);
	groundItems.erase(item.vid);
}

void clearInstances()
{
	DEBUG_INFO_LEVEL_2("Instances Cleared");
	instances.clear();
	groundItems.clear();
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

	Py_DECREF(poArgs);
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

//Get closest unblocked cell
bool getClosestUnblocked(int x_start, int y_start, Point* buffer) {
	Point closestPoints[8];

	//Initalize all coords
	for (int i = 0; i < 8; i++) {
		closestPoints[i] = { x_start,y_start };
	}

	while (true){
		closestPoints[0].x++;

		closestPoints[1].x++;
		closestPoints[1].y++;

		closestPoints[2].x++;
		closestPoints[2].y--;

		closestPoints[3].x--;
		closestPoints[3].y++;

		closestPoints[4].x--;

		closestPoints[5].x--;
		closestPoints[5].y--;

		closestPoints[6].y++;

		closestPoints[7].y--;

		for (int i = 0; i < 8; i++) {
			if (!isBlockedPosition(closestPoints[i].x, closestPoints[i].y)) {
				*buffer = closestPoints[i];
				return true;
			}
		}
	}
	return false;
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
		if (getClosestUnblocked(x_start, y_start,&a)) {
			x_start_unblocked = a.x;
			y_start_unblocked = a.y;
			DEBUG_INFO_LEVEL_3("[PATH-FIDING] Start Position blocked, new position X:%d  Y:%d", x_start_unblocked, y_start_unblocked);
		}
	}

	if (isBlockedPosition(x_end, y_end)) {
		Point a = { 0,0 };
		if (getClosestUnblocked(x_end, y_end, &a)) {
			x_end = a.x;
			y_end = a.y;
			DEBUG_INFO_LEVEL_3("[PATH-FIDING] End Position blocked, new position X:%d  Y:%d", x_end, y_end);
		}
	}



	std::vector<Point> path;
	PyObject* pList = PyList_New(0);
	bool val = false;
	if (x_start_unblocked != -1) {
		val = findPath(x_start_unblocked, y_start_unblocked, x_end, y_end, path);
		if (val)
			PyList_Append(pList, Py_BuildValue("ii", x_start_unblocked * 100, y_start_unblocked * 100));
		else
			return pList;
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
/*
PyObject* pySetKeyState(PyObject* poSelf, PyObject* poArgs) {
	int key, state;
	int numArgs = PyTuple_Size(poArgs);

	KEYBDINPUT keyboardI = { 0 };
	keyboardI.wVk = VK_SPACE;
	keyboardI.wScan = 0;
	keyboardI.time = NULL;
	keyboardI.dwFlags = NULL;
	keyboardI.dwExtraInfo = GetMessageExtraInfo();

	INPUT input = { 0 };
	input.type = INPUT_KEYBOARD;
	input.ki = keyboardI;

	switch(PyTuple_Size(poArgs)) {
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &state)) {
			if (state) {
				input.ki.dwFlags = KEYEVENTF_KEYUP;
			}
			else {
				input.ki.dwFlags = 0;
			}
		}
		break;
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &key)) {
			input.ki.wVk = key;
		}
		if (!PyTuple_GetInteger(poArgs, 1, &state)) {
			if (state) {
				input.ki.dwFlags = KEYEVENTF_KEYUP;
			}
			else {
				input.ki.dwFlags = 0;
			}
		}
		break;

	default:
		return Py_BuildException();
		break;
	}

	SendInput(1, &input, sizeof(input));
	return Py_BuildNone();
}*/

PyObject * pyGetCurrentPhase(PyObject * poSelf, PyObject * poArgs)
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

PyObject* pyBlockFishingPackets(PyObject* poSelf, PyObject* poArgs)
{
	SetFishingPacketsBlock(1);
	return Py_BuildNone();
}

PyObject* pyUnblockFishingPackets(PyObject* poSelf, PyObject* poArgs)
{
	SetFishingPacketsBlock(0);
	return Py_BuildNone();
}

PyObject* pyDisableCollisions(PyObject* poSelf, PyObject* poArgs)
{
	SetBuildingWallHack(1);
	SetMonsterTerrainWallHack(1);
	return Py_BuildNone();
}

PyObject* pyEnableCollisions(PyObject* poSelf, PyObject* poArgs)
{
	SetBuildingWallHack(0);
	SetMonsterTerrainWallHack(0);
	return Py_BuildNone();
}

PyObject* pyRegisterNewShopCallback(PyObject* poSelf, PyObject* poArgs)
{
	if (!PyTuple_GetObject(poArgs, 0, &shopRegisterCallback)) {
		shopRegisterCallback = 0;
		return Py_BuildException();
	}

	if (!PyCallable_Check(shopRegisterCallback)) {
		DEBUG_INFO_LEVEL_1("RegisterNewShopCallback argument is not a function");
		Py_DECREF(shopRegisterCallback);
		shopRegisterCallback = 0;
		return Py_BuildException();
	}

	DEBUG_INFO_LEVEL_2("RegisterNewShopCallback function set sucessfully");

	return Py_BuildNone();
	
}

PyObject* pyRecvDigMotionCallback(PyObject* poSelf, PyObject* poArgs)
{
	if (!PyTuple_GetObject(poArgs, 0, &recvDigMotionCallback)) {
		recvDigMotionCallback = 0;
		return Py_BuildException();
	}

	if (!PyCallable_Check(recvDigMotionCallback)) {
		DEBUG_INFO_LEVEL_1("RegisterNewDigMotionCallback argument is not a function");
		Py_DECREF(recvDigMotionCallback);
		recvDigMotionCallback = 0;
		return Py_BuildException();
	}

	DEBUG_INFO_LEVEL_2("RecvDigMotionCallback function set sucessfully");

	return Py_BuildNone();
}



PyObject* pyItemGrndFilterClear(PyObject* poSelf, PyObject* poArgs)
{
	pickupFilter.clear();
	return nullptr;
}

//PICKUP STUFF
PyObject* pyItemGrndNotOnFilter(PyObject* poSelf, PyObject* poArgs)
{
	pickOnFilter = false;
	return Py_BuildNone();
}

PyObject* pyItemGrndOnFilter(PyObject* poSelf, PyObject* poArgs)
{
	pickOnFilter = true;
	return Py_BuildNone();
}

PyObject* pyItemGrndAddFilter(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	pickupFilter.insert(index);
	return Py_BuildNone();
}

PyObject* pyItemGrndDelFilter(PyObject* poSelf, PyObject* poArgs)
{
	int index = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	pickupFilter.erase(index);
	return Py_BuildNone();
}

PyObject* pyGetCloseItemGround(PyObject* poSelf, PyObject* poArgs)
{
	int x,y;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	DEBUG_INFO_LEVEL_4("Number of items in filter %d", pickupFilter.size())

	float minDist = std::numeric_limits<float>::max();
	DWORD selVID = 0;
	for (auto iter = groundItems.begin(); iter != groundItems.end();iter++) {
		DWORD vid = iter->first;
		GroundItem item = iter->second;

		if (item.ownerVID != getMainCharacterVID() && item.ownerVID != 0) {
			continue;
		}

		bool is_in = pickupFilter.find(item.index) != pickupFilter.end();
		if (pickOnFilter && is_in) {
			float dist = distance(x, y, item.x, item.y);
			if (dist<minDist) {
				minDist = dist;
				selVID = vid;
			}
		}
		if (!pickOnFilter && !is_in) {
			float dist = distance(x, y, item.x, item.y);
			if (dist < minDist) {
				minDist = dist;
				selVID = vid;
			}
		}
	}
	if (selVID != 0) {
		return Py_BuildValue("(iii)", selVID, groundItems[selVID].x, groundItems[selVID].y);
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
	SendPickupItemPacket(vid);
	return Py_BuildNone();
}

PyObject* pySendUseSkillPacket(PyObject* poSelf, PyObject* poArgs) {
	int vid = 0;
	int dwSkillIndex = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &dwSkillIndex))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &vid))
		return Py_BuildException();

	SendUseSkillPacket(dwSkillIndex, vid);
	return Py_BuildNone();

}




static PyMethodDef s_methods[] =
{
	{ "Get",					GetEterPacket,		METH_VARARGS },
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

	//PICKUP
	{ "ItemGrndFilterClear",	pyItemGrndFilterClear,	METH_VARARGS },
	{ "ItemGrndNotOnFilter",	pyItemGrndNotOnFilter,	METH_VARARGS },
	{ "ItemGrndOnFilter",		pyItemGrndOnFilter,		METH_VARARGS },
	{ "ItemGrndAddFilter",		pyItemGrndAddFilter,	METH_VARARGS },
	{ "ItemGrndDelFilter",		pyItemGrndDelFilter,	METH_VARARGS },
	{ "GetCloseItemGround",		pyGetCloseItemGround,	METH_VARARGS },
	{ "SendPickupItem",			pySendPickupItem,		METH_VARARGS },

#ifdef _DEBUG
	{ "RegisterDigMotionCallback",	pyRecvDigMotionCallback,METH_VARARGS },
#endif


#ifdef METIN_GF
	{ "SendStartFishing",		pySendStartFishing,	METH_VARARGS },
	{ "SendStopFishing",		pySendStopFishing,	METH_VARARGS },
	{ "BlockFishingPackets",	pyBlockFishingPackets,	METH_VARARGS },
	{ "UnblockFishingPackets",	pyUnblockFishingPackets,METH_VARARGS },

	//{ "SetKeyState",			pySetKeyState,		METH_VARARGS },
	//{ "SetAttackKeyState",		pySetKeyState,		METH_VARARGS },
	{ "GetPixelPosition",		GetPixelPosition,	METH_VARARGS},
	{ "MoveToDestPosition",     moveToDestPosition, METH_VARARGS},
	{ "SetMoveSpeedMultiplier",	pySetMoveSpeed,		METH_VARARGS},
#endif
	{ NULL, NULL }
};

void initModule() {
	packet_mod = Py_InitModule("eXLib", s_methods);
	pyVIDList = PyDict_New();

	PyModule_AddObject(packet_mod, "InstancesList", pyVIDList);
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
		exit();
		return;
	}
	executeScriptFromMainThread("init.py");
	chr_mod = PyImport_ImportModule("chr");
	player_mod = PyImport_ImportModule("player");
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

void SetMoveToDistPositionFunc(DetoursHook<tMoveToDestPosition>* hook)
{
	moveToDestPositionHook = hook;
	moveToDestPositionHook->HookFunction();
}


void SetMoveToToDirectionFunc(DetoursHook<tMoveToDirection>* hook)
{
	moveToDirectionHook = hook;
	moveToDirectionHook->HookFunction();
}

bool moveToDestPosition(DWORD vid,fPoint& pos) {
	void* p = getInstancePtr(vid);
	if (p) {
		DEBUG_INFO_LEVEL_3("Moving VID %d to posititon X:%f y:%f!",vid,pos.x,pos.y);
		return moveToDestPositionHook->originalFunction(p, pos);
	}
	else {
		return 0;
	}
}
