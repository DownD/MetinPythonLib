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


/*
This project serves the purpose of just testing the patterns and update them on a remote server is needed.
In case some pattern is broken, it will display that it couldn't be find.
*/


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


//The API key that has requeired previleges to update addresses
void get_api_key(std::string* key) {
	*key = "api_key";
}

size_t WriteSingleThreadback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	SetupConsole();
	Patterns* patternFinder = 0;
	try {
		patternFinder = new Patterns(hDll);
	}
	catch (std::exception& e) {
		MessageBox(NULL, e.what(), "Critical Error", MB_OK);
		return false;
	}

	// Dumps the adresses from the current process
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

	//Updates the addresses on a remote location
	//Create Json in the required format
	Json::Value root;
	for (auto& x : addresses) {
		Json::Value offset;
		offset["name"] = x.second.second;
		offset["id"] = x.first;
		offset["address"] = x.second.first;
		offset["server"] = "GF";

		root.append(offset);
	}
	PRINT("JSON created");

	Json::StreamWriterBuilder builders;
	const std::string post_request = Json::writeString(builders, root);
	PRINT("Request:\n %s", post_request.c_str());
	

	//Prepare curl to send data
	CURLcode ret;
	CURL* hnd;
	struct curl_slist* slist1;

	curl_blob sslCert;
	curl_blob sslKey;

	//Use an SSL certificate in order for the server to make sure is sent by this specific applciation
	sslCert.data = (void*)SSL_CERTIFICATE;
	sslCert.len = strlen(SSL_CERTIFICATE);
	sslCert.flags = CURL_BLOB_COPY;

	sslKey.data = (void*)SSL_CERTIFICATE_KEY;
	sslKey.len = strlen(SSL_CERTIFICATE_KEY);
	sslKey.flags = CURL_BLOB_COPY;


	std::string readBuffer;
	Json::Value json_root;//Answer

	slist1 = NULL;
	slist1 = curl_slist_append(slist1, "Content-Type: application/json");
	slist1 = curl_slist_append(slist1, "accept: application/json");
	std::string auth_header;
	std::string key;
	get_api_key(&key);

	PRINT("JWToken= %s", key.c_str());

	auth_header += "Authorization: Bearer ";
	auth_header += key;
	slist1 = curl_slist_append(slist1, auth_header.c_str());

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

	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, FALSE);

	curl_easy_setopt(hnd, CURLOPT_SSLCERT_BLOB, &sslCert);
	curl_easy_setopt(hnd, CURLOPT_SSLKEY_BLOB, &sslKey);


	ret = curl_easy_perform(hnd);
	PRINT("Message Sent");
	if (ret != CURLE_OK) {
		PRINT("Error on curl_easy_perform, error_code=%d, url=%s", ret, url.c_str());
		curl_easy_cleanup(hnd);
		return 0;
	}

	long answer_code;

	//Check status code
	curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &answer_code);
	curl_easy_cleanup(hnd);
	curl_slist_free_all(slist1);
	PRINT("Curl cleaned");

	Json::CharReaderBuilder builder;
	Json::CharReader* json_reader = builder.newCharReader();
	std::string errors = "";

	bool val = json_reader->parse(readBuffer.data(), readBuffer.data() + readBuffer.length(), &json_root, &errors);
	if (val) {
		//Check error code and print errors
		Json::StreamWriterBuilder builders2;

		if (answer_code != 200) {
			std::string message = Json::writeString(builders2, json_root);
			PRINT("Error performing request, error_code: %d, message: \n%s", answer_code, message.c_str());
			return 0;
		}
	}
	else {
		std::string answer(readBuffer.data(), readBuffer.length());
		if (answer_code != 200) {
			PRINT("Error performing request, error_code: %d, message: \n%s", answer_code, answer.c_str());
			return 0;
		}
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
		ThreadProc(0);
		break;

	}

	return true;
}

