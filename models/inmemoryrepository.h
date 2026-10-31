#pragma once

#include <models/repository.h>

#include <unordered_map>
#include <list>
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
   
    void deleteKey(const std::string& key) override;

    bool get(const std::string& key, std::string& value) override;
    void set(const std::string& key, const std::string& value) override;
  private:
    using Value = std::pair<std::string, std::list<std::string>::iterator>;   
    
    std::unordered_map<std::string, Value> _hashtable;
    std::list<std::string> _keys;

    void moveToRecentlyUsed(std::unordered_map<std::string, Value>::iterator it);
};

class LfuRepository : public InMemoryRepository {
 public:
    using InMemoryRepository::InMemoryRepository;

    void deleteKey(const std::string& key) override;

    bool get(const std::string& key, std::string& value) override;
    void set(const std::string& key, const std::string& value) override;
  private:

    using KVs = std::list<std::pair<std::string, std::string>>;

    std::unordered_map<int, KVs> _freqs;
    std::unordered_map<std::string, std::pair<int, KVs::iterator>> _cache;

    void insert(int freq, std::string key, std::string value);
    void updateFreq(const std::string& key);

    int _minfreq;
};

}