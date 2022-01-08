#pragma once
#include "stdafx.h"
#include <curl\curl.h>
#include <curl\multi.h>
#include <json/json.h>
#include <string>
#include <map>
#include "Singleton.h"
#include "PythonUtils.h"
#include "../common/utils.h"
#include "WebsocketHandler.h"

#define CURL_STATICLIB

typedef void(*tGetCallback)(int id, std::string* buffer);

struct ComCallbackFunction {
	ComCallbackFunction(PyObject* pyFunc) : pyFunction(pyFunc){
		isCFunc = false;
	}

	ComCallbackFunction(tGetCallback cFunc) : cFunction(cFunc) {
		isCFunc = true;
	}

	inline void ExecuteCallback(int id, std::string* buffer,bool decrefPy=true) {
		if (isCFunc) {
			cFunction(id, buffer);
		}
		else {
			if (PyCallable_Check(pyFunction)) {
				PyObject* val = Py_BuildValue("(is)", id,buffer->c_str());
				PyObject_CallObject(pyFunction, val);
				Py_XDECREF(val);
				if (decrefPy) {
					Py_XDECREF(pyFunction);
				}
			}
			else {
				DEBUG_INFO_LEVEL_2("Callback is not a python function");
			}
		}
	}

	void Cleanup() {
		if (isCFunc) {
			Py_XDECREF(pyFunction);
		}
	}

	bool isCFunc;
	union {
		tGetCallback	cFunction;
		PyObject*		pyFunction;
	};
};

class CCommunication : public CSingleton<CCommunication>
{
	struct GetInstance {
		int id;
		ComCallbackFunction function;
		std::string msgBuffer;
	};
	struct WebSocketInfo {
		WebSocketInfo(int id, ComCallbackFunction callback) :recvCallback(callback),id(id) { 

		}
		ComCallbackFunction recvCallback; //Called from main thread
		int id;
	};

public:

	CCommunication();
	~CCommunication();

	void Process();

	//Multi-threaded - communication with remote servers
	int GetRequest(std::string& url, ComCallbackFunction callback,int id=0);
	int GetRequest(const char* url, ComCallbackFunction callback, int id = 0);

	//Websockets
	int OpenWebsocket(const char* host, ComCallbackFunction callback);
	bool WebsocketSend(int id,const char* message);
	bool CloseWebsocket(int id);



	//Blocking requests - main server
	int MainServerSetAuthKey(); //Handle request, and write key
	int MainServerGetOffsets(std::map<int,DWORD>* bufferOffsets, const char* server = "GF");
	bool IsPremiumUser();

	//Once called the certificates cannot be setup again
	void clearMemoryCertificates();

private:

	//Blocking requests
	int MainServerRequestAuthKey(std::string* key); //Not used

	int MainServerPreformRequest(std::string& url, Json::Value* response, Json::Value& post_fields, bool use_api_key=true, bool use_https=false);
	int MainServerPreformGetRequest(std::string& url, Json::Value* response, bool use_api_key = true);

private:

	//Used by curl
	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, int id);
	static size_t WriteSingleThreadback(void* contents, size_t size, size_t nmemb, void* userp);


private:
	CURLM* curlMulti;
	curl_blob sslCert;
	curl_blob sslKey;


	//To handle non blocking get requests
	int maxID;
	static std::map<int, GetInstance> getRecvBuffer;

	//Websockets
	CWebSocketHandler websocketHandler;
	std::map<int, WebSocketInfo> webSocketList;


	//Exlib server authkey
	std::string authKey;
};

