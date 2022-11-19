#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <queue>

//#include "Socket.h"
//#include "WorkerSocket.h"
//#include "WorkerCmds.h"

class CWorkerInit
{
public:
	CWorkerInit();
	~CWorkerInit();

	int start(int argc, TCHAR *argv[]);
	int stop(void);

private:
	std::shared_ptr<std::atomic<bool>> m_isRunning;
	std::thread m_initLoop;
	std::thread m_sockLoop;
	std::thread m_cmdLoop;
	std::thread m_logCleanup;
	std::shared_ptr<std::mutex> m_mtxCmds;
	std::shared_ptr<std::mutex> m_mtxResps;

	std::shared_ptr<std::queue<std::string>> m_qCmds;
	std::shared_ptr<std::queue<std::string>> m_qResps;

	//std::shared_ptr<CSetting> m_Setting;
	//CWorkerSocket m_sockWork;
	//CWorkerCmds m_cmdWork;

	//log
	std::string m_logDir;
	std::string m_logNamePrefix;
	int m_logCleanupLoopSleep;

	int initLoop(int argc, TCHAR *argv[]);
	void initAndcleanLogDirLoop();
};

