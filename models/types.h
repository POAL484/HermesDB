#pragma once

#include <unordered_set>
#include <unordered_map>
#include <list>
#include <string>

#include <jsoncpp/json/value.h>

namespace types {

enum class Type : char {
    STRING,
    INT,
    LIST,
    SET,
    HASH
};

class Cacheable {
 public:
    Type type;

    virtual Json::Value to_json() = 0;
};

class Int : public Cacheable {
 public:
    explicit Int(int64_t n) : _n(n) {
        type = Type::INT;    
    }

    void set(int64_t n) {
        _n = n;
    }

    Json::Value to_json() override {
        return Json::Value(_n);
    }

    int64_t inc(int64_t by) {
        _n += by;
        return _n;
    }

    int64_t dec(int64_t by) {
        _n -= by;
        return _n;
    }

 private:
    int64_t _n;
};

class String : public Cacheable {
 public:
    explicit String(std::string str) : _str(str) {
        type = Type::STRING;
    }

    Json::Value to_json() override {
        return Json::Value(_str);
    }

    std::string get() {
        return _str;
    }

    void set(const std::string& str) {
        _str = str;
    }

 private:
    std::string _str;
};

class List : public Cacheable {
 public:
    List() {
        type = Type::LIST;
    }

    explicit List(std::string str) : List() {
        _list.push_back(str);
    }

    Json::Value to_json() override {
        Json::Value json(Json::arrayValue);
        for(auto el : _list) json.append(el);
        
        return json;
    }

    size_t lpush(std::string str) {
        _list.push_front(str);
        return _list.size();
    }

    size_t rpush(std::string str) {
        _list.push_back(str);
        return _list.size();
    }

    std::string lpop() {
        const std::string str = _list.front();
        _list.pop_front();
        return str;
    }

    std::string rpop() {
        const std::string str = _list.back();
        _list.pop_back();
        return str;
    }

 private:
    std::list<std::string> _list;
};

class Set : public Cacheable {
 public:
    Set() {
        type = Type::SET;
    }

    explicit Set(std::string str) : Set() {
        _set.insert(str);
    }
    
    Json::Value to_json() override {
        Json::Value json(Json::arrayValue);
        for(auto el : _set) json.append(el);
        
        return json;
    }

    size_t add(std::string str) {
        _set.insert(str);
        return _set.size();
    }

    size_t rem(std::string str) {
        _set.erase(str);
        return _set.size();
    }

    bool isMember(std::string str) {
        return _set.count(str);
    }

 private:
    std::unordered_set<std::string> _set;
};

class Hash : public Cacheable {
 public:
    Hash() {
        type = Type::HASH;
    }

    Json::Value to_json() override {
        Json::Value json;
        for(auto [k, v] : _hash) {
            json[k] = getJson(v);
        }

        return json;
    }

    size_t set(const std::vector<std::pair<std::string, Type>>& key_value) {
        _hash.clear();
        for(int i = 0; i+1 < key_value.size(); i += 2) {
            _hash[key_value[i].first] = key_value[i+1];
        }
        return size();
    }

    Json::Value get(const std::string& key) {
        return getJson(_hash[key]);
    }

    Json::Value del(const std::string& key) {
        auto value = get(key);
        _hash.erase(key);
        return value;
    }

    size_t size() {
        return _hash.size();
    }

 private:
    std::unordered_map<std::string, std::pair<std::string, Type>> _hash;

    Json::Value getJson(std::pair<std::string, Type> value) {
        if(value.second == Type::INT) return Json::Value(Json::Value::Int64(stoll(value.first)));
        return Json::Value(value.first);
    }
};

};