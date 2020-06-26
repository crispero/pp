#pragma once
#include <Windows.h>

class ITask
{
public:
	virtual ~ITask() = default;
	virtual void Execute() = 0;
};