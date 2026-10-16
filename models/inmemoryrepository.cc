#include <models/inmemoryrepository.h>

namespace repository {

    void InMemoryRepository::deleteKey(const std::string& key) {
        auto it = _hashtable.find(key);
        if(it == _hashtable.end()) return;

        _keys.erase(it->second.second);
        _hashtable.erase(it);
    }

    bool InMemoryRepository::get(const std::string& key, std::string& value) {
        auto it = _hashtable.find(key);
        if(it == _hashtable.end()) return false;
        
        value = it->second.first;
        moveToRecentlyUsed(it);

        return true;
    }

    void InMemoryRepository::set(const std::string& key, const std::string& value) {
        auto it = _hashtable.find(key);
        if(it != _hashtable.end()) {
            it->second.first = value;
            moveToRecentlyUsed(it);
            return;
        }

        if(_hashtable.size() >= _capacity) {
            std::string leastRecentlyKey = _keys.back();
            deleteKey(key);
        }
        
        _keys.push_front(key);
        _hashtable[key] = {value, _keys.begin()};
    }

    void InMemoryRepository::moveToRecentlyUsed(std::unordered_map<std::string, InMemoryRepository::Value>::iterator it) {
        std::string key = it->first;
        
        _keys.erase(it->second.second);
        _keys.push_front(key);
        
        it->second.second = _keys.begin();
    }

}