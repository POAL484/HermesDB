#include <query/semanticmodule.h>

#include <algorithm>

namespace semantic {
namespace {

bool oneOf(types::Type type, std::initializer_list<types::Type> allowed) {
    return std::find(allowed.begin(), allowed.end(), type) != allowed.end();
}

void requireType(types::Type actual, std::initializer_list<types::Type> allowed,
                 const std::string& message) {
    if(!oneOf(actual, allowed)) throw SemanticError(1, message);
}

} // namespace

types::Type SemanticModule::typeFor(const operation::Operation& operation,
                                    const KeyTypes& key_types) {
    const auto it = key_types.find(operation.key);
    return it == key_types.end() ? types::Type::OTHER : it->second;
}

void SemanticModule::validate(const operation::Operation& op, const KeyTypes& key_types) {
    const auto it = key_types.find(op.key);
    const bool exists = it != key_types.end();
    const types::Type current = exists ? it->second : types::Type::OTHER;

    switch(op.type) {
        case operation::OperationType::GET:
        case operation::OperationType::SET:
        case operation::OperationType::DELETE:
            return;
        case operation::OperationType::ISMEMBER:
            if(!exists) return;
            requireType(current, {types::Type::SET, types::Type::HASH},
                        "Operation requires a set or map");
            return;
        case operation::OperationType::INC:
        case operation::OperationType::DEC:
            if(exists) requireType(current, {types::Type::INT},
                                   "Operation requires a number");
            return;
        case operation::OperationType::LPUSH:
        case operation::OperationType::RPUSH:
            if(exists) requireType(current, {types::Type::LIST},
                                   "Operation requires a list");
            return;
        case operation::OperationType::SADD:
            if(exists) requireType(current, {types::Type::SET},
                                   "Operation requires a set");
            return;
        case operation::OperationType::HSET:
            if(exists) requireType(current, {types::Type::HASH},
                                   "Operation requires a map");
            return;
        case operation::OperationType::LPOP:
        case operation::OperationType::RPOP:
            if(!exists) throw SemanticError(2, "Entity does not exist: " + op.key);
            requireType(current, {types::Type::LIST}, "Operation requires a list");
            return;
        case operation::OperationType::SREM:
            if(!exists) throw SemanticError(2, "Entity does not exist: " + op.key);
            requireType(current, {types::Type::SET}, "Operation requires a set");
            return;
        case operation::OperationType::GETITEM:
            if(!exists) throw SemanticError(2, "Entity does not exist: " + op.key);
            requireType(current, {types::Type::LIST, types::Type::HASH},
                        "Operation requires a list or map");
            return;
        case operation::OperationType::HDEL:
            if(!exists) throw SemanticError(2, "Entity does not exist: " + op.key);
            requireType(current, {types::Type::HASH}, "Operation requires a map");
            return;
        case operation::OperationType::SIZE:
            if(!exists) throw SemanticError(2, "Entity does not exist: " + op.key);
            requireType(current, {types::Type::STRING, types::Type::LIST,
                                  types::Type::SET, types::Type::HASH},
                        "Operation requires a list, set, map, or string");
            return;
    }
}

} // namespace semantic
