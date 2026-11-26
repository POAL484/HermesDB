#pragma once

#include <models/operation.h>
#include <models/configuration.h>
#include <repository/repository.h>
#include <repository/diskrepository.h>
#include <repository/inmemoryrepository.h>

#include <memory>
#include <trantor/utils/Logger.h>
#include <drogon/HttpAppFramework.h>
#include <stdexcept>
#include <chrono>

namespace cache {

class CacheService {
 public:
    CacheService() {
        const auto& config = drogon::app().getCustomConfig();
        _policy = configuration::getStringSetting(config, "cache_policy", "CACHE_ALGORITHM", "");
        if(_policy == "") {
            LOG_WARN << "Cache policy not provided. Using LRU now";
            _policy = "LRU";
        }
        if(std::getenv("CACHE_ALGORITHM") &&
           _policy != "LRU" && _policy != "LFU" && _policy != "TLRU") {
            throw std::invalid_argument("Unsupported CACHE_ALGORITHM: " + _policy);
        }

        const std::string compressionMethod = configuration::getStringSetting(
            config, "compression_method", "DISK_COMPRESSION_ALGORITHM", "Snappy");
        if(compressionMethod == "Snappy") _compressionMethod = repository::CompressionMethod::Snappy;
        else if(compressionMethod == "GZIP") _compressionMethod = repository::CompressionMethod::Gzip;
        else if(compressionMethod == "LZ4") _compressionMethod = repository::CompressionMethod::Lz4;
        else throw std::invalid_argument("Unsupported compression_method: " + compressionMethod);

        _persistentDir = config.get("persistent_path", "/tmp/.hermes/").asString();
        _isPersistent = configuration::getBoolSetting(config, "persistent", "DISK_ENABLE", false);

        if(!_isPersistent) return;

        for(const auto& entry : std::filesystem::directory_iterator(_persistentDir)) {
            if(!entry.is_regular_file()) continue;
            LOG_INFO << "Found a backup on disk. Loading...";
            const std::string uuid = entry.path().filename();
            registerRepo(uuid);
            _persistent[uuid]->loadTo(_repos[uuid].get());
        }
    }

    std::string registerRepo() {
        auto uuid = drogon::utils::getUuid();
        registerRepo(uuid);
        return uuid;
    }

    void registerRepo(const std::string& uuid) {
        if(_policy == "LFU") _repos[uuid] = std::make_unique<repository::LfuRepository>();
        else if(_policy == "TLRU") _repos[uuid] = std::make_unique<repository::TlruRepository>();
        else _repos[uuid] = std::make_unique<repository::LruRepository>();

        if(_isPersistent) {
            _persistent[uuid] = std::make_unique<repository::DiskRepository>(_persistentDir + uuid, _compressionMethod);
        }
    }

    bool haveRepo(const std::string& uuid) {
        return _repos.count(uuid);
    }

    operation::OperationResult apply(const std::string& uuid, const operation::Operation& op) {
        operation::Operation recordedOp = op;
        if(recordedOp.type == operation::OperationType::SET &&
           recordedOp.ttl > 0 && recordedOp.expiresAt == 0) {
            const auto now = std::chrono::system_clock::now().time_since_epoch();
            recordedOp.expiresAt =
                std::chrono::duration_cast<std::chrono::milliseconds>(now).count() +
                recordedOp.ttl * 1000;
        }
        if(_isPersistent) _persistent[uuid]->save(recordedOp);
        return _repos[uuid]->apply(recordedOp);
    }

    bool haveKey(const std::string& uuid, const std::string& key) {
        return _repos.count(uuid) && _repos[uuid]->contains(key);
    }

    types::Type keyType(const std::string& uuid, const std::string& key) {
        if(!_repos.count(uuid)) return types::Type::STRING;
        return _repos[uuid]->type(key);
    }

 private:
    std::unordered_map<std::string, std::unique_ptr<repository::Repository>> _repos;
    std::unordered_map<std::string, std::unique_ptr<repository::DiskRepository>> _persistent;

    bool _isPersistent;
    std::string _persistentDir;
    std::string _policy;
    repository::CompressionMethod _compressionMethod = repository::CompressionMethod::Snappy;
};

};