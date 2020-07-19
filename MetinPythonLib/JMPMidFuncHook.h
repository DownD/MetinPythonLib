#pragma once
#include "Hook.h"
class JMPMidFuncHook : public Hook
{
public:
	JMPMidFuncHook(void* addressToHook, void* redirection, BYTE sizeHook);
	~JMPMidFuncHook();

	// Inherited via Hook
	virtual bool HookFunction() override;
	virtual bool UnHookFunction() override;

private:

	void* oldCode;
	Context ctx;
	void* hookLocation;
	void* redirection;
	void* trampoline;
	int hookSize;
	bool isHooked;
};

