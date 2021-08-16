#include "Communication.h"
#include "../common/Config.h"
#include <fstream>


std::map<int, CCommunication::GetInstance> CCommunication::getRecvBuffer;

CCommunication::CCommunication() {
	curl_global_init(CURL_GLOBAL_ALL);
	curlMulti = curl_multi_init();
	maxID = 0;
}

CCommunication::~CCommunication()
{

	curl_multi_cleanup(curlMulti);
}

void CCommunication::Process()
{
	int still_running;
	curl_multi_perform(curlMulti, &still_running);
	if (still_running)
		return;

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


int CCommunication::MainServerSetAuthKey()
{
	std::string path;
	auto val = getKeyPath(&path);
	if (val == S_OK) {
		//Open file and check for key
		std::ifstream in_f(path.c_str());
		if (in_f.good()) {
			std::string line;
			std::getline(in_f, line);
			authKey = line;
			in_f.close();

		}

		//Write a default key if it does not exist yet
		else {
			std::string key_write = DEFAULT_API_KEY;
			authKey = DEFAULT_API_KEY;
			std::ofstream f(path.c_str());
			if (f.good()) {
				f.write(key_write.data(), key_write.size());
				f.close();
			}
			else {
				DEBUG_INFO_LEVEL_1("Error opening api_key file");
				return 0;
			}
		}

		//Check if api_key is valid
		std::string url = WEB_SERVER_ADDRESS;
		url += TEST_AUTH_ENDPOINT;

		Json::Value tmp;
		if (MainServerPreformRequest(url, &tmp)) {
			DEBUG_INFO_LEVEL_2("Api key is set and accepted");
			return 1;
		}
		else {
			DEBUG_INFO_LEVEL_2("Current Api key is not accepted");
		}

		//PRESENT KEY IS NOT ELIGIBLE REQUEST A NEW ONE

		//Get request code
		std::string code = "";
		if (!MainServerRequestCode(&code)) {
			DEBUG_INFO_LEVEL_1("Unable to request auth code");
			return 0;
		}

		//Show login page on browser
		url = WEB_SERVER_ADDRESS;
		url += GET_LOGIN_ENDPOINT;
		url += "&auth_code=" + code;

		ShellExecute(0, 0, url.c_str() , 0, 0, SW_SHOW);

		DEBUG_INFO_LEVEL_1("Waiting for user to login...");
		//Request api key
		if (!MainServerRequestAuthKey(code, &authKey)) {
			DEBUG_INFO_LEVEL_1("Unable to request auth key");
			return 0;
		}
		
		DEBUG_INFO_LEVEL_1("Setting api_key at %s", path.c_str());
		std::ofstream f(path.c_str());
		f.write(authKey.data(),authKey.size());
		f.close();
	}
	else {
		DEBUG_INFO_LEVEL_1("Unable to get auth key path");
		return 0;
	}

	return 1;
}

int CCommunication::MainServerGetOffsets(std::map<int, DWORD>* bufferOffsets)
{
	if (authKey.size() < 1) {
		DEBUG_INFO_LEVEL_1("Missing auth key!");
		return 0;
	}

	Json::Value json_root;
	std::string url = WEB_SERVER_ADDRESS;
	url += OFFSETS_ENDPOINT;
	DEBUG_INFO_LEVEL_2("Requesting offsets from server");
	int val = MainServerPreformRequest(url, &json_root);
	if (!val) {
		DEBUG_INFO_LEVEL_1("Error performing Get offsets request!");
		return 0;
	}

#ifdef _DEBUG
	Json::StreamWriterBuilder builders;
	std::string str = Json::writeString(builders, json_root);
	DEBUG_INFO_LEVEL_2("Server offsets retrieved\n%s", str.c_str());
#endif

	auto content = json_root["content"];
	for (std::string & key : content.getMemberNames()) {
		int ikey = 0;
		DWORD address_num = 0;
		try{
			ikey = std::stoi(key);
			address_num = std::stoul(content[key].asString(), nullptr, 16);
		}
		catch (std::exception& e) {
			DEBUG_INFO_LEVEL_1("Key %s of Json file is not a number, proceding",key.c_str());
			continue;
		}
		bufferOffsets->insert({ ikey,address_num });
	}
	return 1;
}

int CCommunication::MainServerRequestCode(std::string* code)
{
	Json::Value json_root;
	std::string url = WEB_SERVER_ADDRESS;
	url += REQUEST_AUTH_ENDPOINT;
	int val = MainServerPreformRequest(url, &json_root);
	if (!val) {
		DEBUG_INFO_LEVEL_1("Error performing auth code request!");
		return val;
	}

	std::string content = json_root.get("content", "error").asString();
	if (content == "error") {
		DEBUG_INFO_LEVEL_1("Error no content in request!");
		return 0;
	}

	*code = content;

	return 1;
}

int CCommunication::MainServerRequestAuthKey(std::string& code,std::string* key)
{
	Json::Value json_root;
	std::string url = WEB_SERVER_ADDRESS;
	url += REQUEST_API_KEY_ENDPOINT;

	Json::Value args;
	args["auth_code"] = code;

	int val = MainServerPreformRequest(url, &json_root, args);
	if (!val) {
		DEBUG_INFO_LEVEL_1("Error performing auth key request!");
		return val;
	}

	Json::Value content = json_root.get("content", "error");
	if (content == "error") {
		DEBUG_INFO_LEVEL_1("Error no content in request!");
		return 0;
	}

	*key = content.get("apiKey","0").asString();

	return 1;
}

int CCommunication::MainServerPreformRequest(std::string& url, Json::Value* json_root, Json::Value& post_fields, bool use_api_key)
{
	CURL* curl = curl_easy_init();
	CURLcode res;

	std::string readBuffer;
	struct curl_slist* headers = nullptr;
	if (use_api_key) {
		std::string header_fields;
		header_fields += "api_key: ";
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

	std::string post_request;
	if (post_fields.size() > 0) {
		Json::StreamWriterBuilder builders;
		post_request = Json::writeString(builders, post_fields);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_request.c_str());
	}

	res = curl_easy_perform(curl);
	if (headers != nullptr) {
		curl_slist_free_all(headers);
	}
	if (res != CURLE_OK) {
		DEBUG_INFO_LEVEL_1("Error on curl_easy_perform, error_code=%d",res);
		curl_easy_cleanup(curl);
		return 0;
	}
	curl_easy_cleanup(curl);
	Json::CharReaderBuilder builder;
	Json::CharReader* json_reader = builder.newCharReader();
	std::string errors{};
	bool val = json_reader->parse(readBuffer.data(), readBuffer.data() + readBuffer.length(), json_root, &errors);
	if (!val) {
		DEBUG_INFO_LEVEL_1("Error doing parse of response, errors:%s", errors.data());
		return 0;
	}

	std::string value = json_root->get("status", "error").asString();
	if (value == "error") {
		std::string message = json_root->get("message", "Uknown error.").asString();
		DEBUG_INFO_LEVEL_1("Error performing request, message: %s", message.data());
		return 0;
	}

	return 1;
}

int CCommunication::MainServerPreformRequest(std::string& url, Json::Value* response, bool use_api_key)
{
	Json::Value post_fields;
	return MainServerPreformRequest(url, response, post_fields, use_api_key);
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

