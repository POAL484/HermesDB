#include "CacheController.h"

#include <models/repository.h>
#include <drogon/HttpResponse.h>
#include <jsoncpp/json/json.h>

using namespace drogon;

void CacheController::getValue(const HttpRequestPtr& req, 
    std::function<void (const HttpResponsePtr &)> &&callback)
{
    auto jsonPtr = req->jsonObject();  

    const std::string key = jsonPtr->get("key", "").asString();
    
    std::string value; Json::Value json;
    json["value"] = _service->get(key, value) ? value : Json::Value::null;

    auto resp = HttpResponse::newHttpJsonResponse(json);
    callback(resp);
}

void CacheController::setValue(const HttpRequestPtr& req, 
    std::function<void(const HttpResponsePtr&)>&& callback) 
{
    auto jsonPtr = req->jsonObject();  

    const std::string key = jsonPtr->get("key", "").asString();
    const std::string value = jsonPtr->get("value", "").asString();
    _service->set(key, value);

    auto resp = HttpResponse::newHttpResponse();    
    resp->setStatusCode(k200OK);
    callback(resp);
}

void CacheController::deleteKey(const HttpRequestPtr& req, 
    std::function<void(const HttpResponsePtr&)>&& callback) 
{ 
    auto jsonPtr = req->jsonObject();  

    const std::string key = jsonPtr->get("key", "").asString();
    _service->deleteKey(key);

    auto resp = HttpResponse::newHttpResponse();    
    resp->setStatusCode(k200OK);
    callback(resp);
}

