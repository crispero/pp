#pragma once
#include "ITask.h"
#include <vector>

class DefaultThread
{
public:
	DefaultThread(std::vector<ITask*>& tasks);
	void Execute();
private:
	std::vector<HANDLE> m_handles;
};

