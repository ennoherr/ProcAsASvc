#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	int StartServiceCtrlDispatcherWrapper(void);
	//int StopServiceCtrlDispatcherWrapper(void) { return 0; };


	VOID WINAPI ServiceMain(int iArgc, TCHAR **szArgv);
	VOID WINAPI ServiceCtrlHandler(DWORD dwCtrlCode);

	// ´helper
	int CmdInstallService(void);
	int CmdRemoveService(void);
	int CmdStartService(void);
	int CmdStopService(void);
	//void __cdecl StopServiceWrapper(void *dummy) { CmdStopService(); };
	int CmdShellService(int argc, TCHAR **argv);

	BOOL WINAPI CmdControlHandler(DWORD dwCtrlType);
	LPTSTR GetLastErrorText(LPTSTR lpszBuf, DWORD dwSize);

	int ServiceStartEntry(int argc, TCHAR **argv, bool isService);
	int ServiceStopEntry(void);

	// -------------------------------------------------------------------------------------------------
	BOOL ServiceUpdateStatus(DWORD newState);
	//BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
	void AddToMessageLog(LPTSTR lpszMsg);


#ifdef __cplusplus
}
#endif // __cplusplus
