////////////////////////////////////////////////////////////////////// 
// NT Service Stub Code (For XYROOT )
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <winbase.h>
#include <winsvc.h>
#include <process.h>


const int nBufferSize = 500;
TCHAR pServiceName[nBufferSize + 1];
TCHAR pExeFile[nBufferSize + 1];
TCHAR pInitFile[nBufferSize + 1];
TCHAR pLogFile[nBufferSize + 1];
const int nMaxProcCount = 127;
PROCESS_INFORMATION pProcInfo[nMaxProcCount];


SERVICE_STATUS          serviceStatus;
SERVICE_STATUS_HANDLE   hServiceStatusHandle;

VOID WINAPI ProcAsASvcMain(DWORD dwArgc, LPTSTR* lpszArgv);
VOID WINAPI ProcAsASvcHandler(DWORD fdwControl);

CRITICAL_SECTION myCS;

void WriteLog(TCHAR* pMsg)
{
	// write error or other information into log file
	::EnterCriticalSection(&myCS);
	try
	{
		SYSTEMTIME oT;
		::GetLocalTime(&oT);
		FILE* pLog = NULL;
		int err = _tfopen_s(&pLog, pLogFile, _T("a"));
		_ftprintf_s(pLog, _T("%04d-%02d-%02d_T%02d:%02d:%02d; %s\n"), oT.wYear, oT.wMonth, oT.wDay, oT.wHour, oT.wMinute, oT.wSecond, pMsg);
		fclose(pLog);
	}
	catch (...) {}
	::LeaveCriticalSection(&myCS);
}

////////////////////////////////////////////////////////////////////// 
//
// Configuration Data and Tables
//

SERVICE_TABLE_ENTRY   DispatchTable[] =
{
	{pServiceName, ProcAsASvcMain},
	{NULL, NULL}
};


// helper functions

BOOL StartProcess(int nIndex)
{
	// start a process with given index
	STARTUPINFO startUpInfo = { sizeof(STARTUPINFO),
		NULL,
		LPTSTR(""),
		NULL,
		0,0,0,0,0,0,0,
		STARTF_USESHOWWINDOW,
		0,0,NULL,0,0,0 };

	TCHAR pItem[nBufferSize + 1];
	_stprintf_s(pItem, _T("Process%d\0"), nIndex);
	TCHAR pCommandLine[nBufferSize + 1];
	GetPrivateProfileString(pItem, _T("CommandLine"), _T(""), pCommandLine, nBufferSize, pInitFile);

	if (_tcslen(pCommandLine) > 4)
	{
		TCHAR pWorkingDir[nBufferSize + 1];
		GetPrivateProfileString(pItem, _T("WorkingDir"), _T(""), pWorkingDir, nBufferSize, pInitFile);
		TCHAR pUserName[nBufferSize + 1];
		GetPrivateProfileString(pItem, _T("UserName"), _T(""), pUserName, nBufferSize, pInitFile);
		TCHAR pPassword[nBufferSize + 1];
		GetPrivateProfileString(pItem, _T("Password"), _T(""), pPassword, nBufferSize, pInitFile);
		TCHAR pDomain[nBufferSize + 1];
		GetPrivateProfileString(pItem, _T("Domain"), _T(""), pDomain, nBufferSize, pInitFile);
		BOOL bImpersonate = (_tcslen(pUserName) > 0 && _tcslen(pPassword) > 0);
		TCHAR pUserInterface[nBufferSize + 1];
		GetPrivateProfileString(pItem, _T("UserInterface"), _T("N"), pUserInterface, nBufferSize, pInitFile);
		BOOL bUserInterface = (bImpersonate == FALSE) && 
			(pUserInterface[0] == _T('y') || pUserInterface[0] == _T('Y') || pUserInterface[0] == _T('1')) ? TRUE : FALSE;

		TCHAR CurrentDesktopName[512];
		// set the correct desktop for the process to be started
		if (bUserInterface)
		{
			startUpInfo.wShowWindow = SW_SHOW;
			startUpInfo.lpDesktop = NULL;
		}
		else
		{
			HDESK hCurrentDesktop = GetThreadDesktop(GetCurrentThreadId());
			DWORD len;
			GetUserObjectInformation(hCurrentDesktop, UOI_NAME, CurrentDesktopName, MAX_PATH, &len);
			startUpInfo.wShowWindow = SW_HIDE;
			startUpInfo.lpDesktop = (bImpersonate == FALSE) ? CurrentDesktopName : LPTSTR("");
		}
		if (bImpersonate == FALSE)
		{

			// create the process
			if (CreateProcess(NULL, pCommandLine, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, _tcslen(pWorkingDir) == 0 ? NULL : pWorkingDir, &startUpInfo, &pProcInfo[nIndex]))
			{
				TCHAR pPause[nBufferSize + 1];
				GetPrivateProfileString(pItem, _T("PauseStart"), _T("100"), pPause, nBufferSize, pInitFile);
				Sleep(_ttoi(pPause));
				return TRUE;
			}
			else
			{
				long nError = GetLastError();
				TCHAR pTemp[121];
				_stprintf_s(pTemp, _T("Failed to start program '%s', error code = %d"), pCommandLine, nError);
				WriteLog(pTemp);
				return FALSE;
			}
		}
		else
		{
			HANDLE hToken = NULL;
			if (LogonUser(pUserName, (_tcslen(pDomain) == 0) ? _T(".") : pDomain, pPassword, LOGON32_LOGON_SERVICE, LOGON32_PROVIDER_DEFAULT, &hToken))
			{
				if (CreateProcessAsUser(hToken, NULL, pCommandLine, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, (_tcslen(pWorkingDir) == 0) ? NULL : pWorkingDir, &startUpInfo, &pProcInfo[nIndex]))
				{
					TCHAR pPause[nBufferSize + 1];
					GetPrivateProfileString(pItem, _T("PauseStart"), _T("100"), pPause, nBufferSize, pInitFile);
					Sleep(_ttoi(pPause));
					return TRUE;
				}
				long nError = GetLastError();
				TCHAR pTemp[121];
				_stprintf_s(pTemp, _T("Failed to start program '%s' as user '%s', error code = %d"), pCommandLine, pUserName, nError);
				WriteLog(pTemp);
				return FALSE;
			}
			long nError = GetLastError();
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("Failed to logon as user '%s', error code = %d"), pUserName, nError);
			WriteLog(pTemp);
			return FALSE;
		}
	}
	else return FALSE;
}

void EndProcess(int nIndex)
{
	// end a program started by the service
	if (pProcInfo[nIndex].hProcess)
	{
		TCHAR pItem[nBufferSize + 1];
		_stprintf_s(pItem, _T("Process%d\0"), nIndex);
		TCHAR pPause[nBufferSize + 1];
		GetPrivateProfileString(pItem, _T("PauseEnd"), _T("100"), pPause, nBufferSize, pInitFile);
		int nPauseEnd = _ttoi(pPause);
		// post a WM_QUIT message first
		PostThreadMessage(pProcInfo[nIndex].dwThreadId, WM_QUIT, 0, 0);
		// sleep for a while so that the process has a chance to terminate itself
		::Sleep(nPauseEnd > 0 ? nPauseEnd : 50);
		// terminate the process by force
		TerminateProcess(pProcInfo[nIndex].hProcess, 0);
		try // close handles to avoid ERROR_NO_SYSTEM_RESOURCES
		{
			::CloseHandle(pProcInfo[nIndex].hThread);
			::CloseHandle(pProcInfo[nIndex].hProcess);
		}
		catch (...) {}
		pProcInfo[nIndex].hProcess = 0;
		pProcInfo[nIndex].hThread = 0;
	}
}

BOOL BounceProcess(TCHAR* pName, int nIndex)
{
	// bounce the process with given index
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == 0)
	{
		long nError = GetLastError();
		TCHAR pTemp[121];
		_stprintf_s(pTemp, _T("OpenSCManager failed, error code = %d"), nError);
		WriteLog(pTemp);
	}
	else
	{
		// open the service
		SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService == 0)
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("OpenService failed, error code = %d"), nError);
			WriteLog(pTemp);
		}
		else
		{
			// call ControlService to invoke handler
			SERVICE_STATUS status;
			if (nIndex >= 0 && nIndex < 128)
			{
				if (ControlService(schService, (nIndex | 0x80), &status))
				{
					CloseServiceHandle(schService);
					CloseServiceHandle(schSCManager);
					return TRUE;
				}
				long nError = GetLastError();
				TCHAR pTemp[121];
				_stprintf_s(pTemp, _T("ControlService failed, error code = %d"), nError);
				WriteLog(pTemp);
			}
			else
			{
				TCHAR pTemp[121];
				_stprintf_s(pTemp, _T("Invalid argument to BounceProcess: %d"), nIndex);
				WriteLog(pTemp);
			}
			CloseServiceHandle(schService);
		}
		CloseServiceHandle(schSCManager);
	}
	return FALSE;
}

BOOL KillService(TCHAR* pName)
{
	// kill service with given name
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == 0)
	{
		long nError = GetLastError();
		TCHAR pTemp[121];
		_stprintf_s(pTemp, _T("OpenSCManager failed, error code = %d"), nError);
		WriteLog(pTemp);
	}
	else
	{
		// open the service
		SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService == 0)
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("OpenService failed, error code = %d"), nError);
			WriteLog(pTemp);
		}
		else
		{
			// call ControlService to kill the given service
			SERVICE_STATUS status;
			if (ControlService(schService, SERVICE_CONTROL_STOP, &status))
			{
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return TRUE;
			}
			else
			{
				long nError = GetLastError();
				TCHAR pTemp[121];
				_stprintf_s(pTemp, _T("ControlService failed, error code = %d"), nError);
				WriteLog(pTemp);
			}
			CloseServiceHandle(schService);
		}
		CloseServiceHandle(schSCManager);
	}
	return FALSE;
}

BOOL RunService(TCHAR* pName, int nArg, TCHAR** pArg)
{
	// run service with given name
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == 0)
	{
		long nError = GetLastError();
		TCHAR pTemp[121];
		_stprintf_s(pTemp, _T("OpenSCManager failed, error code = %d"), nError);
		WriteLog(pTemp);
	}
	else
	{
		// open the service
		SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService == 0)
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("OpenService failed, error code = %d"), nError);
			WriteLog(pTemp);
		}
		else
		{
			// call StartService to run the service
			if (StartService(schService, nArg, (const TCHAR**)pArg))
			{
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return TRUE;
			}
			else
			{
				long nError = GetLastError();
				TCHAR pTemp[121];
				_stprintf_s(pTemp, _T("StartService failed, error code = %d"), nError);
				WriteLog(pTemp);
			}
			CloseServiceHandle(schService);
		}
		CloseServiceHandle(schSCManager);
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////// 
//
// This routine gets used to start your service
//
VOID WINAPI ProcAsASvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
	DWORD   status = 0;
	DWORD   specificError = 0xfffffff;

	serviceStatus.dwServiceType = SERVICE_WIN32;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	serviceStatus.dwWin32ExitCode = 0;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;

	hServiceStatusHandle = RegisterServiceCtrlHandler(pServiceName, ProcAsASvcHandler);
	if (hServiceStatusHandle == 0)
	{
		long nError = GetLastError();
		TCHAR pTemp[121];
		_stprintf_s(pTemp, _T("RegisterServiceCtrlHandler failed, error code = %d"), nError);
		WriteLog(pTemp);
		return;
	}

	// Initialization complete - report running status 
	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;
	if (!SetServiceStatus(hServiceStatusHandle, &serviceStatus))
	{
		long nError = GetLastError();
		TCHAR pTemp[121];
		_stprintf_s(pTemp, _T("SetServiceStatus failed, error code = %d"), nError);
		WriteLog(pTemp);
	}

	for (int i = 0; i < nMaxProcCount; i++)
	{
		pProcInfo[i].hProcess = 0;
		StartProcess(i);
	}
}

////////////////////////////////////////////////////////////////////// 
//
// This routine responds to events concerning your service, like start/stop
//
VOID WINAPI ProcAsASvcHandler(DWORD fdwControl)
{
	switch (fdwControl)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		serviceStatus.dwCheckPoint = 0;
		serviceStatus.dwWaitHint = 0;
		// terminate all processes started by this service before shutdown
		{
			for (int i = nMaxProcCount - 1; i >= 0; i--)
			{
				EndProcess(i);
			}
			if (!SetServiceStatus(hServiceStatusHandle, &serviceStatus))
			{
				long nError = GetLastError();
				TCHAR pTemp[121];
				_stprintf_s(pTemp, _T("SetServiceStatus failed, error code = %d"), nError);
				WriteLog(pTemp);
			}
		}
		return;
	case SERVICE_CONTROL_PAUSE:
		serviceStatus.dwCurrentState = SERVICE_PAUSED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		serviceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		// bounce processes started by this service
		if (fdwControl >= 128 && fdwControl < 256)
		{
			int nIndex = fdwControl & 0x7F;
			// bounce a single process
			if (nIndex >= 0 && nIndex < nMaxProcCount)
			{
				EndProcess(nIndex);
				StartProcess(nIndex);
			}
			// bounce all processes
			else if (nIndex == 127)
			{
				for (int i = nMaxProcCount - 1; i >= 0; i--)
				{
					EndProcess(i);
				}
				for (int i = 0; i < nMaxProcCount; i++)
				{
					StartProcess(i);
				}
			}
		}
		else
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("Unrecognized opcode %d"), fdwControl);
			WriteLog(pTemp);
		}
	};
	if (!SetServiceStatus(hServiceStatusHandle, &serviceStatus))
	{
		long nError = GetLastError();
		TCHAR pTemp[121];
		_stprintf_s(pTemp, _T("SetServiceStatus failed, error code = %d"), nError);
		WriteLog(pTemp);
	}
}


////////////////////////////////////////////////////////////////////// 
//
// Uninstall
//
VOID UnInstall(TCHAR* pName)
{
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == 0)
	{
		long nError = GetLastError();
		TCHAR pTemp[121];
		_stprintf_s(pTemp, _T("OpenSCManager failed, error code = %d"), nError);
		WriteLog(pTemp);
	}
	else
	{
		SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService == 0)
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("OpenService failed, error code = %d"), nError);
			WriteLog(pTemp);
		}
		else
		{
			if (!DeleteService(schService))
			{
				TCHAR pTemp[121];
				_stprintf_s(pTemp, _T("Failed to delete service %s"), pName);
				WriteLog(pTemp);
			}
			else
			{
				TCHAR pTemp[121];
				_stprintf_s(pTemp, _T("Service %s removed"), pName);
				WriteLog(pTemp);
			}
			CloseServiceHandle(schService);
		}
		CloseServiceHandle(schSCManager);
	}
}

////////////////////////////////////////////////////////////////////// 
//
// Install
//
VOID Install(TCHAR* pPath, TCHAR* pName)
{
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (schSCManager == 0)
	{
		long nError = GetLastError();
		TCHAR pTemp[121];
		_stprintf_s(pTemp, _T("OpenSCManager failed, error code = %d"), nError);
		WriteLog(pTemp);
	}
	else
	{
		SC_HANDLE schService = CreateService
		(
			schSCManager,	/* SCManager database      */
			pName,			/* name of service         */
			pName,			/* service name to display */
			SERVICE_ALL_ACCESS,        /* desired access          */
			SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, /* service type            */
			SERVICE_AUTO_START,      /* start type              */
			SERVICE_ERROR_NORMAL,      /* error control type      */
			pPath,			/* service's binary        */
			NULL,                      /* no load ordering group  */
			NULL,                      /* no tag identifier       */
			NULL,                      /* no dependencies         */
			NULL,                      /* LocalSystem account     */
			NULL
		);                     /* no password             */
		if (schService == 0)
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("Failed to create service %s, error code = %d"), pName, nError);
			WriteLog(pTemp);
		}
		else
		{
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("Service %s installed"), pName);
			WriteLog(pTemp);
			CloseServiceHandle(schService);
		}
		CloseServiceHandle(schSCManager);
	}
}

void WorkerProc(void* pParam)
{
	int nCheckProcessSeconds = 0;
	TCHAR pCheckProcess[nBufferSize + 1];
	GetPrivateProfileString(_T("Settings"), _T("CheckProcess"), _T("0"), pCheckProcess, nBufferSize, pInitFile);
	int nCheckProcess = _ttoi(pCheckProcess);
	if (nCheckProcess > 0) nCheckProcessSeconds = nCheckProcess * 60;
	else
	{
		GetPrivateProfileString(_T("Settings"), _T("CheckProcessSeconds"), _T("600"), pCheckProcess, nBufferSize, pInitFile);
		nCheckProcessSeconds = _ttoi(pCheckProcess);
	}
	while (nCheckProcessSeconds > 0)
	{
		::Sleep(1000 * nCheckProcessSeconds);
		for (int i = 0; i < nMaxProcCount; i++)
		{
			if (pProcInfo[i].hProcess)
			{
				TCHAR pItem[nBufferSize + 1];
				_stprintf_s(pItem, _T("Process%d\0"), i);
				TCHAR pRestart[nBufferSize + 1];
				GetPrivateProfileString(pItem, _T("Restart"), _T("No"), pRestart, nBufferSize, pInitFile);
				if (pRestart[0] == 'Y' || pRestart[0] == 'y' || pRestart[0] == '1')
				{
					DWORD dwCode;
					if (::GetExitCodeProcess(pProcInfo[i].hProcess, &dwCode))
					{
						if (dwCode != STILL_ACTIVE)
						{
							try // close handles to avoid ERROR_NO_SYSTEM_RESOURCES
							{
								::CloseHandle(pProcInfo[i].hThread);
								::CloseHandle(pProcInfo[i].hProcess);
							}
							catch (...) {}
							if (StartProcess(i))
							{
								TCHAR pTemp[121];
								_stprintf_s(pTemp, _T("Restarted process %d"), i);
								WriteLog(pTemp);
							}
						}
					}
					else
					{
						long nError = GetLastError();
						TCHAR pTemp[121];
						_stprintf_s(pTemp, _T("GetExitCodeProcess failed, error code = %d"), nError);
						WriteLog(pTemp);
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////// 
//
// Standard C Main
//
int __cdecl _tmain(int argc, _TCHAR* argv[])
{
	// initialize global critical section
	::InitializeCriticalSection(&myCS);
	// initialize variables for .exe, .ini, and .log file names
	TCHAR pModuleFile[nBufferSize + 1];
	DWORD dwSize = GetModuleFileName(NULL, pModuleFile, nBufferSize);
	pModuleFile[dwSize] = 0;
	if (dwSize > 4 && pModuleFile[dwSize - 4] == '.')
	{
		_stprintf_s(pExeFile, _T("%s"), pModuleFile);
		pModuleFile[dwSize - 4] = 0;
		_stprintf_s(pInitFile, _T("%s.ini"), pModuleFile);
		_stprintf_s(pLogFile, _T("%s.log"), pModuleFile);
	}
	else
	{
		_tprintf_s(_T("Invalid module file name: %s\r\n"), pModuleFile);
		return 1;
	}
	WriteLog(pExeFile);
	WriteLog(pInitFile);
	WriteLog(pLogFile);
	// read service name from .ini file
	GetPrivateProfileString(_T("Settings"), _T("ServiceName"), _T("ProcAsASvc"), pServiceName, nBufferSize, pInitFile);
	WriteLog(pServiceName);
	// uninstall service if switch is "-u"
	if (argc == 2 && _tcsicmp(_T("-u"), argv[1]) == 0)
	{
		UnInstall(pServiceName);
	}
	// install service if switch is "-i"
	else if (argc == 2 && _tcsicmp(_T("-i"), argv[1]) == 0)
	{
		Install(pExeFile, pServiceName);
	}
	// bounce service if switch is "-b"
	else if (argc == 2 && _tcsicmp(_T("-b"), argv[1]) == 0)
	{
		KillService(pServiceName);
		RunService(pServiceName, 0, NULL);
	}
	// bounce a specifc program if the index is supplied
	else if (argc == 3 && _tcsicmp(_T("-b"), argv[1]) == 0)
	{
		int nIndex = _ttoi(argv[2]);
		if (BounceProcess(pServiceName, nIndex))
		{
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("Bounced process %d"), nIndex);
			WriteLog(pTemp);
		}
		else
		{
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("Failed to bounce process %d"), nIndex);
			WriteLog(pTemp);
		}
	}
	// kill a service with given name
	else if (argc == 3 && _tcsicmp(_T("-k"), argv[1]) == 0)
	{
		if (KillService(argv[2]))
		{
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("Killed service %s"), argv[2]);
			WriteLog(pTemp);
		}
		else
		{
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("Failed to kill service %s"), argv[2]);
			WriteLog(pTemp);
		}
	}
	// run a service with given name
	else if (argc >= 3 && _tcsicmp(_T("-r"), argv[1]) == 0)
	{
		if (RunService(argv[2], argc > 3 ? (argc - 3) : 0, argc > 3 ? (&(argv[3])) : NULL))
		{
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("Ran service %s"), argv[2]);
			WriteLog(pTemp);
		}
		else
		{
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("Failed to run service %s"), argv[2]);
			WriteLog(pTemp);
		}
	}
	// assume user is starting this service 
	else
	{
		// start a worker thread to check for dead programs (and restart if necessary)
		if (_beginthread(WorkerProc, 0, NULL) == -1)
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("_beginthread failed, error code = %d"), nError);
			WriteLog(pTemp);
		}
		// pass dispatch table to service controller
		if (!StartServiceCtrlDispatcher(DispatchTable))
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			_stprintf_s(pTemp, _T("StartServiceCtrlDispatcher failed, error code = %d"), nError);
			WriteLog(pTemp);
		}
		// you don't get here unless the service is shutdown
	}
	::DeleteCriticalSection(&myCS);

	return 0;
}

