#include <repository/inmemoryrepository.h>

#include <chrono>

namespace repository {

    void LruRepository::moveToRecentlyUsed(std::unordered_map<std::string, LruRepository::Value>::iterator it) {
        std::string key = it->first;
        
        _keys.erase(it->second.second);
        _keys.push_front(key);
        
        it->second.second = _keys.begin();
    }

    operation::OperationResult LruRepository::deleteKey(const std::string& key) {
        auto it = _hashtable.find(key);
        if(it == _hashtable.end()) {
            Json::Value response;
            response["value"] = Json::Value();
            return {response};
        }

        Json::Value response;
        response["value"] = it->second.first.apply({operation::OperationType::GET, key}).message["value"];
        _keys.erase(it->second.second);
        _hashtable.erase(it);

        return {response};
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

        if(op.type == operation::OperationType::GET) {
            Json::Value response;
            response["value"] = Json::Value();
            return {response};
        }
        if(op.type == operation::OperationType::ISMEMBER) {
            Json::Value response;
            response["isMember"] = false;
            return {response};
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
        auto it = _cache.find(key);
        if(it == _cache.end()) {
            Json::Value response;
            response["value"] = Json::Value();
            return {response};
        }
        int freq = it->second.first;
        auto kv_iter = it->second.second;
        Json::Value response;
        response["value"] = kv_iter->second.apply({operation::OperationType::GET, key}).message["value"];
        
        _freqs[freq].erase(kv_iter);
        if(_freqs[freq].empty()) {
            _freqs.erase(freq);
            if(_minfreq == freq) _minfreq += 1;
        }
        _cache.erase(it);
        return {response};
    }

    operation::OperationResult LfuRepository::apply(const operation::Operation& op) {
        if(op.type == operation::OperationType::DELETE) {
            return deleteKey(op.key);
        }

        auto it = _cache.find(op.key);
        if(it != _cache.end()) {
            return it->second.second->second.apply(op);
        }

        if(op.type == operation::OperationType::GET) {
            Json::Value response;
            response["value"] = Json::Value();
            return {response};
        }
        if(op.type == operation::OperationType::ISMEMBER) {
            Json::Value response;
            response["isMember"] = false;
            return {response};
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

    bool TlruRepository::isExpired(const Value& value) const {
        const auto expiration = std::get<2>(value);
        return expiration != std::chrono::steady_clock::time_point{} &&
               std::chrono::steady_clock::now() >= expiration;
    }

    void TlruRepository::purgeExpired() {
        for(auto it = _hashtable.begin(); it != _hashtable.end();) {
            if(isExpired(it->second)) {
                _keys.erase(std::get<1>(it->second));
                it = _hashtable.erase(it);
            } else {
                ++it;
            }
        }
    }

    void TlruRepository::moveToRecentlyUsed(
        std::unordered_map<std::string, Value>::iterator it) {
        _keys.erase(std::get<1>(it->second));
        _keys.push_front(it->first);
        std::get<1>(it->second) = _keys.begin();
    }

    operation::OperationResult TlruRepository::deleteKey(const std::string& key) {
        auto it = _hashtable.find(key);
        if(it == _hashtable.end()) {
            Json::Value response;
            response["value"] = Json::Value();
            return {response};
        }

        Json::Value response;
        response["value"] = std::get<0>(it->second).apply({operation::OperationType::GET, key}).message["value"];
        _keys.erase(std::get<1>(it->second));
        _hashtable.erase(it);
        return {response};
    }

    bool TlruRepository::contains(const std::string& key) const {
        auto it = _hashtable.find(key);
        if(it == _hashtable.end()) return false;
        if(isExpired(it->second)) return false;
        return true;
    }

    types::Type TlruRepository::type(const std::string& key) const {
        auto it = _hashtable.find(key);
        if(it == _hashtable.end()) return types::Type::STRING;
        if(isExpired(it->second)) return types::Type::STRING;
        return std::get<0>(it->second).type();
    }

    operation::OperationResult TlruRepository::apply(const operation::Operation& op) {
        purgeExpired();

        if(op.type == operation::OperationType::DELETE) {
            return deleteKey(op.key);
        }

        auto expirationFor = [](const operation::Operation& operation) {
            if(operation.expiresAt > 0) {
                const auto deadline = std::chrono::system_clock::time_point(
                    std::chrono::milliseconds(operation.expiresAt));
                const auto remaining = deadline - std::chrono::system_clock::now();
                if(remaining <= std::chrono::system_clock::duration::zero()) {
                    return std::chrono::steady_clock::time_point::min();
                }
                return std::chrono::steady_clock::now() +
                       std::chrono::duration_cast<std::chrono::steady_clock::duration>(remaining);
            }
            if(operation.ttl == 0) return std::chrono::steady_clock::time_point{};
            return std::chrono::steady_clock::now() + std::chrono::seconds(operation.ttl);
        };

        const auto expiration = expirationFor(op);
        if(op.type == operation::OperationType::SET &&
           expiration == std::chrono::steady_clock::time_point::min()) {
            deleteKey(op.key);
            return {Json::Value()};
        }

        auto it = _hashtable.find(op.key);
        if(it != _hashtable.end()) {
            moveToRecentlyUsed(it);
            if(op.type == operation::OperationType::SET) {
                std::get<2>(it->second) = expiration;
            }
            return std::get<0>(it->second).apply(op);
        }

        if(op.type == operation::OperationType::GET) {
            Json::Value response;
            response["value"] = Json::Value();
            return {response};
        }
        if(op.type == operation::OperationType::ISMEMBER) {
            Json::Value response;
            response["isMember"] = false;
            return {response};
        }

        if(expiration == std::chrono::steady_clock::time_point::min()) {
            return {Json::Value()};
        }

        if(_hashtable.size() >= _capacity) {
            if(!_keys.empty()) {
                std::string leastRecentlyKey = _keys.back();
                _hashtable.erase(leastRecentlyKey);
                _keys.pop_back();
            }
        }

        _keys.push_front(op.key);
        _hashtable.emplace(
            op.key,
            Value{cache::Cache(), _keys.begin(), expiration});
        return std::get<0>(_hashtable[op.key]).apply(op);
    }
}