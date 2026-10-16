#pragma once

#include<string>

namespace repository {

class Repository {
 public:
    virtual void deleteKey(const std::string& key) = 0;

    virtual bool get(const std::string& key, std::string& value) = 0;
    virtual void set(const std::string& key, const std::string& value) = 0;
 protected:
    Repository() = default;
};

}

