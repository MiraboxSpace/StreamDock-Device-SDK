/**
 * @file toolkit.h
 * @brief Simple debugging utility class: supports log printing and hex formatting.
 *
 * Features:
 * - ToolKit::print(...): Variadic debug output (supports C++11 and C++17)
 * - ToolKit::format_hex(uint8_t): Format a single byte as an uppercase hex string
 * - ToolKit::printHexBuffer(vector<uint8_t>): Print a buffer as a hex-formatted string
 * - ToolKit::disable_output: Global output control flag (disabled by default)
 *
 * Example usage:
 *   ToolKit::disable_output = false;
 *   ToolKit::print("Value:", 123, 0.5);
 *   ToolKit::printHexBuffer({0x12, 0xAB});
 */

#pragma once
#include <iostream>
#include <sstream>
#include <iomanip>
#include <atomic>

/**
 * @class ToolKit
 * @brief Provides static utility methods for debug output.
 *
 * This class is mainly used for debug message output, including:
 * - A generic variadic `print` function (compatible with both C++11 and C++17)
 * - Hex formatting for individual bytes
 * - Hex display for binary data buffers
 *
 * All output can be controlled via the static atomic flag `disable_output` (disabled by default).
 */
class ToolKit
{
public:
	static inline std::atomic_bool disable_output = true;

public:
#if __cplusplus >= 201703L
	// C++17: use parameter pack expansion (fold expression)
	template <typename... Args>
	static void print(Args &&...args)
	{
		if (disable_output)
			return;
		((std::cerr << std::forward<Args>(args) << " "), ...) << std::endl;
	}	
	template <typename... Args>
	static void wprint(Args &&...args)
	{
		if (disable_output)
			return;
		((std::wcerr << std::forward<Args>(args) << L" "), ...) << std::endl;
	}
#else
	// C++11: use recursive templates to print
	static void print()
	{
		if (disable_output)
			return;
		std::cerr << std::endl;
	}
	template <typename T, typename... Args>
	static void print(T&& first, Args &&...rest)
	{
		if (disable_output)
			return;
		std::cerr << std::forward<T>(first) << " ";
		print(std::forward<Args>(rest)...);
	}
	static void wprint()
	{
		if (disable_output)
			return;
		std::wcerr << std::endl;
	}
	template <typename T, typename... Args>
	static void wprint(T&& first, Args &&...rest)
	{
		if (disable_output)
			return;
		std::wcerr << std::forward<T>(first) << L" ";
		wprint(std::forward<Args>(rest)...);
	}
#endif
	static inline std::string format_hex(uint8_t byte)
	{
		if (disable_output)
			return "";
		std::ostringstream oss;
		oss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(byte);
		return oss.str();
	}

	static void printHexBuffer(const std::vector<uint8_t>& data, std::ostream& os = std::cerr)
	{
		if (disable_output)
			return;
		os << "Buffer (" << data.size() << " bytes): ";
		for (uint8_t byte : data)
		{
			os << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(byte) << " ";
		}
		os << std::dec << std::endl; // Restore decimal mode
	}
};
