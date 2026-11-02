#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <jsoncpp/json/json.h>

using namespace drogon;


DROGON_TEST(GetUserUUID) 
{
    auto client = HttpClient::newHttpClient("http://localhost:80");
    auto req = HttpRequest::newHttpRequest();
    req->setPath("/new");
    req->setMethod(Get);

    client->sendRequest(req, [TEST_CTX](ReqResult res, const HttpResponsePtr& resp) {
        REQUIRE(resp != nullptr);

        CHECK(resp->getStatusCode() == k200OK);

        std::shared_ptr<Json::Value> json = resp->getJsonObject();
        std::string uuid = json->get("uuid", "").asString();

        CHECK(uuid != "");
    });
}

std::string getUuid() {
    auto client = HttpClient::newHttpClient("http://localhost:80");
    auto req = HttpRequest::newHttpRequest();
    req->setPath("/new");
    req->setMethod(Get);

    auto uuidPromise = std::make_shared<std::promise<std::string>>();
    auto uuidFuture = uuidPromise->get_future();
    client->sendRequest(req, [uuidPromise](ReqResult res, const HttpResponsePtr& resp) {
        if (!resp)
        {
            uuidPromise->set_value("");
            return;
        }
        std::shared_ptr<Json::Value> json = resp->getJsonObject();
        uuidPromise->set_value(json ? json->get("uuid", "").asString() : "");
    });

    return uuidFuture.get();
}


DROGON_TEST(CRUD) 
{
    std::string uuid = getUuid();
    REQUIRE(uuid != "");

    LOG_DEBUG << "Using UUID: " << uuid;

    auto client = HttpClient::newHttpClient("http://localhost:80");
    Json::Value postBody;
    postBody["key"] = "key";
    postBody["value"] = "value";
    
    auto postReq = HttpRequest::newHttpJsonRequest(postBody);
    postReq->setPath("/?uuid=" + uuid);
    postReq->setMethod(Post);

    client->sendRequest(postReq, [TEST_CTX](ReqResult res, const HttpResponsePtr& resp) {
        REQUIRE(resp != nullptr);

        CHECK(resp->getStatusCode() == k200OK);
    });

    Json::Value getBody;
    getBody["key"] = "key";

    auto getReq = HttpRequest::newHttpJsonRequest(getBody);
    getReq->setPath("/?uuid=" + uuid);
    getReq->setMethod(Get);

    client->sendRequest(getReq, [TEST_CTX](ReqResult res, const HttpResponsePtr& resp) {
        REQUIRE(resp != nullptr);

        CHECK(resp->getStatusCode() == k200OK);
        std::shared_ptr<Json::Value> json = resp->getJsonObject();
        CHECK(json->get("value", "") == "value");
    });

    Json::Value deleteBody;
    deleteBody["key"] = "key";

    auto deleteReq = HttpRequest::newHttpJsonRequest(deleteBody);
    deleteReq->setPath("/?uuid=" + uuid);
    deleteReq->setMethod(Delete);

    client->sendRequest(deleteReq, [TEST_CTX](ReqResult res, const HttpResponsePtr& resp) {
        REQUIRE(resp != nullptr);

        CHECK(resp->getStatusCode() == k200OK);
    });

    Json::Value checkBody;
    checkBody["key"] = "key";

    auto checkReq = HttpRequest::newHttpJsonRequest(checkBody);
    checkReq->setPath("/?uuid=" + uuid);
    checkReq->setMethod(Get);

    client->sendRequest(checkReq, [TEST_CTX](ReqResult res, const HttpResponsePtr& resp) {
        REQUIRE(resp != nullptr);

        CHECK(resp->getStatusCode() == k200OK);
        std::shared_ptr<Json::Value> json = resp->getJsonObject();
        CHECK(json->get("value", "") == Json::nullValue);
    });
}

int main(int argc, char** argv) 
{
    std::promise<void> p1;
    std::future<void> f1 = p1.get_future();

    // Start the main loop on another thread
    std::thread thr([&]() {
        // Queues the promise to be fulfilled after starting the loop
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    // The future is only satisfied after the event loop started
    f1.get();
    int status = test::run(argc, argv);

    // Ask the event loop to shutdown and wait
    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    return status;
}