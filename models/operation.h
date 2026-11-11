#pragma once

#include <string>
#include <jsoncpp/json/value.h>

namespace operation {

enum class OperationType : char {
    GET,
    SET,
    DELETE,
    LPUSH,
    RPUSH,
    LPOP,
    RPOP,
    SADD,
    SREM,
    ISMEMBER,
    GETITEM,
    HSET,
    HDEL
};

struct Operation {
    OperationType type; 
    std::string key;
    std::vector<std::string> value;
};

struct OperationResult {
    Json::Value message;
};
    
};