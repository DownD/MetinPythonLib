#include "stdafx.h"
#include "SleepFunctionHook.h"



SleepFunctionHook::SleepFunctionHook(tFunction toBeRedirected) 
{
}


SleepFunctionHook::~SleepFunctionHook()
{
}

Hook * SleepFunctionHook::setupHook(tFunction toBeRedirected)
{
	HMODULE mod = GetModuleHandle("KERNELBASE");

	void* sleepFunction = GetProcAddress(mod, "Sleep");

	return new JMPStartFuncHook(sleepFunction, toBeRedirected, 5,THIS_CALL);
}

