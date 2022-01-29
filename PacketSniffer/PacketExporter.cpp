#include "PacketExporter.h"
#include <iostream>
#include <iomanip>

PacketExporter::PacketExporter(std::string& fileName) {
	setFileExport(fileName);
}

PacketExporter::~PacketExporter()
{
	recvExportFile.close();
	sendExportFile.close();
}

bool PacketExporter::setFileExport(std::string& fileName)
{
	if (sendExportFile.is_open()) {
		sendExportFile.close();
	}
	if (recvExportFile.is_open()) {
		recvExportFile.close();
	}

	sendExportFile.open(fileName + "_send.csv");
	recvExportFile.open(fileName + "_recv.csv");

	bool is_open = sendExportFile.is_open() && sendExportFile.is_open();
	if (is_open) {
		recvExportFile << "header,size,bytes";
		sendExportFile << "header,size,bytes";
	}
	return sendExportFile.is_open() && sendExportFile.is_open();
}

bool PacketExporter::exportRecvPacket(ExportedPacket p)
{
	exportPacket(recvExportFile, p);
	return true;
}

bool PacketExporter::exportSendPacket(ExportedPacket p)
{
	exportPacket(sendExportFile, p);
	return true;
}

void PacketExporter::exportPacket(std::ofstream& file, ExportedPacket& p)
{
	file << p.header << ",";
	for (int i = 0; i < p.size; i++) {
		file << std::hex << std::setfill('0') << std::setw(2) << p.content[i] << " ";
	}
	file << "," << (int)p.isFinalSequence << std::endl;
}

ExportedPacket::ExportedPacket(BYTE header, void* content, int size, bool isFinalSequence)
{
	this->header = header;
	this->content = (BYTE*)content;
	this->size = size;
	this->isFinalSequence = isFinalSequence;
}
