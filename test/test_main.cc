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

    service.apply(uuid, {operation::OperationType::SET, "key", "value"});
    
    auto value = service.apply(uuid, {operation::OperationType::GET, "key"}).message;
    CHECK(value == "value");

    service.apply(uuid, {operation::OperationType::SET, "key", "value2"});
    value = service.apply(uuid, {operation::OperationType::GET, "key"}).message;
    CHECK(value == "value2");

    service.apply(uuid, {operation::OperationType::DELETE, "key"});
    value = service.apply(uuid, {operation::OperationType::GET, "key"}).message;
    CHECK(value.empty());
}

DROGON_TEST(List) 
{
    cache::CacheService service;
    const std::string uuid = service.registerRepo();

    service.apply(uuid, {operation::OperationType::SET, "key", "1"});
    
    auto value = service.apply(uuid, {operation::OperationType::GET, "key"}).message;
    CHECK(value == "1");

    auto len = service.apply(uuid, {operation::OperationType::RPUSH, "key", "2"}).message;    
    CHECK(len == "2");

    len = service.apply(uuid, {operation::OperationType::LPUSH, "key", "0"}).message;    
    CHECK(len == "3");

    value = service.apply(uuid, {operation::OperationType::RPOP, "key"}).message;
    CHECK(value == "2");

    value = service.apply(uuid, {operation::OperationType::LPOP, "key"}).message;
    CHECK(value == "0");

    value = service.apply(uuid, {operation::OperationType::LPOP, "key"}).message;
    CHECK(value == "1");    
}

DROGON_TEST(Lru) 
{
    repository::LruRepository repo(1);
    
    repo.apply({operation::OperationType::SET, "key1", "value1"});
    auto value = repo.apply({operation::OperationType::GET, "key1"}).message;
    CHECK(value == "value1");

    repo.apply({operation::OperationType::SET, "key2", "value2"});
    value = repo.apply({operation::OperationType::GET, "key2"}).message;
    CHECK(value == "value2");

    value = repo.apply({operation::OperationType::GET, "key1"}).message;
    CHECK(value.empty());
}

DROGON_TEST(Lfu) 
{
    repository::LfuRepository repo(1);
    
    repo.apply({operation::OperationType::SET, "key1", "value1"});
    auto value = repo.apply({operation::OperationType::GET, "key1"}).message;
    CHECK(value == "value1");

    repo.apply({operation::OperationType::SET, "key2", "value2"});
    value = repo.apply({operation::OperationType::GET, "key2"}).message;
    CHECK(value == "value2");

    value = repo.apply({operation::OperationType::GET, "key1"}).message;
    CHECK(value.empty());
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