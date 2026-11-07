#include <models/inmemoryrepository.h>

namespace repository {

    void LruRepository::moveToRecentlyUsed(std::unordered_map<std::string, LruRepository::Value>::iterator it) {
        std::string key = it->first;
        
        _keys.erase(it->second.second);
        _keys.push_front(key);
        
        it->second.second = _keys.begin();
    }

    operation::OperationResult LruRepository::deleteKey(const std::string& key) {
        operation::OperationResult res{"OK!"};
        
        auto it = _hashtable.find(key);
        if(it == _hashtable.end()) return res;

        _keys.erase(it->second.second);
        _hashtable.erase(it);

        return res;
    } 

    operation::OperationResult LruRepository::apply(const operation::Operation& op) {
        if(op.type == operation::OperationType::DELETE) {
            return deleteKey(op.key);
        }
        
        auto it = _hashtable.find(op.key);
    
        if(it != _hashtable.end()) {
            moveToRecentlyUsed(it);
            return it->second.first.apply(op);
        }

        if(_hashtable.size() >= _capacity) {
            std::string leastRecentlyKey = _keys.back();
            _hashtable.erase(leastRecentlyKey);
            _keys.pop_back();
        }

        _keys.push_front(op.key);
        _hashtable[op.key] = {cache::Cache(), _keys.begin()};
        return _hashtable[op.key].first.apply(op);
    }

    void LfuRepository::insert(int freq, std::string key, cache::Cache value) {
        _freqs[freq].push_back({key, value});
        _cache[key] = {freq, --_freqs[freq].end()};
    }

    void LfuRepository::updateFreq(const std::string& key) {
        auto it = _cache.find(key);
        if(it == _cache.end()) return;
        int freq = it->second.first;
        auto kv_iter = it->second.second;
        std::pair<std::string, cache::Cache> kv = *kv_iter;

        _freqs[freq].erase(kv_iter);
        if(_freqs[freq].empty()) {
            _freqs.erase(freq);
            if(_minfreq == freq) _minfreq += 1;
        }

        insert(freq + 1, kv.first, kv.second);
    }

    operation::OperationResult LfuRepository::deleteKey(const std::string& key) {
        operation::OperationResult res{"OK!"};
        
        auto it = _cache.find(key);
        if(it == _cache.end()) return res;
        int freq = it->second.first;
        auto kv_iter = it->second.second;
        
        _freqs[freq].erase(kv_iter);
        if(_freqs[freq].empty()) {
            _freqs.erase(freq);
            if(_minfreq == freq) _minfreq += 1;
        }  
        return res;
    }

    operation::OperationResult LfuRepository::apply(const operation::Operation& op) {
        if(op.type == operation::OperationType::DELETE) {
            return deleteKey(op.key);
        }

        auto it = _cache.find(op.key);
        if(it != _cache.end()) {
            return it->second.second->second.apply(op);
        }

        if(_cache.size() >= _capacity) {
            _cache.erase(_freqs[_minfreq].front().first);
            _freqs[_minfreq].pop_front();

            if(_freqs[_minfreq].empty()) {
                _freqs.erase(_minfreq);
            }
        }

        cache::Cache ch;
        auto res = ch.apply(op);
        
        _minfreq = 1;
        insert(1, op.key, ch);
        
        return res;
    }
}