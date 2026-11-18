#include <query/semanticmodule.h>

namespace semantic {

types::Type SemanticModule::typeFor(const operation::Operation& operation,
                                    const KeyTypes& key_types) {
    const auto it = key_types.find(operation.key);
    return it == key_types.end() ? types::Type::STRING : it->second;
}

void SemanticModule::validate(const operation::Operation& operation,
                              const KeyTypes& key_types) {
    const auto it = key_types.find(operation.key);
    const bool exists = it != key_types.end();
    const types::Type current = exists ? it->second : types::Type::STRING;

    if(operation.type == operation::OperationType::SET ||
       operation.type == operation::OperationType::DELETE) {
        return;
    }

    if(operation.type == operation::OperationType::GET) {
        return;
    }

    if(!exists) {
        switch(operation.type) {
            case operation::OperationType::LPUSH:
            case operation::OperationType::RPUSH:
            case operation::OperationType::SADD:
            case operation::OperationType::HSET:
                return;
            default:
                throw SemanticError("Entity does not exist: " + operation.key);
        }
    }

    switch(operation.type) {
        case operation::OperationType::INC:
        case operation::OperationType::DEC:
            if(current != types::Type::INT)
                throw SemanticError("Operation requires an integer entity");
            break;
        case operation::OperationType::LPUSH:
        case operation::OperationType::RPUSH:
        case operation::OperationType::LPOP:
        case operation::OperationType::RPOP:
            if(current != types::Type::LIST && current != types::Type::STRING)
                throw SemanticError("Operation requires a list entity");
            break;
        case operation::OperationType::SADD:
            if(current != types::Type::SET && current != types::Type::STRING)
                throw SemanticError("Operation requires a set entity");
            break;
        case operation::OperationType::SREM:
        case operation::OperationType::ISMEMBER:
            if(current != types::Type::SET)
                throw SemanticError("Operation requires a set entity");
            break;
        case operation::OperationType::GETITEM:
            if(current != types::Type::HASH)
                throw SemanticError("Operation requires a hash entity");
            break;
        case operation::OperationType::HSET:
            if(current != types::Type::HASH && current != types::Type::STRING)
                throw SemanticError("Operation requires a hash entity");
            break;
        case operation::OperationType::HDEL:
            if(current != types::Type::HASH)
                throw SemanticError("Operation requires a hash entity");
            break;
        default:
            break;
    }
}

} // namespace semantic
