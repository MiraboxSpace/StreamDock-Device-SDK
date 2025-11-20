/**
 * @file nullrgbcontroller.h
 * @brief Null implementation of IRGBController for devices without RGB support.
 *
 * All methods in this class are no-ops, making it safe to use in contexts
 * where RGB control is optional or not supported.
 */

class NullRGBController : public IRGBController
{
public:
	/// No-op implementation
	virtual void setLedBrightness(uint8_t brightness) override {
	}

	/// No-op implementation
	virtual void setLedColor(uint8_t red, uint8_t green, uint8_t blue) override {
	}

	/// No-op implementation
	virtual void resetLedColor() override {
	}
};
