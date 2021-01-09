#pragma once
#include "Hook.h"
#include <detours.h>

template<class hookedFunc>
class DetoursHook : public Hook{
public:
	DetoursHook(hookedFunc funcToHook, void* redirection) {
		originalFunction = funcToHook;
		this->redirection = redirection;
		isHooked = 1;
	}

	bool HookFunction(){
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)originalFunction, redirection);

		LONG lError = DetourTransactionCommit();
		if (lError != NO_ERROR) {
			return FALSE;
		}
		isHooked = 1;

		return true;
	}

	bool UnHookFunction(){
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)originalFunction, redirection);

		LONG lError = DetourTransactionCommit();
		if (lError != NO_ERROR) {
			return FALSE;
		}
		isHooked = 0;
		return true;
	}

	hookedFunc originalFunction;
	

	~DetoursHook() {
		if (isHooked)
			UnHookFunction();
	}

private:
	hookedFunc addressToHook;
	void* redirection;
	bool isHooked;
};

