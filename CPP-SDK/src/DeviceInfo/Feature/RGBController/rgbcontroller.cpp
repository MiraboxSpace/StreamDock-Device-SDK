#include "rgbcontroller.h"
#include <algorithm>

RGBController::RGBController(StreamDock* instance)
	: _instance(instance)
{
}

RGBController::~RGBController()
{
}

void RGBController::setLedBrightness(uint8_t brightness)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->hasRGBLed)
		_instance->_transport->setLedBrightness(brightness);
}

void RGBController::setLedColor(uint8_t red, uint8_t green, uint8_t blue)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->hasRGBLed)
		_instance->_transport->setLedColor(_instance->_feature->ledCounts, red, green, blue);
}

void RGBController::setSingleLedColor(const std::vector<std::array<uint8_t, 3>> &colors)
{
	if (!_instance || colors.empty())
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->hasRGBLed)
	{
		const auto count = std::min<size_t>(colors.size(), _instance->_feature->ledCounts);
		_instance->_transport->setSingleLedColor(std::vector<std::array<uint8_t, 3>>(colors.begin(), colors.begin() + count));
	}
}

void RGBController::resetLedColor()
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->hasRGBLed)
		_instance->_transport->resetLedColor();
}
