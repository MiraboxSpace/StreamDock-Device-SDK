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

namespace TEST_293V2
{
	void test(std::shared_ptr<StreamDock> device)
	{
		if (device->info()->originType != DeviceOriginType::SD293 || device->feature()->isDualDevice)
			return;
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		// device->setBackgroundImgFile("./img/YiFei.jpg", 5000);
		// device->refresh();
		// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->setKeyImgFile("./img/YiFei.jpg", 9);
		device->refresh();
		device->setKeyImgFile("./img/YiFei.jpg", 8);
		device->refresh();
		device->setKeyImgFile("./img/YiFei.jpg", 7);
		device->refresh();
		device->setKeyImgFile("./img/YiFei.jpg", 6);
		device->refresh();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
			{ ToolKit::print("Key 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(10, []()
			{ ToolKit::print("Key 10"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(15, []()
			{ ToolKit::print("Key 15"); }, RegisterEvent::EveryThing, true);
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
		device->setBackgroundImgFile("./img/YiFei.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->gifer()->setKeyGifFile("./img/1.gif", 1);
		device->gifer()->setKeyGifFile("./img/1.gif", 2);
		device->gifer()->setKeyGifFile("./img/2.gif", 2);
		device->gifer()->setKeyGifFile("./img/1.gif", 3);
		device->gifer()->setKeyGifFile("./img/1.gif", 4);
		device->gifer()->setKeyGifFile("./img/2.gif", 10);
		device->gifer()->setKeyGifFile("./img/3.gif", 11);
		device->gifer()->setKeyGifFile("./img/4.gif", 15);
		device->gifer()->setKeyGifFile("./img/5.gif", 7);
		device->gifer()->setKeyGifFile("./img/6.gif", 3);
		device->gifer()->setKeyGifFile("./img/7.gif", 13, 30);
		device->setKeyImgFile("./img/YiFei.jpg", 9);
		device->rgber()->setLedBrightness(13);
		device->gifer()->startGifLoop();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
			{ ToolKit::print("Key 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(10, []()
			{ ToolKit::print("Key 10"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(15, []()
			{ ToolKit::print("Key 15"); }, RegisterEvent::EveryThing, true);
		device->setBackgroundImgFile("./img/bg.png");
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
		// device->setBackgroundImgFile("./img/YiFei.jpg", 5000);
		// device->refresh();
		// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->setKeyImgFile("./img/YiFei.jpg", 1);
		device->refresh();
		device->setKeyImgFile("./img/YiFei.jpg", 6);
		device->refresh();
		device->setKeyImgFile("./img/YiFei.jpg", 16);
		device->refresh();
		device->setKeyImgFile("./img/YiFei.jpg", 19);
		device->refresh();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
			{ ToolKit::print("Key 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(10, []()
			{ ToolKit::print("Key 10"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(15, []()
			{ ToolKit::print("Key 15"); }, RegisterEvent::EveryThing, true);
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
		device->setBackgroundImgFile("./img/YiFei.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->gifer()->setKeyGifFile("./img/1.gif", 1);
		device->gifer()->setKeyGifFile("./img/1.gif", 2);
		device->gifer()->setKeyGifFile("./img/2.gif", 2);
		device->gifer()->setKeyGifFile("./img/1.gif", 3);
		device->gifer()->setKeyGifFile("./img/1.gif", 4);
		device->gifer()->setKeyGifFile("./img/2.gif", 10);
		device->gifer()->setKeyGifFile("./img/3.gif", 11);
		device->gifer()->setKeyGifFile("./img/4.gif", 15);
		device->gifer()->setKeyGifFile("./img/5.gif", 7);
		device->gifer()->setKeyGifFile("./img/6.gif", 3);
		device->gifer()->setKeyGifFile("./img/7.gif", 13, 30);
		device->setKeyImgFile("./img/YiFei.jpg", 13);
		device->gifer()->setKeyGifFile("./img/6.gif", 17);
		device->rgber()->setLedBrightness(13);
		device->gifer()->startGifLoop();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
			{ ToolKit::print("Key 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(10, []()
			{ ToolKit::print("Key 10"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(15, []()
			{ ToolKit::print("Key 15"); }, RegisterEvent::EveryThing, true);
		device->setBackgroundImgFile("./img/bg.png");
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
		device->setBackgroundImgFile("./img/YiFei.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->gifer()->setKeyGifFile("./img/1.gif", 1);
		device->gifer()->setKeyGifFile("./img/1.gif", 2);
		device->gifer()->setKeyGifFile("./img/2.gif", 2);
		device->gifer()->setKeyGifFile("./img/1.gif", 3);
		device->gifer()->setKeyGifFile("./img/1.gif", 4);
		device->gifer()->setKeyGifFile("./img/2.gif", 10);
		device->gifer()->setKeyGifFile("./img/3.gif", 11);
		device->gifer()->setKeyGifFile("./img/4.gif", 15);
		device->gifer()->setKeyGifFile("./img/5.gif", 7);
		device->gifer()->setKeyGifFile("./img/6.gif", 3);
		device->gifer()->setKeyGifFile("./img/7.gif", 13, 30);
		device->setKeyImgFile("./img/YiFei.jpg", 9);
		device->rgber()->setLedColor(255, 0, 0);
		device->rgber()->setLedBrightness(13);
		device->gifer()->startGifLoop();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
			{ ToolKit::print("Key 1"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(6, []()
			{ ToolKit::print("Key 6 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(6, []()
			{ ToolKit::print("Key 6 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(16, []()
			{ ToolKit::print("left button"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(17, []()
			{ ToolKit::print("middle button"); }, RegisterEvent::EveryThing, true);
		device->setBackgroundImgFile("./img/bg.png");
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
		// device->setBackgroundImgFile("./img/bg.png", 5000);
		// device->refresh();
		// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->setKeyImgFile("./img/YiFei.jpg", 1);
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
			{ ToolKit::print("Key 1"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Button 11 pressed"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(6, []()
			{ ToolKit::print("Knob 6 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(7, []()
			{ ToolKit::print("button 7 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(10, []()
			{ ToolKit::print("right knob press"); }, RegisterEvent::KnobPress);
		device->reader()->registerReadCallback(13, []()
			{ ToolKit::print("right knob left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(14, []()
			{ ToolKit::print("middle knob left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(16, []()
			{ ToolKit::print("right knob right"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(17, []()
			{ ToolKit::print("middle knob right"); }, RegisterEvent::EveryThing, true);
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
		device->setBackgroundImgFile("./img/bg.png");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->gifer()->setKeyGifFile("./img/1.gif", 1);
		device->gifer()->setKeyGifFile("./img/1.gif", 2);
		device->gifer()->setKeyGifFile("./img/2.gif", 2);
		device->gifer()->setKeyGifFile("./img/1.gif", 3);
		device->gifer()->setKeyGifFile("./img/1.gif", 4);
		device->gifer()->setKeyGifFile("./img/2.gif", 10);
		device->gifer()->setKeyGifFile("./img/3.gif", 11);
		device->gifer()->setKeyGifFile("./img/4.gif", 15);
		device->gifer()->setKeyGifFile("./img/5.gif", 7);
		device->gifer()->setKeyGifFile("./img/6.gif", 3);
		device->gifer()->setKeyGifFile("./img/7.gif", 13, 30);
		device->setKeyImgFile("./img/YiFei.jpg", 9);
		device->rgber()->setLedBrightness(13);
		device->gifer()->startGifLoop();
		device->reader()->startReadLoop();
		device->reader()->registerReadCallback(1, []()
			{ ToolKit::print("Key 1"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Button 11 pressed"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(6, []()
			{ ToolKit::print("Knob 6 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(7, []()
			{ ToolKit::print("button 7 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(10, []()
			{ ToolKit::print("right knob press"); }, RegisterEvent::KnobPress);
		device->reader()->registerReadCallback(13, []()
			{ ToolKit::print("right knob left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(14, []()
			{ ToolKit::print("middle knob left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(16, []()
			{ ToolKit::print("right knob right"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(17, []()
			{ ToolKit::print("middle knob right"); }, RegisterEvent::EveryThing, true);
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
		N1device->gifer()->setKeyGifFile("./img/1.gif", 1);
		N1device->gifer()->setKeyGifFile("./img/1.gif", 2);
		N1device->gifer()->setKeyGifFile("./img/2.gif", 2);
		N1device->gifer()->setKeyGifFile("./img/1.gif", 3);
		N1device->gifer()->setKeyGifFile("./img/1.gif", 4);
		N1device->gifer()->setKeyGifFile("./img/2.gif", 10);
		N1device->gifer()->setKeyGifFile("./img/3.gif", 11);
		N1device->gifer()->setKeyGifFile("./img/4.gif", 15);
		N1device->gifer()->setKeyGifFile("./img/5.gif", 7);
		N1device->gifer()->setKeyGifFile("./img/6.gif", 3);
		N1device->gifer()->setKeyGifFile("./img/7.gif", 13, 30);
		N1device->gifer()->setKeyGifFile("./img/7.gif", 18, 30);
		N1device->setKeyImgFile("./img/YiFei.jpg", 9);
		N1device->rgber()->setLedBrightness(13);
		N1device->gifer()->startGifLoop();
		N1device->reader()->startReadLoop();
		N1device->reader()->registerReadCallback(1, []()
			{ ToolKit::print("Key 1"); }, RegisterEvent::EveryThing);
		N1device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 release"); }, RegisterEvent::KeyRelease);
		N1device->reader()->registerReadCallback(6, []()
			{ ToolKit::print("Key 6 release"); }, RegisterEvent::KeyRelease);
		N1device->reader()->registerReadCallback(16, []()
			{ ToolKit::print("button 16 release"); }, RegisterEvent::KeyRelease);
		N1device->reader()->registerReadCallback(17, []()
			{ ToolKit::print("button 17 release"); }, RegisterEvent::KeyRelease);
		N1device->reader()->registerReadCallback(19, []()
			{ ToolKit::print("knob left"); }, RegisterEvent::EveryThing, true);
		N1device->reader()->registerReadCallback(20, []()
			{ ToolKit::print("knob right"); }, RegisterEvent::EveryThing, true);
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
		device->setBackgroundImgFile("./img/YiFei.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->gifer()->setKeyGifFile("./img/1.gif", 1);
		device->gifer()->setKeyGifFile("./img/1.gif", 2);
		device->gifer()->setKeyGifFile("./img/2.gif", 2);
		device->gifer()->setKeyGifFile("./img/1.gif", 3);
		device->gifer()->setKeyGifFile("./img/1.gif", 4);
		device->gifer()->setKeyGifFile("./img/2.gif", 10);
		device->gifer()->setKeyGifFile("./img/3.gif", 11);
		device->gifer()->setKeyGifFile("./img/4.gif", 15);
		device->gifer()->setKeyGifFile("./img/5.gif", 7);
		device->gifer()->setKeyGifFile("./img/6.gif", 3);
		device->gifer()->setKeyGifFile("./img/7.gif", 13, 30);
		device->setKeyImgFile("./img/YiFei.jpg", 9);
		device->rgber()->setLedBrightness(13);
		device->gifer()->startGifLoop();
		device->reader()->registerReadCallback(1, []()
			{ ToolKit::print("secondary screen 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(28, []()
			{ ToolKit::print("secondary screen swipe left"); }, RegisterEvent::SwipeLeft);
		device->reader()->registerReadCallback(28, []()
			{ ToolKit::print("secondary screen Unknown event"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(16, []()
			{ ToolKit::print("knob 1 left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(17, []()
			{ ToolKit::print("knob 1 right"); }, RegisterEvent::KnobRight, true);

		device->setBackgroundImgFile("./img/bg.png");
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
		configs = buildConfigCommand<N4ProConfigEnumerate>({ {N4ProConfigEnumerate::EnableVibration, ConfigState::Off},
															{N4ProConfigEnumerate::EnableBootVideo, ConfigState::Off} });
		device->configer()->setDeviceConfig(configs);
		device->setKeyBrightness(100);
		device->rgber()->setLedColor(0, 255, 0);
		device->rgber()->setLedBrightness(10);
		// device->rgber()->resetLedColor();
		device->reader()->startReadLoop();
		device->wakeupScreen();
		device->clearAllKeys();
		device->setEncoder(std::make_shared<OpenCVImageEncoder>());
		device->setBackgroundImgFile("./img/YiFei.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->clearAllKeys();
		device->gifer()->setKeyGifFile("./img/1.gif", 1);
		device->gifer()->setKeyGifFile("./img/1.gif", 2);
		device->gifer()->setKeyGifFile("./img/2.gif", 2);
		device->gifer()->setKeyGifFile("./img/1.gif", 3);
		device->gifer()->setKeyGifFile("./img/1.gif", 4);
		device->gifer()->setKeyGifFile("./img/2.gif", 10);
		device->gifer()->setKeyGifFile("./img/3.gif", 11);
		device->gifer()->setKeyGifFile("./img/4.gif", 15);
		device->gifer()->setKeyGifFile("./img/5.gif", 6);
		device->gifer()->setKeyGifFile("./img/6.gif", 3);
		device->gifer()->setKeyGifFile("./img/7.gif", 13, 30);
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
			// device->gifer()->setBackgroundGifFile("./img/1.gif", 500, 100);
			/// all screen
			device->gifer()->setBackgroundGifFile("./img/1.gif");
		}

		device->setKeyImgFile("./img/YiFei.jpg", 9);
		device->gifer()->startGifLoop();
		device->reader()->registerReadCallback(1, []()
			{ ToolKit::print("secondary screen 1 release"); }, RegisterEvent::EveryThing);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 pressed"); }, RegisterEvent::KeyPress);
		device->reader()->registerReadCallback(11, []()
			{ ToolKit::print("Key 11 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(28, []()
			{ ToolKit::print("secondary screen swipe left"); }, RegisterEvent::SwipeLeft);
		device->reader()->registerReadCallback(28, []()
			{ ToolKit::print("secondary screen Unknown event"); }, RegisterEvent::EveryThing, true);
		device->reader()->registerReadCallback(16, []()
			{ ToolKit::print("knob 1 left"); }, RegisterEvent::KnobLeft, true);
		device->reader()->registerReadCallback(17, []()
			{ ToolKit::print("knob 1 right"); }, RegisterEvent::KnobRight, true);
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
		device->setBackgroundImgFile("./img/test.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->clearAllKeys();
		// device->setKeyImgFile("./img/test_img.jpg", 2);
		// device->gifer()->setKeyGifFile("./img/test.gif", 1);
		// device->gifer()->setKeyGifFile("./img/test.gif", 2);
		//  device->gifer()->setKeyGifFile("./img/2.gif", 2);
		//  device->gifer()->setKeyGifFile("./img/1.gif", 3);
		//  device->gifer()->setKeyGifFile("./img/1.gif", 4);
		//  device->gifer()->setKeyGifFile("./img/2.gif", 10);
		//  device->gifer()->setKeyGifFile("./img/3.gif", 11);
		for (int i = 1; i <= 32; i++)
			device->gifer()->setKeyGifFile("./img/test.gif", i);
		// device->gifer()->setKeyGifFile("./img/test.gif", 15);
		// device->gifer()->setKeyGifFile("./img/5.gif", 6);
		// device->gifer()->setKeyGifFile("./img/6.gif", 3);
		// device->gifer()->setKeyGifFile("./img/7.gif", 13, 30);
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
			// device->gifer()->setBackgroundGifFile("./img/1.gif", 500, 100);
			/// all screen
			// device->gifer()->setBackgroundGifFile("./img/1.gif");
		}

		// device->setKeyImgFile("./img/YiFei.jpg", 9);
		device->gifer()->startGifLoop();
		for (int i = 1; i <= 32; i++)
		{
			int keyIndex = i;
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
				{ ToolKit::print("Key " + std::to_string(keyIndex) + " pressed"); }, RegisterEvent::KeyPress);
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
				{ ToolKit::print("Key " + std::to_string(keyIndex) + " release"); }, RegisterEvent::KeyRelease);
		}
		// device->reader()->registerReadCallback(11, []()
		// 									   { ToolKit::print("Key 11 pressed"); }, RegisterEvent::KeyPress);
		// device->reader()->registerReadCallback(11, []()
		// 									   { ToolKit::print("Key 11 release"); }, RegisterEvent::KeyRelease);
		device->reader()->registerReadCallback(33, []()
			{ ToolKit::print("left toggle up"); }, RegisterEvent::ToggleUp);
		device->reader()->registerReadCallback(35, []()
			{ ToolKit::print("right toggle up"); }, RegisterEvent::ToggleUp);
		device->reader()->registerReadCallback(34, []()
			{ ToolKit::print("left toggle down"); }, RegisterEvent::ToggleDown);
		device->reader()->registerReadCallback(36, []()
			{ ToolKit::print("right toggle down"); }, RegisterEvent::ToggleDown);
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
		device->setBackgroundImgFile("./img/test.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->clearAllKeys();
		for (int i = 1; i <= 15; i++)
			device->gifer()->setKeyGifFile("./img/test.gif", i);
		device->gifer()->startGifLoop();
		for (int i = 1; i <= 15; i++)
		{
			int keyIndex = i;
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
				{ ToolKit::print("Key " + std::to_string(keyIndex) + " pressed"); }, RegisterEvent::KeyPress);
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
				{ ToolKit::print("Key " + std::to_string(keyIndex) + " release"); }, RegisterEvent::KeyRelease);
		}
		device->reader()->registerReadCallback(22, []()
			{ ToolKit::print("Konb1 pressed"); }, RegisterEvent::KnobPress);
		// device->reader()->registerReadCallback(22, []()
		// 									   { ToolKit::print("Konb1 release"); }, RegisterEvent::KnobRelease);
		device->reader()->registerReadCallback(23, []()
			{ ToolKit::print("Konb2 pressed"); }, RegisterEvent::KnobPress);
		// device->reader()->registerReadCallback(23, []()
		// 									   { ToolKit::print("Konb2 release"); }, RegisterEvent::KnobRelease);
		device->reader()->registerReadCallback(24, []()
			{ ToolKit::print("Konb3 pressed"); }, RegisterEvent::KnobPress);
		// device->reader()->registerReadCallback(24, []()
		// 									   { ToolKit::print("Konb3 release"); }, RegisterEvent::KnobRelease);
		device->reader()->registerReadCallback(16, []()
			{ ToolKit::print("Konb1 left rotation"); }, RegisterEvent::KnobLeft);
		device->reader()->registerReadCallback(17, []()
			{ ToolKit::print("Konb1 right rotation"); }, RegisterEvent::KnobRight);
		device->reader()->registerReadCallback(18, []()
			{ ToolKit::print("Konb2 left rotation"); }, RegisterEvent::KnobLeft);
		device->reader()->registerReadCallback(19, []()
			{ ToolKit::print("Konb2 right rotation"); }, RegisterEvent::KnobRight);
		device->reader()->registerReadCallback(20, []()
			{ ToolKit::print("Konb3 left rotation"); }, RegisterEvent::KnobLeft);
		device->reader()->registerReadCallback(21, []()
			{ ToolKit::print("Konb3 right rotation"); }, RegisterEvent::KnobRight);
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
		device->setBackgroundImgFile("./img/test.jpg");
		device->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		device->clearAllKeys();
		for (int i = 1; i <= 18; i++)
			device->gifer()->setKeyGifFile("./img/test.gif", i);
		device->gifer()->startGifLoop();
		for (int i = 1; i <= 18; i++)
		{
			int keyIndex = i;
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
				{ ToolKit::print("Key " + std::to_string(keyIndex) + " pressed"); }, RegisterEvent::KeyPress);
			device->reader()->registerReadCallback(keyIndex, [keyIndex]()
				{ ToolKit::print("Key " + std::to_string(keyIndex) + " release"); }, RegisterEvent::KeyRelease);
		}
	}
}