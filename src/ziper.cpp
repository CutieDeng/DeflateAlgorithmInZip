#include "ziper.hpp" 
#include "crc32.hpp"
#include "header.hpp"

#include <fstream> 
#include <iostream> 
namespace {

    ZipCollection::ZipDate get_date(std::filesystem::path const &p) {
        std::time_t t_express = std::chrono::file_clock::to_time_t(std::filesystem::last_write_time(p)); 
        auto tp = std::unique_ptr<std::tm>(std::localtime(&t_express)); 
        return {.y=(u16)(tp->tm_year-80), .m=(u16)tp->tm_mon, .d=(u16)tp->tm_yday }; 
    }

    ZipCollection::ZipTime get_time(std::filesystem::path const &p) {
        std::time_t t_express = std::chrono::file_clock::to_time_t(std::filesystem::last_write_time(p)); 
        auto tp = std::unique_ptr<std::tm>(std::localtime(&t_express)); 
        return {.h=(u16)tp->tm_hour, .m=(u16)tp->tm_min, .s=(u16)tp->tm_sec};
    }
}

constexpr std::size_t LENGTH = 1 << 20; // 1 << 25; 
char static cache[LENGTH]; 

void ziper::write_local_header(std::filesystem::path const &p, cutie_tag::stored_strategy_tag) {
    try {
        auto offset = this->out->tellp(); 
        auto name = p.native();  
        auto header_ptr = 
            std::make_unique<ZipCollection::LocalFileHeader>((ZipCollection::LocalFileHeader)
            {.version_need=10, .flag=0x0, .compress_method=0, 
                .modi_time=get_time(p), .modi_date=get_date(p), 
                .name_size=static_cast<u16>(name.size()) }); 
        std::ifstream file_stream (name, std::ios::binary); 
        u16 size {0}; 
        *this->out << *header_ptr; 
        this->out->write(name.data(), name.length()); 
        if (file_stream) {
            file_stream.read(cache, LENGTH); 
            auto l = file_stream.gcount(); 
            size += l; 
            std::cout << size << std::endl; 
            this->out->write(cache, l); 
            header_ptr->crc=crc32_byte(reinterpret_cast<std::uint8_t*>(cache), std::uint32_t(l)); 
            if (!file_stream.eof()) {
                std::cout << "Read file " << name << " which is too long. " << std::endl; 
                exit(1); 
            }
        }
        header_ptr->compressed_size = 
            header_ptr->uncompressed_size = 
            size; 
        this->file_lists.push_back({offset, name, std::move(header_ptr)}); 
    } catch (std::filesystem::filesystem_error &e) {
        std::cerr << e.what() << std::endl; 
    }
}

void ziper::clear() {
    auto now_position = this->out->tellp(); 
    {
        for (auto&& f: this->file_lists) {
            this->out->seekp(std::get<0>(f)); 
            auto &&header_ptr = std::get<2>(f); 
            // this->out->write(reinterpret_cast<char*>(header_ptr.get()), sizeof (std::remove_reference_t<decltype(header_ptr)>::element_type)); 
            *this->out << *header_ptr; 
        }
        this->out->seekp(now_position); 
    }
    for (auto&& f: this->file_lists) {
        using namespace ZipCollection; 
        auto &&header_ptr = std::get<2>(f); 
        FileHeader h {.version_made = 10, .version_need = header_ptr -> version_need, 
            .flag = header_ptr -> flag, .compressing_method = header_ptr -> compress_method, 
            .modi_time = header_ptr -> modi_time, .modi_date = header_ptr -> modi_date, 
            .crc = header_ptr -> crc, .compressed_size = header_ptr -> compressed_size, 
            .uncompressed_size = header_ptr -> uncompressed_size, 
            .name_size = header_ptr -> name_size, .extra_size = header_ptr -> extra_size, 
            .comment_size = 0, .disk_number = 0, .relative_offset = static_cast<u32>(std::get<0>(f))
            }; 
        *this->out << h; 
        this->out->write(std::get<1>(f).data(), header_ptr->name_size); 
    }
    if (!this->file_lists.empty()) {
        using namespace ZipCollection; 
        EndDirectoryRecord record = {.disk_number = 1, .central_starts = 0, 
            .directory_records = static_cast<u16>(this->file_lists.size()), 
            .total_records = static_cast<u16>(this->file_lists.size()), 
            .central_size = static_cast<u32>(this->out->tellp() - now_position), 
            .central_offset = static_cast<u32>(now_position), 
        }; 
        *this->out << record; 
    }
}

ziper::~ziper() {
    this->clear(); 
}

void ziper::write_local_header(std::filesystem::path const &p, cutie_tag::deflate_strategy_tag) {
    
}