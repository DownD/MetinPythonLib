#pragma once
#include <Windows.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream> 
#include <chrono>
#include <ctime>
#include <math.h>
#include <stdarg.h>
#include <chrono>


bool isDebugEnable();
void setDebugOn();
void setDebugOff();

#define SUBPATH_MAPS "Resources\\Maps\\"

#define ADRESS_FILE_NAME "addresses.csv"

#if defined(_DEBUG) || defined(_DEBUG_FILE)
#define DEBUG_INFO_LEVEL_1(...); {if(isDebugEnable()){printf("[%s] - ",serializeTimePoint(std::chrono::system_clock::now(),"%H:%M:%S").c_str()); printf(__VA_ARGS__); printf("\n");fflush(stdout);}}
#define DEBUG_INFO_LEVEL_2(...); {DEBUG_INFO_LEVEL_1(__VA_ARGS__);}
#define DEBUG_INFO_LEVEL_3(...); {DEBUG_INFO_LEVEL_1(__VA_ARGS__);}
#define DEBUG_INFO_LEVEL_4(...); {DEBUG_INFO_LEVEL_1(__VA_ARGS__);}
#define DEBUG_INFO_LEVEL_5(...); {DEBUG_INFO_LEVEL_1(__VA_ARGS__);}
#else
#define DEBUG_INFO_LEVEL_1(...); {}
#define DEBUG_INFO_LEVEL_2(...); {}
#define DEBUG_INFO_LEVEL_3(...); {}
#define DEBUG_INFO_LEVEL_4(...); {}
#define DEBUG_INFO_LEVEL_5(...); {}
#endif




typedef void (__stdcall *tTimerFunction)();
typedef std::chrono::time_point<std::chrono::system_clock> tTimePoint;

struct TPixelPosition {
	float x, y, z;
};

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string base64_decode(std::string const& encoded_string);

std::string decrypt(const std::string& data);
std::string encrypt(const std::string& data);
bool getCurrentPathFromModule(HMODULE hMod, char* dllPath, int size);
void stripFileFromPath(char* dllPath, int size);
const char* getDllPath();
const char* getMapsPath();

//Related to server	
void getJWTToken(std::string* buffer);
std::string getHWID();


void setDllPath(char* file);
void setDebugStreamFiles();
void cleanDebugStreamFiles();

void Tracef(bool val, const char* c_szFormat, ...);


//There are bugs here that migh crash the process
void setTimerFunction(tTimerFunction func,float sec);
void executeTimerFunctions();
std::string serializeTimePoint(const std::chrono::system_clock::time_point& time, const std::string& format);


inline void* getRelativeCallAddress(void* startCallAddr) {
	DWORD addr = (DWORD)startCallAddr;
	DWORD* offset = (DWORD*)(addr + 1);
	void* _final = (void*)(addr + *offset + 5);
	return  _final;
}


inline float distance(float x1, float y1, float x2, float y2) {
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}


struct Point {
	Point(int x=0, int y=0) : x(x), y(y){}
	int x, y;
};

struct fPoint {
	fPoint(float x, float y) : x(x), y(y) {}
	float x, y;
};

struct fPoint3D {
	float x, y, z;
};

inline fPoint getPointAtDistanceTimes(float x1, float y1, float x2, float y2, float multiplier) {
	fPoint vector(x2 - x1, y2 - y1);
	fPoint result(x1 + vector.x * multiplier, y1 + vector.y * multiplier);
	return result;
}

#pragma pack(push, 1)
template<class T>
int fillPacket(void* data, int size, T* _struct) {
	ZeroMemory(_struct, sizeof(T));
	int curr_size = std::min<int>(size, sizeof(T));
	memcpy(_struct, data, curr_size);
	return curr_size;
}
#pragma pack(pop)


inline bool checkPointBetween(float xStart, float yStart, float xCheckPoint, float yCheckPoint, float xEnd, float yEnd ) {
	fPoint vector(xEnd - xStart, yEnd - yStart);
	float kx = 0;
	float ky = 0;
	if (distance(xStart, yStart, xEnd, yEnd) < 1) {
		return false;
	}
	if (vector.x != 0){
		kx = (xCheckPoint - xStart) / vector.x;
	}
	if (vector.y != 0) {
		ky = (yCheckPoint - yStart) / vector.y;
	}

	if (abs(kx - ky) < 2 && kx<1) {
		return true;
	}

	return false;
}



bool addPathToInterpreter(const char* path);

bool executePythonFile(const char* file);