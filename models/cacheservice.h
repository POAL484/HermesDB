#pragma once

#include <models/repository.h>
#include <models/diskrepository.h>
#include <models/inmemoryrepository.h>

#include <trantor/utils/Logger.h>
#include <drogon/HttpAppFramework.h>

namespace cache {

class CacheService {
 public:
    CacheService() {
        _policy = drogon::app().getCustomConfig().get("cache_policy", "").asString();
        if(_policy == "") {
            LOG_WARN << "Cache policy not provided. Using LRU now";
            _policy = "LRU";
        }

        _persistentDir = drogon::app().getCustomConfig().get("persistent_path", "/tmp/.hermes/").asString();
        _isPersistent = drogon::app().getCustomConfig().get("persistent", false).asBool();
        
        if(!_isPersistent) return;

        for(const auto& entry : std::filesystem::directory_iterator(_persistentDir)) {
            if(!entry.is_regular_file()) continue;
            LOG_INFO << "Found a backup on disk. Loading...";
            const std::string uuid = entry.path().filename();
            registerRepo(uuid);
            _persistent[uuid]->loadTo(_repos[uuid]);
        }
    }

    std::string registerRepo() {
        auto uuid = drogon::utils::getUuid();
        registerRepo(uuid);
        return uuid;
    }

    void registerRepo(const std::string& uuid) {
        if(_policy == "LFU") _repos[uuid] = new repository::LfuRepository();
        else _repos[uuid] = new repository::LruRepository(); 

        if(_isPersistent) {
            _persistent[uuid] = new repository::DiskRepository(_persistentDir + uuid);
        }    
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

    bool _isPersistent;
    std::string _persistentDir;
    std::string _policy;
};

};