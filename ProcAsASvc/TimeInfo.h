#pragma once

#include <string>

class CTimeInfo
{
public:
	CTimeInfo(void);
	~CTimeInfo(void);

	unsigned long getTimestamp(void);
	long long getTimestampMs(void);
	
	std::string getTimeReadable(std::string DateTimeSep = ";", std::string TimeSep = ":");
	std::wstring getTimeReadableW(std::wstring DateTimeSep = L";", std::wstring TimeSep = L":");
	std::string getTimeReadableMs(std::string DateTimeSep = ";", std::string TimeSep = ":", std::string MSecSep = ".");
	std::wstring getTimeReadableMsW(std::wstring DateTimeSep = L";", std::wstring TimeSep = L":", std::wstring MSecSep = L".");


	bool isNewDay(void);
	bool isNewHour(void);

private:
	time_t savedDay;
	time_t savedHour;
};

