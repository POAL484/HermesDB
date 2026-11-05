#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <jsoncpp/json/json.h>

#include <models/cacheservice.h>

using namespace drogon;


DROGON_TEST(GetUserUUID) 
{
    cache::CacheService service;
    const std::string uuid1 = service.registerRepo();
    const std::string uuid2 = service.registerRepo();

    CHECK(uuid1 != uuid2);
}


DROGON_TEST(CRUD) 
{
    cache::CacheService service;
    const std::string uuid = service.registerRepo();

    std::string value;
    service.set(uuid, "key", "value");
    CHECK(service.get(uuid, "key", value));
    CHECK(value == "value");

    service.deleteKey(uuid, "key");
    CHECK(!service.get(uuid, "key", value));
}

DROGON_TEST(Lru) 
{
    repository::LruRepository repo(1);
    
    std::string value;
    repo.set("key1", "value1");
    CHECK(repo.get("key1", value));
    CHECK(value == "value1");

    repo.set("key2", "value2");
    CHECK(repo.get("key2", value));
    CHECK(value == "value2");

    CHECK(!repo.get("key1", value));
}

DROGON_TEST(Lfu) 
{
    repository::LfuRepository repo(1);
    
    std::string value;
    repo.set("key1", "value1");
    CHECK(repo.get("key1", value));
    CHECK(value == "value1");

    repo.set("key2", "value2");
    CHECK(repo.get("key2", value));
    CHECK(value == "value2");

    CHECK(!repo.get("key1", value));
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