/**
 * @file irgbcontroller.h
 * @brief Interface for controlling device RGB lighting effects.
 *
 * This interface defines methods for setting LED brightness, color,
 * and resetting LED states. It is intended to be implemented by
 * concrete device-specific RGB controllers.
 */
#pragma once 
#include <streamdock.h>

class StreamDock;
class IRGBController
{
public:
	virtual ~IRGBController() = default;
	/**
		 * @brief Set the brightness of the LED lights.
		 * @param brightness Brightness level, usually in the range 0–100.
		 */
	virtual void setLedBrightness(uint8_t brightness) = 0;

	/**
	 * @brief Set the color of the first N LEDs.
	 * @param red Red channel (0–255).
	 * @param green Green channel (0–255).
	 * @param blue Blue channel (0–255).
	 */
	virtual void setLedColor(uint8_t red, uint8_t green, uint8_t blue) = 0;

	/**
	 * @brief Reset the LED colors to the default state (typically off or white).
	 */
	virtual void resetLedColor() = 0;
};