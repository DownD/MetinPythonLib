#pragma once
#include <Windows.h>
#include "Patterns.h"
#include "JMPStartFuncHook.h"
#include <stdio.h>
#include <intrin.h>
#include "ReturnHook.h"
#include <map>
#include "PythonModule.h"
#include "PythonUtils.h"

HANDLE threadID;
HMODULE hDLL;
Hook* sendHook = 0;
Hook* recvHook = 0;
Hook* getEtherPacketHook = 0;

struct Packet;
struct PlayerCreatePacket;
void logPacket(Packet * packet);

std::map<DWORD,DWORD> instances;

PyObject* packet_mod;
PyObject* instanceList;



enum {
	PHASE_LOADING = 4
};


namespace PacketHeaders{
	enum{
		HEADER_GC_CHARACTER_ADD = 1,
		HEADER_GC_CHARACTER_DEL = 2,
		HEADER_GC_PHASE = 0xFD
	};
}


using namespace PacketHeaders;

struct Pattern {
	Pattern(int ofset, const char* pat,const char*masc) { offset = ofset; pattern = pat; mask=masc; }
	int offset;
	const char* pattern;
	const char* mask;
};

struct Packet {
	BYTE header;
	int data_size;
	BYTE* data;
};


struct EterFile {
	void* data;
	int size;
	std::string name;
};

bool getTrigger = false;
EterFile eterFile = { 0 };

#pragma pack(push, 1)
struct CMappedFile {
	void *v_table;
	//int				m_mode;
	//char			m_filename[MAX_PATH + 1];
	//HANDLE			m_hFile;
	BYTE		uknown[280]; 
	DWORD		m_dwSize;//0x11C needs fix

	BYTE*		m_pbBufLinkData;
	DWORD		m_dwBufLinkSize;

	BYTE*		m_pbAppendResultDataBlock;
	DWORD		m_dwAppendResultDataSize;

	DWORD		m_seekPosition;
	HANDLE		m_hFM;
	DWORD		m_dataOffset;
	DWORD		m_mapSize;
	LPVOID		m_lpMapData;
	LPVOID		m_lpData;

	void *	m_pLZObj;
};




template<class T>
void fillPacket(Packet*p,T* _struct) {
	ZeroMemory(_struct, sizeof(T));
	int size = min(p->data_size, sizeof(T));
	memcpy(_struct, p->data, size);
}

struct ChangPhasePacket
{
	BYTE phase;
};


struct DeletePlayerPacket
{
	DWORD	dwVID;
};


struct PlayerCreatePacket {

	//DEFAULT
	/*DWORD	dwVID;
#if defined(WJ_SHOW_MOB_INFO)
	DWORD	dwLevel;
	DWORD	dwAIFlag;
#endif
	float	angle;
	long	x;
	long	y;
	long	z;
	BYTE	bType;
	WORD	wRaceNum;
	BYTE	bMovingSpeed;
	BYTE	bAttackSpeed;
	BYTE	bStateFlag;
	DWORD	dwAffectFlag[2];*/
	DWORD dwVID;
	DWORD uknown1;
	DWORD uknown2;
	DWORD uknown3;
	CHAR uknown4;
	DWORD uknown5;
	DWORD uknown6;
	BYTE uknown7;
	BYTE uknown8;
	CHAR uknown9;
	CHAR uknown10;
	DWORD uknown11;
	DWORD uknown12;

};
#pragma pack(pop)




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

namespace memory_patterns {

	//wrapper
	//https://gyazo.com/0b5f3bc5a90938b7574db9b9a7020d50
	//Pattern sendFunction = Pattern(1, "\xe8\x00\x00\x00\x00\x84\xc0\x75\x00\x68\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x83\xc4\x00\x32\xc0\x5e\x8b\xe5\x5d\xc2\x00\x00\xcc", "x????xxx?x????x????xx?xxxxxxx??x");
	Pattern sendFunction = Pattern(0, "\x55\x8b\xec\x56\x8b\xf1\x57\x8b\x7d\x00\x8b\x56", "xxxxxxxxx?xx");
	Pattern recvFunction = Pattern(0, "\x55\x8b\xec\x56\x57\xff\x75\x00\x8b\x7d", "xxxxxxx?xx");

	//https://gyazo.com/509c7cc703d48fb8e31bea3150687c07
	Pattern getEtherPackFunction = Pattern(0, "\x55\x8b\xec\x56\x8b\x75\x00\x57\xff\x75\x00\x8b\xf9\x56", "xxxxxx?xxx?xxx");
	//Pattern eterClassPointer = Pattern(2, "\x8b\x0d\x00\x00\x00\x00\x8d\x85\x00\x00\x00\x00\x83\xc4\x00\x6a\x00\x68\x00\x00\x00\x00\x50\x56", "xx????xx????xx?x?x????xx");
	//Pattern cMapFileConstructerPointer = Pattern(16, "\x83\xf8\x00\x0f\x87\x00\x00\x00\x00\x8d\x8d", "xx?xx????xx");
}

void logPacket(Packet * packet) {
	printf("Header: %d\n",packet->header);
	printf("Content-Bytes: ");
	for (int i = 0; i < packet->data_size; i++) {
		printf("%#x ", packet->data[i]);
	}
	printf("\n\n");

}
#pragma optimize( "", off )
void __cdecl SendPacket(void* buffer, int size) {
	//Packet packet(size, (BYTE*)buffer);
	//logPacket(&packet);
}
DWORD __stdcall _GetEter(DWORD return_value,CMappedFile* file, const char* fileName, void** buffer, const char* uknown, bool uknown_2) {
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

//THREADING MITGH BE A PROBLEM
void __declspec(naked) GetEter(CMappedFile& file, const char* fileName, void* buffer, const char* uknown, bool uknown_2) {
	__asm {
		POP EDX
		PUSH eax
		PUSH edx
		JMP _GetEter
	}
}

bool __stdcall RecvPacket(int size, void* buffer) {
	DWORD return_value;
	__asm {
		MOV return_value, eax
	}
	if (return_value != 0 && size >0) {
		BYTE* bbuffer = (BYTE*)buffer;
		Packet packet;
		packet.header = *bbuffer;
		packet.data_size = size - 1;
		packet.data = (BYTE*)((int)bbuffer + 1);
		//logPacket(&packet);
		switch (packet.header) {
		case HEADER_GC_CHARACTER_ADD: {
			if (packet.data_size == 0) {
				break;
			}
			PlayerCreatePacket instance;
			fillPacket(&packet, &instance);
			PyObject* vid = PyLong_FromLong(instance.dwVID);
			PyDict_SetItem(instanceList, vid, vid);
			break;
		}
		case HEADER_GC_CHARACTER_DEL: {
			if (packet.data_size == 0) {
				break;
			}
			DeletePlayerPacket instance_;
			fillPacket(&packet, &instance_);
			PyObject* vid = PyLong_FromLong(instance_.dwVID);
			PyDict_DelItem(instanceList, vid);
			break;
		}
		case HEADER_GC_PHASE: {
			ChangPhasePacket phase;
			fillPacket(&packet, &phase);
			if (phase.phase == PHASE_LOADING){
				PyDict_Clear(instanceList);

			}
			break;

		}
		}
	}
	return (bool)return_value;

}

#pragma optimize( "", on )



void SetupConsole()
{
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
	freopen("CONOUT$", "wb", stderr);
	freopen("CONIN$", "rb", stdin);
	SetConsoleTitle("Debug Console");
}



void Leave() {
	if (getEtherPacketHook)
		getEtherPacketHook->UnHookFunction();
	if(sendHook)
		sendHook->UnHookFunction();
	if (recvHook)
		recvHook->UnHookFunction();

	delete sendHook;
	delete recvHook;

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	FreeConsole();
}

void initModule() {
	auto send = memory_patterns::sendFunction;
	auto recv = memory_patterns::recvFunction;
	auto getEther = memory_patterns::getEtherPackFunction;
	Patterns patternFinder(hDLL);

	void* recvAddr = patternFinder.GetPatternAddress(recv.pattern, recv.mask, recv.offset);
	void* sendAddr = patternFinder.GetPatternAddress(send.pattern, send.mask, send.offset);
	void* getEtherPackAddr = patternFinder.GetPatternAddress(getEther.pattern, getEther.mask, getEther.offset);

#ifdef _DEBUG
	printf("Send Addr: %#x\n", sendAddr);
	printf("Recv Addr: %#x\n", recvAddr);
	printf("GetEterFunction Addr: %#x\n", getEtherPackAddr);
	system("pause");
#endif



	if (sendAddr == NULL) {
		system("pause");
		return Leave();
	}

	char dllPath[MAX_PATH] = { 0 };
	getCurrentPath(hDLL, dllPath, MAX_PATH);

	packet_mod = Py_InitModule("net_packet", s_methods);
	instanceList = PyDict_New();
	PyModule_AddObject(packet_mod, "InstancesList", instanceList);
	PyModule_AddStringConstant(packet_mod, "PATH", dllPath);

	getEtherPacketHook = new ReturnHook(getEtherPackAddr, GetEter, 7, 5);
	recvHook = new ReturnHook(recvAddr, RecvPacket, 5,2);
	recvHook->HookFunction();

	executeScript("script.py", dllPath);
	getEtherPacketHook->HookFunction();
	dllPath[0] = 0;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
#ifdef _DEBUG
	SetupConsole();
#endif
	initModule();
	return true;
}

BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{


	//  Perform global initialization.

	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:

		hDLL = (HMODULE)hDllHandle;
		threadID = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
		break;

	case DLL_PROCESS_DETACH:
		Leave();
		break;
	}

	return true;
}

