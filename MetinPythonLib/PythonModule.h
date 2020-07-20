#pragma once
#include "SleepFunctionHook.h"
#include <stdio.h>
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#include <string>
#include "shlwapi.h"
#include "PythonUtils.h"


extern PyObject* packet_mod;
extern PyObject* instanceList;

#pragma pack(push, 1)
struct CMappedFile {
	void *v_table;
	//int				m_mode;
	//char			m_filename[MAX_PATH + 1];
	//HANDLE			m_hFile;
	BYTE		uknown[280];
	DWORD		m_dwSize;//0x11C needs fix

	BYTE*		m_pbBufLinkData;
	DWORD		m_dwBufLinkSize;

	BYTE*		m_pbAppendResultDataBlock;
	DWORD		m_dwAppendResultDataSize;

	DWORD		m_seekPosition;
	HANDLE		m_hFM;
	DWORD		m_dataOffset;
	DWORD		m_mapSize;
	LPVOID		m_lpMapData;
	LPVOID		m_lpData;

	void *	m_pLZObj;
};
#pragma pack(pop)

void executeScript(const char* name, char* _path);

//NEEDS TO BE CALLED AFTER SCRIPT EXECUTION
//TEST FOR MEMORY LEAKS
PyObject* GetEterPacket(PyObject * poSelf, PyObject * poArgs);

DWORD __stdcall _GetEter(DWORD return_value, CMappedFile* file, const char* fileName, void** buffer, const char* uknown, bool uknown_2);


void initModule();