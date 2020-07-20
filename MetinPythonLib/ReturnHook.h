#pragma once
#include "Hook.h"

//To be Tested

//Hooks the start of a function and pushes a new return address that corresponds to the new function
//Do not replace relative instruciton like JMP
//The function is called when the original returns
//Return value is not saved, the caller is responsible for the same
//Also the caller is responsible for poping the arguments a __stdcall is recomended
class ReturnHook : public Hook
{

public:
	virtual bool HookFunction() override;
	virtual bool UnHookFunction() override;
	
	ReturnHook(void* addressToHook, void* redirection, BYTE size, int numArguments);
	~ReturnHook();

private:
	void* generateStackReplaceFunction();

private:
	void* redirection;
	void* addressToHook;
	BYTE sizeHook;
	void *oldCode;

	void* stackReplaceFunction;
	bool isHooked;
	int numArguments;
};

