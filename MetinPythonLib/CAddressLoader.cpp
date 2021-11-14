#include "stdafx.h"
#include "CAddressLoader.h"
#include "../common/Config.h"
#include "defines.h"
#include <sstream>
#include <string>
#include <fstream>
#include "Communication.h"

#ifdef USE_BUILTIN_PATTERNS
#include "../common/Offsets.h"
#endif

CAddressLoader::CAddressLoader()
{
}

CAddressLoader::~CAddressLoader()
{
}

bool CAddressLoader::setAddress(HMODULE hDll)
{
	Patterns* patternFinder = 0;
	Pattern global("GLOBAL_PATTERN", GLOBAL_PATTERN_OFFSET, GLOBAL_PATTERN, GLOBAL_PATTERN_MASK);
	try {
		patternFinder = new Patterns(hDll, &global);
	}
	catch (std::exception& e) {
		MessageBox(NULL, e.what(), "Critical Error Setting Pattern", MB_OK);
		return false;
	}

	void* baseDllAddress = patternFinder->GetStartModuleAddress();

#ifdef GET_ADDRESS_FROM_SERVER
	bool val = setAddressByServer(baseDllAddress);
	delete patternFinder;
	return val;
#endif

#ifdef GET_ADDRESS_FROM_FILE
	std::string temp = getDllPath();
	temp += ADRESS_FILE_NAME;
	bool val = setAddressByFile(temp.data(),baseDllAddress);
	delete patternFinder;
	return val;
#endif

#ifdef USE_BUILTIN_PATTERNS
	bool val = setAddressByPatterns(patternFinder);
	delete patternFinder;
	return val;
#endif


	return false;
}

void* CAddressLoader::GetAddress(int id)
{
	if (memoryAddress.find(id) == memoryAddress.end()) {
		//DEBUG_INFO_LEVEL_1("Error getting address by id %d", id);
		return 0;
	}
	else {
		void* result = (void*)memoryAddress.at(id);
		//DEBUG_INFO_LEVEL_1("Address ID:%d with address -> %#x", id, result);
		return result;
	}
}

bool CAddressLoader::setAddressByPatterns(Patterns* p)
{
#ifdef USE_BUILTIN_PATTERNS
	for (auto& pattern : memPatterns){
		int id = pattern.first;
		DWORD addr = (DWORD)p->GetPatternAddress(&pattern.second);
		memoryAddress.insert({ id,addr });
	}
	DEBUG_INFO_LEVEL_1("Address set using patterns");
#endif
	return true;
}

bool CAddressLoader::setAddressByFile(const char* path, void* baseDllAddress)
{
	std::ifstream f(path,std::ios::ate);
	if (!f.is_open()) {
		DEBUG_INFO_LEVEL_1("Cannot open address file path %s", path);
		return false;
	}
	std::string file_buffer;
	
	f.seekg(0, std::ios::end);
	file_buffer.reserve(f.tellg());
	f.seekg(0, std::ios::beg);

	file_buffer.assign((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());
	parseFileBuffer(file_buffer.data(), file_buffer.size(), baseDllAddress);

	DEBUG_INFO_LEVEL_1("%d addresses set using file at %s ",memoryAddress.size() ,path);
	return true;
}

bool CAddressLoader::setAddressByServer(void* baseDllAddress)
{
	CCommunication& coms = CCommunication::Instance();
	
	if (coms.MainServerGetOffsets(&memoryAddress)) {
		for (auto& key : memoryAddress) {
			key.second += (DWORD)baseDllAddress;
		}
		DEBUG_INFO_LEVEL_1("Address set by the server");
		return true;
	}
	return false;
}

void CAddressLoader::parseFileBuffer(const char* buffer, int size, void* baseDllAddress)
{
	std::stringstream ss;
	ss << buffer;
	std::string line;
	while (std::getline(ss, line)) {

		std::stringstream ss_line(line);
		std::string id;
		std::string address;
		if (std::getline(ss_line, id,'\t') ){
			if (std::getline(ss_line, address, '\t')) {
				int id_num = std::stoi(id);
				DWORD address_num = std::stoul(address,nullptr,16);
				memoryAddress.insert({ id_num,address_num + (DWORD)baseDllAddress });
				continue;
			}
		}
		DEBUG_INFO_LEVEL_1("Failed to parse the following line '%s' from address buffer",line);
	}
}
