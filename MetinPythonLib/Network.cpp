#include "Network.h"
#include "PythonModule.h"

using namespace PacketHeaders;

void logPacket(Packet * packet) {
	printf("Header: %d\n", packet->header);
	printf("Content-Bytes: ");
	for (int i = 0; i < packet->data_size; i++) {
		printf("%#x ", packet->data[i]);
	}
	printf("\n\n");

}

DWORD __stdcall __RecvPacket(DWORD return_value,int size, void* buffer) {
	//DWORD return_value;
	//__asm {
	//	MOV return_value, eax
	//}
	if (return_value != 0 && size > 0) {
		BYTE* bbuffer = (BYTE*)buffer;
		Packet packet;
		packet.header = *bbuffer;
		packet.data_size = size - 1;
		packet.data = (BYTE*)((int)bbuffer + 1);
		logPacket(&packet);
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
			if (phase.phase == PHASE_LOADING) {
				PyDict_Clear(instanceList);

			}
			break;

		}
		}
	}
	return return_value;

}

void SendPacket(int size, void*buffer) {
	//Packet packet(size, (BYTE*)buffer);
	//logPacket(&packet);
}


void __SendPacket(int size, void*buffer){
}