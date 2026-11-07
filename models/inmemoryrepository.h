#pragma once

#include <models/repository.h>
#include <models/cache.h>

#include <unordered_map>
#include <list>
#include <mutex>
#include <trantor/utils/Logger.h>

namespace repository {

class InMemoryRepository : public Repository {
 public:
    InMemoryRepository(size_t capacity = 500) : _capacity(capacity) {
      LOG_INFO << "Created in-memory repo with cap " << _capacity;
    }

 protected:
    size_t _capacity;
};

class LruRepository : public InMemoryRepository {
 public:
    using InMemoryRepository::InMemoryRepository;

    operation::OperationResult apply(const operation::Operation& op) override;

  private:
    using Value = std::pair<cache::Cache, std::list<std::string>::iterator>;   
    
    std::unordered_map<std::string, Value> _hashtable;
    std::list<std::string> _keys;

    void moveToRecentlyUsed(std::unordered_map<std::string, Value>::iterator it);
    operation::OperationResult deleteKey(const std::string& key);
};

class LfuRepository : public InMemoryRepository {
 public:
    using InMemoryRepository::InMemoryRepository;

    operation::OperationResult apply(const operation::Operation& op) override;

  private:

    using KVs = std::list<std::pair<std::string, cache::Cache>>;

    std::unordered_map<int, KVs> _freqs;
    std::unordered_map<std::string, std::pair<int, KVs::iterator>> _cache;

    void insert(int freq, std::string key, cache::Cache value);
    void updateFreq(const std::string& key);
    operation::OperationResult deleteKey(const std::string& key);

    int _minfreq;
};

}