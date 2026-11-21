#include <query/syntaxmodule.h>

#include <unordered_map>
#include <utility>

namespace syntax {
namespace {

operation::OperationType parseOperation(const std::string& name) {
    static const std::unordered_map<std::string, operation::OperationType> operations = {
        {"GET", operation::OperationType::GET},
        {"SET", operation::OperationType::SET},
        {"DEL", operation::OperationType::DELETE},
        {"DELETE", operation::OperationType::DELETE},
        {"GETITEM", operation::OperationType::GETITEM},
        {"ISMEMBER", operation::OperationType::ISMEMBER},
        {"SIZE", operation::OperationType::SIZE},
        {"INC", operation::OperationType::INC},
        {"DEC", operation::OperationType::DEC},
        {"LPUSH", operation::OperationType::LPUSH},
        {"RPUSH", operation::OperationType::RPUSH},
        {"LPOP", operation::OperationType::LPOP},
        {"RPOP", operation::OperationType::RPOP},
        {"SADD", operation::OperationType::SADD},
        {"SREM", operation::OperationType::SREM},
        {"HSET", operation::OperationType::HSET},
        {"HDEL", operation::OperationType::HDEL},
    };

    const auto it = operations.find(name);
    if(it == operations.end()) throw SyntaxError(-2, "Unknown operation: " + name);
    return it->second;
}

void requireMember(const Json::Value& request, const char* name) {
    if(!request.isMember(name) || request[name].isNull()) {
        throw SyntaxError(-3, std::string("Required field '") + name + "' is missing");
    }
}

operation::Value makeValue(const Json::Value& value) {
    return operation::Value(value);
}

} // namespace

operation::Operation SyntaxModule::parse(const Json::Value& request) {
    if(!request.isObject()) throw SyntaxError(-1, "Request must be a JSON object");
    requireMember(request, "op");
    if(!request["op"].isString()) throw SyntaxError(-1, "Field 'op' must be a string");
    requireMember(request, "key");
    if(!request["key"].isString()) throw SyntaxError(-1, "Field 'key' must be a string");

    const operation::OperationType type = parseOperation(request["op"].asString());
    bool needsValue = false;
    const char* valueName = "value";
    switch(type) {
        case operation::OperationType::SET:
        case operation::OperationType::INC:
        case operation::OperationType::DEC:
        case operation::OperationType::LPUSH:
        case operation::OperationType::RPUSH:
        case operation::OperationType::SADD:
        case operation::OperationType::SREM:
        case operation::OperationType::ISMEMBER:
        case operation::OperationType::HSET:
            needsValue = true;
            break;
        case operation::OperationType::GETITEM:
            needsValue = true;
            valueName = "item";
            break;
        case operation::OperationType::HDEL:
            needsValue = true;
            valueName = "mapKeys";
            break;
        default:
            break;
    }
    if(needsValue) requireMember(request, valueName);

    std::int64_t ttl = 0;
    if(request.isMember("ttl")) {
        if(!request["ttl"].isIntegral() || request["ttl"].asInt64() < 0) {
            throw SyntaxError(-1, "Field 'ttl' must be a non-negative integer");
        }
        ttl = request["ttl"].asInt64();
    }

    std::vector<operation::Value> values;
    if(type == operation::OperationType::SET) {
        const Json::Value& value = request["value"];
        if(value.isObject() && value.isMember("$set") && !value["$set"].isArray()) {
            throw SyntaxError(-1, "The '$set' value must be an array");
        }
        values.push_back(makeValue(value));
    } else if(type == operation::OperationType::HSET) {
        if(!request["value"].isObject()) throw SyntaxError(-1, "HSET value must be a map");
        for(const auto& key : request["value"].getMemberNames()) {
            operation::Value field{Json::Value(key)};
            field.first = key;
            values.push_back(std::move(field));
            values.push_back(makeValue(request["value"][key]));
        }
    } else if(type == operation::OperationType::GETITEM ||
              type == operation::OperationType::HDEL) {
        const Json::Value& items = request[valueName];
        if(!items.isArray()) throw SyntaxError(-1, std::string("Field '") + valueName + "' must be an array");
        for(const auto& item : items) {
            if(type == operation::OperationType::HDEL && !item.isString()) {
                throw SyntaxError(-1, "mapKeys must contain only strings");
            }
            if(type == operation::OperationType::GETITEM &&
               !(item.isString() || item.isIntegral())) {
                throw SyntaxError(-1, "item must contain string keys or integer indices");
            }
            values.push_back(makeValue(item));
        }
    } else if(needsValue) {
        const Json::Value& value = request["value"];
        if((type == operation::OperationType::INC || type == operation::OperationType::DEC) &&
           !value.isNumeric()) {
            throw SyntaxError(-1, "INC and DEC require a numeric value");
        }
        values.push_back(makeValue(value));
    }

    return {type, request["key"].asString(), std::move(values), ttl};
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
