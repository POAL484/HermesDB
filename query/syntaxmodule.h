#pragma once

#include <jsoncpp/json/value.h>

#include <stdexcept>
#include <string>
#include <unordered_map>

#include <models/operation.h>

namespace syntax {

class SyntaxError : public std::runtime_error {
 public:
    explicit SyntaxError(const std::string& message) : std::runtime_error(message) {}
};

class SyntaxModule {
 public:
    static operation::Operation parse(const Json::Value& request);
    static bool validate(const Json::Value& request);

 private:
    static std::string jsonTypeName(const Json::Value& value);
};

} // namespace syntax
