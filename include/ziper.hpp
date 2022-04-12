#pragma once 

#include <iosfwd>
#include <memory>  
#include <vector> 
#include <tuple> 
#include <filesystem>
#include <functional>

#include "tag.hpp"
#include "header.hpp"

class ziper {
    std::unique_ptr<std::ostream, std::function<void (std::ostream*)>> out; 
    std::vector<std::tuple<std::ios::pos_type, std::string, 
        std::unique_ptr<ZipCollection::LocalFileHeader>>>
        file_lists; 
    
    public: 
        ziper(std::ostream *o, void (p)(std::ostream*)): out(o, p) {}; 
        void write_local_header(std::filesystem::path const &, cutie_tag::stored_strategy_tag); 
        void write_local_header(std::filesystem::path const &, cutie_tag::deflate_strategy_tag); 
        void clear(); 
        ~ziper(); 
}; 