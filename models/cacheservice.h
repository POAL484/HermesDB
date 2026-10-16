#pragma once

#include <models/repository.h>
#include <models/diskrepository.h>
#include <models/inmemoryrepository.h>


namespace cache {

class CacheService {
 public:
    CacheService(const std::string& path="") { 
        if(path.empty()) _repo = new repository::InMemoryRepository();
        else _repo = new repository::DiskRepository(path);
    }

    ~CacheService() {
        delete _repo;
    }

    void deleteKey(const std::string& key) {
        _repo->deleteKey(key);
    }

    bool get(const std::string& key, std::string& value) {
        return _repo->get(key, value);
    }
    
    void set(const std::string& key, const std::string& value) {
        _repo->set(key, value);
    }

 private:
    repository::Repository* _repo;
};

};