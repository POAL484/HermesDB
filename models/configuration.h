#pragma once

#include <jsoncpp/json/json.h>

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>

namespace cache::configuration {

inline std::string getStringSetting(const Json::Value& config,
                                    const char* configKey,
                                    const char* environmentVariable,
                                    const std::string& defaultValue) {
    if(const char* value = std::getenv(environmentVariable)) return value;
    return config.get(configKey, defaultValue).asString();
}

inline bool getBoolSetting(const Json::Value& config,
                           const char* configKey,
                           const char* environmentVariable,
                           bool defaultValue) {
    const char* value = std::getenv(environmentVariable);
    if(!value) return config.get(configKey, defaultValue).asBool();

    const std::string_view setting(value);
    if(setting == "true" || setting == "1") return true;
    if(setting == "false" || setting == "0") return false;
    throw std::invalid_argument(std::string("Invalid ") + environmentVariable +
                                " value: " + value + " (expected true, false, 1, or 0)");
}

}
