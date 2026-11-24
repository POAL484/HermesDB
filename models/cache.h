#pragma once

#include <models/operation.h>

#include <jsoncpp/json/json.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

namespace cache {

class Cache {
 public:
  explicit Cache(types::Type type = types::Type::OTHER) : _type(type) {
    switch(type) {
      case types::Type::STRING: _value = ""; break;
      case types::Type::INT: _value = Json::Int64(0); break;
      case types::Type::LIST:
      case types::Type::SET: _value = Json::Value(Json::arrayValue); break;
      case types::Type::HASH: _value = Json::Value(Json::objectValue); break;
      case types::Type::OTHER: _value = Json::Value(); break;
    }
  }

  types::Type type() const { return _type; }

  operation::OperationResult apply(const operation::Operation& op) {
    operation::OperationResult result;
    switch(op.type) {
      case operation::OperationType::GET:
        result.message["value"] = jsonValue();
        break;
      case operation::OperationType::SET:
        if(op.value.empty()) throw operation::Error(-3, "Field 'value' is required");
        set(op.value.back().data);
        result.message["OK"] = "OK!";
        break;
      case operation::OperationType::INC:
      case operation::OperationType::DEC:
        result.message["value"] = changeNumber(op);
        break;
      case operation::OperationType::LPUSH:
      case operation::OperationType::RPUSH:
        result.message["size"] = push(op);
        break;
      case operation::OperationType::LPOP:
      case operation::OperationType::RPOP:
        result.message["value"] = pop(op.type == operation::OperationType::LPOP);
        break;
      case operation::OperationType::SADD:
        result.message["size"] = addToSet(op);
        break;
      case operation::OperationType::SREM: {
        const auto removed = removeFromSet(op.value.front().data);
        result.message["value"] = removed.first ? removed.second : Json::Value();
        result.message["size"] = _value.size();
        break;
      }
      case operation::OperationType::ISMEMBER:
        result.message["isMember"] = isMember(op.value.front().data);
        break;
      case operation::OperationType::SIZE:
        result.message["size"] = size();
        break;
      case operation::OperationType::GETITEM:
        result.message["value"] = getItems(op);
        break;
      case operation::OperationType::HSET:
        result.message["size"] = setHash(op);
        break;
      case operation::OperationType::HDEL:
        result.message["deleted"] = deleteHashItems(op);
        break;
      default:
        break;
    }
    return result;
  }

 private:
  static types::Type typeOf(const Json::Value& value) {
    if(value.isString()) return types::Type::STRING;
    if(value.isIntegral() || value.isDouble()) return types::Type::INT;
    if(value.isArray()) return types::Type::LIST;
    if(value.isObject()) return types::Type::HASH;
    return types::Type::OTHER;
  }

  void set(const Json::Value& value) {
    if(value.isObject() && value.isMember("$set")) {
      if(!value["$set"].isArray()) {
        throw operation::Error(-1, "The '$set' value must be an array");
      }
      _value = value["$set"];
      _type = types::Type::SET;
    } else {
      _value = value;
      _type = typeOf(value);
    }
  }

  Json::Value jsonValue() const {
    if(_type == types::Type::SET) {
      Json::Value wrapped(Json::objectValue);
      wrapped["$set"] = _value;
      return wrapped;
    }
    return _value;
  }

  Json::Value changeNumber(const operation::Operation& op) {
    if(_type == types::Type::OTHER && _value.isNull()) {
      _value = Json::Int64(0);
      _type = types::Type::INT;
    }
    if(_type != types::Type::INT || !_value.isNumeric()) {
      throw operation::Error(1, "Operation requires a number");
    }
    const Json::Value& increment = op.value.front().data;
    if(_value.isIntegral() && increment.isIntegral()) {
      const Json::Int64 current = _value.asInt64();
      const Json::Int64 delta = increment.asInt64();
      constexpr Json::Int64 maximum = std::numeric_limits<Json::Int64>::max();
      constexpr Json::Int64 minimum = std::numeric_limits<Json::Int64>::min();
      Json::Int64 updated = 0;
      if(op.type == operation::OperationType::INC) {
        if((delta > 0 && current > maximum - delta) ||
           (delta < 0 && current < minimum - delta)) {
          throw operation::Error(1, "Numeric result is out of range");
        }
        updated = current + delta;
      } else {
        if((delta < 0 && current > maximum + delta) ||
           (delta > 0 && current < minimum + delta)) {
          throw operation::Error(1, "Numeric result is out of range");
        }
        updated = current - delta;
      }
      _value = updated;
    } else {
      const double current = _value.asDouble();
      const double delta = increment.asDouble();
      const double updated = op.type == operation::OperationType::INC
                                 ? current + delta
                                 : current - delta;
      if(!std::isfinite(updated)) throw operation::Error(1, "Numeric result is out of range");
      _value = updated;
    }
    return _value;
  }

  Json::Value::UInt64 push(const operation::Operation& op) {
    if(_type == types::Type::OTHER && _value.isNull()) {
      _value = Json::Value(Json::arrayValue);
      _type = types::Type::LIST;
    }
    if(_type != types::Type::LIST) {
      throw operation::Error(1, "Operation requires a list");
    }
    for(const auto& item : op.value) {
      if(op.type == operation::OperationType::LPUSH) _value.insert(0, item.data);
      else _value.append(item.data);
    }
    return _value.size();
  }

  Json::Value pop(bool fromFront) {
    if(_type != types::Type::LIST) {
      throw operation::Error(1, "Operation requires a list");
    }
    if(_value.empty()) throw operation::Error(3, "Cannot pop from an empty list");
    const Json::Value item = _value[fromFront ? 0 : _value.size() - 1];
    Json::Value ignored;
    if(fromFront) _value.removeIndex(0, &ignored);
    else _value.removeIndex(_value.size() - 1, &ignored);
    return item;
  }

  Json::Value::UInt64 addToSet(const operation::Operation& op) {
    if(_type == types::Type::OTHER && _value.isNull()) {
      _value = Json::Value(Json::arrayValue);
      _type = types::Type::SET;
    }
    if(_type != types::Type::SET) throw operation::Error(1, "Operation requires a set");
    for(const auto& item : op.value) {
      const bool exists = std::any_of(_value.begin(), _value.end(),
                                      [&item](const Json::Value& current) {
                                        return current == item.data;
                                      });
      if(!exists) _value.append(item.data);
    }
    return _value.size();
  }

  std::pair<bool, Json::Value> removeFromSet(const Json::Value& item) {
    if(_type != types::Type::SET) throw operation::Error(1, "Operation requires a set");
    if(_value.empty()) throw operation::Error(3, "Cannot remove from an empty set");
    for(Json::ArrayIndex i = 0; i < _value.size(); ++i) {
      if(_value[i] == item) {
        const Json::Value removed = _value[i];
        Json::Value ignored;
        _value.removeIndex(i, &ignored);
        return {true, removed};
      }
    }
    return {false, Json::Value()};
  }

  bool isMember(const Json::Value& item) const {
    if(_type == types::Type::SET) {
      return std::any_of(_value.begin(), _value.end(),
                         [&item](const Json::Value& current) { return current == item; });
    }
    if(_type == types::Type::HASH) {
      return item.isString() && _value.isMember(item.asString());
    }
    throw operation::Error(1, "Operation requires a set or map");
  }

  Json::Value::UInt64 size() const {
    if(_type == types::Type::STRING) return _value.asString().size();
    if(_type == types::Type::LIST || _type == types::Type::SET ||
       _type == types::Type::HASH) return _value.size();
    throw operation::Error(1, "Operation requires a list, set, map, or string");
  }

  Json::Value getItems(const operation::Operation& op) const {
    if(_type == types::Type::LIST) {
      Json::Value values(Json::arrayValue);
      for(const auto& item : op.value) {
        if(!item.data.isIntegral() || item.data.asInt64() < 0 ||
           static_cast<Json::UInt64>(item.data.asInt64()) >= _value.size()) {
          throw operation::Error(4, "List index is out of bounds");
        }
        values.append(_value[static_cast<Json::ArrayIndex>(item.data.asUInt64())]);
      }
      return values;
    }
    if(_type == types::Type::HASH) {
      Json::Value values(Json::objectValue);
      for(const auto& item : op.value) {
        if(!item.data.isString()) throw operation::Error(-1, "Map keys must be strings");
        const std::string key = item.data.asString();
        values[key] = _value.isMember(key) ? _value[key] : Json::Value();
      }
      return values;
    }
    throw operation::Error(1, "Operation requires a list or map");
  }

  Json::Value::UInt64 setHash(const operation::Operation& op) {
    if(_type == types::Type::OTHER && _value.isNull()) {
      _value = Json::Value(Json::objectValue);
      _type = types::Type::HASH;
    }
    if(_type != types::Type::HASH) throw operation::Error(1, "Operation requires a map");
    for(size_t i = 0; i + 1 < op.value.size(); i += 2) {
      _value[op.value[i].first] = op.value[i + 1].data;
    }
    return _value.size();
  }

  Json::Value deleteHashItems(const operation::Operation& op) {
    if(_type != types::Type::HASH) throw operation::Error(1, "Operation requires a map");
    Json::Value deleted(Json::objectValue);
    for(const auto& item : op.value) {
      if(!item.data.isString()) throw operation::Error(-1, "Map keys must be strings");
      const std::string key = item.data.asString();
      if(_value.isMember(key)) {
          deleted[key] = _value[key];
          _value.removeMember(key);
      }
    }
    return deleted;
  }

  Json::Value _value;
  types::Type _type;
};

} // namespace cache
