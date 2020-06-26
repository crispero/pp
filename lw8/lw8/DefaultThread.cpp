#include "DefaultThread.h"

static DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	ITask* task = static_cast<ITask*>(lpParam);
	task->Execute();
	ExitThread(0);
}

DefaultThread::DefaultThread(std::vector<ITask*>& tasks)
{
	m_handles.resize(tasks.size());
	for (int i = 0; i < tasks.size(); i++)
	{
		m_handles[i] = CreateThread(NULL, 0, &ThreadProc, tasks[i], CREATE_SUSPENDED, NULL);
	}
}

void DefaultThread::Execute()
{
	for (const auto& handle : m_handles)
	{
		ResumeThread(handle);
	}

	WaitForMultipleObjects((DWORD)m_handles.size(), m_handles.data(), true, INFINITE);
}
