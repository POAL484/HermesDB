#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

class helloWorld : public drogon::HttpSimpleController<helloWorld>
{
  public:
    void asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
    // list path definitions here;
    // PATH_ADD("/path", "filter1", "filter2", HttpMethod1, HttpMethod2...);
    PATH_ADD("/hello", Get);
    PATH_ADD("/", Get);
    PATH_LIST_END
};
