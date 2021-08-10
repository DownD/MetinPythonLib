#pragma once
#include <curl\curl.h>
#include <curl\multi.h>
#include <json/json.h>
#include <string>
#include <map>
#include "Singleton.h"
#include "PythonUtils.h"
#include "../common/utils.h"

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
				PyObject* val = Py_BuildValue("(is)", id,buffer->data());
				PyObject_CallObject(pyFunction, val);
				Py_XDECREF(val);
				if (decrefPy) {
					Py_DECREF(pyFunction);
				}
			}
			else {
				DEBUG_INFO_LEVEL_2("Callback is not a python function");
			}
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
public:

	CCommunication();
	~CCommunication();

	void Process();

	//Multi-threaded
	int GetRequest(std::string& url, ComCallbackFunction callback,int id=0);
	int GetRequest(const char* url, ComCallbackFunction callback, int id = 0);

	//Blocking requests - main server
	int MainServerSetAuthKey(); //Handle request, and write key
	int MainServerGetOffsets(std::map<int,DWORD>* bufferOffsets);


private:

	//Blocking requests
	int MainServerRequestCode(std::string* code);
	int MainServerRequestAuthKey(std::string &code, std::string* key);

	int MainServerPreformRequest(std::string& url, Json::Value* response, Json::Value& post_fields, bool use_api_key=true);
	int MainServerPreformRequest(std::string& url, Json::Value* response, bool use_api_key = true);

private:

	//Used by curl
	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, int id);
	static size_t WriteSingleThreadback(void* contents, size_t size, size_t nmemb, void* userp);

private:
	CURLM* curlMulti;

	//To handle non blocking get requests
	int maxID;
	static std::map<int, GetInstance> getRecvBuffer;

	std::string authKey;
};

