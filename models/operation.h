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
    RPOP
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