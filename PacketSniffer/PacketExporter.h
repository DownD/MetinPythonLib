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
Responsible for logging the packets sent and recieved
This is a Singleton class
*/
class PacketExporter : public CSingleton<PacketExporter>
{

public:
	PacketExporter(std::string& fileName);
	~PacketExporter();


	//Set the name for the exported files
	bool setFileExport(std::string& fileName);


	//Writes the exported packet to a file
	bool exportRecvPacket(ExportedPacket* p);
	bool exportSendPacket(ExportedPacket* p);

private:

	void exportPacket(std::ofstream& file, ExportedPacket* p);

	std::ofstream recvExportFile;
	std::ofstream sendExportFile;
};

