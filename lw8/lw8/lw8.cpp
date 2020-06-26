#include "Blur.h"
#include "Pool.h"
#include "DefaultThread.h"
#include <filesystem>
#include <string>
#include <iostream>
#include <ctime>

std::vector<ThreadData> GetThreadsData(bitmap_image& inputImage, bitmap_image* outputImage, int blockCount)
{
	std::vector<ThreadData> threadsData;

	const int imageWidth = inputImage.width();
	const int widthForThread = imageWidth / blockCount;

	for (int i = 0; i < blockCount; i++)
	{
		const int startBlurWidth = widthForThread * i;
		int endBlurWidth;

		if (i + 1 == blockCount)
		{
			endBlurWidth = imageWidth;
		}
		else
		{
			endBlurWidth = widthForThread * (i + 1);
		}

		ThreadData threadData = {
			inputImage,
			outputImage,
			blockCount,
			startBlurWidth,
			endBlurWidth,
		};

		threadsData.push_back(threadData);
	}

	return threadsData;
}

int main(int argc, char* argv[])
{
	if (argc != 6)
	{
		std::cout << "Invalid arguments count\nUsage <processingMode> <blocksCount> <inputDirectoryPath> <outputDirectoryPath> <threadCountInPool>\n" << std::endl;
		return 1;
	}

	try
	{
		std::clock_t start_time = std::clock();

		const std::string processingMode = argv[1];
		const int blockCount = std::stoi(argv[2]);
		const std::string inputDirectoryPath = argv[3];
		const std::string outputDirectoryPath = argv[4];
		const int threadCountInPool = std::stoi(argv[5]);

		if (!std::filesystem::exists(outputDirectoryPath))
		{
			std::filesystem::create_directory(outputDirectoryPath);
		}

		for (auto& file : std::filesystem::directory_iterator(inputDirectoryPath))
		{
			auto path = file.path();	
			bitmap_image inputImage(path.string());
			bitmap_image outputImage(inputImage);

			std::vector<ThreadData> threadsData = GetThreadsData(inputImage, &outputImage, blockCount);

			std::vector<ITask*> tasks;

			for (auto& threadData : threadsData)
			{
				tasks.push_back(new Blur(threadData));
			}

			if (processingMode == "pool")
			{
				Pool pool(tasks, blockCount);
				pool.Execute();
			}
			else if (processingMode == "thread")
			{
				DefaultThread defaultThread(tasks);
				defaultThread.Execute();
			}
			else
			{
				throw std::invalid_argument("Invalid mode\nUsage 'thread' or 'pool'");
			}

			outputImage.save_image(outputDirectoryPath + '/' + path.filename().string());
		}

		std::clock_t end_time = std::clock();
		std::cout << end_time - start_time << " ms" << std::endl;
	}
	catch (const std::exception)
	{
		return 1;
	}

	return 0;
}