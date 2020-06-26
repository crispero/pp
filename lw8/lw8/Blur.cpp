#include "Blur.h"

Blur::Blur(ThreadData& threadData)
	: m_threadData(threadData)
{
}

std::vector<rgb_t> Blur::GetPixels(const int width, const int height, bitmap_image& inputImage)
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

void Blur::Execute() {
	const int startBlurWidth = m_threadData.startBlurWidth;
	const int endBlurWidth = m_threadData.endBlurWidth;
	const int imageHeight = m_threadData.inputImage.height();
	bitmap_image inputImage = m_threadData.inputImage;
	bitmap_image* outputImage = m_threadData.outputImage;

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
}
