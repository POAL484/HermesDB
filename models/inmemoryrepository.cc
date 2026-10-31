#include <models/inmemoryrepository.h>

namespace repository {

    void LruRepository::deleteKey(const std::string& key) {
        std::lock_guard guard(_mtx);
        auto it = _hashtable.find(key);
        if(it == _hashtable.end()) return;

        _keys.erase(it->second.second);
        _hashtable.erase(it);
    }

    bool LruRepository::get(const std::string& key, std::string& value) {
        std::lock_guard guard(_mtx);
        auto it = _hashtable.find(key);
        if(it == _hashtable.end()) return false;
        
        value = it->second.first;
        moveToRecentlyUsed(it);

        return true;
    }

    void LruRepository::set(const std::string& key, const std::string& value) {
        std::lock_guard guard(_mtx);
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

    void LruRepository::moveToRecentlyUsed(std::unordered_map<std::string, LruRepository::Value>::iterator it) {
        std::lock_guard guard(_mtx);
        std::string key = it->first;
        
        _keys.erase(it->second.second);
        _keys.push_front(key);
        
        it->second.second = _keys.begin();
    }


    void LfuRepository::insert(int freq, std::string key, std::string value) {
        std::lock_guard guard(_mtx);
        _freqs[freq].push_back({key, value});
        _cache[key] = {freq, --_freqs[freq].end()};
    }

    void LfuRepository::updateFreq(const std::string& key) {
        std::lock_guard guard(_mtx);
        auto it = _cache.find(key);
        if(it == _cache.end()) return;
        int freq = it->second.first;
        auto kv_iter = it->second.second;
        std::pair<std::string, std::string> kv = *kv_iter;

        _freqs[freq].erase(kv_iter);
        if(_freqs[freq].empty()) {
            _freqs.erase(freq);
            if(_minfreq == freq) _minfreq += 1;
        }

        insert(freq + 1, kv.first, kv.second);
    }

    void LfuRepository::deleteKey(const std::string& key) {
        std::lock_guard guard(_mtx);
        auto it = _cache.find(key);
        if(it == _cache.end()) return;
        int freq = it->second.first;
        auto kv_iter = it->second.second;
        
        _freqs[freq].erase(kv_iter);
        if(_freqs[freq].empty()) {
            _freqs.erase(freq);
            if(_minfreq == freq) _minfreq += 1;
        }        
    }

    bool LfuRepository::get(const std::string& key, std::string& value) {
        std::lock_guard guard(_mtx);
        auto it = _cache.find(key);
        if(it == _cache.end()) return false;
        auto kv = *(it->second.second);
        
        value = kv.second;
        updateFreq(kv.first);
        return true;
    }

    void LfuRepository::set(const std::string& key, const std::string& value) {
        std::lock_guard guard(_mtx);
        auto it = _cache.find(key);
        if(it != _cache.end()) {
            it->second.second->second = value;            
            return;
        }

        if(_capacity == _cache.size()) {
            _cache.erase(_freqs[_minfreq].front().first);
            _freqs[_minfreq].pop_front();

            if(_freqs[_minfreq].empty()) {
                _freqs.erase(_minfreq);
            }
        }

        _minfreq = 1;
        insert(1, key, value);
    }
}