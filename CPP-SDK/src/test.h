#pragma once
#include <array>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>
#include <streamdock.h>
#include <OpenCVImageEncoder.h>
#include <toolkit.h>
#include <HotspotDevice/StreamDockN1/streamdockN1.h>
#include <HotspotDevice/StreamDockN4Pro/streamdockN4Pro.h>
#include <HotspotDevice/StreamDockXL/streamdockXL.h>
#include <HotspotDevice/StreamDockM3/streamdockM3.h>
#include <HotspotDevice/StreamDockM18V3/streamdockM18V3.h>
#include <HotspotDevice/StreamDockMini/streamdockMini.h>
#include <HotspotDevice/K1Pro/K1Pro.h>
#include <HotspotDevice/StreamDockH1Pro/streamdockH1Pro.h>

template <typename T, typename... Args>
static void debugPrint(T &&first, Args &&...rest)
{
	std::cerr << std::forward<T>(first) << " ";
	((std::cerr << std::forward<Args>(rest) << " "), ...);
	std::cerr << std::endl;
}

namespace TEST_COMMON
{
	void registerRange(const std::shared_ptr<StreamDock>& device, uint16_t first, uint16_t last,
		RegisterEvent event, const std::string& name, const std::string& action)
	{
		for (uint16_t key = first; key <= last; ++key)
		{
			const auto message = name + " " + std::to_string(key) + " " + action;
			device->reader()->registerReadCallback(static_cast<uint8_t>(key), [message]()
				{ debugPrint(message); }, event);
		}
	}

	void registerButtons(const std::shared_ptr<StreamDock>& device, uint16_t first, uint16_t last)
	{
		registerRange(device, first, last, RegisterEvent::KeyPress, "Key", "pressed");
		registerRange(device, first, last, RegisterEvent::KeyRelease, "Key", "released");
	}

	void registerKnob(const std::shared_ptr<StreamDock>& device, uint8_t number, uint8_t press, uint8_t left,
		uint8_t right)
	{
		const auto registerEvent = [&](uint8_t key, RegisterEvent event, const std::string& action)
		{
			const auto message = "Knob " + std::to_string(number) + " " + action;
			device->reader()->registerReadCallback(key, [message]()
				{ debugPrint(message); }, event);
		};
		registerEvent(press, RegisterEvent::KnobPress, "pressed");
		registerEvent(press, RegisterEvent::KnobRelease, "released");
		registerEvent(left, RegisterEvent::KnobLeft, "rotated left");
		registerEvent(right, RegisterEvent::KnobRight, "rotated right");
	}

	void registerDip(const std::shared_ptr<StreamDock>& device, uint8_t left, uint8_t right, uint8_t press)
	{
		registerRange(device, left, left, RegisterEvent::DIPLeft, "DIP", "left");
		registerRange(device, left, left, RegisterEvent::DIPLeftEnd, "DIP", "left end");
		registerRange(device, right, right, RegisterEvent::DIPRight, "DIP", "right");
		registerRange(device, right, right, RegisterEvent::DIPRightEnd, "DIP", "right end");
		registerRange(device, press, press, RegisterEvent::DIPPress, "DIP", "pressed");
		registerRange(device, press, press, RegisterEvent::DIPRelease, "DIP", "released");
	}

	void registerCallbacks(const std::shared_ptr<StreamDock>& device)
	{
		registerButtons(device, device->info()->minKey, device->info()->maxKey);
		switch (device->info()->originType)
		{
		case DeviceOriginType::SDN1:
			// These IDs identify physical buttons and a knob, not secondary-screen presses.
			registerButtons(device, 16, 17);
			registerKnob(device, 1, 18, 19, 20);
			break;
		case DeviceOriginType::SDN3:
			registerButtons(device, 7, 9);
			for (uint8_t knob = 0; knob < 3; ++knob)
				registerKnob(device, knob + 1, 10 + knob, 13 + knob, 16 + knob);
			break;
		case DeviceOriginType::SDM18:
			registerButtons(device, 16, 18);
			break;
		case DeviceOriginType::SDN4:
		case DeviceOriginType::SDN4Pro:
		{
			const bool isN4 = device->info()->originType == DeviceOriginType::SDN4;
			registerRange(device, 1, 4, RegisterEvent::KeyRelease, "Key", "released");
			if (isN4)
				registerRange(device, 1, 4, RegisterEvent::KeyPress, "Key", "pressed");
			for (uint8_t knob = 0; knob < 4; ++knob)
				registerKnob(device, knob + 1, 24 + knob, 16 + knob * 2, 17 + knob * 2);
			registerRange(device, 28, 28, RegisterEvent::SwipeLeft, "Swipe", "left");
			registerRange(device, 29, 29, RegisterEvent::SwipeRight, "Swipe", "right");
			break;
		}
		case DeviceOriginType::SDM3:
			for (uint8_t knob = 0; knob < 3; ++knob)
				registerKnob(device, knob + 1, 22 + knob, 16 + knob * 2, 17 + knob * 2);
			break;
		case DeviceOriginType::K1Pro:
			for (uint8_t knob = 0; knob < 3; ++knob)
				registerKnob(device, knob + 1, 7 + knob, 10 + knob, 13 + knob);
			break;
		case DeviceOriginType::SDXL:
			registerDip(device, 33, 34, 37);
			registerDip(device, 35, 36, 38);
			break;
		case DeviceOriginType::SDMini:
			registerDip(device, 7, 8, 9);
			registerDip(device, 10, 11, 12);
			break;
		default:
			break;
		}
	}

	void displayRange(const std::shared_ptr<StreamDock>& device, uint16_t first, uint16_t last)
	{
		const bool animated = device->feature()->isDualDevice;
		for (uint16_t key = first; key <= last; ++key)
		{
			const auto keyValue = static_cast<uint8_t>(key);
			if (animated)
				device->gifer()->clearKeyGif(keyValue);
			if (animated && key % 3 == 0)
				device->gifer()->setKeyGifFile("../../img/test.gif", keyValue);
			else if (animated ? key % 3 == 1 : key % 2 == 1)
				device->setKeyImgFile("../../img/button_test.jpg", keyValue);
			else
				device->setKeyImgFile("../../img/mark.png", keyValue);
		}
	}

	void testKeys(const std::shared_ptr<StreamDock>& device)
	{
		device->reader()->stopReadLoop();
		if (device->feature()->isDualDevice)
			device->gifer()->stopGifLoop();
		device->wakeupScreen();
		device->setKeyBrightness(100);
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		registerCallbacks(device);
		displayRange(device, device->info()->minKey, device->info()->maxKey);
		if (device->feature()->hasSecondScreen)
		{
			// Secondary display ranges do not imply input events.
			displayRange(device, device->feature()->min2rdScreenKey, device->feature()->max2rdScreenKey);
		}
		device->refresh();
		device->reader()->startReadLoop();
		if (device->feature()->isDualDevice)
			device->gifer()->startGifLoop();
	}
}

namespace TEST_293V2
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SD293 || device->feature()->isDualDevice)
			return;
		TEST_COMMON::testKeys(device);
	}
}

namespace TEST_293V3
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SD293 || !device->feature()->isDualDevice)
			return;
		TEST_COMMON::testKeys(device);
		device->rgber()->setLedBrightness(13);
	}
}

namespace TEST_293sV2
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SD293s || device->feature()->isDualDevice)
			return;
		TEST_COMMON::testKeys(device);
	}
}

namespace TEST_293sV3
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SD293s || !device->feature()->isDualDevice)
			return;
		TEST_COMMON::testKeys(device);
		device->rgber()->setLedBrightness(13);
	}
}

namespace TEST_M18
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDM18 || device->feature()->isDualDevice)
			return;
		TEST_COMMON::testKeys(device);
	}
}

namespace TEST_N3V2
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDN3 || device->feature()->isDualDevice)
			return;
		TEST_COMMON::testKeys(device);
	}
}

namespace TEST_N3V25
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDN3 || !device->feature()->isDualDevice)
			return;
		TEST_COMMON::testKeys(device);
	}
}

namespace TEST_N1
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDN1 || !device->feature()->isDualDevice)
			return;
		auto N1device = std::dynamic_pointer_cast<StreamDockN1>(device);
		N1device->changeMode(StreamDockN1::N1MODE::SOFTWARE_MODE);
		N1device->wakeupScreen();
		N1device->clearAllKeys();

		N1device->changeMode(StreamDockN1::N1MODE::KEYBOARD_MODE);
		for (int i = 0; i <= 5; i++)
		{
			N1device->changePage(i);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		N1device->changeMode(StreamDockN1::N1MODE::CALCULATOR_MODE);
		for (int i = 0; i <= 5; i++)
		{
			N1device->changePage(i);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		// ⚠️ The target page cannot be the same as the current page. ⚠️
		// N1device->changeMode(StreamDockN1::N1MODE::CALCULATOR_MODE);
		// N1device->changePage(2);
		// N1device->setSkinBitmap("../../img/button_test.jpg", StreamDockN1::SkinMode::CALCULATOR, 1, StreamDockN1::SkinStatus::PRESS, 1);
		// N1device->setSkinBitmap("../../img/button_test.jpg", StreamDockN1::SkinMode::CALCULATOR, 1, StreamDockN1::SkinStatus::RELEASE, 1);
		// N1device->changePage(1);
		// std::this_thread::sleep_for(std::chrono::milliseconds(10000));

		N1device->changeMode(StreamDockN1::N1MODE::SOFTWARE_MODE);
		TEST_COMMON::testKeys(device);
	}
}

namespace TEST_N4
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDN4)
			return;
		device->heartbeater()->startHeartBeatLoop();
		TEST_COMMON::testKeys(device);
	}
}

namespace TEST_N4Pro
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDN4Pro)
			return;
		device->heartbeater()->startHeartBeatLoop();
		auto configs = buildConfigCommand(N4ProConfigEnumerate::EnableVibration, ConfigState::Off);
		device->configer()->setDeviceConfig(configs);
		configs = buildConfigCommand<N4ProConfigEnumerate>({{N4ProConfigEnumerate::EnableVibration, ConfigState::Off},
															{N4ProConfigEnumerate::EnableBootVideo, ConfigState::Off}});
		device->configer()->setDeviceConfig(configs);
		device->setKeyBrightness(100);
		// setLedColor uses the device configured LED count automatically.
		// device->rgber()->setLedColor(255, 0, 0);
		device->rgber()->setLedBrightness(255);
		// N4Pro Supports setting single LED color
		device->rgber()->setSingleLedColor({
			{255, 0, 0},
			{0, 0, 255},
			{255, 0, 255},
			{255, 255, 0},
		});
		// device->rgber()->resetLedColor();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setFrameBackgroundFile("../../img/backgroud_test2.png");
		auto N4pro = std::dynamic_pointer_cast<StreamDockN4Pro>(device);
		N4pro->registerTouchBarCallback(nullptr, true);
		TEST_COMMON::testKeys(device);
	}
}

namespace TEST_XL
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDXL)
			return;
		device->heartbeater()->startHeartBeatLoop();
		TEST_COMMON::testKeys(device);
		device->rgber()->setLedColor(0, 0, 255);
	}
}

namespace TEST_M3
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDM3)
			return;
		auto m3Device = std::dynamic_pointer_cast<StreamDockM3>(device);
		device->heartbeater()->startHeartBeatLoop();
		TEST_COMMON::testKeys(device);
		m3Device->magneticCalibration();
	}
}

namespace TEST_M18V3
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDM18 || !device->feature()->isDualDevice)
			return;
		device->heartbeater()->startHeartBeatLoop();
		TEST_COMMON::testKeys(device);
		if (device->feature()->hasRGBLed)
			device->rgber()->setLedColor(0, 255, 0);
	}
}

namespace TEST_K1Pro
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::K1Pro)
			return;
		auto K1ProDevice = std::dynamic_pointer_cast<K1Pro>(device);
		device->heartbeater()->startHeartBeatLoop();
		TEST_COMMON::testKeys(device);
		K1ProDevice->setKeyboardLightingEffects(1);
		K1ProDevice->setKeyboardBacklightBrightness(6);
		K1ProDevice->setKeyboardRgbBacklight(255, 0, 0);
	}
}

namespace TEST_Mini
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDMini)
			return;
		device->heartbeater()->startHeartBeatLoop();
		TEST_COMMON::testKeys(device);
		device->rgber()->setLedColor(0, 0, 255);
	}
}

namespace TEST_H1Pro
{
	struct UploadState
	{
		std::mutex mutex;
		std::condition_variable reconnectedCv;
		std::unordered_set<std::wstring> uploadedDevices;
		std::unordered_set<std::wstring> reconnectedDevices;
	};

	UploadState& uploadState()
	{
		static UploadState state;
		return state;
	}

	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDH1Pro)
			return;
		auto h1pro = std::dynamic_pointer_cast<StreamDockH1Pro>(device);
		const auto serial = device->info()->serialNumber;
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->refresh();
		std::this_thread::sleep_for(std::chrono::seconds(2));

		// Upload GIF
		// auto& state = uploadState();
		// bool uploadNeeded = false;
		// {
		// 	std::lock_guard<std::mutex> lock(state.mutex);
		// 	uploadNeeded = state.uploadedDevices.insert(serial).second;
		// 	if (!uploadNeeded)
		// 	{
		// 		// The upload restarted the device. Continue setup on its new HID handle.
		// 		state.reconnectedDevices.insert(serial);
		// 		state.reconnectedCv.notify_all();
		// 	}
		// }

		// if (uploadNeeded)
		// {
		// 	// ffmpeg must be on PATH. Uploading a GIF restarts and re-enumerates the device.
		// 	// test.gif is the GIF asset available in this C++ SDK's img directory.
		// 	std::cout << "H1Pro special function: uploading a GIF" << std::endl;
		// 	const auto result = h1pro->uploadGifFile("../../img/test.gif");
		// 	if (result != TRANSPORT_SUCCESS && result != TRANSPORT_ERROR_TIMEOUT_OPERATION)
		// 	{
		// 		std::lock_guard<std::mutex> lock(state.mutex);
		// 		state.uploadedDevices.erase(serial);
		// 		std::cerr << "H1 Pro: GIF upload failed, result " << result
		// 			<< " (check that ffmpeg is on PATH and img/test.gif is available)" << std::endl;
		// 		return;
		// 	}
		// 	std::unique_lock<std::mutex> lock(state.mutex);
		// 	const bool reconnected = state.reconnectedCv.wait_for(lock, std::chrono::seconds(5), [&]()
		// 		{ return state.reconnectedDevices.count(serial) != 0; });
		// 	if (reconnected)
		// 	{
		// 		std::cout << "H1Pro special function: upload restarted the device; continuing on the new handle" << std::endl;
		// 		return;
		// 	}
		// 	if (result != TRANSPORT_SUCCESS)
		// 	{
		// 		state.uploadedDevices.erase(serial);
		// 		std::cerr << "H1 Pro: upload timed out without reconnection" << std::endl;
		// 		return;
		// 	}
		// }
		// else
		// 	std::cout << "H1Pro special function: GIF already uploaded, skipping upload" << std::endl;

		std::cout << "H1Pro special function: switching to GIF mode" << std::endl;
		h1pro->switchMode(StreamDockH1Pro::Mode::Gif);
		std::this_thread::sleep_for(std::chrono::seconds(5));
		std::cout << "H1Pro special function: switching to SCREENSAVER mode" << std::endl;
		h1pro->switchMode(StreamDockH1Pro::Mode::Screensaver);
		std::this_thread::sleep_for(std::chrono::seconds(5));
		std::cout << "H1Pro special function: switching to KEY mode" << std::endl;
		h1pro->switchMode(StreamDockH1Pro::Mode::Keys);
		device->clearAllKeys();
		device->refresh();
		std::this_thread::sleep_for(std::chrono::seconds(5));

		TEST_COMMON::testKeys(device);
	}
}
