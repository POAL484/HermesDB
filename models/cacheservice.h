#pragma once

#include <models/repository.h>
#include <models/diskrepository.h>
#include <models/inmemoryrepository.h>

#include <trantor/utils/Logger.h>
#include <drogon/HttpAppFramework.h>

namespace cache {

class CacheService {
 public:

    template<typename Repo>
    std::string registerRepo() {
        auto uuid = drogon::utils::getUuid();
        _repos[uuid] = new Repo();
        if(drogon::app().getCustomConfig().get("persistent", false).asBool()) {
            _persistent[uuid] = new repository::DiskRepository(drogon::app().getCustomConfig().get("persistent_path", "/tmp/").asString() + uuid + ".csv");
        } 

        return uuid;
    }

    bool haveRepo(const std::string& uuid) {
        return _repos.count(uuid);
    }

    void deleteKey(const std::string& uuid, const std::string& key) {
        if(_persistent.count(uuid)) _persistent[uuid]->deleteKey(key);
        _repos[uuid]->deleteKey(key);
    }

    bool get(const std::string& uuid, const std::string& key, std::string& value) {
        return _repos[uuid]->get(key, value);
    }
    
    void set(const std::string& uuid, const std::string& key, const std::string& value) {
        if(_persistent.count(uuid)) _persistent[uuid]->set(key, value);
        _repos[uuid]->set(key, value);
    }

 private:
    std::unordered_map<std::string, repository::Repository*> _repos;
    std::unordered_map<std::string, repository::DiskRepository*> _persistent;
};

};