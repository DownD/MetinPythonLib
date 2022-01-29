#include "../common/Offsets.h"
#include "../common/Patterns.h"
#include "DetoursHook.h"
#include <exception>
#include "PacketExporter.h"

/*typedef bool(__thiscall* tCheckPacket)(ClassPointer classPointer, BYTE* packetHeader);
typedef bool(__thiscall* tPeek)(ClassPointer classPointer, int len, void* buffer);
typedef bool(__thiscall* tSendPacket)(ClassPointer classPointer, int size, void* buffer);*/


HANDLE threadID;
HMODULE hDll;

// Hook functions
DetoursHook<tSendPacket>* sendHook;
DetoursHook<tCheckPacket>* checkPacketHook;
DetoursHook<tRecvPacket>* recvHook;
DetoursHook<tSendSequencePacket>* sendSequenceHook;

BYTE lastPacket;
std::vector<ExportedPacket> sendPacketQueue;

bool __fastcall __CheckPacket(ClassPointer classPointer, DWORD EDX, BYTE* header){

	bool result = checkPacketHook->originalFunction(classPointer, header);
	if (result) {
		lastPacket = *header;
	}
	return result;
}

bool __fastcall __SendPacket(ClassPointer classPointer, DWORD edx, int size, void* buffer) {

	bool result = sendHook->originalFunction(classPointer, size, buffer);
	if (result) {
		sendPacketQueue.push_back(ExportedPacket(lastPacket, buffer, size, true));
	}
	return result;
}

bool __fastcall __RecvPacket(ClassPointer p, DWORD edx, int size, void* buffer) {

	bool result = recvHook->originalFunction(p, size, buffer);
	if (result) {
		PacketExporter::Instance().exportRecvPacket(ExportedPacket(lastPacket, buffer, size, true));
	}
}

bool __fastcall __SendSequencePacket(DWORD classPointer)
{
	bool result = sendSequenceHook->originalFunction(classPointer);
	if (result) {
		ExportedPacket& p = sendPacketQueue.at(sendPacketQueue.size());
		p.isFinalSequence = true;
		for (ExportedPacket& p : sendPacketQueue) {
			PacketExporter::Instance().exportSendPacket(p);
		}
	}
	sendPacketQueue.clear();
	return result;
}



void SetupConsole()
{
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
	freopen("CONOUT$", "wb", stderr);
	freopen("CONIN$", "rb", stdin);
	SetConsoleTitle("Debug Console");

}

int main() {

	SetupConsole();
	Patterns* patternFinder = 0;
	try {
		patternFinder = new Patterns(hDll);
	}
	catch (std::exception& e) {
		MessageBox(NULL, e.what(), "Critical Error", MB_OK);
		return false;
	}

	tCheckPacket fCheckPacket = (tCheckPacket)patternFinder->GetPatternAddress(&memPatterns.at(CHECK_PACKET_FUNCTION));
	tSendPacket fSendPacket = (tSendPacket)patternFinder->GetPatternAddress(&memPatterns.at(SEND_FUNCTION));
	tSendSequencePacket fSendSequencePacket = (tSendSequencePacket)patternFinder->GetPatternAddress(&memPatterns.at(SENDSEQUENCE_FUNCTION));
	void* recvAddr = patternFinder->GetPatternAddress(&memPatterns.at(RECV_FUNCTION));
	void* recvAddr = getRelativeCallAddress(recvAddr);

	sendSequenceHook = new DetoursHook<tSendSequencePacket>(fSendSequencePacket, __SendSequencePacket);
	sendHook = new DetoursHook<tSendPacket>(fSendPacket, __SendPacket);
	checkPacketHook = new DetoursHook<tCheckPacket>(fCheckPacket, __SendPacket);
	recvHook = new DetoursHook<tRecvPacket>((tRecvPacket)recvAddr, __RecvPacket);

	sendSequenceHook->HookFunction();
	sendHook->HookFunction();
	checkPacketHook->HookFunction();
	recvHook->HookFunction();
}




BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{


	//  Perform global initialization.
	char test[256] = { 0 };
	//DLLArgs* args = (DLLArgs*)Reserved;
	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:
		if (Reserved) {
			//DLLArgs* args = (DLLArgs*)Reserved;
			//setDllPath(args->path);
		}
		else {
			GetModuleFileNameA(hDllHandle, test, 256);
			setDllPath(test);
		}
		hDll = (HMODULE)hDllHandle;
		//threadID = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
		main();
		break;

		/*case DLL_PROCESS_DETACH:
			CApp & app = CApp::Instance();
			PRINT("DETACHED CALLED");
			app.exit();
			break;*/
	}

	return true;
}