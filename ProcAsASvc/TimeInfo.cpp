#include "stdafx.h"

#include <sys/timeb.h>
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>

#include "TimeInfo.h"



CTimeInfo::CTimeInfo(void)
	: savedDay(0)
	, savedHour(0)
{
}


CTimeInfo::~CTimeInfo(void)
{
}

unsigned long CTimeInfo::getTimestamp(void)
{
	unsigned long ulRet = 0L;

	time_t t = time(0);
	ulRet = (unsigned long) t;

	return ulRet;
}

long long CTimeInfo::getTimestampMs(void)
{
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	return ms.count();
}

std::string CTimeInfo::getTimeReadable(std::string DateTimeSep, std::string TimeSep)
{
	std::string res = "";
	const int l = 64;

	char date[l] = "";
	char time_hour[l] = "";
	char time_min[l] = "";
	char time_sec[l] = "";

	time_t t = time(0);

#ifdef _WIN32
	tm tm = { 0 };
	localtime_s(&tm, &t);
	//strftime(wc, l-1, L"%Y-%m-%d;%H:%M:%S", &tm);
	strftime(date, l - 1, "%Y-%m-%d", &tm);
	strftime(time_hour, l - 1, "%H", &tm);
	strftime(time_min, l - 1, "%M", &tm);
	strftime(time_sec, l - 1, "%S", &tm);
#else
        struct tm *tm;
        tm = localtime(&t);
	strftime(date, l - 1, "%Y-%m-%d", tm);
	strftime(time_hour, l - 1, "%H", tm);
	strftime(time_min, l - 1, "%M", tm);
	strftime(time_sec, l - 1, "%S", tm);
#endif

	res = date + DateTimeSep + time_hour + TimeSep + time_min + TimeSep + time_sec;
	
	return res;
}

std::wstring CTimeInfo::getTimeReadableW(std::wstring DateTimeSep, std::wstring TimeSep)
{
	std::wstring res = L"";
	const int l = 64;

	wchar_t wc_date[l] = L"\0";
	wchar_t wc_time_hour[l] = L"\0";
	wchar_t wc_time_min[l] = L"\0";
	wchar_t wc_time_sec[l] = L"\0";

	time_t t = time(0);
        
#ifdef _WIN32
	tm tm = { 0 };

	localtime_s(&tm, &t);
	//wcsftime(wc, l-1, L"%Y-%m-%d;%H:%M:%S", &tm);
	wcsftime(wc_date, l-1, L"%Y-%m-%d", &tm);
	wcsftime(wc_time_hour, l-1, L"%H", &tm);
	wcsftime(wc_time_min, l-1, L"%M", &tm);
	wcsftime(wc_time_sec, l-1, L"%S", &tm);
#else
       	struct tm *tm;

	tm = localtime(&t);
	//wcsftime(wc, l-1, L"%Y-%m-%d;%H:%M:%S", &tm);
	wcsftime(wc_date, l-1, L"%Y-%m-%d", tm);
	wcsftime(wc_time_hour, l-1, L"%H", tm);
	wcsftime(wc_time_min, l-1, L"%M", tm);
	wcsftime(wc_time_sec, l-1, L"%S", tm);
#endif	
        
	res = wc_date + DateTimeSep + wc_time_hour + TimeSep + wc_time_min + TimeSep + wc_time_sec;

	return res;
}

std::string CTimeInfo::getTimeReadableMs(std::string DateTimeSep, std::string TimeSep, std::string MSecSep)
{
	std::string res = "";
	std::string msec = "";

#ifdef _WIN32
	struct _timeb t = { 0 };
	_ftime_s(&t);
#else
        struct timeb t = { 0 };
        ftime(&t);
#endif
        
	msec = std::to_string(t.millitm);
	if (msec.length() == 1) msec = "00" + msec;
	if (msec.length() == 2) msec = "0" + msec;
        
	res = getTimeReadable(DateTimeSep, TimeSep) + MSecSep + msec;

	return res;
}

std::wstring CTimeInfo::getTimeReadableMsW(std::wstring DateTimeSep, std::wstring TimeSep, std::wstring MSecSep)
{
	std::wstring res = L"";
	std::wstring msec = L"";
        
#ifdef _WIN32
	struct _timeb t = { 0 };
	_ftime_s(&t);
#else
        struct timeb t = { 0 };
        ftime(&t);
#endif 

	msec = std::to_wstring(t.millitm);
	if (msec.length() == 1) msec = L"00" + msec;
	if (msec.length() == 2) msec = L"0" + msec;

        
	res = getTimeReadableW(DateTimeSep, TimeSep) + MSecSep + msec;

	return res;
}


bool CTimeInfo::isNewDay(void)
{
	bool bRet = false;

	// init
	if (savedDay == 0)
	{
		savedDay = time(0);
	}

	time_t nowTime = time(0);
        
#ifdef _WIN32
	tm tmSave = { 0 };
	tm tmNow = { 0 };

	localtime_s(&tmSave, &savedDay);
	localtime_s(&tmNow, &nowTime);
        
	int saved = tmSave.tm_hour;
	int now = tmNow.tm_hour;
#else
	struct tm *tmSave;
	struct tm *tmNow;

	tmSave = localtime(&savedDay);
	tmNow = localtime(&nowTime);

	int saved = tmSave->tm_yday;
	int now = tmNow->tm_yday;
#endif
        
	savedDay = nowTime;

	if (saved != now)
	{
		bRet = true;
	}

	return bRet;
}

bool CTimeInfo::isNewHour(void)
{
	bool bRet = false;

	// init
	if (savedHour == 0)
	{
		savedHour = time(0);
	}

	time_t nowTime = time(0);
        
#ifdef _WIN32
	tm tmSave = { 0 };
	tm tmNow = { 0 };

	localtime_s(&tmSave, &savedHour);
	localtime_s(&tmNow, &nowTime);

	int saved = tmSave.tm_hour;
	int now = tmNow.tm_hour;
#else
	struct tm *tmSave;
	struct tm *tmNow;

	tmSave = localtime(&savedHour);
	tmNow = localtime(&nowTime);
        
        int saved = tmSave->tm_hour;
	int now = tmNow->tm_hour;
#endif        


	savedHour = nowTime;

	if (saved != now)
	{
		bRet = true;
	}

	return bRet;
}

