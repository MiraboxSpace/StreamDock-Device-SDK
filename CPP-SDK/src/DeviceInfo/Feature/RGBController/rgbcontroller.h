/**
 * @file rgbcontroller.h
 * @brief Implementation of IRGBController for controlling RGB lighting on a StreamDock device.
 *
 * This class interacts with the device's Transport layer to apply LED brightness,
 * color, and reset operations.
 */
#pragma once
#include "irgbcontroller.h"
#include "nullrgbcontroller.h"

class RGBController : public IRGBController
{
public:
	/**
	 * @brief Constructor.
	 * @param instance Pointer to the owning StreamDock instance.
	 */
	RGBController(StreamDock* instance);

	/**
	 * @brief Destructor.
	 */
	~RGBController();

	/// @copydoc IRGBController::setLedBrightness
	virtual void setLedBrightness(uint8_t brightness) override;

	/// @copydoc IRGBController::setLedColor
	virtual void setLedColor(uint8_t red, uint8_t green, uint8_t blue) override;

	/// @copydoc IRGBController::resetLedColor
	virtual void resetLedColor() override;

private:
	StreamDock* _instance = nullptr; ///< Pointer to the associated StreamDock device.
};