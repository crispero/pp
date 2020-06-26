#pragma once
#include "bitmap_image.hpp"
#include "ITask.h"
#include <vector>

const int BLUR_RADIUS = 10;

struct ThreadData
{
	bitmap_image inputImage;
	bitmap_image* outputImage;
	int blockCount;
	int startBlurWidth;
	int endBlurWidth;
};

class Blur : public ITask
{
public:
	Blur(ThreadData& threadData);
	void Execute() override;
private:
	std::vector<rgb_t> GetPixels(const int width, const int height, bitmap_image& inputImage);
	ThreadData m_threadData;
};

