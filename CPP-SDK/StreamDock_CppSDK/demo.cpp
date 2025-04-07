#include "DeviceManager.h"

void test1(const std::shared_ptr<StreamDock>& s, bool test)
{
	if (!test)
		return;
	s->setBackgroundImg("./img/YiFei320.png");
	s->refresh();
}

void test2(const std::shared_ptr<StreamDock>& s, bool test)
{
	if (!test)
		return;
	fipImage img;
	img.load("./img/YiFei.png");
	s->setBackgroundImgData(img.accessPixels(), RgbaFormat::RGBA, 800, 480); // we will rescale to 800 * 480
	s->refresh();
}

void test3(const std::shared_ptr<StreamDock>& s, bool test)
{
	if (!test)
		return;
	for (int i = 1; i < 19;++i)
	{
		s->setKeyImg("./img/YiFei176.png", i);
	}
	s->refresh();
	s->refresh();
}

void test4(const std::shared_ptr<StreamDock>& s, bool test)
{
	if (!test)
		return;
	fipImage img;
	img.load("./img/tiga112.png");for (int i = 1; i < 19;++i)
	{
		s->setKeyImgData(img.accessPixels(), i, RgbaFormat::RGBA, 112, 112);
	}
	s->refresh();
}

int main()
{
	std::unique_ptr<DeviceManager> manager(new DeviceManager());
	auto streamDocks = manager->enumerate();
	std::cout << "find [" << streamDocks.size() << "] device" << "\n";
	manager->asyncListen();
	joining_thread j;
	for (auto it = streamDocks.begin(); it != streamDocks.end(); it++)
	{
		// get device manager instance
		auto s = it->second;
		s->open();
		s->init();
		j = joining_thread(std::thread([](decltype(s) st) {
			while (1)
			{
				auto a = st->read();
				auto str = StreamDock::parseRead(a);
				if (!str.empty()) std::cout << str << std::endl;
			}
			}, s));
		test1(s, true);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		test2(s, true);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		test3(s, true);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		test4(s, true);
	}
}
