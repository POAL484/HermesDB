#include <query/syntaxmodule.h>

#include <limits>

namespace syntax {

static operation::OperationType parseOperation(const std::string& name) {
    static const std::unordered_map<std::string, operation::OperationType> operations = {
        {"GET", operation::OperationType::GET},
        {"SET", operation::OperationType::SET},
        {"DELETE", operation::OperationType::DELETE},
        {"LPUSH", operation::OperationType::LPUSH},
        {"RPUSH", operation::OperationType::RPUSH},
        {"LPOP", operation::OperationType::LPOP},
        {"RPOP", operation::OperationType::RPOP},
        {"SADD", operation::OperationType::SADD},
        {"SREM", operation::OperationType::SREM},
        {"ISMEMBER", operation::OperationType::ISMEMBER},
        {"GETITEM", operation::OperationType::GETITEM},
        {"HSET", operation::OperationType::HSET},
        {"HDEL", operation::OperationType::HDEL},
        {"INC", operation::OperationType::INC},
        {"DEC", operation::OperationType::DEC},
    };

    const auto it = operations.find(name);
    if(it == operations.end()) {
        throw SyntaxError("Unknown operation: " + name);
    }
    return it->second;
}

static types::Type jsonValueType(const Json::Value& value) {
    if(value.isIntegral()) return types::Type::INT;
    if(value.isString()) return types::Type::STRING;
    throw SyntaxError("Values must contain only JSON strings or integers");
}

operation::Operation SyntaxModule::parse(const Json::Value& request) {
    if(!request.isObject()) {
        throw SyntaxError("Request must be a JSON object");
    }
    if(!request.isMember("op") || !request["op"].isString()) {
        throw SyntaxError("Field 'op' must be a string");
    }
    if(!request.isMember("key") || !request["key"].isString()) {
        throw SyntaxError("Field 'key' must be a string");
    }
    if(request["key"].asString().empty()) {
        throw SyntaxError("Field 'key' must not be empty");
    }

    const operation::OperationType type = parseOperation(request["op"].asString());
    if(!request.isMember("value")) {
        throw SyntaxError("Field 'value' must be present");
    }

    const Json::Value& input = request["value"];
    std::vector<std::pair<std::string, types::Type>> values;

    if(input.isObject()) {
        if(type != operation::OperationType::HSET) {
            throw SyntaxError("Object value is supported only for HSET");
        }
        for(const auto& name : input.getMemberNames()) {
            const Json::Value& value = input[name];
            values.emplace_back(name, types::Type::STRING);
            values.emplace_back(value.asString(), jsonValueType(value));
        }
    } else if(input.isArray()) {
        if(type == operation::OperationType::HSET && input.size() % 2 != 0) {
            throw SyntaxError("HSET array must contain field/value pairs");
        }
        for(Json::ArrayIndex i = 0; i < input.size(); ++i) {
            const Json::Value& value = input[i];
            if(type == operation::OperationType::HSET) {
                if(i % 2 == 0) {
                    if(!value.isString()) throw SyntaxError("HSET field names must be strings");
                    values.emplace_back(value.asString(), types::Type::STRING);
                } else {
                    values.emplace_back(value.asString(), jsonValueType(value));
                }
            } else {
                values.emplace_back(value.asString(), jsonValueType(value));
            }
        }
    } else {
        throw SyntaxError("Field 'value' must be a JSON object or array");
    }

    switch(type) {
        case operation::OperationType::GET:
        case operation::OperationType::DELETE:
        case operation::OperationType::LPOP:
        case operation::OperationType::RPOP:
            if(!values.empty()) throw SyntaxError("This operation does not accept value elements");
            break;
        case operation::OperationType::SET:
            if(values.size() != 1) throw SyntaxError("SET requires exactly one value");
            break;
        case operation::OperationType::INC:
        case operation::OperationType::DEC:
            if(values.empty()) throw SyntaxError("This operation requires at least one integer");
            for(const auto& value : values) {
                if(value.second != types::Type::INT) {
                    throw SyntaxError("INC and DEC accept only integers");
                }
            }
            break;
        case operation::OperationType::ISMEMBER:
        case operation::OperationType::GETITEM:
        case operation::OperationType::HDEL:
            if(values.size() != 1) throw SyntaxError("This operation requires exactly one value");
            break;
        case operation::OperationType::LPUSH:
        case operation::OperationType::RPUSH:
        case operation::OperationType::SADD:
        case operation::OperationType::SREM:
            if(values.empty()) throw SyntaxError("This operation requires at least one value");
            break;
        case operation::OperationType::HSET:
            if(values.empty()) throw SyntaxError("HSET requires at least one field/value pair");
            break;
    }

    return {type, request["key"].asString(), std::move(values)};
}

bool SyntaxModule::validate(const Json::Value& request) {
    try {
        parse(request);
        return true;
    } catch(const SyntaxError&) {
        return false;
    }
}

} // namespace syntax
