#pragma once

#include <drogon/HttpController.h>

#include <models/cacheservice.h>

using namespace drogon;

class CacheController : public HttpController<CacheController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(CacheController::applyOperation, "/?uuid={1}", Post, Put);
    
    ADD_METHOD_TO(CacheController::registerUser, "/new", Get);
    METHOD_LIST_END 

  protected:
    void applyOperation(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&&, std::string&& uuid);
    void registerUser(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&&);
    
  public:
    CacheController() {
      _service = new cache::CacheService();
    }
    ~CacheController() {
      delete _service;
    }

  private:
    cache::CacheService* _service;
    std::unordered_map<std::string, operation::OperationType> _mapper = {
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
    
    HttpResponsePtr makeUnknownUserResponse(const std::string&);
    HttpResponsePtr makeBadRequestResponse(const std::string&);
};  
