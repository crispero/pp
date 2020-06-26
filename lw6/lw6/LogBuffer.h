#pragma once
#include "CMyStack.h"
#include "LogFileWriter.h"
#include <Windows.h>

const size_t MAX_SIZE = 300;

class LogBuffer
{
public:
	LogBuffer(LogFileWriter* logFileWriter);
	~LogBuffer();
	void Log(std::string str);

private:
	static DWORD WINAPI LogSizeMonitoringThread(CONST LPVOID lpParam);
	void InitLogSizeMonitoringThread();
	void InitEventHandle();
	CMyStack<std::string> m_stack;
	LogFileWriter* m_logFileWriter;
	CRITICAL_SECTION m_crititalSection;
	HANDLE m_handleLogSizeMonitoring;
	HANDLE m_handleEvent;
};

