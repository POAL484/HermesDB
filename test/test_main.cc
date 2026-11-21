#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <jsoncpp/json/json.h>

#include <models/cacheservice.h>
#include <query/syntaxmodule.h>
#include <query/semanticmodule.h>
#include <filesystem>
#include <fstream>
#include <snappy.h>
#include <chrono>
#include <thread>

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

    service.apply(uuid, {operation::OperationType::SET, "key", {{"value", types::Type::STRING}}});
    
    auto value = service.apply(uuid, {operation::OperationType::GET, "key"}).message["value"].asString();
    CHECK(value == "value");

    service.apply(uuid, {operation::OperationType::SET, "key", {{"value2", types::Type::STRING}}});
    value = service.apply(uuid, {operation::OperationType::GET, "key"}).message["value"].asString();
    CHECK(value == "value2");

    auto resp = service.apply(uuid, {operation::OperationType::DELETE, "key"});
    CHECK(resp.message["value"].asString() == "value2");

    value = service.apply(uuid, {operation::OperationType::GET, "key"}).message["value"].asString();
    CHECK(value.empty());
}

DROGON_TEST(GetMissingKeyReturnsNull) {
    cache::CacheService service;
    const std::string uuid = service.registerRepo();

    const auto result = service.apply(uuid,
                                      {operation::OperationType::GET, "missing"});
    CHECK(result.message.isMember("value"));
    CHECK(result.message["value"].isNull());
}

DROGON_TEST(Int) {
    cache::CacheService service;
    const std::string uuid = service.registerRepo();

    service.apply(uuid, {operation::OperationType::SET, "key", {{"0", types::Type::INT}}});
    auto value = service.apply(uuid, {operation::OperationType::GET, "key"}).message["value"].asInt();
    CHECK(value == 0);

    service.apply(uuid, {operation::OperationType::INC, "key", {{"13", types::Type::INT}}});
    value = service.apply(uuid, {operation::OperationType::GET, "key"}).message["value"].asInt();
    CHECK(value == 13);

    service.apply(uuid, {operation::OperationType::DEC, "key", {{"6", types::Type::INT}}});
    value = service.apply(uuid, {operation::OperationType::GET, "key"}).message["value"].asInt();
    CHECK(value == 7);

    service.apply(uuid, {operation::OperationType::SET, "key", {{"111", types::Type::INT}}});
    value = service.apply(uuid, {operation::OperationType::GET, "key"}).message["value"].asInt();
    CHECK(value == 111);
}

DROGON_TEST(List) 
{
    cache::CacheService service;
    const std::string uuid = service.registerRepo();

    service.apply(uuid, {operation::OperationType::RPUSH, "key", {{"1", types::Type::STRING}}});
    
    auto value = service.apply(uuid, {operation::OperationType::GET, "key"}).message["value"];
    CHECK(value.isArray());
    CHECK(value[0].asString() == "1");

    auto len = service.apply(uuid, {operation::OperationType::RPUSH, "key", {{"2", types::Type::STRING}}}).message["size"].asUInt();    
    CHECK(len == 2);

    len = service.apply(uuid, {operation::OperationType::LPUSH, "key", {{"0", types::Type::STRING}}}).message["size"].asUInt();    
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

    service.apply(uuid, {operation::OperationType::SADD, "key", {{"1", types::Type::STRING}}});
    
    auto value = service.apply(uuid, {operation::OperationType::GET, "key"}).message["value"];
    CHECK(value["$set"].isArray());
    CHECK(value["$set"][0].asString() == "1");

    auto len = service.apply(uuid, {operation::OperationType::SADD, "key", {{"2", types::Type::STRING}}}).message["size"].asUInt();
    CHECK(len == 2);

    len = service.apply(uuid, {operation::OperationType::SADD, "key", {{"2", types::Type::STRING}}}).message["size"].asUInt();
    CHECK(len == 2);

    auto is_member = service.apply(uuid, {operation::OperationType::ISMEMBER, "key", {{"1", types::Type::STRING}}}).message["isMember"].asBool();
    CHECK(is_member == true);

    is_member = service.apply(uuid, {operation::OperationType::ISMEMBER, "key", {{"2", types::Type::STRING}}}).message["isMember"].asBool();
    CHECK(is_member == true);
    
    is_member = service.apply(uuid, {operation::OperationType::ISMEMBER, "key", {{"3", types::Type::STRING}}}).message["isMember"].asBool();
    CHECK(is_member == false);
    
    len = service.apply(uuid, {operation::OperationType::SREM, "key", {{"1", types::Type::STRING}}}).message["size"].asUInt();
    CHECK(len == 1);

    is_member = service.apply(uuid, {operation::OperationType::ISMEMBER, "key", {{"1", types::Type::STRING}}}).message["isMember"].asBool();
    CHECK(is_member == false);

}

DROGON_TEST(Hash) {
    cache::CacheService service;
    const std::string uuid = service.registerRepo();

    auto len = service.apply(uuid, {operation::OperationType::HSET, "key", 
        {{"key1", types::Type::STRING}, {"value1", types::Type::STRING}, {"key2", types::Type::STRING}, {"2", types::Type::INT}}})
        .message["size"].asUInt();
    CHECK(len == 2);

    auto hash = service.apply(uuid, {operation::OperationType::GET, "key"}).message["value"];
    CHECK(hash.get("key1", "").asString() == "value1");
    CHECK(hash.get("key2", "").asInt64() == 2);

    auto once = service.apply(uuid, {operation::OperationType::GETITEM, "key", {{"key2", types::Type::STRING}}}).message;
    CHECK(once["value"].get("key2", "").asInt64() == 2);

    auto deleted = service.apply(uuid, {operation::OperationType::HDEL, "key", {{"key1", types::Type::STRING}}}).message;
    CHECK(deleted["deleted"].get("key1", "").asString() == "value1");

}

DROGON_TEST(CacheCopySemantics) {
    cache::Cache original(types::Type::STRING);
    original.apply({operation::OperationType::SET, "key", {{"hello", types::Type::STRING}}});

    cache::Cache copy = original;
    copy.apply({operation::OperationType::SET, "key", {{"world", types::Type::STRING}}});

    CHECK(original.apply({operation::OperationType::GET, "key"}).message["value"].asString() == "hello");
    CHECK(copy.apply({operation::OperationType::GET, "key"}).message["value"].asString() == "world");
}

DROGON_TEST(Lru) 
{
    repository::LruRepository repo(1);
    
    repo.apply({operation::OperationType::SET, "key1", {{"value1", types::Type::STRING}}});
    auto value = repo.apply({operation::OperationType::GET, "key1"}).message["value"].asString();
    CHECK(value == "value1");

    repo.apply({operation::OperationType::SET, "key2", {{"value2", types::Type::STRING}}});
    value = repo.apply({operation::OperationType::GET, "key2"}).message["value"].asString();
    CHECK(value == "value2");

    value = repo.apply({operation::OperationType::GET, "key1"}).message["value"].asString();
    CHECK(value.empty());
}

DROGON_TEST(Lfu) 
{
    repository::LfuRepository repo(1);
    
    repo.apply({operation::OperationType::SET, "key1", {{"value1", types::Type::STRING}}});
    auto value = repo.apply({operation::OperationType::GET, "key1"}).message["value"].asString();
    CHECK(value == "value1");

    repo.apply({operation::OperationType::SET, "key2", {{"value2", types::Type::STRING}}});
    value = repo.apply({operation::OperationType::GET, "key2"}).message["value"].asString();
    CHECK(value == "value2");

    value = repo.apply({operation::OperationType::GET, "key1"}).message["value"].asString();
    CHECK(value.empty());
}

DROGON_TEST(TlruTtl) {
    repository::TlruRepository repo(2);

    repo.apply({operation::OperationType::SET, "short", {{"value", types::Type::STRING}}, 1});
    CHECK(repo.apply({operation::OperationType::GET, "short"}).message["value"].asString() == "value");

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    CHECK(repo.apply({operation::OperationType::GET, "short"}).message["value"].isNull());
}

DROGON_TEST(TlruLruEviction) {
    repository::TlruRepository repo(2);

    repo.apply({operation::OperationType::SET, "one", {{"1", types::Type::STRING}}, 0});
    repo.apply({operation::OperationType::GET, "one"});
    repo.apply({operation::OperationType::SET, "two", {{"2", types::Type::STRING}}, 0});
    repo.apply({operation::OperationType::SET, "three", {{"3", types::Type::STRING}}, 0});

    CHECK(repo.apply({operation::OperationType::GET, "one"}).message["value"].isNull());
    CHECK(repo.apply({operation::OperationType::GET, "three"}).message["value"].asString() == "3");
}

DROGON_TEST(TlruPersistenceKeepsAbsoluteExpiration) {
    const auto path = std::filesystem::temp_directory_path() /
                      ("hermes-tlru-" + drogon::utils::getUuid());
    const auto deadline = std::chrono::system_clock::now() +
                          std::chrono::milliseconds(1500);
    const auto expiresAt = std::chrono::duration_cast<std::chrono::milliseconds>(
                               deadline.time_since_epoch())
                               .count();

    repository::DiskRepository disk(path.string(), repository::CompressionMethod::Snappy);
    disk.save({operation::OperationType::SET, "key",
               {{"value", types::Type::STRING}}, 60, expiresAt});

    std::this_thread::sleep_for(std::chrono::milliseconds(700));
    repository::TlruRepository restored;
    disk.loadTo(&restored);

    CHECK(restored.contains("key"));
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    CHECK(!restored.contains("key"));
    CHECK(restored.apply({operation::OperationType::GET, "key"}).message["value"].isNull());

    std::filesystem::remove(path);
}

DROGON_TEST(PersistenceCompression) {
    const std::array methods = {
        repository::CompressionMethod::Snappy,
        repository::CompressionMethod::Gzip,
        repository::CompressionMethod::Lz4
    };

    for(const auto method : methods) {
        const auto path = std::filesystem::temp_directory_path() /
                          ("hermes-compression-" + drogon::utils::getUuid());
        const std::string value = "persistent value\nwith a newline";
        repository::DiskRepository disk(path.string(), method);
        disk.save({operation::OperationType::SET, "key", {{value, types::Type::STRING}}});

        repository::LruRepository restored;
        disk.loadTo(&restored);
        CHECK(restored.apply({operation::OperationType::GET, "key"}).message["value"].asString() == value);
        std::filesystem::remove(path);
    }
}

DROGON_TEST(PersistenceKeepsJsonValues) {
    const auto path = std::filesystem::temp_directory_path() /
                      ("hermes-json-values-" + drogon::utils::getUuid());
    Json::Value values(Json::arrayValue);
    values.append(17);
    values.append(Json::Value(Json::objectValue));
    values[1]["nested"] = "value;with separator";

    repository::DiskRepository disk(path.string(), repository::CompressionMethod::Snappy);
    disk.save({operation::OperationType::SET, "key;with separator",
               {operation::Value(values)}});

    repository::LruRepository restored;
    disk.loadTo(&restored);
    const Json::Value stored =
        restored.apply({operation::OperationType::GET, "key;with separator"}).message["value"];
    CHECK(stored == values);
    std::filesystem::remove(path);
}

DROGON_TEST(PersistenceRejectsLegacyFormat) {
    const auto path = std::filesystem::temp_directory_path() /
                      ("hermes-legacy-snappy-" + drogon::utils::getUuid());
    const std::string serialized = "1;key;legacy value;0";
    std::string compressed;
    snappy::Compress(serialized.data(), serialized.size(), &compressed);
    {
        std::ofstream file(path, std::ios::binary);
        file.write(compressed.data(), compressed.size());
        file.put('\n');
    }

    repository::DiskRepository disk(path.string(), repository::CompressionMethod::Lz4);
    repository::LruRepository restored;
    bool rejected = false;
    try {
        disk.loadTo(&restored);
    } catch(const std::runtime_error&) {
        rejected = true;
    }
    CHECK(rejected);
    std::filesystem::remove(path);
}

DROGON_TEST(SyntaxAndSemanticModules) {
    Json::Value request;
    request["op"] = "INC";
    request["key"] = "counter";
    request["value"] = 3;

    const auto parsed = syntax::SyntaxModule::parse(request);
    CHECK(parsed.type == operation::OperationType::INC);
    CHECK(parsed.value.size() == 1);
    CHECK(parsed.value[0].second == types::Type::INT);
    CHECK(parsed.value[0].data.asInt() == 3);

    semantic::SemanticModule::KeyTypes key_types;
    key_types["counter"] = types::Type::INT;
    CHECK_NOTHROW(semantic::SemanticModule::validate(parsed, key_types));

    key_types["counter"] = types::Type::STRING;
    CHECK_THROWS(semantic::SemanticModule::validate(parsed, key_types));
}

DROGON_TEST(QueryLanguageFormat) {
    cache::CacheService service;
    const std::string uuid = service.registerRepo();
    auto execute = [&service, &uuid](const Json::Value& request) {
        const auto operation = syntax::SyntaxModule::parse(request);
        semantic::SemanticModule::KeyTypes keyTypes;
        if(service.haveKey(uuid, operation.key)) {
            keyTypes[operation.key] = service.keyType(uuid, operation.key);
        }
        semantic::SemanticModule::validate(operation, keyTypes);
        return service.apply(uuid, operation).message;
    };

    Json::Value request;
    request["op"] = "SET";
    request["key"] = "list";
    request["value"] = Json::arrayValue;
    request["value"].append(10);
    request["value"].append("text");
    request["value"].append(Json::Value(Json::objectValue));
    request["value"][2]["nested"] = true;
    CHECK(execute(request)["OK"].asString() == "OK!");

    request = Json::Value(Json::objectValue);
    request["op"] = "GETITEM";
    request["key"] = "list";
    request["item"].append(0);
    request["item"].append(2);
    const Json::Value items = execute(request)["value"];
    CHECK(items.size() == 2);
    CHECK(items[0].asInt() == 10);
    CHECK(items[1]["nested"].asBool());

    request = Json::Value(Json::objectValue);
    request["op"] = "SIZE";
    request["key"] = "list";
    CHECK(execute(request)["size"].asUInt() == 3);

    request = Json::Value(Json::objectValue);
    request["op"] = "SET";
    request["key"] = "set";
    request["value"]["$set"].append("member");
    request["value"]["$set"].append(7);
    execute(request);
    request = Json::Value(Json::objectValue);
    request["op"] = "ISMEMBER";
    request["key"] = "set";
    request["value"] = 7;
    CHECK(execute(request)["isMember"].asBool());
    request["key"] = "missing-set";
    CHECK(!execute(request)["isMember"].asBool());

    request = Json::Value(Json::objectValue);
    request["op"] = "HSET";
    request["key"] = "map";
    request["value"]["kept"] = true;
    execute(request);
    request["value"] = Json::Value(Json::objectValue);
    request["value"]["added"] = Json::arrayValue;
    request["value"]["added"].append(1);
    execute(request);

    request = Json::Value(Json::objectValue);
    request["op"] = "GETITEM";
    request["key"] = "map";
    request["item"].append("added");
    request["item"].append("absent");
    const Json::Value mapItems = execute(request)["value"];
    CHECK(mapItems["added"][0].asInt() == 1);
    CHECK(mapItems["absent"].isNull());

    request = Json::Value(Json::objectValue);
    request["op"] = "HDEL";
    request["key"] = "map";
    request["mapKeys"].append("kept");
    request["mapKeys"].append("absent");
    const Json::Value deleted = execute(request)["deleted"];
    CHECK(deleted["kept"].asBool());
    CHECK(deleted["absent"].isNull());

    request = Json::Value(Json::objectValue);
    request["op"] = "INC";
    request["key"] = "new-counter";
    request["value"] = 2.5;
    CHECK(execute(request)["value"].asDouble() == 2.5);

    request = Json::Value(Json::objectValue);
    request["op"] = "DEL";
    request["key"] = "list";
    CHECK(execute(request)["value"].isArray());
}

DROGON_TEST(QueryLanguageErrors) {
    Json::Value request(Json::objectValue);
    request["op"] = "UNKNOWN";
    request["key"] = "key";
    bool unknownOperation = false;
    try {
        syntax::SyntaxModule::parse(request);
    } catch(const syntax::SyntaxError& error) {
        unknownOperation = error.errorCode() == -2;
    }
    CHECK(unknownOperation);

    request = Json::Value(Json::objectValue);
    request["op"] = "GET";
    bool missingKey = false;
    try {
        syntax::SyntaxModule::parse(request);
    } catch(const syntax::SyntaxError& error) {
        missingKey = error.errorCode() == -3;
    }
    CHECK(missingKey);

    cache::Cache cache;
    bool outOfBounds = false;
    Json::Value list(Json::arrayValue);
    list.append(1);
    cache.apply({operation::OperationType::SET, "list", {operation::Value(list)}});
    try {
        cache.apply({operation::OperationType::GETITEM, "list",
                     {operation::Value(Json::Value(2))}});
    } catch(const operation::Error& error) {
        outOfBounds = error.errorCode() == 4;
    }
    CHECK(outOfBounds);

    cache::Cache emptySet(types::Type::SET);
    bool emptySetRemoval = false;
    try {
        emptySet.apply({operation::OperationType::SREM, "set",
                        {operation::Value(Json::Value("member"))}});
    } catch(const operation::Error& error) {
        emptySetRemoval = error.errorCode() == 3;
    }
    CHECK(emptySetRemoval);
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