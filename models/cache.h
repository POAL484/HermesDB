#pragma once

#include <models/operation.h>
#include <models/types.h>

#include <string>
#include <trantor/utils/Logger.h>

namespace cache {

class Cache {
 public:
  Cache() {
    _obj = new types::String("");
  }

  operation::OperationResult apply(const operation::Operation& op) {
    switch (_obj->type)
    {
    case types::Type::STRING:
      return applyString(op);    
    case types::Type::LIST:
      return applyList(op);
    default:
      break;
    }
  }

 private:

  operation::OperationResult applyString(const operation::Operation& op) {
    operation::OperationResult res;

    LOG_DEBUG << "Here";

    types::String* str = reinterpret_cast<types::String*>(_obj);

    switch (op.type)
    {
    case operation::OperationType::GET:
      res.message = str->get();
      break;
    case operation::OperationType::SET:
      str->set(op.value);
      res.message = "OK!";
      break;
    case operation::OperationType::LPUSH:
    case operation::OperationType::RPUSH:
    case operation::OperationType::LPOP:
    case operation::OperationType::RPOP:
      _obj = new types::List(str->get()); 
      res = applyList(op);
      break;
    default:
      break;
    }
    return res;
  }

  operation::OperationResult applyList(const operation::Operation& op) {
    operation::OperationResult res;

    types::List* list = reinterpret_cast<types::List*>(_obj);

    switch (op.type)
    {
    case operation::OperationType::LPUSH:
      res.message = std::to_string(list->lpush(op.value));
      break;
    case operation::OperationType::RPUSH:
      res.message = std::to_string(list->rpush(op.value));
      break;
    case operation::OperationType::LPOP:
      res.message = list->lpop();
      break;
    case operation::OperationType::RPOP:
      res.message = list->rpop();
      break;
    default:
      break;
    }
    return res;
  }

  types::Cacheable* _obj;
};


};