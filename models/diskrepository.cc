#include "diskrepository.h"

namespace repository {
    void DiskRepository::loadTo(Repository* const repo) {
        std::ifstream ifs(_filepath);
        operation::Operation op;

        while(load(ifs, op)) {
            repo->apply(op);
        } 
    }

    void DiskRepository::save(const operation::Operation& op) {
        std::ofstream ofs(_filepath, std::ios_base::app);
        ofs << (int)op.type << _separator << op.key << _separator << op.value << '\n';
        ofs.close();   
    }
    
    bool DiskRepository::load(std::ifstream& ifs, operation::Operation& op) {
        std::string line;
        if(!std::getline(ifs, line)) return false;
        
        auto sep1 = line.find(_separator);
        auto sep2 = line.find(_separator, sep1 + 1); 

        op.type = (operation::OperationType)(std::stoi(line.substr(0, sep1)));
        op.key = line.substr(sep1+1, sep2 - sep1 - 1);
        op.value = line.substr(sep2+1, line.size() - sep2);

        return true;
    }  
};