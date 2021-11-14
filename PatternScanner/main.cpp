#pragma once
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <io.h>
#include "../common/Offsets.h"
#include "../common/Patterns.h"
#include "../common/Config.h"
#include <json/json.h>
#include <sstream>
#include <curl\curl.h>

#define CURL_STATICLIB
#define PRINT(...); {{printf(__VA_ARGS__); printf("\n");fflush(stdout);}}

HANDLE threadID;
HMODULE hDll;

struct DLLArgs {
	int size;
	int reserved;
	char path[256];
};

void SetupConsole()
{
	AllocConsole();
	freopen("CONOUT$", "wb", stdout);
	freopen("CONOUT$", "wb", stderr);
	freopen("CONIN$", "rb", stdin);
	SetConsoleTitle("Debug Console");

}



int get_api_key(std::string* key) {
	*key = "s3msAiwKABfS0+8KxztaZUKhisC/F7PPJB8SiLOvalk=";
//*key = "dsfdsf";
	return 1;
}

size_t WriteSingleThreadback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	SetupConsole();
	//MessageBox(NULL, "Success Loading", "SUCCESS", MB_OK);
	Patterns* patternFinder = 0;
	Pattern global("GLOBAL_PATTERN", GLOBAL_PATTERN_OFFSET, GLOBAL_PATTERN, GLOBAL_PATTERN_MASK);
	try {
		patternFinder = new Patterns(hDll);
	}
	catch (std::exception& e) {
		MessageBox(NULL, e.what(), "Critical Error", MB_OK);
		return false;
	}

	std::map<int, std::pair<int,std::string>> addresses;

	for (auto & x : memPatterns) {
		int addr = (int)patternFinder->GetPatternAddress(&x.second);
		addr -= (int)patternFinder->GetStartModuleAddress();
		addresses.insert({ x.first,{addr,std::string(x.second.name)} });
	}

#ifdef DUMP_TO_FILE
	std::ofstream myfile;
	std::string path= getDllPath();
	path += ADRESS_FILE_NAME;
	myfile.open(path.c_str());
	for (auto& x : addresses) {
		myfile << std::dec << x.first << "\t0x" << std::hex << std::setfill('0') << std::setw(6) << x.second.first << "\t" << x.second.second << "\n";
	}
	PRINT("Addresses wrote to %s", path.c_str());
	myfile.close();
#endif

#ifdef SEND_TO_SERVER
	Json::Value root;
	for (auto& x : addresses) {
		std::stringstream address;
		address << "0x" << std::hex << std::setfill('0') << std::setw(6) << x.second.first;

		root[std::to_string(x.first)] = address.str();
	}
	PRINT("JSON created");

	Json::StreamWriterBuilder builders;
	const std::string post_request = Json::writeString(builders, root);
	

	CURLcode ret;
	CURL* hnd;
	struct curl_slist* slist1;

	std::string readBuffer;
	Json::Value json_root;//Answer

	slist1 = NULL;
	slist1 = curl_slist_append(slist1, "Content-Type: application/json");
	std::string auth_header;
	std::string key;

	if (get_api_key(&key)) {
		auth_header += "api_key: ";
		auth_header += key;
		slist1 = curl_slist_append(slist1, auth_header.c_str());
	}
	else {
		PRINT("API key not set");
		return 0;
	}

	PRINT("Sending information to server");

	std::string url = WEB_SERVER_ADDRESS;
	url += UPDATE_OFFSETS_ENDPOINT;

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, post_request.c_str());
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteSingleThreadback);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &readBuffer);

	ret = curl_easy_perform(hnd);
	if (ret != CURLE_OK) {
		PRINT("Error on curl_easy_perform, error_code=%d, url=%s", ret, url.c_str());
		curl_easy_cleanup(hnd);
		return 0;
	}
	curl_easy_cleanup(hnd);
	curl_slist_free_all(slist1);

	Json::CharReaderBuilder builder;
	Json::CharReader* json_reader = builder.newCharReader();
	std::string errors;

	bool val = json_reader->parse(readBuffer.data(), readBuffer.data() + readBuffer.length(), &json_root, &errors);
	if (!val) {
		PRINT("Error doing parse of response, errors:%s", errors.c_str());
		return 0;
	}

	std::string value = json_root.get("status", "error").asString();
	if (value == "error") {
		std::string message = json_root.get("message", "Uknown error.").asString();
		PRINT("Error performing request, message: %s", message.c_str());
		return 0;
	}

	PRINT("Server adresses updated!");
#endif

	delete patternFinder;
	return 1;
}


BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{


	//  Perform global initialization.
	char test[256] = { 0 };
	DLLArgs* args = (DLLArgs*)Reserved;
	switch (nReason)
	{
	case DLL_PROCESS_ATTACH:
		if (Reserved) {
			DLLArgs* args = (DLLArgs*)Reserved;
			setDllPath(args->path);
		}
		else {
			GetModuleFileNameA(hDllHandle, test, 256);
			setDllPath(test);
		}
		hDll = (HMODULE)hDllHandle;
		//threadID = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
		ThreadProc(0);
		break;

		/*case DLL_PROCESS_DETACH:
			CApp & app = CApp::Instance();
			PRINT("DETACHED CALLED");
			app.exit();
			break;*/
	}

	return true;
}

