// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN

#include "targetver.h"

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

// not working in Win10 sdk
#ifndef _T
#define _T TEXT
#endif // !_T
