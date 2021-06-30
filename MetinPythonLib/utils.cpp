#include "utils.h"
#include <map>
#include <iomanip>

static std::map<tTimePoint, tTimerFunction> timer_functions;
char DLLPATH[256] = { 0 };
char MAP_PATH[256] = { 0 };
bool debug_print = 1;

FILE* traceFile;
FILE* tracenFile;

int split(char * str, char c, std::vector<std::string>* vec)
{
	std::istringstream f(str);
	std::string s;
	for (int i = 0; std::getline(f, s, c); i++) {
		vec->push_back(s);
	}
	return vec->size();
}

Stack::Stack(int Capacity)
{
	arr.reserve(Capacity);
}

Stack::~Stack()
{
}

int Stack::getSize()
{
	return arr.size();
}

void Stack::pushBYTE(BYTE value)
{
	arr.push_back(value);
}

void Stack::pushDWORD(DWORD value)
{
	BYTE* bytes = (BYTE*)&value;
	arr.push_back(bytes[0]);
	arr.push_back(bytes[1]);
	arr.push_back(bytes[2]);
	arr.push_back(bytes[3]);
}

void Stack::pushPOINTER(void * value)
{
	pushDWORD((DWORD)value);
}

void Stack::pushWORD(WORD value)
{
	BYTE* bytes = (BYTE*)value;
	arr.push_back(bytes[0]);
	arr.push_back(bytes[1]);
}

void Stack::pushARRAY(BYTE * arr, int size)
{
	for (int i = 0; i < size; i++) {
		this->arr.push_back(arr[i]);
	}
}

bool Stack::copy(BYTE * buffer, int copySize)
{
	if (copySize >= arr.size()) {
		memcpy(buffer, arr.data(), arr.size());
		return true;
	}

	return false;
}

void Stack::printDebug()
{
	for (BYTE b : arr) {
		printf("%#02x ", b);
	}
}

bool isDebugEnable()
{
#ifdef _DEBUG
	return debug_print;
#else
	return false;
#endif
}

void setDebugOn()
{
	debug_print = 1;
}

void setDebugOff()
{
	debug_print = 0;
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

void setDllPath(char* file)
{
	strcpy(DLLPATH, file);
	stripFileFromPath(DLLPATH,256);
	strcpy(MAP_PATH, DLLPATH);
	strcat(MAP_PATH, SUBPATH_MAPS);
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


AssemblerX86::AssemblerX86(Stack * stack)
{
	if (stack == nullptr) {
		freeOnDestruction = true;
		this->stack = new Stack();
	}
	else {
		this->stack = stack;
		freeOnDestruction = false;
	}
}

AssemblerX86::~AssemblerX86()
{
	if (freeOnDestruction)
		delete stack;
}


void AssemblerX86::saveECX(DWORD * addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x0D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveEAX(DWORD * addr)
{
	stack->pushBYTE(0xA3);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveEDX(DWORD * addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x15);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveEDI(DWORD * addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x3D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveESI(DWORD * addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x35);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveEBP(DWORD * addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x2D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveEBX(DWORD * addr)
{
	stack->pushBYTE(0x89);
	stack->pushBYTE(0x1D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadECX(DWORD * addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x0D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadEAX(DWORD * addr)
{
	stack->pushBYTE(0xA1);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadEDX(DWORD * addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x15);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadEDI(DWORD * addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x3D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadESI(DWORD * addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x35);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadEBP(DWORD * addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x2D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::loadEBX(DWORD * addr)
{
	stack->pushBYTE(0x8B);
	stack->pushBYTE(0x1D);
	stack->pushPOINTER(addr);
}

void AssemblerX86::saveAllRegister(Context * loc)
{
	saveECX(&(loc->ECX));
	saveEAX(&(loc->EAX));
	saveEDX(&(loc->EDX));
	saveEDI(&(loc->EDI));
	saveESI(&(loc->ESI));
	saveEBP(&(loc->EBP));
	saveEBX(&(loc->EBX));
}

void AssemblerX86::loadAllRegister(Context * loc)
{
	loadECX(&(loc->ECX));
	loadEAX(&(loc->EAX));
	loadEDX(&(loc->EDX));
	loadEDI(&(loc->EDI));
	loadESI(&(loc->ESI));
	loadEBP(&(loc->EBP));
	loadEBX(&(loc->EBX));
}

void AssemblerX86::pushAllRegister()
{
	stack->pushBYTE(0x60);
}

void AssemblerX86::popAllRegister()
{
	stack->pushBYTE(0x61);
}

void AssemblerX86::pushRelativeToESP(BYTE offset)
{
	if (offset <= 0 || offset > 128) {
		throw "Error offset cannot be null or bigger then 128";
	}
	stack->pushBYTE(0xFF);
	stack->pushBYTE(0x74);
	stack->pushBYTE(0x24);
	stack->pushBYTE(offset);
}

void AssemblerX86::pushAddress(DWORD * valueAddr)
{
	stack->pushBYTE(0xFF);
	stack->pushBYTE(0x35);
	stack->pushPOINTER(valueAddr);
}

void AssemblerX86::popToAddress(DWORD * addr)
{
	stack->pushBYTE(0x8F);
	stack->pushBYTE(0x05);
	stack->pushPOINTER(addr);
}

void AssemblerX86::callNearRelative(DWORD offset)
{
	stack->pushBYTE(0xE8);
	stack->pushDWORD(offset);
}

void AssemblerX86::jmpNearRelative(DWORD offset)
{
	stack->pushBYTE(0xE9);
	stack->pushDWORD(offset);
}

void AssemblerX86::appendInstructions(BYTE * bytes, int size)
{
	stack->pushARRAY(bytes, size);
}

bool AssemblerX86::patchRelativeInstruction(int index, void* target, void* baseAlloc)
{
	BYTE* arr = stack->getData();
	BYTE instruction = arr[index];
	DWORD* offset = (DWORD*)(arr + index + 1);

	switch (instruction) {
		case 0xE9:
		case 0xE8: {
			*offset = (DWORD)target - ((DWORD)baseAlloc + index + 5);
			break;
		}
		default:
			DEBUG_INFO_LEVEL_1("Not a Relative instrcution");
			return false;
	}
	return true;
}

