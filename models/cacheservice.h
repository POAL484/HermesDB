#pragma once

#include <models/repository.h>
#include <models/diskrepository.h>
#include <models/inmemoryrepository.h>

#include <trantor/utils/Logger.h>

namespace cache {

class CacheService {
 public:

    template<typename Repo>
    std::string registerRepo() {
        auto uuid = drogon::utils::getUuid();
        _repos[uuid] = new Repo();
        return uuid;
    }

    bool haveRepo(const std::string& uuid) {
        return _repos.count(uuid);
    }

    void deleteKey(const std::string& uuid, const std::string& key) {
        _repos[uuid]->deleteKey(key);
    }

    bool get(const std::string& uuid, const std::string& key, std::string& value) {
        return _repos[uuid]->get(key, value);
    }
    
    void set(const std::string& uuid, const std::string& key, const std::string& value) {
        _repos[uuid]->set(key, value);
    }

 private:
    std::unordered_map<std::string, repository::Repository*> _repos;
};

};