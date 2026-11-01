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
        // There's nothing we can do if the request didn't reach the server
        // or the server generated garbage.
        //REQUIRE(res == ReqResult::Ok);
        REQUIRE(resp != nullptr);

        CHECK(resp->getStatusCode() == k200OK);

        std::shared_ptr<Json::Value> json = resp->getJsonObject();
        std::string uuid = json->get("uuid", "").asString();

        CHECK(uuid != "");
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