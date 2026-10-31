#pragma once

#include <drogon/HttpController.h>

#include <models/cacheservice.h>

using namespace drogon;

class CacheController : public HttpController<CacheController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(CacheController::getValue, "/?uuid={1}", Get);
    ADD_METHOD_TO(CacheController::setValue, "/?uuid={1}", Post, Put);
    ADD_METHOD_TO(CacheController::deleteKey, "/?uuid={1}", Delete);   
    
    ADD_METHOD_TO(CacheController::registerUser, "/new", Get);
    METHOD_LIST_END 

  protected:
    void getValue(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&&, std::string&& uuid);  
    void setValue(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&&, std::string&& uuid);
    void deleteKey(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&&, std::string&& uuid);
    
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
