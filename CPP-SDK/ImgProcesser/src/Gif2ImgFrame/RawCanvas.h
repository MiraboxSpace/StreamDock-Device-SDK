#pragma once
#include <vector>
#include <cstdint>
#include <cassert>

#define USE_OPENCVV

struct RawCanvas
{
	int width;
	int height;
	bool alpha;
	std::vector<uint8_t> pixels;

	RawCanvas(int w, int h, bool useAlpha)
		: width(w), height(h), alpha(useAlpha),
		pixels(w* h* (useAlpha ? 4 : 3), 0) {
	}

	inline uint8_t* pixel(int x, int y)
	{
		assert(x >= 0 && x < width && y >= 0 && y < height);
		return &pixels[(y * width + x) * (alpha ? 4 : 3)];
	}

	template <typename T>
	T as() const;
};
