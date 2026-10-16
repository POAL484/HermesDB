#include "diskrepository.h"

namespace repository {
    void DiskRepository::deleteKey(const std::string& key) {
        Operation op(DiskRepository::OperationType::DELETE, key, "");
        save(op);
    }

    bool DiskRepository::get(const std::string& key, std::string& value) {
        std::ifstream ifs(_filepath); 
        
        std::string line;
        Operation op;

        bool have = false;
        
        while(load(ifs, op)) {
            if(op.key == key) {
                have = op.type != DiskRepository::OperationType::DELETE;
                if(op.type == DiskRepository::OperationType::SET) value = op.value;
            }
        }

        ifs.close();
        return have;
    }

    void DiskRepository::set(const std::string& key, const std::string& value) {
        Operation op(DiskRepository::OperationType::SET, key, value);
        save(op);
    }

    void DiskRepository::save(const Operation& op) {
        std::lock_guard<std::mutex> lock(_filemtx);
        std::ofstream ofs(_filepath, std::ios_base::app);
        ofs << op.type << _separator << op.key << _separator << op.value << '\n';
        ofs.close();   
    }
    
    bool DiskRepository::load(std::ifstream& ifs, Operation& op) {
        std::lock_guard<std::mutex> lock(_filemtx);

        std::string line;
        if(!std::getline(ifs, line)) return false;
        
        auto sep1 = line.find(_separator);
        auto sep2 = line.find(_separator, sep1 + 1); 

        op.type = (DiskRepository::OperationType)(std::stoi(line.substr(0, sep1)));
        op.key = line.substr(sep1+1, sep2 - sep1 - 1);
        op.value = line.substr(sep2+1, line.size() - sep2);

        return true;
    }  
};