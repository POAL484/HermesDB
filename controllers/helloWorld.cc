#include "helloWorld.h"

void helloWorld::asyncHandleHttpRequest(const HttpRequestPtr& req, 
    std::function<void (const HttpResponsePtr &)> &&callback)
{
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k200OK);
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody("Hello world!");

    callback(resp);
}
