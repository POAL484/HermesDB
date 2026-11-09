#pragma once

#include <unordered_set>
#include <list>
#include <string>

namespace types {

enum class Type : char {
    STRING,
    LIST,
    SET
};

class Cacheable {
 public:
    Type type;
};

class String : public Cacheable {
 public:
    explicit String(std::string str) : _str(str) {
        type = Type::STRING;
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
    explicit List(std::string str) {
        type = Type::LIST;
        _list.push_back(str);
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
    explicit Set(std::string str) {
        type = Type::SET;
        _set.insert(str);
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

};