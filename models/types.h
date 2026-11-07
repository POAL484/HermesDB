#pragma once

#include <list>
#include <string>

namespace types {

enum class Type : char {
    STRING,
    LIST
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


};