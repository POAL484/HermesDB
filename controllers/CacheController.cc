#include "CacheController.h"

#include <models/repository.h>

#include <drogon/HttpResponse.h>
#include <jsoncpp/json/json.h>

using namespace drogon;

void CacheController::getValue(const HttpRequestPtr& req, 
    std::function<void (const HttpResponsePtr &)> &&callback, std::string&& uuid)
{
    if(!_service->haveRepo(uuid)) return callback(makeUnknownUserResponse());

    auto jsonPtr = req->jsonObject();  

    const std::string key = jsonPtr->get("key", "").asString();
    
    auto* loop = drogon::app().getLoop();

    loop->runInLoop([this, uuid = std::move(uuid), key, callback = std::move(callback)](){
        std::string value; Json::Value json;
        json["value"] = _service->get(uuid, key, value) ? value : Json::Value::null;

        auto resp = HttpResponse::newHttpJsonResponse(json);
        callback(resp);
    });
}

void CacheController::setValue(const HttpRequestPtr& req, 
    std::function<void(const HttpResponsePtr&)>&& callback, std::string&& uuid) 
{
    if(!_service->haveRepo(uuid)) return callback(makeUnknownUserResponse());
    
    auto jsonPtr = req->jsonObject();  

    const std::string key = jsonPtr->get("key", "").asString();
    const std::string value = jsonPtr->get("value", "").asString();
    
    auto* loop = drogon::app().getLoop();

    loop->runInLoop([this, uuid = std::move(uuid), key, value, callback=std::move(callback)](){
        _service->set(uuid, key, value);

        auto resp = HttpResponse::newHttpResponse();    
        resp->setStatusCode(k200OK);
        callback(resp);
    });
}

void CacheController::deleteKey(const HttpRequestPtr& req, 
    std::function<void(const HttpResponsePtr&)>&& callback, std::string&& uuid) 
{ 
    if(!_service->haveRepo(uuid)) return callback(makeUnknownUserResponse());
    
    auto jsonPtr = req->jsonObject();  
    const std::string key = jsonPtr->get("key", "").asString();

    auto* loop = drogon::app().getLoop();

    loop->runInLoop([this, uuid = std::move(uuid), key, callback = std::move(callback)](){
        _service->deleteKey(uuid, key);
        auto resp = HttpResponse::newHttpResponse();    
        resp->setStatusCode(k200OK);
        callback(resp);
    });
}

void CacheController::registerUser(const HttpRequestPtr& req, 
    std::function<void(const HttpResponsePtr&)>&& callback) 
{
    const std::string uuid = _service->registerRepo<repository::LruRepository>();

    Json::Value json;
    json["uuid"] = uuid;

    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(k200OK);
    
    callback(resp);
}
  
HttpResponsePtr CacheController::makeUnknownUserResponse() 
{
    Json::Value json;
    json["error"] = "Unknown user uuid";    
    
    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(k403Forbidden);
    
    return resp;
}