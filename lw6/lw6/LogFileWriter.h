#pragma once
#include <string>
#include <fstream>

class LogFileWriter
{
public:
	LogFileWriter(std::string fileName);

	void WriteInFile(std::string str);
	
private:
	std::ofstream m_output;
};

