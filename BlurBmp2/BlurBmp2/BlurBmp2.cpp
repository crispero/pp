#include "bitmap_image.hpp"
#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <ctime>
#include <fstream>

std::string HELPER_SYMBOL = "/?";
const int MIN_ARGUMENTS_COUNT = 5;
const int BLUR_RADIUS = 10;

struct ThreadData
{
	bitmap_image inputImage;			
	bitmap_image* outputImage;
	int threadCount;
	int coreCount;
	int threadNumber;
	int startBlurWidth;
	int endBlurWidth;
	std::ofstream* outputFile;
	std::clock_t startTime;
};

enum ThreadPriority
{
	below_normal = -1,
	normal,
	above_normal,
};

std::vector<rgb_t> GetPixels(const int width, const int height, bitmap_image& inputImage)
{
	std::vector<rgb_t> pixels;

	for (int i = width - BLUR_RADIUS; i <= width + BLUR_RADIUS; i++)
	{
		for (int j = height - BLUR_RADIUS; j <= height + BLUR_RADIUS; j++)
		{
			if (i >= 0 && j >= 0 && i < (int)inputImage.width() && j < (int)inputImage.height())
			{
				rgb_t pixel = inputImage.get_pixel(i, j);
				pixels.push_back(pixel);
			}
		}
	}

	return pixels;
}

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	ThreadData* threadData = static_cast<ThreadData*>(lpParam);

	const int startBlurWidth = threadData->startBlurWidth;
	const int endBlurWidth = threadData->endBlurWidth;
	const int imageHeight = threadData->inputImage.height();
	bitmap_image inputImage = threadData->inputImage;
	bitmap_image* outputImage = threadData->outputImage;
	std::ofstream* outputFile = threadData->outputFile;

	for (int i = startBlurWidth; i < endBlurWidth; i++)
	{
		for (int j = 0; j < imageHeight; j++)
		{
			std::vector<rgb_t> pixels = GetPixels(i, j, inputImage);

			int countRed = 0, countGreen = 0, countBlue = 0;

			for (const auto& pixel : pixels)
			{
				countRed += pixel.red;
				countGreen += pixel.green;
				countBlue += pixel.blue;
			}

			outputImage->set_pixel(
				i,
				j,
				(unsigned char)(countRed / pixels.size()),
				(unsigned char)(countGreen / pixels.size()),
				(unsigned char)(countBlue / pixels.size())
			);

			std::clock_t end_time = std::clock();
			*outputFile << threadData->threadNumber << "\t" << end_time - threadData->startTime << std::endl;
		}
	}

	ExitThread(0);
}

void Blur(const std::string input, const std::string output, const int threadCount, const int coreCount, const std::vector<ThreadPriority> threadPriorities)
{
	std::clock_t startTime = std::clock();
	bitmap_image inputImage(input);
	bitmap_image outputImage(inputImage);

	std::vector<HANDLE> handles(threadCount);
	const int affinityMask = (1 << coreCount) - 1;

	const int imageWidth = inputImage.width();
	const int widthForThread = imageWidth / threadCount;

	std::vector<ThreadData> threadsData;

	for (int i = 0; i < threadCount; i++)
	{
		const int startBlurWidth = widthForThread * i;
		int endBlurWidth;

		if (i + 1 == threadCount)
		{
			endBlurWidth = imageWidth;
		}
		else
		{
			endBlurWidth = widthForThread * (i + 1);
		}

		ThreadData threadData = {
			inputImage,
			&outputImage,
			threadCount,
			coreCount,
			i,
			startBlurWidth,
			endBlurWidth,
			new std::ofstream("thread" + std::to_string(i + 1) + ".txt"),
			startTime,
		};

		threadsData.push_back(threadData);
	}

	for (int i = 0; i < threadCount; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadProc, &threadsData[i], CREATE_SUSPENDED, NULL);
		SetThreadAffinityMask(handles[i], affinityMask);	
		SetThreadPriority(handles[i], threadPriorities[i]);
	}

	for (const auto& handle : handles)
	{
		ResumeThread(handle);
	}

	WaitForMultipleObjects((DWORD)handles.size(), handles.data(), true, INFINITE);
	outputImage.save_image(output);
}

int main(int argc, char* argv[])
{
	if (argc == 2 && argv[1] == HELPER_SYMBOL)
	{
		std::cout
			<< "Usage <input image> <output image> <thread count> <core count> <thread priorities>\n" 
			<< "thread priority must be: above_normal or normal or below_normal\n"
			<< "the number of thread priorities must be equal to the number of threads"
			<< std::endl;
		return 1;
	}
	else
	{
		try
		{
			if (argc < MIN_ARGUMENTS_COUNT)
			{
				throw std::invalid_argument("Invalid arguments count\nUsage '/?' for help");
			}

			std::clock_t start_time = std::clock();
			const std::string inputImage = argv[1];
			const std::string outputImage = argv[2];
			const int threadCount = std::stoi(argv[3]);
			const int coreCount = std::stoi(argv[4]);

			const int REQUIRED_ARGUMENTS_COUNT = MIN_ARGUMENTS_COUNT + threadCount;

			if (argc != REQUIRED_ARGUMENTS_COUNT)
			{
				throw std::invalid_argument("Invalid arguments count\nUsage '/?' for help");
			}

			std::vector<ThreadPriority> threadPriorities;
			for (int i = MIN_ARGUMENTS_COUNT; i < REQUIRED_ARGUMENTS_COUNT; i++)
			{
				const std::string threadPriority = argv[i];
				if (threadPriority == "above_normal")
				{
					threadPriorities.push_back(ThreadPriority::above_normal);
				}
				else if (threadPriority == "below_normal")
				{
					threadPriorities.push_back(ThreadPriority::below_normal);
				}
				else if (threadPriority == "normal")
				{
					threadPriorities.push_back(ThreadPriority::normal);
				}
				else
				{
					throw std::exception("Unknown thread priority name\nUsage '/?' for help");
				}
			}

			Blur(inputImage, outputImage, threadCount, coreCount, threadPriorities);

			std::clock_t end_time = std::clock();
			std::cout << end_time - start_time << " ms" << std::endl;
		}
		catch (const std::exception & e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	return 0;
}