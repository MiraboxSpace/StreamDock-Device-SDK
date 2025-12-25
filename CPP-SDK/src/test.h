#pragma once
#include <memory>
#include <streamdock.h>
#include <OpenCVImageEncoder.h>
#include <toolkit.h>
#include <HotspotDevice/StreamDockN1/streamdockN1.h>
#include <HotspotDevice/StreamDockN4Pro/streamdockN4Pro.h>
#include <HotspotDevice/StreamDockXL/streamdockXL.h>
#include <HotspotDevice/StreamDockM3/streamdockM3.h>
#include <HotspotDevice/StreamDockM18V3/streamdockM18V3.h>
#include <HotspotDevice/K1Pro/K1Pro.h>

template <typename T, typename... Args>
static void debugPrint(T &&first, Args &&...rest)
{
	std::cerr << std::forward<T>(first) << " ";
	((std::cerr << std::forward<Args>(rest) << " "), ...);
	std::cerr << std::endl;
}

namespace TEST_293V2
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SD293 || device->feature()->isDualDevice)
			return;
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		// device->setBackgroundImgFile("../../img/YiFei.jpg", 5000);
		// device->refresh();
		// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->setKeyImgFile("../../img/YiFei.jpg", 9);
		device->refresh();
		device->setKeyImgFile("../../img/YiFei.jpg", 8);
		device->refresh();
		device->setKeyImgFile("../../img/YiFei.jpg", 7);
		device->refresh();
		device->setKeyImgFile("../../img/YiFei.jpg", 6);
		device->refresh();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
											   { debugPrint("Key 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(10, []()
											   { debugPrint("Key 10"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(15, []()
											   { debugPrint("Key 15"); }, RegisterEvent::EveryThing, true);
	}
}

namespace TEST_293V3
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SD293 || !device->feature()->isDualDevice)
			return;
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setBackgroundImgFile("../../img/YiFei.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->gifer()->setKeyGifFile("../../img/test.gif", 1);
		device->gifer()->setKeyGifFile("../../img/test.gif", 2);
		device->gifer()->setKeyGifFile("../../img/2.gif", 2);
		device->gifer()->setKeyGifFile("../../img/test.gif", 3);
		device->gifer()->setKeyGifFile("../../img/test.gif", 4);
		device->gifer()->setKeyGifFile("../../img/2.gif", 10);
		device->gifer()->setKeyGifFile("../../img/3.gif", 11);
		device->gifer()->setKeyGifFile("../../img/4.gif", 15);
		device->gifer()->setKeyGifFile("../../img/5.gif", 7);
		device->gifer()->setKeyGifFile("../../img/6.gif", 3);
		device->gifer()->setKeyGifFile("../../img/7.gif", 13, 30);
		device->setKeyImgFile("../../img/YiFei.jpg", 9);
		device->rgber()->setLedBrightness(13);
		device->gifer()->startGifLoop();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
											   { debugPrint("Key 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(10, []()
											   { debugPrint("Key 10"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(15, []()
											   { debugPrint("Key 15"); }, RegisterEvent::EveryThing, true);
		device->setBackgroundImgFile("../../img/bg.png");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

namespace TEST_293sV2
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SD293s || device->feature()->isDualDevice)
			return;
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		// device->setBackgroundImgFile("../../img/YiFei.jpg", 5000);
		// device->refresh();
		// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->setKeyImgFile("../../img/YiFei.jpg", 1);
		device->refresh();
		device->setKeyImgFile("../../img/YiFei.jpg", 6);
		device->refresh();
		device->setKeyImgFile("../../img/YiFei.jpg", 16);
		device->refresh();
		device->setKeyImgFile("../../img/YiFei.jpg", 19);
		device->refresh();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
											   { debugPrint("Key 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(10, []()
											   { debugPrint("Key 10"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(15, []()
											   { debugPrint("Key 15"); }, RegisterEvent::EveryThing, true);
	}
}

namespace TEST_293sV3
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SD293s || !device->feature()->isDualDevice)
			return;
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setBackgroundImgFile("../../img/YiFei.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->gifer()->setKeyGifFile("../../img/test.gif", 1);
		device->gifer()->setKeyGifFile("../../img/test.gif", 2);
		device->gifer()->setKeyGifFile("../../img/2.gif", 2);
		device->gifer()->setKeyGifFile("../../img/test.gif", 3);
		device->gifer()->setKeyGifFile("../../img/test.gif", 4);
		device->gifer()->setKeyGifFile("../../img/2.gif", 10);
		device->gifer()->setKeyGifFile("../../img/3.gif", 11);
		device->gifer()->setKeyGifFile("../../img/4.gif", 15);
		device->gifer()->setKeyGifFile("../../img/5.gif", 7);
		device->gifer()->setKeyGifFile("../../img/6.gif", 3);
		device->gifer()->setKeyGifFile("../../img/7.gif", 13, 30);
		device->setKeyImgFile("../../img/YiFei.jpg", 13);
		device->gifer()->setKeyGifFile("../../img/6.gif", 17);
		device->rgber()->setLedBrightness(13);
		device->gifer()->startGifLoop();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
											   { debugPrint("Key 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(10, []()
											   { debugPrint("Key 10"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(15, []()
											   { debugPrint("Key 15"); }, RegisterEvent::EveryThing, true);
		device->setBackgroundImgFile("../../img/bg.png");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

namespace TEST_M18
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDM18 || !device->feature()->supportBackGroundGif)
			return;
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setBackgroundImgFile("../../img/YiFei.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->gifer()->setKeyGifFile("../../img/test.gif", 1);
		device->gifer()->setKeyGifFile("../../img/test.gif", 2);
		device->gifer()->setKeyGifFile("../../img/2.gif", 2);
		device->gifer()->setKeyGifFile("../../img/test.gif", 3);
		device->gifer()->setKeyGifFile("../../img/test.gif", 4);
		device->gifer()->setKeyGifFile("../../img/2.gif", 10);
		device->gifer()->setKeyGifFile("../../img/3.gif", 11);
		device->gifer()->setKeyGifFile("../../img/4.gif", 15);
		device->gifer()->setKeyGifFile("../../img/5.gif", 7);
		device->gifer()->setKeyGifFile("../../img/6.gif", 3);
		device->gifer()->setKeyGifFile("../../img/7.gif", 13, 30);
		device->setKeyImgFile("../../img/YiFei.jpg", 9);
		device->rgber()->setLedColor(255, 0, 0);
		device->rgber()->setLedBrightness(13);
		device->gifer()->startGifLoop();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
											   { debugPrint("Key 1"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(6, []()
											   { debugPrint("Key 6 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(6, []()
											   { debugPrint("Key 6 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(16, []()
											   { debugPrint("left button"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(17, []()
											   { debugPrint("middle button"); }, RegisterEvent::EveryThing, true);
		device->setBackgroundImgFile("../../img/bg.png");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

namespace TEST_N3V2
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDN3 || device->feature()->isDualDevice)
			return;
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		// device->setBackgroundImgFile("../../img/bg.png", 5000);
		// device->refresh();
		// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->setKeyImgFile("../../img/YiFei.jpg", 1);
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
											   { debugPrint("Key 1"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Button 11 pressed"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(6, []()
											   { debugPrint("Knob 6 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(7, []()
											   { debugPrint("button 7 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(10, []()
											   { debugPrint("right knob press"); }, RegisterEvent::KnobPress);
		device->reader()->registerReadCallback(13, []()
											   { debugPrint("right knob left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(14, []()
											   { debugPrint("middle knob left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(16, []()
											   { debugPrint("right knob right"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(17, []()
											   { debugPrint("middle knob right"); }, RegisterEvent::EveryThing, true);
	}
}

namespace TEST_N3V25
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDN3 || !device->feature()->isDualDevice)
			return;
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setBackgroundImgFile("../../img/bg.png");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->gifer()->setKeyGifFile("../../img/test.gif", 1);
		device->gifer()->setKeyGifFile("../../img/test.gif", 2);
		device->gifer()->setKeyGifFile("../../img/2.gif", 2);
		device->gifer()->setKeyGifFile("../../img/test.gif", 3);
		device->gifer()->setKeyGifFile("../../img/test.gif", 4);
		device->gifer()->setKeyGifFile("../../img/2.gif", 10);
		device->gifer()->setKeyGifFile("../../img/3.gif", 11);
		device->gifer()->setKeyGifFile("../../img/4.gif", 15);
		device->gifer()->setKeyGifFile("../../img/5.gif", 7);
		device->gifer()->setKeyGifFile("../../img/6.gif", 3);
		device->gifer()->setKeyGifFile("../../img/7.gif", 13, 30);
		device->setKeyImgFile("../../img/YiFei.jpg", 9);
		device->rgber()->setLedBrightness(13);
		device->gifer()->startGifLoop();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
											   { debugPrint("Key 1"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Button 11 pressed"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(6, []()
											   { debugPrint("Knob 6 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(7, []()
											   { debugPrint("button 7 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(10, []()
											   { debugPrint("right knob press"); }, RegisterEvent::KnobPress);
		device->reader()->registerReadCallback(13, []()
											   { debugPrint("right knob left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(14, []()
											   { debugPrint("middle knob left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(16, []()
											   { debugPrint("right knob right"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(17, []()
											   { debugPrint("middle knob right"); }, RegisterEvent::EveryThing, true);
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
		N1device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		N1device->gifer()->setKeyGifFile("../../img/test.gif", 1);
		N1device->gifer()->setKeyGifFile("../../img/test.gif", 2);
		N1device->gifer()->setKeyGifFile("../../img/2.gif", 2);
		N1device->gifer()->setKeyGifFile("../../img/test.gif", 3);
		N1device->gifer()->setKeyGifFile("../../img/test.gif", 4);
		N1device->gifer()->setKeyGifFile("../../img/2.gif", 10);
		N1device->gifer()->setKeyGifFile("../../img/3.gif", 11);
		N1device->gifer()->setKeyGifFile("../../img/4.gif", 15);
		N1device->gifer()->setKeyGifFile("../../img/5.gif", 7);
		N1device->gifer()->setKeyGifFile("../../img/6.gif", 3);
		N1device->gifer()->setKeyGifFile("../../img/7.gif", 13, 30);
		N1device->gifer()->setKeyGifFile("../../img/7.gif", 18, 30);
		N1device->setKeyImgFile("../../img/YiFei.jpg", 9);
		N1device->rgber()->setLedBrightness(13);
		N1device->gifer()->startGifLoop();
		N1device->reader()->startReadLoop();
		N1device->reader()->registerReadCallback(1, []()
												 { debugPrint("Key 1"); }, RegisterEvent::EveryThing);
		N1device->reader()->registerReadCallback(11, []()
												 { debugPrint("Key 11 release"); }, RegisterEvent::KeyRelease);
		N1device->reader()->registerReadCallback(6, []()
												 { debugPrint("Key 6 release"); }, RegisterEvent::KeyRelease);
		N1device->reader()->registerReadCallback(16, []()
												 { debugPrint("button 16 release"); }, RegisterEvent::KeyRelease);
		N1device->reader()->registerReadCallback(17, []()
												 { debugPrint("button 17 release"); }, RegisterEvent::KeyRelease);
		N1device->reader()->registerReadCallback(19, []()
												 { debugPrint("knob left"); }, RegisterEvent::EveryThing, true);
		N1device->reader()->registerReadCallback(20, []()
												 { debugPrint("knob right"); }, RegisterEvent::EveryThing, true);
	}
}

namespace TEST_N4
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDN4)
			return;
		device->reader()->startReadLoop();
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setBackgroundImgFile("../../img/YiFei.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->gifer()->setKeyGifFile("../../img/test.gif", 1);
		device->gifer()->setKeyGifFile("../../img/test.gif", 2);
		device->gifer()->setKeyGifFile("../../img/2.gif", 2);
		device->gifer()->setKeyGifFile("../../img/test.gif", 3);
		device->gifer()->setKeyGifFile("../../img/test.gif", 4);
		device->gifer()->setKeyGifFile("../../img/2.gif", 10);
		device->gifer()->setKeyGifFile("../../img/3.gif", 11);
		device->gifer()->setKeyGifFile("../../img/4.gif", 15);
		device->gifer()->setKeyGifFile("../../img/5.gif", 7);
		device->gifer()->setKeyGifFile("../../img/6.gif", 3);
		device->gifer()->setKeyGifFile("../../img/7.gif", 13, 30);
		device->setKeyImgFile("../../img/YiFei.jpg", 9);
		device->rgber()->setLedBrightness(13);
		device->gifer()->startGifLoop();
		device->reader()->registerReadCallback(1, []()
											   { debugPrint("secondary screen 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(28, []()
											   { debugPrint("secondary screen swipe left"); }, RegisterEvent::SwipeLeft);
		device->reader()->registerReadCallback(28, []()
											   { debugPrint("secondary screen Unknown event"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(16, []()
											   { debugPrint("knob 1 left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(17, []()
											   { debugPrint("knob 1 right"); }, RegisterEvent::KnobRight, true);

		device->setBackgroundImgFile("../../img/bg.png");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

namespace TEST_N4Pro
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDN4Pro || !device->feature()->supportBackGroundGif)
			return;
		device->heartbeater()->startHeartBeatLoop();
		auto configs = buildConfigCommand(N4ProConfigEnumerate::EnableVibration, ConfigState::Off);
		device->configer()->setDeviceConfig(configs);
		configs = buildConfigCommand<N4ProConfigEnumerate>({{N4ProConfigEnumerate::EnableVibration, ConfigState::Off},
															{N4ProConfigEnumerate::EnableBootVideo, ConfigState::Off}});
		device->configer()->setDeviceConfig(configs);
		device->setKeyBrightness(100);
		device->rgber()->setLedColor(0, 255, 0);
		device->rgber()->setLedBrightness(10);
		// device->rgber()->resetLedColor();
		device->reader()->startReadLoop();
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setBackgroundImgFile("../../img/backgroud_test.png");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->clearAllKeys();
		device->gifer()->setKeyGifFile("../../img/test.gif", 1);
		// device->gifer()->setKeyGifFile("../../img/test.gif", 2);
		// device->gifer()->setKeyGifFile("../../img/test.gif", 2);
		// device->gifer()->setKeyGifFile("../../img/test.gif", 3);
		// device->gifer()->setKeyGifFile("../../img/test.gif", 4);
		// device->gifer()->setKeyGifFile("../../img/test.gif", 10);
		// device->gifer()->setKeyGifFile("../../img/test.gif", 11);
		// device->gifer()->setKeyGifFile("../../img/test.gif", 15);
		device->gifer()->startGifLoop();
		// device->gifer()->setKeyGifFile("../../img/test.gif", 13, 30);
		{
			/// crop by point(x, y) and size(width, height)
			// device->getBackgroundGifHelper()->_crop_offset_x = 100;
			// device->getBackgroundGifHelper()->_crop_offset_y = 100;
			// device->getBackgroundGifHelper()->_width = 300;
			// device->getBackgroundGifHelper()->_height = 100;
			// device->getBackgroundGifHelper()->_processer = ImgProcess::Crop;
			// device->getBackgroundGifHelper()->_crop_offset_y = 100;
			// device->gifer()->clearBackgroundGifStream();
			// device->gifer()->clearBackgroundGifStream(static_cast<uint8_t>(N4ProBackgroundGifPostion::TouchScreen));
			// device->gifer()->setBackgroundGifFile("../../img/test.gif", 500, 100);
			/// all screen
			// device->gifer()->setBackgroundGifFile("../../img/test.gif");
		}

		device->setKeyImgFile("../../img/button_test.jpg", 9);
		device->reader()->registerReadCallback(1, []()
											   { debugPrint("secondary screen 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Key 11 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(28, []()
											   { debugPrint("secondary screen swipe left"); }, RegisterEvent::SwipeLeft);
		device->reader()->registerReadCallback(28, []()
											   { debugPrint("secondary screen Unknown event"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(16, []()
											   { debugPrint("knob 1 left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(17, []()
											   { debugPrint("knob 1 right"); }, RegisterEvent::KnobRight, true);
		auto N4pro = std::dynamic_pointer_cast<StreamDockN4Pro>(device);
		N4pro->registerTouchBarCallback(nullptr, true);
	}
}

namespace TEST_XL
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDXL || !device->feature()->supportBackGroundGif)
			return;
		device->heartbeater()->startHeartBeatLoop();
		auto configs = buildConfigCommand(N4ProConfigEnumerate::EnableVibration, ConfigState::Off);
		device->configer()->setDeviceConfig(configs);
		configs = buildConfigCommand<XLConfigEnumerate>({
			{XLConfigEnumerate::LedFollowKeyLight, ConfigState::On},
		});
		device->configer()->setDeviceConfig(configs);
		device->setKeyBrightness(100);
		device->rgber()->setLedColor(0, 255, 0);
		device->rgber()->setLedBrightness(10);
		// device->rgber()->resetLedColor();
		device->reader()->startReadLoop();
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setBackgroundImgFile("../../img/button_test.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->clearAllKeys();
		// device->setKeyImgFile("../../img/test_img.jpg", 2);
		// device->gifer()->setKeyGifFile("../../img/test.gif", 1);
		// device->gifer()->setKeyGifFile("../../img/test.gif", 2);
		//  device->gifer()->setKeyGifFile("../../img/2.gif", 2);
		//  device->gifer()->setKeyGifFile("../../img/test.gif", 3);
		//  device->gifer()->setKeyGifFile("../../img/test.gif", 4);
		//  device->gifer()->setKeyGifFile("../../img/2.gif", 10);
		//  device->gifer()->setKeyGifFile("../../img/3.gif", 11);
		for (int i = 1; i <= 32; i++)
			device->gifer()->setKeyGifFile("../../img/test.gif", i);
		// device->gifer()->setKeyGifFile("../../img/test.gif", 15);
		// device->gifer()->setKeyGifFile("../../img/5.gif", 6);
		// device->gifer()->setKeyGifFile("../../img/6.gif", 3);
		// device->gifer()->setKeyGifFile("../../img/7.gif", 13, 30);
		{
			/// crop by point(x, y) and size(width, height)
			// device->getBackgroundGifHelper()->_crop_offset_x = 100;
			// device->getBackgroundGifHelper()->_crop_offset_y = 100;
			// device->getBackgroundGifHelper()->_width = 300;
			// device->getBackgroundGifHelper()->_height = 100;
			// device->getBackgroundGifHelper()->_processer = ImgProcess::Crop;
			// device->getBackgroundGifHelper()->_crop_offset_y = 100;
			// device->gifer()->clearBackgroundGifStream();
			// device->gifer()->clearBackgroundGifStream(static_cast<uint8_t>(N4ProBackgroundGifPostion::TouchScreen));
			// device->gifer()->setBackgroundGifFile("../../img/test.gif", 500, 100);
			/// all screen
			// device->gifer()->setBackgroundGifFile("../../img/test.gif");
		}

		// device->setKeyImgFile("../../img/YiFei.jpg", 9);
		device->gifer()->startGifLoop();
		for (int i = 1; i <= 32; i++)
		{
			int keyIndex = i;
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
												   { debugPrint("Key " + std::to_string(keyIndex) + " pressed"); }, RegisterEvent::KeyPress);
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
												   { debugPrint("Key " + std::to_string(keyIndex) + " release"); }, RegisterEvent::KeyRelease);
		}
		// device->reader()->registerReadCallback(11, []()
		// 									   { debugPrint("Key 11 pressed"); }, RegisterEvent::KeyPress);
		// device->reader()->registerReadCallback(11, []()
		// 									   { debugPrint("Key 11 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(33, []()
											   { debugPrint("left toggle up"); }, RegisterEvent::ToggleUp);
		device->reader()->registerReadCallback(35, []()
											   { debugPrint("right toggle up"); }, RegisterEvent::ToggleUp);
		device->reader()->registerReadCallback(34, []()
											   { debugPrint("left toggle down"); }, RegisterEvent::ToggleDown);
		device->reader()->registerReadCallback(36, []()
											   { debugPrint("right toggle down"); }, RegisterEvent::ToggleDown);
	}
}

namespace TEST_M3
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDM3 || !device->feature()->supportBackGroundGif)
			return;
		device->heartbeater()->startHeartBeatLoop();
		device->setKeyBrightness(100);
		device->reader()->startReadLoop();
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setBackgroundImgFile("../../img/button_test.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->clearAllKeys();
		for (int i = 1; i <= 15; i++)
			device->gifer()->setKeyGifFile("../../img/test.gif", i);
		device->gifer()->startGifLoop();
		for (int i = 1; i <= 15; i++)
		{
			int keyIndex = i;
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
												   { debugPrint("Key " + std::to_string(keyIndex) + " pressed"); }, RegisterEvent::KeyPress);
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
												   { debugPrint("Key " + std::to_string(keyIndex) + " release"); }, RegisterEvent::KeyRelease);
		}
		device->reader()->registerReadCallback(22, []()
											   { debugPrint("Konb1 pressed"); }, RegisterEvent::KnobPress);
		// device->reader()->registerReadCallback(22, []()
		// 									   { debugPrint("Konb1 release"); }, RegisterEvent::KnobRelease);
		device->reader()->registerReadCallback(23, []()
											   { debugPrint("Konb2 pressed"); }, RegisterEvent::KnobPress);
		// device->reader()->registerReadCallback(23, []()
		// 									   { debugPrint("Konb2 release"); }, RegisterEvent::KnobRelease);
		device->reader()->registerReadCallback(24, []()
											   { debugPrint("Konb3 pressed"); }, RegisterEvent::KnobPress);
		// device->reader()->registerReadCallback(24, []()
		// 									   { debugPrint("Konb3 release"); }, RegisterEvent::KnobRelease);
		device->reader()->registerReadCallback(16, []()
											   { debugPrint("Konb1 left rotation"); }, RegisterEvent::KnobLeft);
		device->reader()->registerReadCallback(17, []()
											   { debugPrint("Konb1 right rotation"); }, RegisterEvent::KnobRight);
		device->reader()->registerReadCallback(18, []()
											   { debugPrint("Konb2 left rotation"); }, RegisterEvent::KnobLeft);
		device->reader()->registerReadCallback(19, []()
											   { debugPrint("Konb2 right rotation"); }, RegisterEvent::KnobRight);
		device->reader()->registerReadCallback(20, []()
											   { debugPrint("Konb3 left rotation"); }, RegisterEvent::KnobLeft);
		device->reader()->registerReadCallback(21, []()
											   { debugPrint("Konb3 right rotation"); }, RegisterEvent::KnobRight);
	}
}

namespace TEST_M18V3
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SDM18)
			return;
		device->heartbeater()->startHeartBeatLoop();
		device->setKeyBrightness(100);
		device->reader()->startReadLoop();
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setBackgroundImgFile("../../img/button_test.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->clearAllKeys();
		for (int i = 1; i <= 18; i++)
			device->gifer()->setKeyGifFile("../../img/test.gif", i);
		device->gifer()->startGifLoop();
		for (int i = 1; i <= 18; i++)
		{
			int keyIndex = i;
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
												   { debugPrint("Key " + std::to_string(keyIndex) + " pressed"); }, RegisterEvent::KeyPress);
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
												   { debugPrint("Key " + std::to_string(keyIndex) + " release"); }, RegisterEvent::KeyRelease);
		}
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
		device->wakeupScreen();
		device->setKeyBrightness(100);
		device->reader()->startReadLoop();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		for (int i = 1; i <= 6; i++)
			device->setKeyImgFile("../../img/button_test.jpg", i);
		device->refresh();
		for (int i = 1; i <= 6; i++)
		{
			int keyIndex = i;
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
												   { debugPrint("Key " + std::to_string(keyIndex) + " pressed"); }, RegisterEvent::KeyPress);
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
												   { debugPrint("Key " + std::to_string(keyIndex) + " release"); }, RegisterEvent::KeyRelease);
		}
		device->reader()->registerReadCallback(7, []()
											   { debugPrint("Konb1 pressed"); }, RegisterEvent::KnobPress);
		device->reader()->registerReadCallback(7, []()
											   { debugPrint("Konb1 release"); }, RegisterEvent::KnobRelease);
		device->reader()->registerReadCallback(8, []()
											   { debugPrint("Konb2 pressed"); }, RegisterEvent::KnobPress);
		device->reader()->registerReadCallback(8, []()
											   { debugPrint("Konb2 release"); }, RegisterEvent::KnobRelease);
		device->reader()->registerReadCallback(9, []()
											   { debugPrint("Konb3 pressed"); }, RegisterEvent::KnobPress);
		device->reader()->registerReadCallback(9, []()
											   { debugPrint("Konb3 release"); }, RegisterEvent::KnobRelease);
		device->reader()->registerReadCallback(10, []()
											   { debugPrint("Konb1 left rotation"); }, RegisterEvent::KnobLeft);
		device->reader()->registerReadCallback(13, []()
											   { debugPrint("Konb1 right rotation"); }, RegisterEvent::KnobRight);
		device->reader()->registerReadCallback(11, []()
											   { debugPrint("Konb2 left rotation"); }, RegisterEvent::KnobLeft);
		device->reader()->registerReadCallback(14, []()
											   { debugPrint("Konb2 right rotation"); }, RegisterEvent::KnobRight);
		device->reader()->registerReadCallback(12, []()
											   { debugPrint("Konb3 left rotation"); }, RegisterEvent::KnobLeft);
		device->reader()->registerReadCallback(15, []()
											   { debugPrint("Konb3 right rotation"); }, RegisterEvent::KnobRight);
		K1ProDevice->setKeyboardLightingEffects(1);
		K1ProDevice->setKeyboardBacklightBrightness(6);
		K1ProDevice->setKeyboardRgbBacklight(255, 0, 0);
	}
}