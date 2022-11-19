#pragma once

#include <Windows.h>

#include <string>

#if UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

class CUniConvert
{
public:
	CUniConvert(void);
	~CUniConvert(void);

	static int s2int(std::string s);
	static std::string int2s(int in);
	static int ws2int(std::wstring s);
	static std::wstring int2ws(int in);
	static int ts2int(tstring s);
	static tstring int2ts(int in);

	static unsigned long s2ul(std::string s);
	static std::string ul2s(unsigned long in);
	static unsigned long ws2ul(std::wstring s);
	static std::wstring ul2ws(unsigned long in);
	static unsigned long ts2ul(tstring s);
	static tstring ul2ts(unsigned long in);

#ifdef _WIN32        
	static void WCharToChar(const WCHAR* Src, char* Dest, int Size);
	static void CharToWChar(const char* Src, WCHAR* Dest, int Size);
	static void TCharToChar(const TCHAR* Src, char* Dest, int Size);
	static void TCharToWChar(const TCHAR* Src, wchar_t* Dest, int Size);
	static void CharToTChar(const char* Src, TCHAR* Dest, int Size);

	static bool AnsiToUnicode16(CHAR *in_Src, WCHAR *out_Dst, INT in_MaxLen);
	static bool AnsiToUnicode16L(CHAR *in_Src, INT in_SrcLen, WCHAR *out_Dst, INT in_MaxLen);
#endif

	static std::wstring s2ws(const std::string& str);
	static std::wstring ts2ws(const tstring& tstr);

	static std::string ws2s(const std::wstring& wstr);
	static std::string ts2s(const tstring& tstr);
	
	static tstring ws2ts(const std::wstring& wstr);
	static tstring s2ts(const std::string& str);

private:
	template <typename T>
	static std::string num2s(T num);
	template <typename T>
	static T s2num(std::string s);

	template <typename T>
	static std::wstring num2ws(T num);
	template <typename T>
	static T ws2num(std::wstring s);

	template <typename T>
	static tstring num2ts(T num);
	template <typename T>
	static T ts2num(tstring s);

};

