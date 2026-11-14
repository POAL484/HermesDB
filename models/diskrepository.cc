#include "diskrepository.h"
#include <string_view>
#include <ranges>
#include <snappy.h>

namespace repository {
    void DiskRepository::loadTo(Repository* const repo) {
        std::ifstream ifs(_filepath);
        operation::Operation op;

        while(load(ifs, op)) {
            repo->apply(op);
        } 
    }

    void DiskRepository::save(const operation::Operation& op) {
        std::stringstream oss;
        oss << (int)op.type << _separator << op.key;
        for(auto value : op.value) oss << _separator << value.first << _separator << (int)value.second;
        std::string str = oss.str();
        std::string compressed;
        snappy::Compress(str.data(), str.size(), &compressed);

        std::ofstream ofs(_filepath, std::ios_base::app);
        ofs << compressed << '\n';
        ofs.close();   
    }
    
    bool DiskRepository::load(std::ifstream& ifs, operation::Operation& op) {
        std::string line;
        if(!std::getline(ifs, line)) return false;
        
        std::string uncompresed;
        snappy::Uncompress(line.data(), line.size(), &uncompresed);
        
        auto sp = std::views::split(uncompresed, _separator) | std::ranges::to<std::vector<std::string>>();
        op.type = (operation::OperationType)(std::stoi(sp[0]));
        op.key = sp[1];

        for(int i = 2; i+1 < sp.size(); i += 2) {
            op.value.push_back({sp[i], (types::Type)(std::stoi(sp[i+1]))});
        }

        return true;
    }  
};