#include "configer.h"

Configer::Configer(StreamDock* instance)
	: _instance(instance)
{
}

Configer::~Configer()
{
}

void Configer::setDeviceConfig(std::vector<uint8_t> configs)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->supportConfig)
		_instance->_transport->setDeviceConfig(configs);
}
