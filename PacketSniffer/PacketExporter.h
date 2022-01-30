#pragma once
#include "Singleton.h"
#include <string>
#include <iostream>
#include <fstream>


typedef unsigned char BYTE;

struct ExportedPacket {
	ExportedPacket(BYTE header, void* content, int size, bool isFinalSequence = false);
	~ExportedPacket();
	BYTE header;
	BYTE* content;
	int size;
	bool isFinalSequence;
};


/*
Responsible for logging the packets sent
*/
class PacketExporter : public CSingleton<PacketExporter>
{

public:
	PacketExporter(std::string& fileName);
	~PacketExporter();


	bool setFileExport(std::string& fileName);


	bool exportRecvPacket(ExportedPacket* p);
	bool exportSendPacket(ExportedPacket* p);

private:

	void exportPacket(std::ofstream& file, ExportedPacket* p);

	std::ofstream recvExportFile;
	std::ofstream sendExportFile;
};

