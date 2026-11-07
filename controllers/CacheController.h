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
    
    HttpResponsePtr makeUnknownUserResponse();
};  
