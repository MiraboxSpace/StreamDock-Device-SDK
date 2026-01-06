// ImageEncoder.h
#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <vector>
#include "RawCanvas.h"
#include "ImgType.h"
#include <ImgHelper.h>

class IImageEncoder
{
public:
	virtual ~IImageEncoder() = default;

	virtual bool encodeToFile(const std::string& filename,
		const RawCanvas& canvas,
		int quality = 95,
		const ImgHelper& imgHelper = ImgHelper()) const = 0;

	virtual bool encodeToMemory(std::vector<uint8_t>& out,
		const RawCanvas& canvas,
		int quality = 95,
		const ImgHelper& imgHelper = ImgHelper()) const = 0;

	virtual bool encodeToFile(const std::string& filename,
		std::vector<uint8_t>& out,
		int quality = 95,
		const ImgHelper& imgHelper = ImgHelper()) const = 0;

	virtual bool encodeToMemory(std::vector<uint8_t>& out,
		const std::vector<uint8_t>& in,
		int quality = 95,
		const ImgHelper& imgHelper = ImgHelper()) const = 0;

	virtual bool encodeToBitmap(std::vector<uint8_t>& out,
		const std::vector<uint8_t>& in,
		const ImgHelper& imgHelper = ImgHelper()) const = 0;

	static std::string imgTypeToExt(ImgType type)
	{
		switch (type)
		{
		case ImgType::JPG:
			return ".jpg";
		case ImgType::PNG:
			return ".png";
		case ImgType::BMP:
			return ".bmp";
		case ImgType::WEBP:
			return ".webp";
		default:
			return ".jpg";
		}
	}

	static bool supportsAlpha(ImgType type)
	{
		return type == ImgType::PNG || type == ImgType::WEBP;
	}

	static bool saveImageToFile(const std::vector<uint8_t>& imageData, const std::string& filename)
	{
		std::ofstream file(filename, std::ios::binary);
		if (!file)
		{
			std::cerr << "Unable to open file " << filename << " for writing." << std::endl;
			return false;
		}
		file.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
		if (!file)
		{
			std::cerr << "Error writing file " << filename << "." << std::endl;
			return false;
		}
		return true;
	}
};
