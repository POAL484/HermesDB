#pragma once

#include <string>

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
    
};

struct Operation {
    OperationType type; 
    std::string key;
    std::string value;
};

struct OperationResult {
    std::string message;
};
    
};