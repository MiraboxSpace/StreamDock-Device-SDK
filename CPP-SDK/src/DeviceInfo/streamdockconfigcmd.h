/**
 * @file ConfigBuilder.h
 * @brief Generic configuration command builder for enum-based device settings.
 *
 * This utility provides functions to build a configuration byte vector
 * from one or more enum-state pairs, where each enum value maps to a position
 * in a fixed-size config array.
 *
 * Features:
 * - Supports single or multiple config item updates
 * - Default values are filled for unspecified items
 * - Requires users to specialize ConfigTraits<T> for their enum types
 *
 * Example usage:
 * @code
 * // Define enum representing configuration items
 * enum class N4ProConfigEnumerate : uint8_t {
 *     LedFollowKeyLight,
 *     KeyLightOnDisconne,
 *     CheckUsbPower,
 *     EnableVibration,
 *     ResetUsbReport,
 *     EnableBootVideo,
 *     ConfigCount  // Used only for count
 * };
 *
 * // Provide ConfigTraits specialization
 * template <>
 * struct ConfigTraits<N4ProConfigEnumerate> {
 *     static constexpr size_t count = static_cast<size_t>(N4ProConfigEnumerate::ConfigCount);
 * };
 *
 * // Build configuration commands
 * auto single = buildConfigCommand(N4ProConfigEnumerate::EnableVibration, ConfigState::On);
 * auto batch = buildConfigCommand<N4ProConfigEnumerate>({
 *     {N4ProConfigEnumerate::LedFollowKeyLight, ConfigState::Off},
 *     {N4ProConfigEnumerate::EnableBootVideo, ConfigState::Follow}
 * });
 * @endcode
 */
#pragma once
#include <vector>
#include <cstdint>
#include <initializer_list>
#include <utility>
#include <type_traits>

/**
 * @enum ConfigState
 * @brief Represents the state for a configuration entry.
 *
 * Values:
 * - Default: Fills unspecified config entries
 * - On: Enabled
 * - Off: Disabled
 * - Follow: Follow system/default behavior
 */
enum class ConfigState : uint8_t {
	Default = 0x11,  ///< Default filler value
	On = 0x11,       ///< Enabled state
	Off = 0xFF,      ///< Disabled state
	Follow = 0x1F,   ///< Follow system/default behavior
};

/**
 * @brief Trait template used to define the number of configuration items.
 * @tparam T Enum type representing configuration keys
 *
 * Users must specialize this template for each enum type used with buildConfigCommand.
 * Example:
 * @code
 * template<>
 * struct ConfigTraits<MyEnum> {
 *     static constexpr size_t count = 10;
 * };
 * @endcode
 */
template <typename T>
struct ConfigTraits;

/**
 * @brief Build a configuration command with a single item and its desired state.
 * @tparam ConfigEnum Enum type representing the configuration item
 * @param item The configuration item to set
 * @param state The desired state (On/Off/Follow)
 * @return Byte vector with configuration states filled
 */
template <typename ConfigEnum>
std::vector<uint8_t> buildConfigCommand(ConfigEnum item, ConfigState state)
{
	std::vector<uint8_t> configs(ConfigTraits<ConfigEnum>::count, static_cast<uint8_t>(ConfigState::Default));
	configs[static_cast<size_t>(item)] = static_cast<uint8_t>(state);
	return configs;
}

/**
 * @brief Build a configuration command with multiple item-state pairs.
 * @tparam ConfigEnum Enum type representing configuration items
 * @param changes Initializer list of (item, state) pairs
 * @return Byte vector with configuration states filled accordingly
 */
template <typename ConfigEnum>
std::vector<uint8_t> buildConfigCommand(std::initializer_list<std::pair<ConfigEnum, ConfigState>> changes)
{
	std::vector<uint8_t> configs(ConfigTraits<ConfigEnum>::count, static_cast<uint8_t>(ConfigState::Default));
	for (const auto& [key, state] : changes) {
		size_t index = static_cast<size_t>(key);
		if (index < configs.size()) {
			configs[index] = static_cast<uint8_t>(state);
		}
	}
	return configs;
}
