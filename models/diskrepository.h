#pragma once

#include <models/repository.h>

#include<fstream>
#include<mutex>

namespace repository {

class DiskRepository : public Repository {
 public:
    DiskRepository(const std::string& path) : _filepath(path) {}

    void deleteKey(const std::string& key) override;

    bool get(const std::string& key, std::string& value) override;
    void set(const std::string& key, const std::string& value) override;
 private:
    std::mutex _filemtx;

    const std::string _filepath;
    const char _separator = ';';
    
    enum OperationType {
        SET,
        DELETE
    };

    struct Operation {
        Operation() {}
        Operation(OperationType type, const std::string& key, const std::string& value) : type(type), key(key), value(value) {}

        OperationType type;
        std::string key;
        std::string value;  
    };

    void save(const Operation& op);
    bool load(std::ifstream& ifs, Operation& op);
};

}