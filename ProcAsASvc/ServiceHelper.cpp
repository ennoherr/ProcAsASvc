#include "stdafx.h"
#include "DbgOut.h"
#include "WorkerInit.h"
#include "ServiceHelper.h"

#include "ProcAsASvc.h"

#define SERVICE_REFRESH 0xFFFFFFFF

SERVICE_STATUS          ssStatus;
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD					dwErr;

bool _isService = false;
volatile bool _isRunning = false;
CWorkerInit _worker;


int StartServiceCtrlDispatcherWrapper(void)
{
	SERVICE_TABLE_ENTRY dispatchTable[] =
	{
		{ g_pServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher(dispatchTable))
	{
		AddToMessageLog(LPTSTR("StartServiceCtrlDispatcher failed."));
		CDbgOut::dbgprintfT(_T("ERROR: Service Dispatcher returned with error = %d!"), GetLastError());

		return 1;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Entry point for starting the services main loop. </summary>
///
/// <remarks>   Enno Herr, 14.09.2011. </remarks>
///
/// <param name="iArgc">        number of args given on the cmd line </param>
/// <param name="szArgv">       [in] arguments values. </param>
///
/// <returns>   0 on success, else error. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

int ServiceStartEntry(int argc, TCHAR **argv, bool isService)
{
	int iRet = 0;

	if (_worker.start(argc, argv) == 0)
	{
		_isRunning = true;
	}

	_tprintf_s(_T("\n---------------------\n%s INFO: Press CTRL+C to quit...\n---------------------\n"), g_pServiceName);

	while (!isService && _isRunning)
	{
		Sleep(1);
	}

	return iRet;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Entry point for stopping the service, after the 
///                     main loop should have been stopped. 
///                     Should perform error handling if the service could not
///                     be stopped.
///                     Called by the control handler by default when stopping 
///                     the service. </summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <returns>   0 on success, else error. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

int ServiceStopEntry()
{
	CDbgOut::dbgprintfT(_T("%s DEBUG: ServiceStopEntry(...) stopping all threads. Please wait..."), g_pServiceName);

	_isRunning = false;

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Service main, which performs the actual initialization of the service. 
///                     The routine performs the service initialization and then calls 
///                     the ServiceStartEntry(...) routine to perform the actual work.</summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <param name="iArgc">        number of args given on the cmd line </param>
/// <param name="szArgv">       [in] arguments values. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

void WINAPI ServiceMain(int iArgc, TCHAR **szArgv)
{
#ifdef _DEBUG
	Sleep(20000);
#endif // _DEBUG

	_isService = true;

	// register our service control handler:
	sshStatusHandle = RegisterServiceCtrlHandler(g_pServiceName, ServiceCtrlHandler);

	if (!sshStatusHandle)
	{
		ServiceUpdateStatus(SERVICE_STOPPED);
		return;
	}

	// report the status to the service control manager.
	if (!ServiceUpdateStatus(SERVICE_START_PENDING)) 
	{
		(VOID)ServiceUpdateStatus(SERVICE_STOPPED);
		return;
	}

	// starting the services main loop
	ServiceStartEntry(iArgc, szArgv, true);

	ServiceUpdateStatus(SERVICE_RUNNING);

	// try to report the stopped status to the service control manager.
	//if (sshStatusHandle)
	//{
	//	(VOID)ServiceUpdateStatus(SERVICE_STOPPED);
	//}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Service control handler: 
///                     The function is called by the SCM whenever ControlService()is 
///                     called on this service.</summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <param name="dwCtrlCode">   control handler event </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

VOID WINAPI ServiceCtrlHandler(DWORD dwCtrlCode)
{
	// Handle the requested control code.
	switch (dwCtrlCode)
	{
		// Stop the service.
		//
		// SERVICE_STOP_PENDING should be reported before
		// setting the Stop Event - hServerStopEvent - in
		// ServiceStopEntry().  This avoids a race condition
		// which may result in a 1053 - The Service did not respond...
		// error.
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		ServiceUpdateStatus(SERVICE_STOP_PENDING);
		ServiceStopEntry();
		ServiceUpdateStatus(SERVICE_STOPPED);
		return;

		// Update the service status.
	case SERVICE_CONTROL_INTERROGATE:
		break;

		// invalid control code
	default:
		break;
	}

	ServiceUpdateStatus(ssStatus.dwCurrentState);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Sets the current status of the service and 
///                     reports it to the Service Control Manager. </summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <param name="dwCurrentState">       the state of the service </param>
/// <param name="dwWin32ExitCode">      error code to report </param>
/// <param name="dwWaitHint">           worst case estimate to next checkpoint </param>
///
/// <returns>   true if it succeeds, false if it fails. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL ServiceUpdateStatus(DWORD newState)
{
	if (!_isService) return TRUE;

	//static SERVICE_STATUS serviceStatus;

	ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ssStatus.dwWin32ExitCode = NO_ERROR;
	ssStatus.dwServiceSpecificExitCode = NO_ERROR;

	switch (newState)
	{
	case SERVICE_START_PENDING:
		ssStatus.dwControlsAccepted = 0;
		ssStatus.dwCheckPoint = 1;
		ssStatus.dwWaitHint = 1000;
		ssStatus.dwCurrentState = SERVICE_START_PENDING;
		AddToMessageLog(LPTSTR("Service start pending"));
		break;
	case SERVICE_RUNNING:
		ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
		ssStatus.dwCheckPoint = 0;
		ssStatus.dwCurrentState = SERVICE_RUNNING;
		AddToMessageLog(LPTSTR("Service running"));
		break;
	case SERVICE_STOP_PENDING:
		ssStatus.dwControlsAccepted = 0;
		ssStatus.dwCheckPoint = 1;
		ssStatus.dwWaitHint = 1000;
		ssStatus.dwCurrentState = SERVICE_STOP_PENDING;
		AddToMessageLog(LPTSTR("Service stop pending"));
		break;
	case SERVICE_STOPPED:
		ssStatus.dwControlsAccepted = 0;
		ssStatus.dwCheckPoint = 0;
		ssStatus.dwCurrentState = SERVICE_STOPPED;
		AddToMessageLog(LPTSTR("Service stopped"));
		break;
	case SERVICE_REFRESH:
		if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING ||
			ssStatus.dwCurrentState == SERVICE_START_PENDING)
			ssStatus.dwCheckPoint++;
		break;
	}
	if (!SetServiceStatus(sshStatusHandle, &ssStatus)) {
		TCHAR sz[512];
		_tcprintf_s(sz, _T("SetServiceStatus failed, error 0x%08X"), GetLastError());
		AddToMessageLog(sz);
		return FALSE;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Allows any thread to log an error message.  </summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <param name="lpszMsg">      The message to be logged. </param>
////////////////////////////////////////////////////////////////////////////////////////////////////

void AddToMessageLog(LPTSTR lpszMsg)
{
	CDbgOut::dbgprintfT(lpszMsg);

	TCHAR szMsg[(sizeof(g_pServiceName) / sizeof(TCHAR)) + 100];
	HANDLE  hEventSource = NULL;
	LPTSTR  lpszStrings[2];
	LPTSTR* lpszStrPointer = lpszStrings;

	dwErr = GetLastError();

	// Use event logging to log the error.
	hEventSource = RegisterEventSource(NULL, g_pServiceName);

	_stprintf_s(szMsg, _T("%s ERROR: %d"), g_pServiceName, dwErr);
	CDbgOut::dbgprintfT(szMsg);

	lpszStrings[0] = szMsg;
	lpszStrings[1] = lpszMsg;

	if (hEventSource != NULL)
	{
		ReportEvent(hEventSource,   // handle of event source
			EVENTLOG_ERROR_TYPE,    // event type
			0,                      // event category
			0,                      // event ID
			NULL,                   // current user's SID
			2,                      // strings in lpszStrings
			0,                      // no bytes of raw data
			(LPCTSTR*)lpszStrings,  // array of error strings
			NULL);                  // no raw data

		(void)DeregisterEventSource(hEventSource);
	}
}


// -------------------------------------------------------------------------------------------------
//
//  The following code handles service installation and removal
//


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Command installs the service. </summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <returns>   0 on success, else error </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

int CmdInstallService(void)
{
	SC_HANDLE   schService;
	SC_HANDLE   schSCManager;

	TCHAR szPath[512];
	int iRet = 0;

	// get path and program name
	if (GetModuleFileName(NULL, szPath, 512) == 0)
	{
		LPTSTR szErr = LPTSTR(_T(""));
		CDbgOut::dbgprintfT(_T("%s ERROR: Unable to install service, Error: %s!"), 
			g_pServiceName, 
			GetLastErrorText(szErr, 256));

		return 1;
	}
	// add the param "-service"
	// not needed anymore
	//_tcscat_s(szPath, _T(" -service"));

	schSCManager = OpenSCManager(NULL,                                                                                           // machine (NULL == local)
		NULL,                                                                                           // database (NULL == default)
		SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);

	if (schSCManager)
	{
		schService = CreateService(schSCManager,    // SCManager database
			g_pServiceName,							// name of service
			g_pServiceName,							// name to display
			SERVICE_QUERY_STATUS,                   // desired access
			SERVICE_WIN32_OWN_PROCESS,              // service type
			SERVICE_AUTO_START,                     // start type
			SERVICE_ERROR_NORMAL,                   // error control type
			szPath,                                 // service's binary
			NULL,                                   // no load ordering group
			NULL,                                   // no tag identifier
			NULL,				                    // dependencies
			NULL,                                   // LocalSystem account
			NULL);                                  // no password

		if (schService)
		{
			CDbgOut::dbgprintfT(_T("%s STATUS: service installed."), g_pServiceName);
			CloseServiceHandle(schService);
		}
		else
		{
			LPTSTR szErr = LPTSTR(_T(""));
			CDbgOut::dbgprintfT(_T("%s ERROR: CreateService failed, Error: %s"),
				g_pServiceName, 
				GetLastErrorText(szErr, 256));
			iRet = 2;
		}

		CloseServiceHandle(schSCManager);
	}
	else
	{
		LPTSTR szErr = LPTSTR(_T(""));
		CDbgOut::dbgprintfT(_T("%s ERROR: OpenSCManager failed, Error: %s"),
			g_pServiceName, 
			GetLastErrorText(szErr, 256));
		iRet = 3;
	}

	return iRet;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Command removes the service. </summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <returns>   0 on success, else error </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

int CmdRemoveService(void)
{
	// stop the service before removing it.
	if (CmdStopService() != 0)
	{
		return 1;
	}

	SC_HANDLE   schService;
	SC_HANDLE   schSCManager;

	int iRet = 0;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);

	if (schSCManager)
	{
		schService = OpenService(schSCManager, g_pServiceName, DELETE | SERVICE_STOP | SERVICE_QUERY_STATUS);

		if (schService)
		{
			// now remove the service
			if (DeleteService(schService))
			{
				CDbgOut::dbgprintfT(_T("%s STATUS: service removed."), g_pServiceName);
			}
			else
			{
				LPTSTR szErr = LPTSTR(_T(""));
				CDbgOut::dbgprintfT(_T("%s ERROR: DeleteService failed, Error: %s"),
					g_pServiceName, 
					GetLastErrorText(szErr, 256));

				iRet = 2;
			}

			CloseServiceHandle(schService);
		}
		else
		{
			LPTSTR szErr = LPTSTR(_T(""));
			CDbgOut::dbgprintfT(_T("%s ERROR: OpenService failed, Error: %s"),
				g_pServiceName, 
				GetLastErrorText(szErr, 256));

			iRet = 3;
		}

		CloseServiceHandle(schSCManager);
	}
	else
	{
		LPTSTR szErr = LPTSTR(_T(""));
		CDbgOut::dbgprintfT(_T("%s ERROR: OpenSCManager failed, Error: %s"),
			GetLastErrorText(szErr, 256));
		iRet = 4;
	}

	return iRet;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Command starts the service. </summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <returns>   0 on success, else error </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

int CmdStartService(void)
{
	SC_HANDLE   schService;
	SC_HANDLE   schSCManager;

	int iRet = 0;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

	if (schSCManager)
	{
		schService = OpenService(schSCManager, g_pServiceName, SERVICE_START | SERVICE_QUERY_STATUS);

		if (schService)
		{
			// try to start the service
			if (StartService(schService, 0, 0))
			{
				CDbgOut::dbgprintfT(_T("%s STATUS: starting service."), g_pServiceName);
				Sleep(1000);

				while (QueryServiceStatus(schService, &ssStatus))
				{
					if (ssStatus.dwCurrentState == SERVICE_START_PENDING)
					{
						Sleep(1000);
					}
					else
					{
						break;
					}
				}

				if (ssStatus.dwCurrentState == SERVICE_RUNNING)
				{
					CDbgOut::dbgprintfT(_T("%s STATUS: service started."), g_pServiceName);
				}
				else
				{
					CDbgOut::dbgprintfT(_T("%s ERROR: failed to start service."), g_pServiceName);
					iRet = 1;
				}
			}

			CloseServiceHandle(schService);
		}
		else
		{
			LPTSTR szErr = LPTSTR(_T(""));
			CDbgOut::dbgprintfT(_T("%s ERROR: OpenService failed, Error: %s"),
				g_pServiceName, 
				GetLastErrorText(szErr, 256));

			iRet = 2;
		}

		CloseServiceHandle(schSCManager);
	}
	else
	{
		LPTSTR szErr = LPTSTR(_T(""));
		CDbgOut::dbgprintfT(_T("%s ERROR: OpenSCManager failed, Error: %s"),
			GetLastErrorText(szErr, 256));

		iRet = 3;
	}

	return iRet;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Command stops the service. </summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <returns>   0 on success, else error </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

int CmdStopService(void)
{
	SC_HANDLE   schService;
	SC_HANDLE   schSCManager;

	int iRet = 0;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

	if (schSCManager)
	{
		schService = OpenService(schSCManager, g_pServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS);

		if (schService)
		{
			// try to stop the service
			if (ControlService(schService, SERVICE_CONTROL_STOP, &ssStatus))
			{
				CDbgOut::dbgprintfT(_T("%s STATUS: stopping service."), g_pServiceName);
				Sleep(1000);

				long lCounter = 0;
				while (QueryServiceStatus(schService, &ssStatus))
				{
					if (ssStatus.dwCurrentState == SERVICE_STOPPED)
					{
						CDbgOut::dbgprintfT(_T("%s STATUS: service stopped."), g_pServiceName);
						break;
					}
					if (lCounter == 20) // 10 seconds
					{
						CDbgOut::dbgprintfT(_T("%s STATUS: timeout occured while trying to stop service."), g_pServiceName);
						break;
					}

					Sleep(500);
					lCounter++;
				}

				if (ssStatus.dwCurrentState != SERVICE_STOPPED)
				{
					CDbgOut::dbgprintfT(_T("%s ERROR: failed to stop service."), g_pServiceName);
					iRet = 1;
				}
			}

			CloseServiceHandle(schService);
		}
		else
		{
			LPTSTR szErr = LPTSTR(_T(""));
			CDbgOut::dbgprintfT(_T("%s ERROR: OpenService failed, Error: %s"),
				g_pServiceName, 
				GetLastErrorText(szErr, 256));

			iRet = 2;
		}

		CloseServiceHandle(schSCManager);
	}
	else
	{
		LPTSTR szErr = LPTSTR(_T(""));
		CDbgOut::dbgprintfT(_T("%s ERROR: OpenSCManager failed, Error: %s"), 
			GetLastErrorText(szErr, 256));

		iRet = 2;
	}

	return iRet;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Start program in a shell, not as a service. </summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <param name="argc"> number of arguments. </param>
/// <param name="argv"> [in] array of arguments, directly from cmd line. </param>
///
/// <returns>   0 on success, else error </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

int CmdShellService(int argc, TCHAR** argv)
{
	int iRet = 0;

	if (iRet == 0 && !SetConsoleCtrlHandler(CmdControlHandler, TRUE))
	{
		CDbgOut::dbgprintfT(_T("%s ERROR (Console): SetConsoleCtrlHandler(...) failed!"), 
			g_pServiceName);

		iRet = 2;
	}

	if (iRet == 0)
	{
		_isService = false;
	}

	if (iRet == 0 && ServiceStartEntry(argc, argv, false) != 0)
	{
		CDbgOut::dbgprintfT(_T("%s ERROR (Console): ServiceStartEntry(...) returned an error!"), g_pServiceName);
		iRet = 2;
	}

	return iRet;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Control handler when the program is used as console program. </summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <param name="dwCtrlCode">   control handler event </param>
///
/// <returns>   true when handled, else false </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI CmdControlHandler(DWORD dwCtrlCode)
{
	switch (dwCtrlCode)
	{
		// use Ctrl+C or Ctrl+Break to simulate
	case CTRL_BREAK_EVENT:
		// SERVICE_CONTROL_STOP in debug mode
	case CTRL_C_EVENT:
		CDbgOut::dbgprintfT(_T("%s STATUS: Stopping program."), g_pServiceName);
		ServiceStopEntry();
		_isRunning = false;
		return TRUE;
	}
	return FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>   Copies error message text to string. </summary>
///
/// <remarks>   Enno Herr, 15.09.2011. </remarks>
///
/// <param name="lpszBuf">      [out] destination buffer </param>
/// <param name="dwSize">       size of buffer </param>
///
/// <returns>   destination buffer </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////

LPTSTR GetLastErrorText(LPTSTR lpszBuf, DWORD dwSize)
{
	DWORD dwRet;
	LPTSTR lpszTemp = NULL;

	dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL,
		GetLastError(),
		LANG_NEUTRAL,
		(LPTSTR)&lpszTemp,
		0,
		NULL);

	// supplied buffer is not long enough
	if (!dwRet || ((long)dwSize < (long)dwRet + 14))
	{
		lpszBuf[0] = _T('\0');
	}
	else
	{
		if (NULL != lpszTemp)
		{
			lpszTemp[lstrlen(lpszTemp) - 2] = _T('\0'); //remove cr and newline character
			_stprintf_s(lpszBuf, dwSize, _T("%s (0x%x)"), lpszTemp, GetLastError());
		}
	}

	if (NULL != lpszTemp)
	{
		LocalFree((HLOCAL)lpszTemp);
	}

	return lpszBuf;
}
