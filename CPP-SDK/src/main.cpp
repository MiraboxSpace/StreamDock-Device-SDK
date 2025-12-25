#include <iostream>
#include <DeviceManager/devicemanager.h>
#include "test.h"

void doSomething(std::shared_ptr<StreamDock> device)
{
	TEST_293V2::test(device);
	TEST_293V3::test(device);
	TEST_293sV2::test(device);
	TEST_293sV3::test(device);
	TEST_M18::test(device);
	TEST_N3V2::test(device);
	TEST_N3V25::test(device);
	TEST_N1::test(device);
	TEST_N4::test(device);
	TEST_N4Pro::test(device);
	TEST_XL::test(device);
	TEST_M3::test(device);
	TEST_M18V3::test(device);
	TEST_K1Pro::test(device);
}

int main()
{
	DeviceManager::instance().enumerator();
	DeviceManager::instance().listen([](std::shared_ptr<StreamDock> device)
									 { doSomething(device); });
	auto &streamdocks = DeviceManager::instance().getStreamDocks();
	for (const auto &device : streamdocks)
	{
		try
		{
			doSomething(device.second);
		}
		catch (const std::exception &e)
		{
			std::cerr << "Exception: " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Unknown exception occurred" << std::endl;
		}
	}

	std::cerr << "Press enter to exit..." << std::endl;
	std::cin.get();
	return 0;
}
