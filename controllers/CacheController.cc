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
    if(!_service->haveRepo(uuid)) return callback(makeUnknownUserResponse());
    
    auto json = req->getJsonObject();
    operation::OperationType type = _mapper[json->get("op", "").asString()];
    const std::string key = json->operator[]("key").asString();
    const Json::Value& jvalues = json->operator[]("value");

    std::vector<std::pair<std::string, types::Type>> value;
    value.reserve(jvalues.size());
    for(int i = 0; i < jvalues.size(); ++i) {
        auto str = jvalues[i].asString();

        types::Type type = types::Type::STRING;
        if(jvalues[i].isNumeric()) type = types::Type::INT; 

        value.push_back({str, type});
    }

    operation::OperationResult res = _service->apply(uuid, {type, key, value});

    Json::Value json_response;
    json_response["status"] = 200;
    json_response["data"] = res.message;

    auto resp = HttpResponse::newHttpJsonResponse(json_response);
    resp->setStatusCode(k200OK);
    
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