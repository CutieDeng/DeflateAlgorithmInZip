#include <iostream> 
#include <fstream> 
#include <type_traits>
#include <filesystem> 
#include <vector> 

#define NDEBUG

#include "ziper.hpp"

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << "There is no argument input. " << std::endl; 
        return 0; 
    }

    using std::filesystem::path; 

    ziper z (new std::ofstream("ans.zip", std::ios::binary), [](auto p) -> void {delete p; }); 

    for (auto s: reinterpret_cast<char *(&)[argc-1]>(*(argv+1))) {
        z.write_local_header(std::string(s), deflate_strategy_tag); 
    }
}