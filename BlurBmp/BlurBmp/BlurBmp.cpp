#include "bitmap_image.hpp"
#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <ctime>

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
		}
	}

	ExitThread(0);
}

void Blur(const std::string input, const std::string output, const int threadCount, const int coreCount)
{
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
		};

		threadsData.push_back(threadData);
	}

	for (int i = 0; i < threadCount; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadProc, &threadsData[i], CREATE_SUSPENDED, NULL);
		SetThreadAffinityMask(handles[i], affinityMask);
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
	if (argc != 5)
	{
		std::cout << "Invalid arguments count\nUsage <input image> <output image> <thread count> <core count>\n" << std::endl;
		return 1;
	}

	try
	{
		std::clock_t start_time = std::clock();
		const std::string inputImage = argv[1];
		const std::string outputImage = argv[2];
		const int threadCount = std::stoi(argv[3]);
		const int coreCount = std::stoi(argv[4]);

		Blur(inputImage, outputImage, threadCount, coreCount);

		std::clock_t end_time = std::clock();
		std::cout << end_time - start_time << " ms" << std::endl;
	}
	catch (const std::invalid_argument)
	{
		return 1;
	}

	return 0;
}

