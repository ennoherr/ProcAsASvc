#include "stdafx.h"
#include <atomic>
#include <thread>
#include <string>

#include "DbgOut.h"
//#include "UniConvert.h"
//#include "Setting.h"

#include "ProcAsASvc.h"
#include "WorkerInit.h"


CWorkerInit::CWorkerInit()
	: m_isRunning(new std::atomic<bool>(false))
	, m_qCmds(new std::queue<std::string>())
	, m_qResps(new std::queue<std::string>())
	, m_mtxCmds(new std::mutex)
	, m_mtxResps(new std::mutex)
	, m_initLoop()
	//, m_Setting(new CSetting)
	//, m_sockWork(m_isRunning, m_qCmds, m_mtxCmds, m_qResps, m_mtxResps, m_Setting)
	//, m_cmdWork(m_isRunning, m_qCmds, m_mtxCmds, m_qResps, m_mtxResps, m_Setting)
	// logs
	, m_logDir("logs")
	//, m_logNamePrefix(g_pBaseName)
	, m_logCleanupLoopSleep(60 * 60 * 1000) // once an hour
{
}


CWorkerInit::~CWorkerInit()
{
	stop();
}

int CWorkerInit::start(int argc, TCHAR * argv[])
{
	*m_isRunning = true;

	m_logCleanup = std::thread(&CWorkerInit::initAndcleanLogDirLoop, this);
	m_initLoop = std::thread(&CWorkerInit::initLoop, this, argc, argv);
	//m_sockLoop = std::thread(&CWorkerSocket::startLoop, m_sockWork);
	//m_cmdLoop = std::thread(&CWorkerCmds::startLoop, m_cmdWork);

	return 0;
}

int CWorkerInit::stop(void)
{
	*m_isRunning = false;

	if (m_initLoop.joinable())
	{
		m_initLoop.join();
	}

	if (m_sockLoop.joinable())
	{
		m_sockLoop.join();
	}

	if (m_cmdLoop.joinable())
	{
		m_cmdLoop.join();
	}

	if (m_logCleanup.joinable())
	{
		m_logCleanup.join();
	}

	return 0;

}

int CWorkerInit::initLoop(int argc, TCHAR *argv[])
{
	while (*m_isRunning)
	{
		Sleep(1);
	}

	return 0;
}

void CWorkerInit::initAndcleanLogDirLoop()
{
	/*
	tstring logDir = m_Setting->ExePath() + CUniConvert::s2ts(m_logDir);
	if (!CreateDirectory(logDir.c_str(), NULL))
	{
		DWORD err = GetLastError();
		tstring s = _T("");
		if (err == ERROR_ALREADY_EXISTS) s = _T("DEBUG: init - CreateDirectory - Directory \'") + logDir + _T("\' already exists.");
		else if (err == ERROR_PATH_NOT_FOUND) s = _T("ERROR: CreateDirectory - PathNotFound to directory \'") + logDir + _T("\'");
		else s = _T("ERROR: init - CreateDirectory - Error ") + CUniConvert::ul2ts(err) + _T(" when creating directory \'") + logDir + _T("\'");
		CDbgOut::DebugInfoT(s.c_str());
	}

	CDbgOut::setLogFileBase(CUniConvert::ts2s(logDir) + "\\" + m_logNamePrefix);

	if (m_logCleanupLoopSleep <= 0) return;

	while (*m_isRunning)
	{
		CDbgOut::cleanLogDir(CUniConvert::ts2s(logDir), m_Setting->GetLogKeepDays());

		for (int i = 0; i < m_logCleanupLoopSleep; i++)
		{
			if (!*m_isRunning) break;
			Sleep(1);
		}
	}
	*/
}


