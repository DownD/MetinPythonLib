#include "utils.h"
#include <map>
#include <iomanip>
#include <shlobj.h>
#include <fstream>
#include "SimpleIni.h"
#include "Config.h"

static std::map<tTimePoint, tTimerFunction> timer_functions;
using namespace std::chrono;


char DLLPATH[256] = { 0 };
char MAP_PATH[256] = { 0 };
char INI_FILE_PATH[256] = { 0 };
bool debug_print = 1;

FILE* traceFile;
FILE* tracenFile;

static std::string key = "678345EYRUJKLM";

int split(char * str, char c, std::vector<std::string>* vec)
{
	std::istringstream f(str);
	std::string s;
	for (int i = 0; std::getline(f, s, c); i++) {
		vec->push_back(s);
	}
	return vec->size();
}



bool isDebugEnable()
{
#ifdef _DEBUG
	return debug_print;
#endif
#ifdef _DEBUG_FILE
	return debug_print;
#endif
	return false;
}

void setDebugOn()
{
	debug_print = 1;
}

void setDebugOff()
{
	debug_print = 0;
}

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+";


static inline bool is_base64(BYTE c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}
std::string base64_decode(std::string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

std::string decrypt(const std::string& data)
{
	auto decoded_data = base64_decode(data);
	std::string result(decoded_data.size(), '\0');

	for (std::size_t i = 0; i < decoded_data.size(); i++) {
		result[i] = decoded_data[i] ^ key[i % key.size()];
	}

	return result;
}

std::string encrypt(const std::string& data)
{
	std::string result(data.size(), '\0');

	for (std::size_t i = 0; i < data.size(); i++) {
		result[i] = data[i] ^ key[i % key.size()];
	}
	std::string result2 = base64_encode((const unsigned char*)result.data(), result.size());
	return result2;
}

bool getCurrentPathFromModule(HMODULE hMod, char* dllPath, int size)
{
	int len = GetModuleFileNameA(hMod, dllPath, size);
	if (!len) {
		return false;
	}
	stripFileFromPath(dllPath, len);

	return true;
}

void stripFileFromPath(char* dllPath, int size)
{
	for (int i = size; i >= 0; --i) {
		if (dllPath[i] == '\\')
			return;
		dllPath[i] = 0;
	}
	return;
}

const char* getDllPath()
{
	return DLLPATH;
}

const char* getMapsPath()
{
	return MAP_PATH;
}


void getJWTToken(std::string* buffer)
{
	CSimpleIniA ini;

	ini.SetUnicode();
	SI_Error rc = ini.LoadFile(INI_FILE_PATH);
	if (rc >= 0) {
		*buffer = ini.GetValue("Config", JWT_INI_VALUE_NAME, DEFAULT_API_KEY);		
	}
	else {
		*buffer = DEFAULT_API_KEY;
		ini.SetValue("Config", JWT_INI_VALUE_NAME, DEFAULT_API_KEY);
		ini.SaveFile(INI_FILE_PATH);
	}
}

std::string getHWID()
{
	DWORD VolumeSerialNumber = 0;
	GetVolumeInformation("c:\\", NULL, NULL, &VolumeSerialNumber, NULL, NULL, NULL, NULL);
	return std::to_string(VolumeSerialNumber);
}

void setDllPath(char* file)
{
	strcpy(DLLPATH, file);
	stripFileFromPath(DLLPATH,256);
	strcpy(MAP_PATH, DLLPATH);
	strcpy(INI_FILE_PATH, DLLPATH);
	strcat(MAP_PATH, SUBPATH_MAPS);
	strcat(INI_FILE_PATH, CONFIG_INI_PATH);

}

void setDebugStreamFiles()
{
	std::string path(getDllPath());
	auto traceFileName = path + "Tracef.txt";
	auto tracenFileName = path + "Tracenf.txt";
	traceFile = fopen(traceFileName.c_str(), "w");
	tracenFile = fopen(tracenFileName.c_str(), "w");
}

void cleanDebugStreamFiles()
{
	fclose(traceFile);
	fclose(tracenFile);
}

void Tracef(bool val,const char* c_szFormat, ...)
{
	va_list args;
	va_start(args, c_szFormat);
	if(val)
		vfprintf(traceFile, c_szFormat, args);
	else
		vfprintf(tracenFile, c_szFormat, args);
	
	va_end(args);
}


std::string return_current_time_and_date(tTimePoint date)
{
	auto now = date;
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
	return ss.str();
}

void setTimerFunction(tTimerFunction func,float sec)
{
	auto timer = std::chrono::system_clock::now();
	timer += std::chrono::milliseconds((int)(sec*1000));
	timer_functions.insert(std::pair<tTimePoint,tTimerFunction>(timer,func));
	//DEBUG_INFO_LEVEL_4("Function set to trigger at %s ", return_current_time_and_date(timer).c_str());
}



//Smoehow deleting directly inside the loop is giving segmentation fault
void executeTimerFunctions()
{
	if (timer_functions.empty())
		return;

	//DEBUG_INFO_LEVEL_4("Going Trough");
	auto start = std::chrono::system_clock::now();
	auto it = timer_functions.begin();

	for (auto it = timer_functions.begin(); it != timer_functions.end();)
	{
		auto end = it->first;
		auto delta = end - start;
		if (delta.count() > 0) {
			return;
		}
		else {
			auto func = it->second;
			DEBUG_INFO_LEVEL_4("Executing Timer Function");
			func();
			it = timer_functions.erase(it); //WHY ERROR?
			return;
		}
		++it;
	}
}

std::string serializeTimePoint(const std::chrono::system_clock::time_point& time, const std::string& format)
{
	std::time_t tt = system_clock::to_time_t(time);
	std::tm tm = *std::gmtime(&tt); //GMT (UTC)
	//std::tm tm = *std::localtime(&tt); //Locale time-zone, usually UTC by default.
	std::stringstream ss;
	ss << std::put_time(&tm, format.c_str());
	return ss.str();
}

