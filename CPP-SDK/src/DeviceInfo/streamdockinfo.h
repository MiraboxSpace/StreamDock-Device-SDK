/**
 * @file streamdockinfo.h
 * @brief Defines basic enums and data structures related to StreamDock device metadata and input events.
 *
 * Contents:
 * - DeviceOriginType: Enumeration of different StreamDock device models
 * - StreamDockInfo: Structure containing device specifications and capabilities
 * - RegisterEvent: Enumeration of supported input events (key, knob, swipe)
 *
 * These types are used throughout the StreamDock control and communication logic.
 */
#pragma once
#include <cstdint>
#include <string>
#include <ImgType.h>

enum class DeviceOriginType
{
	Unknown = 0x00,
	SD293 = 0x01,	///< StreamDock SD293
	SD293s = 0x02,	///< StreamDock SD293s
	SDN3 = 0x03,	///< StreamDock SDN3
	SDN1 = 0x04,	///< StreamDock SDN1
	SDN4 = 0x05,	///< StreamDock SDN4
	SDM18 = 0x06,	///< StreamDock SDM18
	SDN4Pro = 0x07, ///< StreamDock SDN4Pro
	SDXL = 0x08,	///< StreamDock StreamDockXL
	SDM3 = 0x09,	///< StreamDock StreamDockM3
	K1Pro = 0x0A,	///< StreamDock K1Pro
};

struct StreamDockInfo
{
	DeviceOriginType originType = DeviceOriginType::Unknown;
	uint16_t vendor_id = 0x00;
	uint16_t product_id = 0x00;
	std::wstring serialNumber{};   ///< 设备序列号
	std::string firmwareVersion{}; ///< 固件号
	uint16_t width = 0;
	uint16_t height = 0;
	uint16_t keyWidth = 0;
	uint16_t keyHeight = 0;
	uint16_t minKey = 0;
	uint16_t maxKey = 0;
	ImgType keyEncodeType = ImgType::JPG;
	ImgType backgroundEncodeType = ImgType::JPG;
	ImgFormat backgroundEncodeFormat = ImgFormat::BGR888;
	bool key_flip_vertical = false;
	bool key_flip_horizonal = false;
	bool bg_flip_vertical = false;
	bool bg_flip_horizonal = false;
	double key_rotate_angle = 0.0f;
	double bg_rotate_angle = 0.0f;
	std::string deviceUUID{};
	std::string displayName{};
};

enum class RegisterEvent : uint8_t
{
	EveryThing = 0xFF,	///< Unknown event
	KeyPress = 0x01,	///< Key press event
	KeyRelease = 0x02,	///< Key release event
	KnobLeft = 0x03,	///< Knob rotated left
	KnobRight = 0x04,	///< Knob rotated right
	KnobPress = 0x05,	///< Knob press event
	KnobRelease = 0x06, ///< Knob release event
	SwipeLeft = 0x07,	///< Swipe left gesture
	SwipeRight = 0x08,	///< Swipe right gesture
	ToggleUp = 0x09,    ///< Toggle switch up event
	ToggleDown = 0x0A   ///< Toggle switch down event
};
