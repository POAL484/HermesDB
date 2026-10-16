#pragma once

#include <unordered_map>
#include <list>
#include <models/repository.h>

namespace repository {

class InMemoryRepository : public Repository {
 public:
    InMemoryRepository(size_t capacity = 500) : _capacity(capacity) {}

    void deleteKey(const std::string& key) override;

    bool get(const std::string& key, std::string& value) override;
    void set(const std::string& key, const std::string& value) override;
 private:
    using Value = std::pair<std::string, std::list<std::string>::iterator>;   

    size_t _capacity;

    std::unordered_map<std::string, Value> _hashtable;
    std::list<std::string> _keys;

    void moveToRecentlyUsed(std::unordered_map<std::string, Value>::iterator it);
};

}