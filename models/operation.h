#pragma once

#include <string>
#include <cstdint>
#include <jsoncpp/json/value.h>
#include <models/types.h>
#include <stdexcept>
#include <utility>
#include <vector>

namespace operation {

class Error : public std::runtime_error {
 public:
    Error(int errorCode, const std::string& message)
        : std::runtime_error(message), _errorCode(errorCode) {}

    int errorCode() const noexcept { return _errorCode; }

 private:
    int _errorCode;
};

enum class OperationType : char {
    GET,
    SET,
    DELETE,
    DEL = DELETE,
    LPUSH,
    RPUSH,
    LPOP,
    RPOP,
    SADD,
    SREM,
    ISMEMBER,
    GETITEM,
    HSET,
    HDEL,
    INC,
    DEC,
    SIZE
};

struct Value {
    std::string first;
    types::Type second = types::Type::OTHER;
    Json::Value data;

    Value() = default;
    Value(std::string text, types::Type valueType)
        : first(std::move(text)), second(valueType) {
        if(second == types::Type::INT) {
            size_t parsed = 0;
            const auto number = std::stoll(first, &parsed);
            if(parsed != first.size()) throw std::invalid_argument("Invalid integer operation value");
            data = Json::Value(static_cast<Json::Int64>(number));
        } else {
            data = Json::Value(first);
        }
    }
    explicit Value(const Json::Value& value) : data(value) {
        if(value.isString()) {
            first = value.asString();
            second = types::Type::STRING;
        } else if(value.isIntegral()) {
            first = std::to_string(value.asInt64());
            second = types::Type::INT;
        } else if(value.isDouble()) {
            first = std::to_string(value.asDouble());
            second = types::Type::INT;
        } else if(value.isArray()) {
            second = types::Type::LIST;
        } else if(value.isObject()) {
            second = types::Type::HASH;
        } else if(value.isBool()) {
            first = value.asBool() ? "true" : "false";
            second = types::Type::OTHER;
        } else {
            first = "null";
            second = types::Type::OTHER;
        }
    }
};

struct Operation {
    OperationType type; 
    std::string key;
    std::vector<Value> value;
    std::int64_t ttl = 0;
    std::int64_t expiresAt = 0;
};

struct OperationResult {
    Json::Value message;
};
    
};