#include "CacheController.h"

#include <drogon/HttpResponse.h>
#include <jsoncpp/json/json.h>
#include <query/syntaxmodule.h>
#include <query/semanticmodule.h>

using namespace drogon;

void CacheController::applyOperation(const HttpRequestPtr& req, 
    std::function<void(const HttpResponsePtr&)>&& callback, std::string&& uuid) 
{   
    if(!_service->haveRepo(uuid)) return callback(makeUnknownUserResponse("Unknown user uuid"));
    
    auto json = req->getJsonObject();
    if(!json) {
        return callback(makeErrorResponse(k400BadRequest, -1, "Request body must be valid JSON."));
    }

    operation::Operation operation;
    try {
        operation = syntax::SyntaxModule::parse(*json);
    } catch(const syntax::SyntaxError& error) {
        return callback(makeErrorResponse(k400BadRequest, error.errorCode(), error.what()));
    }

    semantic::SemanticModule::KeyTypes key_types;
    if(_service->haveKey(uuid, operation.key)) {
        key_types[operation.key] = _service->keyType(uuid, operation.key);
    }
    try {
        semantic::SemanticModule::validate(operation, key_types);
    } catch(const semantic::SemanticError& error) {
        return callback(makeErrorResponse(k400BadRequest, error.errorCode(), error.what()));
    }

    operation::OperationResult res;
    try {
        res = _service->apply(uuid, operation);
    } catch(const operation::Error& error) {
        return callback(makeErrorResponse(k400BadRequest, error.errorCode(), error.what()));
    }

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
  
HttpResponsePtr CacheController::makeUnknownUserResponse(const std::string& msg="") 
{
    Json::Value json;
    json["status"] = 403;
    json["errCode"] = 0;
    json["err"] = msg;
    
    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(k403Forbidden);
    
    return resp;
}

HttpResponsePtr CacheController::makeErrorResponse(int status, int errorCode,
                                                    const std::string& message)
{
    Json::Value json;
    json["status"] = status;
    json["errCode"] = errorCode;
    json["err"] = message;
    
    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(static_cast<HttpStatusCode>(status));
    
    return resp;
}
