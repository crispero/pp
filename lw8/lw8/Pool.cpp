#include "Pool.h"

static DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	ITask* task = static_cast<ITask*>(lpParam);
	task->Execute();
	ExitThread(0);
}

Pool::Pool(std::vector<ITask*>& tasks, int threadCountInPool)
	: m_threadCountInPool(threadCountInPool)
{
	m_handles.resize(tasks.size());
	for (size_t i = 0; i < tasks.size(); i++)
	{
		m_handles[i] = CreateThread(NULL, 0, &ThreadProc, tasks[i], CREATE_SUSPENDED, NULL);
	}
}

void Pool::Execute()
{
	int index = 0;
	for (size_t i = 0; i < m_handles.size(); i++)
	{
		index++;
		ResumeThread(m_handles[i]);
		if (index == m_threadCountInPool)
		{
			index = 0;
			WaitForMultipleObjects((DWORD)i + 1, m_handles.data(), true, INFINITE);
		}
	}
	WaitForMultipleObjects((DWORD)m_handles.size(), m_handles.data(), true, INFINITE);
}