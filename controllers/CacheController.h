#pragma once

#include <drogon/HttpController.h>

#include <models/cacheservice.h>

using namespace drogon;

class CacheController : public HttpController<CacheController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(CacheController::getValue, "/", Get);
    ADD_METHOD_TO(CacheController::setValue, "/", Post);
    ADD_METHOD_TO(CacheController::deleteKey, "/", Delete);    
    METHOD_LIST_END 

  void getValue(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&&);  
  void setValue(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&&);
  void deleteKey(const HttpRequestPtr&, std::function<void(const HttpResponsePtr&)>&&);
  
  public:
    CacheController() {
      _service = new cache::CacheService();
    }
    ~CacheController() {
      delete _service;
    }
  private:
    cache::CacheService* _service;
};  
