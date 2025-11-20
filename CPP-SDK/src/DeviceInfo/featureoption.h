/**
 * @file featureoption.h
 * @brief Defines FeatureOption structure to describe capabilities of a StreamDock device.
 *
 * This structure captures optional features supported by different device models,
 * such as dual-device support, RGB lighting, secondary screens, background animation,
 * transparency, and more.
 *
 * It is used for feature detection and conditional logic throughout the StreamDock platform.
 */
#pragma once
#include <ImgType.h>
struct FeatureOption
{
	bool isDualDevice = false;
	bool hasSecondScreen = false;
	bool hasRGBLed = false;
	bool supportBackGroundGif = false;
	bool supportTransparentIcon = false;
	bool supportConfig = false;
	uint16_t min2rdScreenKey = 0;
	uint16_t max2rdScreenKey = 0;
	uint16_t _2rdScreenWidth = 0;
	uint16_t _2rdScreenHeights = 0;
	ImgType _2rdScreenEncodeType = ImgType::JPG;
	ImgType backgroundGifEncodeType = ImgType::JPG;
	uint16_t ledCounts = 0;
};