#pragma once

#include <string>
#include <jsoncpp/json/value.h>
#include <models/types.h>

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
    HDEL,
    INC,
    DEC
};

struct Operation {
    OperationType type; 
    std::string key;
    std::vector<std::pair<std::string, types::Type>> value;
};

struct OperationResult {
    Json::Value message;
};
    
};