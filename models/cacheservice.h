#pragma once

#include <models/operation.h>
#include <models/repository.h>
#include <models/diskrepository.h>
#include <models/inmemoryrepository.h>

#include <trantor/utils/Logger.h>
#include <drogon/HttpAppFramework.h>
#include <stdexcept>

namespace cache {

class CacheService {
 public:
    CacheService() {
        _policy = drogon::app().getCustomConfig().get("cache_policy", "").asString();
        if(_policy == "") {
            LOG_WARN << "Cache policy not provided. Using LRU now";
            _policy = "LRU";
        }

        const std::string compressionMethod =
            drogon::app().getCustomConfig().get("compression_method", "Snappy").asString();
        if(compressionMethod == "Snappy") _compressionMethod = repository::CompressionMethod::Snappy;
        else if(compressionMethod == "GZIP") _compressionMethod = repository::CompressionMethod::Gzip;
        else if(compressionMethod == "LZ4") _compressionMethod = repository::CompressionMethod::Lz4;
        else throw std::invalid_argument("Unsupported compression_method: " + compressionMethod);

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
            _persistent[uuid] = new repository::DiskRepository(_persistentDir + uuid, _compressionMethod);
        }    
    }

    bool haveRepo(const std::string& uuid) {
        return _repos.count(uuid);
    }

    operation::OperationResult apply(const std::string& uuid, const operation::Operation& op) {
        if(_isPersistent) _persistent[uuid]->save(op);
        return _repos[uuid]->apply(op);
    }

 private:
    std::unordered_map<std::string, repository::Repository*> _repos;
    std::unordered_map<std::string, repository::DiskRepository*> _persistent;

    bool _isPersistent;
    std::string _persistentDir;
    std::string _policy;
    repository::CompressionMethod _compressionMethod = repository::CompressionMethod::Snappy;
};

};