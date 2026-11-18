#pragma once

#include <models/operation.h>
#include <models/types.h>

#include <memory>
#include <string>
#include <trantor/utils/Logger.h>

namespace cache {

class Cache {
 public:
  Cache(types::Type type = types::Type::STRING) {
    _obj = makeValue(type);
  }

  Cache(const Cache& other)
      : _obj(other._obj ? other._obj->clone() : nullptr) {}

  Cache& operator=(const Cache& other) {
    if (this != &other) {
      _obj = other._obj ? other._obj->clone() : nullptr;
    }
    return *this;
  }

  Cache(Cache&&) noexcept = default;
  Cache& operator=(Cache&&) noexcept = default;

  types::Type type() const {
    return _obj ? _obj->type : types::Type::STRING;
  }

  operation::OperationResult apply(const operation::Operation& op) {
    switch (_obj->type)
    {
    case types::Type::STRING:
      return applyString(op);
    case types::Type::INT:
      return applyInt(op);
    case types::Type::LIST:
      return applyList(op);
    case types::Type::SET:
      return applySet(op);
    case types::Type::HASH:
      return applyHash(op);
    default:
      return {};
    }
  }

 private:
  static std::unique_ptr<types::Cacheable> makeValue(types::Type type) {
    switch (type)
    {
    case types::Type::STRING:
      return std::make_unique<types::String>("");
    case types::Type::INT:
      return std::make_unique<types::Int>(0);
    case types::Type::LIST:
      return std::make_unique<types::List>();
    case types::Type::SET:
      return std::make_unique<types::Set>();
    case types::Type::HASH:
      return std::make_unique<types::Hash>();
    default:
      return std::make_unique<types::String>("");
    }
  }

  operation::OperationResult applyString(const operation::Operation& op) {
    operation::OperationResult res;

    auto* str = static_cast<types::String*>(_obj.get());

    switch (op.type)
    {
    case operation::OperationType::GET:
      res.message = str->to_json();
      break;
    case operation::OperationType::SET:
      if(op.value.back().second == types::Type::INT) {
        _obj = std::make_unique<types::Int>(stoll(op.value.back().first));
        break;
      }
      str->set(op.value.back().first);
      break;
    case operation::OperationType::LPUSH:
    case operation::OperationType::RPUSH:
    case operation::OperationType::LPOP:
    case operation::OperationType::RPOP:
      _obj = std::make_unique<types::List>(str->get());
      res = applyList(op);
      break;
    case operation::OperationType::SADD:
      _obj = std::make_unique<types::Set>(str->get());
      res = applySet(op);
      break;
    case operation::OperationType::HSET:
      _obj = std::make_unique<types::Hash>();
      res = applyHash(op);
      break;
    default:
      break;
    }
    return res;
  }

  operation::OperationResult applyInt(const operation::Operation& op) {
    operation::OperationResult res;

    auto* num = static_cast<types::Int*>(_obj.get());

    switch (op.type)
    {
    case operation::OperationType::GET:
      res.message = num->to_json();
      break;
    case operation::OperationType::SET:
      num->set(stoll(op.value.back().first));
      break;
    case operation::OperationType::INC:
      for(auto value: op.value) res.message = num->inc(stoll(value.first));
      break;
    case operation::OperationType::DEC:
      for(auto value: op.value) res.message = num->dec(stoll(value.first));
      break;
    default:
      break;
    }

    return res;
  }

  operation::OperationResult applyList(const operation::Operation& op) {
    operation::OperationResult res;

    auto* list = static_cast<types::List*>(_obj.get());

    switch (op.type)
    {
    case operation::OperationType::GET:
      res.message = list->to_json();
      break;
    case operation::OperationType::LPUSH:
      for(auto value : op.value) res.message["size"] = list->lpush(value.first);
      break;
    case operation::OperationType::RPUSH:
      for(auto value : op.value) res.message["size"] = list->rpush(value.first);
      break;
    case operation::OperationType::LPOP:
      res.message["value"] = list->lpop();
      break;
    case operation::OperationType::RPOP:
      res.message["value"] = list->rpop();
      break;
    default:
      break;
    }
    return res;
  }

  operation::OperationResult applySet(const operation::Operation& op) {
    operation::OperationResult res;

    auto* set = static_cast<types::Set*>(_obj.get());

    switch (op.type)
    {
    case operation::OperationType::GET:
      res.message = set->to_json();
      break;
    case operation::OperationType::SADD:
      for(auto value : op.value) res.message["size"] = set->add(value.first);
      break;
    case operation::OperationType::SREM:
      for(auto value : op.value) res.message["size"] = set->rem(value.first);
      break;
    case operation::OperationType::ISMEMBER:
      res.message["is_member"] = set->isMember(op.value.front().first);
      break;
    default:
      break;
    }
    return res;
  }

  operation::OperationResult applyHash(const operation::Operation& op) {
    operation::OperationResult res;

    auto* hash = static_cast<types::Hash*>(_obj.get());
    std::vector<std::pair<std::string, types::Type>> v;

    switch (op.type)
    {
    case operation::OperationType::GET:
      res.message = hash->to_json();
      break;
    case operation::OperationType::GETITEM:
      for(auto key: op.value) res.message[key.first] = hash->get(key.first);
      break;
    case operation::OperationType::HSET:
      for(auto e: op.value) v.push_back(e);
      res.message["size"] = hash->set(v);
      break;
    case operation::OperationType::HDEL:
      for(auto key: op.value) res.message[key.first] = hash->del(key.first);
      break;
    default:
      break;
    }
    return res;
  }

  std::unique_ptr<types::Cacheable> _obj;
};

};