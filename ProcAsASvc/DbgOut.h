#pragma once

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <tchar.h>

#ifndef  tstring
#ifdef UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif
#endif // ! tstring

class CDbgOut
{
public:
	CDbgOut();
	~CDbgOut();

	static void dbgprintfA(const char *in, ...);
	static void dbgprintfW(const wchar_t *in, ...);
	static void dbgprintfT(LPCTSTR in, ...);

	static void DebugInfoA(const char* szInfo, 
		bool bTime = true, 
		bool bDebug = true, 
		bool bPrint = true,
		bool bFile = true,
		bool bHwnd = true);
	static void DebugInfoW(const wchar_t* szInfo,
		bool bTime = true,
		bool bDebug = true,
		bool bPrint = true,
		bool bFile = true,
		bool bHwnd = true);
	static void DebugInfoT(const TCHAR* szInfo,
		bool bTime = true, 
		bool bDebug = true, 
		bool bPrint = true,
		bool bFile = true,
		bool bHwnd = true);

	static void setLogFileBase(std::string file);
	static std::string getLogFileBase(void);
	static int cleanLogDir(std::string logDir, int keepDays = 7);

	static void setHwnd(HWND hWnd);

	// todo: put this in extra helper class
	static void GetDateTimeNowA(char* buffer, bool bDate = true, bool bTime = true);
	static void GetDateTimeNowW(wchar_t* buffer, bool bDate = true, bool bTime = true);
	static void GetDateTimeNowT(TCHAR* buffer, bool bDate = true, bool bTime = true);


private:

	static void writeToHwndEditA(HWND hWnd, std::string txt);
	static void writeToHwndEditW(HWND hWnd, std::wstring txt);
	static void writeToHwndEdit(HWND hWnd, tstring txt);

	static int writeToFileA(const char *in);
	static int writeToFileW(const wchar_t *in);

#ifdef UNICODE
#define writeToFile writeToFileW
#else
#define writeToFile writeToFileA
#endif


	static void insertCharA(char* out, int outLen, const char *in, char after, char insert);
	static void insertCharW(wchar_t* out, int outLen, const wchar_t *in, wchar_t after, wchar_t insert);
	static void insertCharT(TCHAR* out, int outLen, const TCHAR *in, TCHAR after, TCHAR insert);
};




