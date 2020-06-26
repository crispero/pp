#pragma once
#include "ITask.h"
#include <vector>

class Pool
{
public:
	Pool(std::vector<ITask*>& tasks, int threadCountInPool);
	void Execute();

private:
	std::vector<HANDLE> m_handles;
	int m_threadCountInPool;
};

