#include "stdafx.h"

#include <time.h>
#include <mutex>

#include "UniConvert.h"
#include "TimeInfo.h"
#include "DateDiff.h"
#include "dirent.h"

#include "DbgOut.h"
#include <regex>


bool bFirstRun = false;

std::string m_sFileName = "";
std::string m_sFileNameBase = "";

std::ofstream m_logFileA;
std::wofstream m_logFileW;
std::mutex m_mtxFile;

HWND m_hWnd = NULL;

CTimeInfo TI;


CDbgOut::CDbgOut()
{
}


CDbgOut::~CDbgOut()
{
}

void CDbgOut::dbgprintfA(const char *in, ...)
{
	if (!in) return;
	
	int iLen = 0;
	char *szBuffer = NULL;

	try
	{
		va_list args;
		va_start(args, in);
		iLen = _vscprintf(in, args) + 3; // add CRLF and 0 at end
		szBuffer = new char[iLen];
		vsprintf_s(szBuffer, iLen, in, args);
		va_end(args);
		strcat_s(szBuffer, iLen, "\r\n\0");

		OutputDebugStringA(szBuffer);
	}
	catch (const std::exception&)
	{
		
	}

	if (szBuffer) delete[] szBuffer;
}

void CDbgOut::dbgprintfW(const wchar_t *in, ...)
{
	if (!in) return;

	int iLen = 0;
	WCHAR *szBuffer = NULL;

	try
	{
		va_list args;
		va_start(args, in);
		iLen = _vscwprintf(in, args) + 3; // add CRLF and 0 at end
		szBuffer = new WCHAR[iLen];
		vswprintf_s(szBuffer, iLen, in, args);
		va_end(args);
		_tcscat_s(szBuffer, iLen, L"\r\n\0");

		OutputDebugStringW(szBuffer);
	}
	catch (const std::exception&)
	{

	}

	if (szBuffer) delete[] szBuffer;
}

void CDbgOut::dbgprintfT(LPCTSTR in, ...)
{
	if (!in) return;

	int iLen = 0;
	TCHAR *szBuffer = NULL;
	
	try
	{
		va_list args;
		va_start(args, in);
		iLen = _vsctprintf(in, args) + 3; // add CRLF and 0 at end
		szBuffer = new TCHAR[iLen];
		_vstprintf_s(szBuffer, iLen, in, args);
		va_end(args);
		_tcscat_s(szBuffer, iLen, _T("\r\n\0"));

		OutputDebugString(szBuffer);
	}
	catch (const std::exception&)
	{

	}

	if (szBuffer) delete[] szBuffer;
}


//-----------------------------------------------------------------------------------
// wrapper for output via outputdebugstring and via printf
//
void CDbgOut::DebugInfoA(const char* szInfo, 
	bool bTime, 
	bool bDebug, 
	bool bPrint,
	bool bFile,
	bool bHwnd)
{
	if (!szInfo) return;

	char szTime[128] = "\0";
	if (bTime) GetDateTimeNowA(szTime);

	if (bDebug)
	{
		// if a % is in string we need to double it for printf/OutputDebugString
		int outLen = (strlen(szInfo) * 2) + 1;
		char *szOut = new char[outLen];
		insertCharA(szOut, outLen, szInfo, '%', '%');

		dbgprintfA(szOut);

		delete[] szOut;
	}

	if (bPrint)
	{
		if (strlen(szTime) == 0) printf("%s\n", szInfo);
		else printf("[%s] %s\n", szTime, szInfo);
	}

	if (bFile) writeToFileA(szInfo);

	if (bHwnd && m_hWnd != NULL) writeToHwndEditA(m_hWnd, std::string(szTime) + std::string(" :: ") + std::string(szInfo) + std::string("\r\n"));
}

void CDbgOut::DebugInfoW(const wchar_t* szInfo,
	bool bTime,
	bool bDebug,
	bool bPrint,
	bool bFile,
	bool bHwnd)
{
	if (!szInfo) return;

	wchar_t szTime[128] = _T("\0");
	if (bTime) GetDateTimeNowW(szTime);

	if (bDebug)
	{
		// if a % is in string we need to double it for printf/OutputDebugString
		int outLen = (wcslen(szInfo) * 2) + 1;
		wchar_t *szOut = new wchar_t[outLen];
		insertCharW(szOut, outLen, szInfo, L'%', L'%');

		dbgprintfW(szOut);

		delete[] szOut;
	}

	if (bPrint)
	{
		if (wcslen(szTime) == 0) wprintf(L"%s\n", szInfo);
		else wprintf(L"[%s] %s\n", szTime, szInfo);
	}

	if (bFile) writeToFileW(szInfo);

	if (bHwnd && m_hWnd != NULL) writeToHwndEditW(m_hWnd, std::wstring(szTime) + std::wstring(L" :: ") + std::wstring(szInfo) + std::wstring(L"\r\n"));
}

void CDbgOut::DebugInfoT(const TCHAR* szInfo, 
	bool bTime, 
	bool bDebug, 
	bool bPrint,
	bool bFile,
	bool bHwnd)
{
	if (!szInfo) return;

	TCHAR szTime[128] = _T("\0");
	if (bTime) GetDateTimeNowT(szTime);

	if (bDebug)
	{
		// if a % is in string we need to double it for printf/OutputDebugString
		int outLen = (_tcslen(szInfo) * 2) + 1;
		TCHAR *szOut = new TCHAR[outLen];
		insertCharT(szOut, outLen, szInfo, _T('%'), _T('%'));
		
		dbgprintfT(szOut);

		delete[] szOut;
	}

	if (bPrint)
	{
		if (_tcslen(szTime) == 0) _tprintf(_T("%s\n"), szInfo);
		else _tprintf(_T("[%s] %s\n"), szTime, szInfo);
	}

	if (bFile) writeToFile(szInfo);

	if (bHwnd && m_hWnd != NULL) writeToHwndEdit(m_hWnd, tstring(szTime) + tstring(_T(" :: ")) + tstring(szInfo) + tstring(_T("\r\n")));
}

void CDbgOut::setLogFileBase(std::string file)
{
	m_sFileNameBase = file;
}

std::string CDbgOut::getLogFileBase(void)
{
	return m_sFileNameBase;
}

void CDbgOut::setHwnd(HWND hWnd)
{
	m_hWnd = hWnd;
}

void CDbgOut::writeToHwndEditA(HWND hWnd, std::string txt)
{
	writeToHwndEdit(hWnd, CUniConvert::s2ts(txt));
}

void CDbgOut::writeToHwndEditW(HWND hWnd, std::wstring txt)
{
	writeToHwndEdit(hWnd, CUniConvert::ws2ts(txt));
}

void CDbgOut::writeToHwndEdit(HWND hWnd, tstring txt)
{
	if (hWnd == NULL || txt.length() == 0)
	{
		return;
	}

	int maxChars = 32767 - 1; // default max value of edit control
	DWORD dwStart = 0;
	DWORD dwEnd = 0;
	HWND hOut = hWnd;

	// get the current selection
	SendMessage(hOut, EM_GETSEL, reinterpret_cast<WPARAM>(&dwStart), reinterpret_cast<WPARAM>(&dwEnd));

	//int outLength = GetWindowTextLength(hOut);
	int outLength = SendMessage(hOut, WM_GETTEXTLENGTH, 0, 0);
	int newLine = 0;
	if (outLength > maxChars)
	{
		//SetWindowText(hOut, _T(""));
		SendMessage(hOut, WM_SETTEXT, 0, (LPARAM)_T(""));

		// todo - something not correct, disabled
		// this should replace the first line... fix it later
		//TCHAR *allText = new TCHAR[outLength + 1];
		//TCHAR *found = NULL;
		//GetWindowText(hOut, allText, outLength + 1);
		//found = _tcschr(allText, _T('\n'));
		//int newLine = _tcslen(found) + 1;
		//delete[] allText;

		//SendMessage(hOut, EM_SETSEL, newLine, outLength);
	}
	else
	{
		// move to end
		SendMessage(hOut, EM_SETSEL, outLength, outLength);
	}

	

	// insert
	SendMessage(hOut, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(txt.c_str()));

	// restore the previous selection
	SendMessage(hOut, EM_SETSEL, dwStart, dwEnd);
}



//-----------------------------------------------------------------------------------
// write data to file
//
int CDbgOut::writeToFileA(const char* in)
{
	if (in == NULL) return -1;
	if (getLogFileBase().length() == 0) return -2;

	m_mtxFile.lock();

	if (TI.isNewDay() || !bFirstRun)
	{
		m_sFileName = getLogFileBase() + "_" + TI.getTimeReadable("_", "-") + ".log";
		bFirstRun = true;
	}

	m_logFileA.open(m_sFileName, std::ios::out | std::ios::app);

	if (m_logFileA.is_open())
	{
		char dt[128];
		GetDateTimeNowA(dt);
		m_logFileA << dt << "\t";
		m_logFileA << in;
		m_logFileA << std::endl;
		m_logFileA.close();

		m_mtxFile.unlock();
		return 0;
	}

	m_mtxFile.unlock();
	return 1;
}

int CDbgOut::writeToFileW(const wchar_t* in)
{	
	if (in == NULL) return -1;
	if (getLogFileBase().length() == 0) return -2;

	m_mtxFile.lock();

	if (TI.isNewDay() || !bFirstRun)
	{
		m_sFileName = getLogFileBase() + "_" + TI.getTimeReadable("_", "-") + ".log";
		bFirstRun = true;
	}

	m_logFileW.open(m_sFileName.c_str(), std::ios::out | std::ios::app);

	if (m_logFileW.is_open())
	{
		wchar_t dt[128];
		GetDateTimeNowW(dt);
		m_logFileW << dt << L"\t";
		m_logFileW << in;
		m_logFileW << std::endl;
		m_logFileW.close();

		m_mtxFile.unlock();
		return 0;
	}

	m_mtxFile.unlock();
	return 1;
}

int CDbgOut::cleanLogDir(std::string logDir, int keepDays)
{
	CDateDiff dateDiff;
	DIR* dir = NULL;
	struct dirent* ent = { };
	char dateNow[16] = "";

	GetDateTimeNowA(dateNow, true, false);
	int yearNow = CUniConvert::s2int(std::string(dateNow).substr(0, 4));
	int monthNow = CUniConvert::s2int(std::string(dateNow).substr(5, 2));
	int dayNow = CUniConvert::s2int(std::string(dateNow).substr(8, 2));

	if ((dir = opendir(std::string(".\\" + logDir).c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
#ifdef  _DEBUG
			dbgprintfA("DEBUG: cleanLogDir - found file %s", ent->d_name);
#endif // DEBUG

			std::string data = ent->d_name;
			std::regex pattern("\\d{4}[-]\\d{2}[-]\\d{2}");
			std::smatch result;

			while (regex_search(data, result, pattern)) {
				std::string logDate = result[0].str();
#ifdef  _DEBUG
				dbgprintfA("DEBUG: cleanLogDir - found file %s with date %s", ent->d_name, logDate.c_str());
#endif

				int yearLog  = CUniConvert::s2int(logDate.substr(0, 4));
				int monthLog = CUniConvert::s2int(logDate.substr(5, 2));
				int dayLog   = CUniConvert::s2int(logDate.substr(8, 2));

				long long dayDiff = dateDiff.DiffDays(dayNow, monthNow, yearNow, dayLog, monthLog, yearLog);
#ifdef  _DEBUG
				dbgprintfA("DEBUG: cleanLogDir - file %s is %lld old", ent->d_name, dayDiff);
#endif

				if (dayDiff > 0 && dayDiff > keepDays)
				{
					std::string delFile = ".\\" + logDir + ".\\" + std::string(ent->d_name);
					if (DeleteFileA(delFile.c_str()))
						dbgprintfA("INFO: cleanLogDir - file %s is deleted", ent->d_name);
					else
						dbgprintfA("ERROR: cleanLogDir - file %s is NOT deleted, Error = %d", ent->d_name, GetLastError());
				}

				data = result.suffix().str();
			}
		}
		closedir(dir);

		return 0;
	}

	return 1;
}

//-----------------------------------------------------------------------------------
// get current date and time
//
void CDbgOut::GetDateTimeNowA(char* buffer, bool bDate, bool bTime)
{
	time_t rawtime;
	struct tm timeinfo;
	errno_t err;
	const int rSize = 128 * sizeof(char); // maybe give this as arg
	char szFormat[rSize];

	time(&rawtime);
	//timeinfo = localtime( &rawtime );
	if ((err = localtime_s(&timeinfo, &rawtime)) > 0)
		return;

	// formatting
	if (bDate && bTime)
		strcpy_s(szFormat, "%Y-%m-%d %H:%M:%S");
	else if (bDate && !bTime)
		strcpy_s(szFormat, "%Y-%m-%d");
	else if (!bDate && bTime)
		strcpy_s(szFormat, "%H:%M:%S");
	else
		return;

	strftime(buffer, rSize, szFormat, &timeinfo);
}

void CDbgOut::GetDateTimeNowW(wchar_t* buffer, bool bDate, bool bTime)
{
	time_t rawtime;
	struct tm timeinfo;
	errno_t err;
	const int rSize = 128 * sizeof(wchar_t); // maybe give this as arg
	wchar_t szFormat[rSize] = L"";

	time(&rawtime);
	//timeinfo = localtime( &rawtime );
	if ((err = localtime_s(&timeinfo, &rawtime)) > 0)
		return;

	// formatting
	if (bDate && bTime)
		wcscpy_s(szFormat, L"%Y-%m-%d %H:%M:%S");
	else if (bDate && !bTime)
		wcscpy_s(szFormat, L"%Y-%m-%d");
	else if (!bDate && bTime)
		wcscpy_s(szFormat, L"%H:%M:%S");
	else
		return;

	wcsftime(buffer, rSize, szFormat, &timeinfo);
}

void CDbgOut::GetDateTimeNowT(TCHAR* buffer, bool bDate, bool bTime)
{
	time_t rawtime;
	struct tm timeinfo;
	errno_t err;
	const int rSize = 128 * sizeof(char); // maybe give this as arg
	TCHAR szFormat[rSize];
	szFormat[0] = '\0';

	time(&rawtime);
	//timeinfo = localtime( &rawtime );
	if ((err = localtime_s(&timeinfo, &rawtime)) > 0)
		return;

	// formatting
	if (bDate && bTime)
		_tcscpy_s(szFormat, _T("%Y-%m-%d %H:%M:%S"));
	else if (bDate && !bTime)
		_tcscpy_s(szFormat, _T("%Y-%m-%d"));
	else if (!bDate && bTime)
		_tcscpy_s(szFormat, _T("%H:%M:%S"));
	else
		return;

	_tcsftime(buffer, rSize, szFormat, &timeinfo);
}

void CDbgOut::insertCharA(char* out, int outLen, const char *in, char after, char insert)
{
	if (out == NULL || in == NULL || insert == NULL)
	{
		return;
	}

	int lenIn = strlen(in);

	if (outLen / 2 < lenIn)
	{
		return;
	}

	int j = 0;
	for (int i = 0; i < lenIn; ++i)
	{
		out[j] = in[i];
		++j;

		if (in[i] == after)
		{
			out[j] = insert;
			++j;
		}
	}

	out[j] = '\0';
}

void CDbgOut::insertCharW(wchar_t* out, int outLen, const wchar_t *in, wchar_t after, wchar_t insert)
{
	if (out == NULL || in == NULL || insert == NULL)
	{
		return;
	}

	int lenIn = wcslen(in);

	if (outLen / 2 < lenIn)
	{
		return;
	}

	int j = 0;
	for (int i = 0; i < lenIn; ++i)
	{
		out[j] = in[i];
		++j;

		if (in[i] == after)
		{
			out[j] = insert;
			++j;
		}
	}

	out[j] = L'\0';
}

void CDbgOut::insertCharT(TCHAR* out, int outLen, const TCHAR *in, TCHAR after, TCHAR insert)
{
	if (out == NULL || in == NULL || insert == NULL)
	{
		return;
	}

	int lenIn = _tcslen(in);

	if (outLen / 2 < lenIn)
	{
		return;
	}


	int j = 0;
	for (int i = 0; i < lenIn; ++i)
	{
		out[j] = in[i];
		++j;

		if (in[i] == after)
		{
			out[j] = insert;
			++j;
		}
	}

	out[j] = _T('\0');
}
