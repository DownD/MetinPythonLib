#include "stdafx.h"
#include "App.h"
#include "PythonModule.h"
#include "Memory.h"
#include "InstanceManager.h"
#include "Player.h"
#include "Background.h"
#include "NetworkStream.h"
#include "Communication.h"
#include "../common/Config.h"

HMODULE hDll = 0;


bool CApp::__AppProcess(ClassPointer p) {
	CMemory& memory = CMemory::Instance();

	if (!passed) {
		passed = true;
		initMainThread();
		DEBUG_INFO_LEVEL_1("Main Objects loaded!");
	}

	if (!mainScriptExec && passed) {
		CNetworkStream& net = CNetworkStream::Instance();
		if (net.GetCurrentPhase() >=PHASE_LOADING) {
			mainScriptExec = true;
			executePythonFile("script.py");
		}
	}

	CCommunication& c = CCommunication::Instance();
	c.Process();

	return memory.callProcess(p);
}

void CApp::init() {
#ifdef _DEBUG
	SetupConsole();
	setDebugStreamFiles();
	DEBUG_INFO_LEVEL_1("Dll Loaded From %s", getDllPath());
#endif

#ifdef _DEBUG_FILE
	setDebugStreamFiles();
	SetupDebugFile();
#endif
	static CBackground bck = CBackground();
	static CCommunication coms; //Weird compile error with constructor
	static CInstanceManager mgr = CInstanceManager();
	static CPlayer pl = CPlayer();
	static CNetworkStream ns = CNetworkStream();
	static CMemory memory = CMemory();

	DEBUG_INFO_LEVEL_1("Loaded Objects");

#ifdef GET_ADDRESS_FROM_SERVER
	if (!coms.MainServerSetAuthKey()) {
		MessageBoxA(NULL, "Error while connecting to the server.", "Authentication error", MB_OK);
		return;
	}
	DEBUG_INFO_LEVEL_1("Authentication was sucessfull");
#endif

	memory.setupPatterns(hDll);
#ifdef _DEBUG
	system("pause");
#else
	//Sleep(1000);
#endif

	DEBUG_INFO_LEVEL_1("Patterns have been set sucessfully");
	if (memory.setupProcessHook()) {
		DEBUG_INFO_LEVEL_1("Process Hook sucessfull");
	}
	else {
		DEBUG_INFO_LEVEL_1("Error on process hook");
	}
}

void CApp::initMainThread() {
	CMemory& memory = CMemory::Instance();
	initModule();
	initPythonModules();
	memory.setupHooks();
}

void CApp::SetupConsole()
{
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
	freopen("CONOUT$", "wb", stderr);
	freopen("CONIN$", "rb", stdin);
	SetConsoleTitle("Debug Console");

}

void CApp::SetupDebugFile()
{
	std::string log_path(getDllPath());
	log_path += "ex_log.txt";
	freopen(log_path.c_str(), "wb", stdout);
	freopen(log_path.c_str(), "wb", stderr);
}


void CApp::initPythonModules() {
	CBackground& bck = CBackground::Instance();
	CInstanceManager& mgr = CInstanceManager::Instance();
	CPlayer& pl = CPlayer::Instance();
	CNetworkStream& ns = CNetworkStream::Instance();
	
	bck.importPython();
	mgr.importPython();
	pl.importPython();
	ns.importPython();
}

void CApp::exit() {
	DEBUG_INFO_LEVEL_1("LEAVING!");
	/*CMemory& memory = CMemory::Instance();
	CBackground& bck = CBackground::Instance();
	CInstanceManager& mgr = CInstanceManager::Instance();
	CPlayer& pl = CPlayer::Instance();
	CNetworkStream& ns = CNetworkStream::Instance();
	memory.~CMemory();
	bck.~CBackground();
	mgr.~CInstanceManager();
	pl.~CPlayer();
	ns.~CNetworkStream();*/

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	FreeConsole();
}

void CApp::setSkipRenderer()
{
	CMemory& mem = CMemory::Instance();
	mem.setSkipRenderer();
}

void CApp::unsetSkipRenderer()
{
	CMemory& mem = CMemory::Instance();
	mem.unsetSkipRenderer();
}

CApp::CApp()
{

	mainScriptExec = false;
	passed = false;

}

CApp::~CApp()
{
	exit();
}
