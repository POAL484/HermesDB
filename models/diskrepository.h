#pragma once

#include <models/repository.h>
#include <models/operation.h>

#include<fstream>
#include <trantor/utils/Logger.h>

namespace repository {

class DiskRepository {
 public:
    DiskRepository(const std::string& path) : _filepath(path) {
        LOG_INFO << "Created disk repo. File is " << path;
    }

    void save(const operation::Operation& op);
    void loadTo(Repository* const repo);

 private:
    const std::string _filepath;
    const char _separator = ';';
    
    bool load(std::ifstream& ifs, operation::Operation& op);
};

}