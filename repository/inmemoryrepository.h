#pragma once

#include <repository/repository.h>
#include <models/cache.h>

#include <unordered_map>
#include <list>
#include <mutex>
#include <tuple>
#include <chrono>
#include <trantor/utils/Logger.h>

namespace repository {

class InMemoryRepository : public Repository {
 public:
    InMemoryRepository(size_t capacity = 100) : _capacity(capacity) {
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
 public:
    bool contains(const std::string& key) const override {
      return _hashtable.find(key) != _hashtable.end();
    }
    types::Type type(const std::string& key) const override {
      const auto it = _hashtable.find(key);
      return it == _hashtable.end() ? types::Type::STRING : it->second.first.type();
    }
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
 public:
    bool contains(const std::string& key) const override {
      return _cache.find(key) != _cache.end();
    }
    types::Type type(const std::string& key) const override {
      const auto it = _cache.find(key);
      if(it == _cache.end()) return types::Type::STRING;
      return it->second.second->second.type();
    }

    int _minfreq;
};

class TlruRepository : public InMemoryRepository {
 public:
    using InMemoryRepository::InMemoryRepository;

    operation::OperationResult apply(const operation::Operation& op) override;

 private:
    using Value = std::tuple<cache::Cache,
                             std::list<std::string>::iterator,
                             std::chrono::steady_clock::time_point>;

    std::unordered_map<std::string, Value> _hashtable;
    std::list<std::string> _keys;

    bool isExpired(const Value& value) const;
    void purgeExpired();
    void moveToRecentlyUsed(std::unordered_map<std::string, Value>::iterator it);
    operation::OperationResult deleteKey(const std::string& key);

 public:
    bool contains(const std::string& key) const override;
    types::Type type(const std::string& key) const override;
};

using TLRURepository = TlruRepository;

}
