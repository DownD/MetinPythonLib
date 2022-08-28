#include "../common/Offsets.h"
#include "../common/Patterns.h"
#include "DetoursHook.h"
#include <exception>
#include "PacketExporter.h"


HANDLE threadID;
HMODULE hDll;

/*
This project is a packet sniffer that will hook only network functions (After decryption by the Client)
and will register them in a file.
*/


// Hooks variables
DetoursHook<tSendPacket>* sendHook;
DetoursHook<tCheckPacket>* checkPacketHook;
DetoursHook<tRecvPacket>* recvHook;
DetoursHook<tSendSequencePacket>* sendSequenceHook;


// Helper global variables to handle some specific packets
BYTE lastPacket;
std::vector<ExportedPacket*> sendPacketQueue;

bool __fastcall __CheckPacket(ClassPointer classPointer, DWORD EDX, BYTE* header){
	bool result = checkPacketHook->originalFunction(classPointer, header);
	if (result) {
		printf("CheckPacket header(%d)\n",*header);
		lastPacket = *header;
	}
	return result;
}

bool __fastcall __SendPacket(ClassPointer classPointer, DWORD edx, int size, void* buffer) {
	bool result = sendHook->originalFunction(classPointer, size, buffer);
	if (result) {
		printf("SendPacket size(%d)\n",size);
		sendPacketQueue.push_back(new ExportedPacket(lastPacket, buffer, size, true));
	}
	return result;
}

bool __fastcall __RecvPacket(ClassPointer p, DWORD edx, int size, void* buffer) {
	bool result = recvHook->originalFunction(p, size, buffer);
	if (result && size > 0 && lastPacket != 0) {
		printf("RecvPacket header size(%d)\n",size);
		ExportedPacket packet(lastPacket, buffer, size, true);
		PacketExporter& pe = PacketExporter::Instance();
		pe.exportRecvPacket(&packet);
	}
	return result;
}

bool __fastcall __SendSequencePacket(DWORD classPointer)
{
	bool result = sendSequenceHook->originalFunction(classPointer);
	if (result) {
		printf("SendSequence\n");
		if (sendPacketQueue.size() == 0) {
			return result;
		}
		ExportedPacket* p = sendPacketQueue.at(sendPacketQueue.size()-1);
		p->isFinalSequence = true;
		PacketExporter& pe = PacketExporter::Instance();
		for (ExportedPacket* p : sendPacketQueue) {
			pe.exportSendPacket(p);
			delete p;
		}
		sendPacketQueue.clear();
		return result;
	
	}
	for (ExportedPacket* p : sendPacketQueue) {
		delete p;
	}
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


// Get addresses from stored patterns
// Set's the hooks and start them
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
	std::string file_name = "packets";
	PacketExporter* p = new PacketExporter(file_name);

	tCheckPacket fCheckPacket = (tCheckPacket)patternFinder->GetPatternAddress(&memPatterns.at(CHECK_PACKET_FUNCTION));
	tSendPacket fSendPacket = (tSendPacket)patternFinder->GetPatternAddress(&memPatterns.at(SEND_FUNCTION));
	tSendSequencePacket fSendSequencePacket = (tSendSequencePacket)patternFinder->GetPatternAddress(&memPatterns.at(SENDSEQUENCE_FUNCTION));
	void* recvAddr = patternFinder->GetPatternAddress(&memPatterns.at(RECV_FUNCTION));
	recvAddr = getRelativeCallAddress(recvAddr);

	sendSequenceHook = new DetoursHook<tSendSequencePacket>(fSendSequencePacket, __SendSequencePacket);
	sendHook = new DetoursHook<tSendPacket>(fSendPacket, __SendPacket);
	checkPacketHook = new DetoursHook<tCheckPacket>(fCheckPacket, __CheckPacket);
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

	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:
		hDll = (HMODULE)hDllHandle;
		main();
		break;

	}
	return true;
}