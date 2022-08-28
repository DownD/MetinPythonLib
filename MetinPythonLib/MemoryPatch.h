#pragma once
class CMemoryPatch
{
public:
	//In case of overflow will be replace by NOPs
	CMemoryPatch(BYTE* bufferToPatch,int sizeBuffer, void * location, int sizeLocation);

	//Will keep the original bytes where the mask is the char '?', similar to the pattern
	//The buffer and the mask has to have the same length
	CMemoryPatch(BYTE* bufferToPatch,const char* mask, int sizeBuffer, void* location);
	~CMemoryPatch();


	inline bool isPatched() { return m_isPatched; };

	bool patchMemory();
	bool unPatchMemory();

protected:
	bool changeProtectedMemory(void* target, void* src, int size);

private:
	void* location;
	BYTE* patchInstructions;
	BYTE* originalInstructions;
	int sizePatch;

	bool m_isPatched;

};

