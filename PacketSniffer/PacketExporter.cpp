#include "PacketExporter.h"
#include <iostream>
#include <iomanip>

PacketExporter::PacketExporter(std::string& fileName) {
	if (!setFileExport(fileName)) {
		printf("Error openeing files\n");
	}
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

	printf("Opening files\n");

	sendExportFile.open(fileName + "_send.csv");
	recvExportFile.open(fileName + "_recv.csv");

	bool is_open = sendExportFile.is_open() && sendExportFile.is_open();
	if (is_open) {
		recvExportFile << "header,size,bytes,isFinalSequence\n";
		sendExportFile << "header,size,bytes,isFinalSequence\n";
	}
	return sendExportFile.is_open() && sendExportFile.is_open();
}

bool PacketExporter::exportRecvPacket(ExportedPacket * p)
{
	exportPacket(recvExportFile, p);
	return true;
}

bool PacketExporter::exportSendPacket(ExportedPacket * p)
{
	exportPacket(sendExportFile, p);
	return true;
}

void PacketExporter::exportPacket(std::ofstream& file, ExportedPacket* p)
{
	file << std::to_string(p->header) << "," << std::to_string(p->size) << ",";
	for (int i = 0; i < p->size; i++) {
		file << std::hex << std::setw(2) << std::setfill('0') << (int)p->content[i] << " ";
	}
	file << "," << (int)p->isFinalSequence << std::endl;
	file.flush();
}

ExportedPacket::ExportedPacket(BYTE header, void* content, int size, bool isFinalSequence)
{
	this->size = size;
	if (size >0) {
		this->content = (BYTE*)malloc(size);
		memcpy(this->content, content, size);
	}
	else {
		this->content = 0;
	}
	this->header = header;
	this->isFinalSequence = isFinalSequence;
}

ExportedPacket::~ExportedPacket()
{
	if (size > 0) {
		free(this->content);
	}
}
