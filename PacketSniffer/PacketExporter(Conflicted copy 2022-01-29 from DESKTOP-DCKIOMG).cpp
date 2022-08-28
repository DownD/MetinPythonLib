#include "PacketExporter.h"


PacketExporter::PacketExporter() {

}

PacketExporter::~PacketExporter()
{
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
	return sendExportFile.is_open() && sendExportFile.is_open();
}

ExportPacket::ExportPacket(BYTE header, void* content, int size)
{
	this->header = header;
	this->content = (BYTE*)content;
	this->size = size;
}
