#include "stdafx.h"

#define CODECVT 

#include <iostream>
#include <string>
#include <sstream>
#ifdef CODECVT
#include <codecvt>
#else
#include <vector>
#include <locale>
#endif
#include "UniConvert.h"

#if UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

CUniConvert::CUniConvert(void)
{
}


CUniConvert::~CUniConvert(void)
{
}

int CUniConvert::s2int(std::string s)
{ 
	return s2num<int>(s); 
}

std::string CUniConvert::int2s(int in) 
{ 
	return num2s<int>(in); 
}

int CUniConvert::ws2int(std::wstring s) 
{ 
	return ws2num<int>(s); 
}

std::wstring CUniConvert::int2ws(int in) 
{ 
	return num2ws<int>(in); 
}

int CUniConvert::ts2int(tstring s) 
{ 
	return ts2num<int>(s); 
}

tstring CUniConvert::int2ts(int in) 
{ 
	return num2ts<int>(in); 
}

unsigned long CUniConvert::s2ul(std::string s)
{
	return s2num<unsigned long>(s);
}

std::string CUniConvert::ul2s(unsigned long in)
{
	return num2s<unsigned long>(in);
}

unsigned long CUniConvert::ws2ul(std::wstring s)
{
	return ws2num<unsigned long>(s);
}

std::wstring CUniConvert::ul2ws(unsigned long in)
{
	return num2ws<unsigned long>(in);
}

unsigned long CUniConvert::ts2ul(tstring s)
{
	return ts2num<unsigned long>(s);
}

tstring CUniConvert::ul2ts(unsigned long in)
{
	return num2ts<unsigned long>(in);
}

template <typename T>
std::string CUniConvert::num2s(T num)
{
	std::stringstream ss;
	ss << num;
	return ss.str();
}

// input string not by const reference so that the function can be used with a 
// character array as argument
template <typename T>
T CUniConvert::s2num(std::string s)
{
	std::stringstream ss(s);
	T res;
	return ss >> res ? res : 0;
}

template <typename T>
std::wstring CUniConvert::num2ws(T num)
{
	std::wstringstream ss;
	ss << num;
	return ss.str();
        
}

// input string not by const reference so that the function can be used with a 
// character array as argument
template <typename T>
T CUniConvert::ws2num(std::wstring s)
{
	std::wstringstream ss(s);
	T res;
	return ss >> res ? res : 0;
}

template <typename T>
tstring CUniConvert::num2ts(T num)
{
#ifdef UNICODE
    return num2ws(num);
#else
    return num2s(num);
#endif
}

template <typename T>
T CUniConvert::ts2num(tstring s)
{
#ifdef UNICODE
	T res = ws2num<T>(s);
	return res;
#else
	T res = s2num<T>(s);
	return res;
#endif
}

#ifdef _WIN32
void CUniConvert::WCharToChar(const WCHAR* Src, char* Dest, int Size)
{
	WideCharToMultiByte(CP_ACP, 0, Src, wcslen(Src)+1, Dest , Size, NULL, NULL); 
}


void CUniConvert::CharToWChar(const char* Src, WCHAR* Dest, int Size)
{
	MultiByteToWideChar(CP_ACP, 0, Src, -1, Dest, Size);
}

void CUniConvert::TCharToChar(const TCHAR* Src, char* Dest, int Size) 
{
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, Src, wcslen(Src)+1, Dest , Size, NULL, NULL); 
#else
	wcscpy_s(Dest, Size, Src);
#endif
} 

void CUniConvert::TCharToWChar(const TCHAR* Src, wchar_t* Dest, int Size)
{
#ifdef UNICODE
	wcscpy_s(Dest, Size, Src);
#else
	MultiByteToWideChar(CP_ACP, 0, Src, -1, Dest, Size);
#endif
}

void CUniConvert::CharToTChar(const char* Src, TCHAR* Dest, int Size)
{
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, Src, -1, Dest, Size);
#else
	strcpy_s(Dest, Size, Src);
#endif	
}


/***************************/
/* ansi-unicode conversion */
/***************************/

bool CUniConvert::AnsiToUnicode16(CHAR *in_Src, WCHAR *out_Dst, INT in_MaxLen)
{
	/* locals */
	INT lv_Len;

	// do NOT decrease maxlen for the eos
	if (in_MaxLen <= 0)
		return false;

	// let windows find out the meaning of ansi
	// - the SrcLen=-1 triggers MBTWC to add a eos to Dst and fails if MaxLen is too small.
	// - if SrcLen is specified then no eos is added
	// - if (SrcLen+1) is specified then the eos IS added
	lv_Len = MultiByteToWideChar(CP_ACP, 0, in_Src, -1, out_Dst, in_MaxLen);

	// validate
	if (lv_Len < 0)
		lv_Len = 0;

	// ensure eos, watch out for a full buffersize
	// - if the buffer is full without an eos then clear the output like MBTWC does
	//   in case of too small outputbuffer
	// - unfortunately there is no way to let MBTWC return shortened strings,
	//   if the outputbuffer is too small then it fails completely
	if (lv_Len < in_MaxLen)
		out_Dst[lv_Len] = 0;
	else if (out_Dst[in_MaxLen-1])
		out_Dst[0] = 0;

	// done
	return true;
}


bool CUniConvert::AnsiToUnicode16L(CHAR *in_Src, INT in_SrcLen, WCHAR *out_Dst, INT in_MaxLen)
{
	/* locals */
	INT lv_Len;


	// do NOT decrease maxlen for the eos
	if (in_MaxLen <= 0)
		return false;

	// let windows find out the meaning of ansi
	// - the SrcLen=-1 triggers MBTWC to add a eos to Dst and fails if MaxLen is too small.
	// - if SrcLen is specified then no eos is added
	// - if (SrcLen+1) is specified then the eos IS added
	lv_Len = MultiByteToWideChar(CP_ACP, 0, in_Src, in_SrcLen, out_Dst, in_MaxLen);

	// validate
	if (lv_Len < 0)
		lv_Len = 0;

	// ensure eos, watch out for a full buffersize
	// - if the buffer is full without an eos then clear the output like MBTWC does
	//   in case of too small outputbuffer
	// - unfortunately there is no way to let MBTWC return shortened strings,
	//   if the outputbuffer is too small then it fails completely
	if (lv_Len < in_MaxLen)
		out_Dst[lv_Len] = 0;
	else if (out_Dst[in_MaxLen-1])
		out_Dst[0] = 0;

	// done
	return true;
}
#endif

std::wstring CUniConvert::s2ws(const std::string& str)
{
#ifdef CODECVT
    typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
#else
    return L"";
#endif
}

std::string CUniConvert::ws2s(const std::wstring& wstr)
{

#ifdef CODECVT    
    typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
#else


#ifdef _WIN32
	setlocale(LC_ALL, "");
#else
	std::setlocale(LC_ALL, "");
#endif

    const std::locale locale("");
    
    typedef std::codecvt<wchar_t, char, std::mbstate_t> converter_type;
    const converter_type& converter = std::use_facet<converter_type>(locale);
    
    std::vector<char> to(wstr.length() * converter.max_length());
    std::mbstate_t state;
    const wchar_t* from_next;
    char* to_next;
    const converter_type::result result = converter.out(state, wstr.data(), wstr.data() + wstr.length(), from_next, &to[0], &to[0] + to.size(), to_next);
    
    if (result == converter_type::ok || result == converter_type::noconv) 
    {
        const std::string s(&to[0], to_next);
        return s;
    }
    
    return "";
  
#endif
}

std::wstring CUniConvert::ts2ws(const tstring& tstr)
{
#if UNICODE
	return tstr;
#else
	return s2ws(tstr);
#endif
}

std::string CUniConvert::ts2s(const tstring& tstr)
{
#if UNICODE
	return ws2s(tstr);
#else
	return tstr;
#endif
}

tstring CUniConvert::ws2ts(const std::wstring& wstr)
{
#if UNICODE
    return wstr;
#else
	return ws2s(wstr);
#endif

}

tstring CUniConvert::s2ts(const std::string& str)
{
#if UNICODE
	return s2ws(str);
#else
	return str;
#endif
}

