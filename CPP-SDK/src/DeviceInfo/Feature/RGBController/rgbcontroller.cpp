#include "rgbcontroller.h"

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

void RGBController::resetLedColor()
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->hasRGBLed)
		_instance->_transport->resetLedColor();
}
