#pragma once


#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>

#define HAVE_SNPRINTF
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif