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

    service.apply(uuid, {operation::OperationType::SET, "key", {"value"}});
    
    auto value = service.apply(uuid, {operation::OperationType::GET, "key"}).message.asString();
    CHECK(value == "value");

    service.apply(uuid, {operation::OperationType::SET, "key", {"value2"}});
    value = service.apply(uuid, {operation::OperationType::GET, "key"}).message.asString();
    CHECK(value == "value2");

    service.apply(uuid, {operation::OperationType::DELETE, "key"});
    value = service.apply(uuid, {operation::OperationType::GET, "key"}).message.asString();
    CHECK(value.empty());
}

DROGON_TEST(List) 
{
    cache::CacheService service;
    const std::string uuid = service.registerRepo();

    service.apply(uuid, {operation::OperationType::SET, "key", {"1"}});
    
    auto value = service.apply(uuid, {operation::OperationType::GET, "key"}).message.asString();
    CHECK(value == "1");

    auto len = service.apply(uuid, {operation::OperationType::RPUSH, "key", {"2"}}).message["size"].asUInt();    
    CHECK(len == 2);

    len = service.apply(uuid, {operation::OperationType::LPUSH, "key", {"0"}}).message["size"].asUInt();    
    CHECK(len == 3);

    value = service.apply(uuid, {operation::OperationType::RPOP, "key"}).message["value"].asString();
    CHECK(value == "2");

    value = service.apply(uuid, {operation::OperationType::LPOP, "key"}).message["value"].asString();
    CHECK(value == "0");

    value = service.apply(uuid, {operation::OperationType::LPOP, "key"}).message["value"].asString();
    CHECK(value == "1");    
}

DROGON_TEST(Set) 
{
    cache::CacheService service;
    const std::string uuid = service.registerRepo();

    service.apply(uuid, {operation::OperationType::SET, "key", {"1"}});
    
    auto value = service.apply(uuid, {operation::OperationType::GET, "key"}).message.asString();
    CHECK(value == "1");

    auto len = service.apply(uuid, {operation::OperationType::SADD, "key", {"2"}}).message["size"].asUInt();
    CHECK(len == 2);

    len = service.apply(uuid, {operation::OperationType::SADD, "key", {"2"}}).message["size"].asUInt();
    CHECK(len == 2);

    auto is_member = service.apply(uuid, {operation::OperationType::ISMEMBER, "key", {"1"}}).message["is_member"].asBool();
    CHECK(is_member == true);

    is_member = service.apply(uuid, {operation::OperationType::ISMEMBER, "key", {"2"}}).message["is_member"].asBool();
    CHECK(is_member == true);
    
    is_member = service.apply(uuid, {operation::OperationType::ISMEMBER, "key", {"3"}}).message["is_member"].asBool();
    CHECK(is_member == false);
    
    len = service.apply(uuid, {operation::OperationType::SREM, "key", {"1"}}).message["size"].asUInt();
    CHECK(len == 1);

    is_member = service.apply(uuid, {operation::OperationType::ISMEMBER, "key", {"1"}}).message["is_member"].asBool();
    CHECK(is_member == false);

}

DROGON_TEST(Hash) {
    cache::CacheService service;
    const std::string uuid = service.registerRepo();

    auto len = service.apply(uuid, {operation::OperationType::HSET, "key", {"key1", "value1", "key2", "value2"}}).message["size"].asUInt();
    CHECK(len == 2);

    auto hash = service.apply(uuid, {operation::OperationType::GET, "key"}).message;
    CHECK(hash.get("key1", "").asString() == "value1");
    CHECK(hash.get("key2", "").asString() == "value2");

    auto once = service.apply(uuid, {operation::OperationType::GETITEM, "key", {"key1"}}).message;
    CHECK(once.get("key1", "").asString() == "value1");

    auto deleted = service.apply(uuid, {operation::OperationType::HDEL, "key", {"key1"}}).message;
    CHECK(deleted.get("key1", "").asString() == "value1");

}

DROGON_TEST(Lru) 
{
    repository::LruRepository repo(1);
    
    repo.apply({operation::OperationType::SET, "key1", {"value1"}});
    auto value = repo.apply({operation::OperationType::GET, "key1"}).message.asString();
    CHECK(value == "value1");

    repo.apply({operation::OperationType::SET, "key2", {"value2"}});
    value = repo.apply({operation::OperationType::GET, "key2"}).message.asString();
    CHECK(value == "value2");

    value = repo.apply({operation::OperationType::GET, "key1"}).message.asString();
    CHECK(value.empty());
}

DROGON_TEST(Lfu) 
{
    repository::LfuRepository repo(1);
    
    repo.apply({operation::OperationType::SET, "key1", {"value1"}});
    auto value = repo.apply({operation::OperationType::GET, "key1"}).message.asString();
    CHECK(value == "value1");

    repo.apply({operation::OperationType::SET, "key2", {"value2"}});
    value = repo.apply({operation::OperationType::GET, "key2"}).message.asString();
    CHECK(value == "value2");

    value = repo.apply({operation::OperationType::GET, "key1"}).message.asString();
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