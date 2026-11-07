#include "CacheController.h"

#include <models/repository.h>
#include <models/types.h>
#include <models/operation.h>

#include <drogon/HttpResponse.h>
#include <jsoncpp/json/json.h>

using namespace drogon;

void CacheController::applyOperation(const HttpRequestPtr& req, 
    std::function<void(const HttpResponsePtr&)>&& callback, std::string&& uuid) 
{   
    auto json = req->getJsonObject();
    operation::OperationType type = (operation::OperationType)json->get("op", Json::Value(0)).asInt();
    const std::string key = json->get("key", "").asString();
    const std::string value = json->get("value", "").asString();

    LOG_DEBUG << "Operation type is " << (int)type; 
    
    operation::OperationResult res = _service->apply(uuid, {type, key, value});

    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k200OK);
    resp->setBody(res.message);

    callback(resp);
}

void CacheController::registerUser(const HttpRequestPtr& req, 
    std::function<void(const HttpResponsePtr&)>&& callback) 
{
    const std::string uuid = _service->registerRepo();
 
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