#include "LogBuffer.h"

LogBuffer::LogBuffer(LogFileWriter* logFileWriter)
	: m_logFileWriter(logFileWriter)
	, m_crititalSection(CRITICAL_SECTION())
{
	if (!InitializeCriticalSectionAndSpinCount(&m_crititalSection, 0x00000400))
	{
		throw std::exception("Can't init critical section");
	}

	InitEventHandle();
	InitLogSizeMonitoringThread();
}

LogBuffer::~LogBuffer()
{
	CloseHandle(m_handleLogSizeMonitoring);
	CloseHandle(m_handleEvent);
	DeleteCriticalSection(&m_crititalSection);
}

void LogBuffer::Log(std::string str)
{
	EnterCriticalSection(&m_crititalSection);

	if (m_stack.GetSize() > MAX_SIZE)
	{
		SetEvent(m_handleEvent);

		if (WaitForSingleObject(m_handleLogSizeMonitoring, INFINITE) == WAIT_OBJECT_0)
		{
			ResetEvent(m_handleEvent);
			InitLogSizeMonitoringThread();
		}
	}

	m_stack.Push(str);
	LeaveCriticalSection(&m_crititalSection);
}

DWORD WINAPI LogBuffer::LogSizeMonitoringThread(CONST LPVOID lpParam)
{
	LogBuffer* logBuffer = static_cast<LogBuffer*>(lpParam);

	if (WaitForSingleObject(logBuffer->m_handleEvent, INFINITE) == WAIT_OBJECT_0)
	{
		for (size_t i = 0; i < logBuffer->m_stack.GetSize(); i++)
		{
			logBuffer->m_logFileWriter->WriteInFile(logBuffer->m_stack.GetTop());
			logBuffer->m_stack.Pop();
		}
	}

	ExitThread(0);
}

void LogBuffer::InitLogSizeMonitoringThread()
{
	m_handleLogSizeMonitoring = CreateThread(nullptr, 0, &LogSizeMonitoringThread, (void*)this, 0, nullptr);
}

void LogBuffer::InitEventHandle()
{
	m_handleEvent = CreateEvent(nullptr, TRUE, FALSE, (LPCWSTR)"HandleEvent");
}
