#pragma once

#include <models/operation.h>
#include <models/types.h>

#include <string>
#include <trantor/utils/Logger.h>

namespace cache {

class Cache {
 public:
  Cache(types::Type type = types::Type::STRING) {
    switch (type)
    {
    case types::Type::STRING:
      _obj = new types::String("");
      break;
    case types::Type::INT:
      _obj = new types::Int(0);
      break;
    case types::Type::LIST:
      _obj = new types::List();
      break;
    case types::Type::SET:
      _obj = new types::Set();
      break;
    case types::Type::HASH:
      _obj = new types::Hash();
      break;
    default:
      break;
    }
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
      break;
    }
  }

 private:

  operation::OperationResult applyString(const operation::Operation& op) {
    operation::OperationResult res;

    types::String* str = static_cast<types::String*>(_obj);

    switch (op.type)
    {
    case operation::OperationType::GET:
      res.message = str->to_json();
      break;
    case operation::OperationType::SET:
      if(op.value.back().second == types::Type::INT) {
        _obj = new types::Int(stoll(op.value.back().first));
        break;
      }
      str->set(op.value.back().first);
      break;
    case operation::OperationType::LPUSH:
    case operation::OperationType::RPUSH:
    case operation::OperationType::LPOP:
    case operation::OperationType::RPOP:
      _obj = new types::List(str->get()); 
      res = applyList(op);
      break;
    case operation::OperationType::SADD:
      _obj = new types::Set(str->get());
      res = applySet(op);
      break;
    case operation::OperationType::HSET:
      _obj = new types::Hash();
      res = applyHash(op);
      break;
    default:
      break;
    }
    return res;
  }

  operation::OperationResult applyInt(const operation::Operation& op) {
    operation::OperationResult res;

    types::Int* num = static_cast<types::Int*>(_obj);
  
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

    types::List* list = static_cast<types::List*>(_obj);

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

    types::Set* set = static_cast<types::Set*>(_obj);

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

    types::Hash* hash = static_cast<types::Hash*>(_obj);
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

  types::Cacheable* _obj;
};


};