#include "stdafx.h"
#include "Communication.h"
#include "../common/Config.h"
#include <fstream>
#include "VMProtectSDK.h"


std::map<int, CCommunication::GetInstance> CCommunication::getRecvBuffer;

CCommunication::CCommunication() {
	curl_global_init(CURL_GLOBAL_ALL);
	curlMulti = curl_multi_init();
	maxID = 0;

	const char* cert = SSL_CERTIFICATE;
	const char* cert_key = SSL_CERTIFICATE_KEY;

	sslCert.data = (void*)cert;
	sslCert.len = strlen(cert);
	sslCert.flags = CURL_BLOB_COPY;

	sslKey.data = (void*)cert_key;
	sslKey.len = strlen(cert_key);
	sslKey.flags = CURL_BLOB_COPY;

	/*VMProtectFreeString(cert);
	VMProtectFreeString(cert_key);*/
}

CCommunication::~CCommunication()
{
	curl_multi_cleanup(curlMulti);
}

void CCommunication::Process()
{

	//Get requests
	int still_running;
	curl_multi_perform(curlMulti, &still_running);
	if (!still_running) {

		int msgq = 0;
		struct CURLMsg* m;
		m = curl_multi_info_read(curlMulti, &msgq);
		if (m && (m->msg == CURLMSG_DONE)) {
			CURL* e = m->easy_handle;
			int id;
			curl_easy_getinfo(e, CURLINFO_PRIVATE, &id);
			curl_multi_remove_handle(curlMulti, e);
			curl_easy_cleanup(e);

			GetInstance& info = getRecvBuffer.at(id);
			info.function.ExecuteCallback(id, &info.msgBuffer);

			getRecvBuffer.erase(id);
			if (id >= maxID) {
				--maxID;
			}
		}
	}

	//Websockets
	auto conIter = webSocketList.begin();
	while (conIter != webSocketList.end()) {
		WebSocketInfo& socket = conIter->second;

		//Check if is still conected, otherwise delete it

		std::string buffer;
		while (websocketHandler.Recv(socket.id, &buffer)) {
			DEBUG_INFO_LEVEL_3("Websocket with id=%d recived message: %s", socket.id,buffer.c_str());
			socket.recvCallback.ExecuteCallback(socket.id, &buffer, false);
		}
		if (!websocketHandler.IsConnected(socket.id)) {
			DEBUG_INFO_LEVEL_1("Websocket with id %d is not connected, removing it", socket.id);
			socket.recvCallback.Cleanup();
			webSocketList.erase(conIter++);
			continue;
		}
		++conIter;
	}


}

int CCommunication::GetRequest(std::string& url, ComCallbackFunction callback, int id)
{
	return GetRequest(url.c_str(), callback, id);
}

int CCommunication::GetRequest(const char* url, ComCallbackFunction callback, int id)
{
	if (!id) {
		++maxID;
		id = maxID;
	}


	getRecvBuffer.insert({ id,{id,callback} });
	CURL* eh = curl_easy_init();
	curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(eh, CURLOPT_URL, url);
	curl_easy_setopt(eh, CURLOPT_WRITEDATA, id);
	curl_easy_setopt(eh, CURLOPT_PRIVATE, id);
	curl_multi_add_handle(curlMulti, eh);

	return id;
}

int CCommunication::OpenWebsocket(const char* host, ComCallbackFunction callback)
{
	int id = websocketHandler.Connect(host);
	if (id == -1)
		return -1;
	webSocketList.insert({ id,WebSocketInfo(id,callback) });

	DEBUG_INFO_LEVEL_1("Websocket to %s created with ID %d", host, id);
	return id;
}

bool CCommunication::WebsocketSend(int id, const char* message)
{
	return websocketHandler.Send(id, message);
}

bool CCommunication::CloseWebsocket(int id)
{
	bool val = websocketHandler.Close(id);
	if (val) {
		webSocketList.at(id).recvCallback.Cleanup();
		webSocketList.erase(id);
	}
	return val;
}

int CCommunication::MainServerSetAuthKey()
{
	getJWTToken(&authKey);
	return 1;
}

int CCommunication::MainServerGetOffsets(std::map<int, DWORD>* bufferOffsets, const char* server)
{
	VMProtectBeginUltra("GetOffsets");
	if (authKey.size() < 1) {
		DEBUG_INFO_LEVEL_1("Missing auth key!");
		return 0;
	}

	Json::Value json_root;
	
	Json::Value json_post;
	json_post["hwid"] = getHWID();
	json_post["server"] = server;
	
	std::string url = WEB_SERVER_ADDRESS;
	url += OFFSETS_ENDPOINT;

	DEBUG_INFO_LEVEL_2("Requesting offsets from server");
	int val = MainServerPreformRequest(url, &json_root,json_post,1,1);
	if (!val) {
		DEBUG_INFO_LEVEL_1("Error performing Get offsets request!");
		return 0;
	}

#ifdef _DEBUG
	Json::StreamWriterBuilder builders;
	std::string str = Json::writeString(builders, json_root);
	DEBUG_INFO_LEVEL_2("Server offsets retrieved\n%s", str.c_str());
#endif

	auto content = json_root;
	for (Json::Value::ArrayIndex i = 0; i != json_root.size(); i++) {
		auto object = json_root[i];
		DWORD ikey = 0;
		DWORD address_num = 0;
		try{
			ikey = object["id"].asInt();
			address_num = object["address"].asInt();
		}
		catch (std::exception& e) {
			DEBUG_INFO_LEVEL_1("Error parsing server json offset at index %d",i);
			continue;
		}
		bufferOffsets->insert({ ikey,address_num });
	}
	return 1;
}

bool CCommunication::IsPremiumUser()
{
	VMProtectBeginUltra("Premium");
	Json::Value json_root;
	
	std::string url = WEB_SERVER_ADDRESS;
	url += USER_ENDPOINT;

	if (!MainServerPreformGetRequest(url, &json_root,1)) {
		DEBUG_INFO_LEVEL_1("Error performing Getting user information");
		return false;
	}

	try {
		auto json_premium = json_root.get("isPremium", 0);
		if (json_premium.asInt() == 1) {
			DEBUG_INFO_LEVEL_1("User is premium!");
			return true;
		}
		else {
			DEBUG_INFO_LEVEL_1("User is not premium!");
			return false;
		}
	}
	catch (std::exception& e) {
		DEBUG_INFO_LEVEL_1("Error getting premium status");
		return false;
	}
}

void CCommunication::clearMemoryCertificates()
{
	sslKey = { 0 };
	sslCert = { 0 };
}


int CCommunication::MainServerRequestAuthKey(std::string* key)
{
	getJWTToken(key);
	return 1;
}

int CCommunication::MainServerPreformRequest(std::string& url, Json::Value* json_root, Json::Value& post_fields, bool use_api_key, bool use_https)
{
	VMProtectBeginUltra("PreformRequest");
	CURL* curl = curl_easy_init();
	CURLcode res;


	std::string readBuffer;
	struct curl_slist* headers = nullptr;
	if (use_api_key) {
		std::string header_fields;
		header_fields += "Authorization: Bearer ";
		header_fields += authKey;
		headers = curl_slist_append(headers, header_fields.c_str());
	}

	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "charset: utf-8");

	if (headers != nullptr) {
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	}
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteSingleThreadback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, FALSE);

	if (use_https) {
		curl_easy_setopt(curl, CURLOPT_SSLCERT_BLOB, &sslCert);
		curl_easy_setopt(curl, CURLOPT_SSLKEY_BLOB, &sslKey);
		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
	}

	std::string post_request;
	if (post_fields.size() > 0) {
		Json::StreamWriterBuilder builders;
		post_request = Json::writeString(builders, post_fields);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_request.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_STDERR, stdout);

	res = curl_easy_perform(curl);
	if (headers != nullptr) {
		curl_slist_free_all(headers);
	}
	if (res != CURLE_OK) {
		DEBUG_INFO_LEVEL_1("Error on curl_easy_perform, error_code=%d",res);
		curl_easy_cleanup(curl);
		return 0;
	}

	long answer_code;

	//Check status code
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &answer_code);

	//Cleanup
	curl_easy_cleanup(curl);

	//Parse response to json object
	Json::CharReaderBuilder builder;
	Json::CharReader* json_reader = builder.newCharReader();
	std::string errors = "";

	bool val = json_reader->parse(readBuffer.data(), readBuffer.data() + readBuffer.length(), json_root, &errors);
	if (val) {
		//Check error code and print errors
		Json::StreamWriterBuilder builders;

		if (answer_code != 200) {
			std::string message = Json::writeString(builders, json_root);
			DEBUG_INFO_LEVEL_1("Error performing request, error_code: %d, message: \n%s", answer_code, message.c_str());
			return 0;
		}
	}
	else {
		std::string answer(readBuffer.data(), readBuffer.length());
		if (answer_code != 200) {
			DEBUG_INFO_LEVEL_1("Error performing request, error_code: %d, message: \n%s", answer_code, answer.c_str());
			return 0;
		}
	}

	return 1;
}

int CCommunication::MainServerPreformGetRequest(std::string& url, Json::Value* response, bool use_api_key)
{
	Json::Value post_fields;
	return MainServerPreformRequest(url, response, post_fields,1, use_api_key);
}


size_t CCommunication::WriteCallback(void* contents, size_t size, size_t nmemb, int id)
{
	size_t realsize = size * nmemb;
	getRecvBuffer.at(id).msgBuffer.append((const char*)contents, realsize);
	return realsize;
}

size_t CCommunication::WriteSingleThreadback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

