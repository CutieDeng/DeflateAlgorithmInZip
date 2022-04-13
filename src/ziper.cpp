#include "ziper.hpp" 
#include "crc_algorithm.hpp"
#include "static_huffman.hpp"
#include "header.hpp"

#include <fstream> 
#include <iostream> 
#include <array> 
#include <unordered_map> 

void static_deflate_compress(u8 const *, std::uint32_t , auto& );

namespace {

    ZipCollection::ZipDate get_date(std::filesystem::path const &p) {
        std::time_t t_express = std::chrono::file_clock::to_time_t(std::filesystem::last_write_time(p)); 
        auto tp = std::unique_ptr<std::tm>(std::localtime(&t_express)); 
        return {.y=(u16)(tp->tm_year-80), .m=(u16)(tp->tm_mon+1), .d=(u16)tp->tm_mday }; 
    }

    ZipCollection::ZipTime get_time(std::filesystem::path const &p) {
        std::time_t t_express = std::chrono::file_clock::to_time_t(std::filesystem::last_write_time(p)); 
        auto tp = std::unique_ptr<std::tm>(std::localtime(&t_express)); 
        return {.h=(u16)tp->tm_hour, .m=(u16)tp->tm_min, .s=(u16)tp->tm_sec};
    }
}

constexpr std::size_t LENGTH = 1 << 25; 
char static cache[LENGTH]; 

constexpr std::size_t BLOCK_LENGTH = LENGTH; 
static std::uint8_t static_deflate[BLOCK_LENGTH]; 

struct controller {
    std::uint32_t offset {}; 
    int position {}; 

    controller& operator() (std::uint32_t value, int bit_width, bool reverse = false) {
        #ifndef NDEBUG
        std::cerr << "Invoke (" << value << ", " << bit_width << ", " << (reverse?"true":"false") << "). " << std::endl; 
        #endif
        if (!reverse) {
            std::uint32_t v {}; 
            for (int i = 0; i < bit_width; ++i) {
                v <<= 1; 
                if (value & 0x1)
                    v |= 1; 
                value >>= 1; 
            }
            value = v; 
        }
        #ifndef NDEBUG
        std::cerr << "Determine to push value " << value << " with bit = " << bit_width << " to stream. " << std::endl; 
        #endif
        while (bit_width-- > 0) {
            (*this)(value & 0x1); 
            value >>= 1;
        }
        return *this; 
    }

    controller& operator() (bool b) {
        #ifndef NDEBUG
        static std::int64_t index = 0; 
        #endif 
        if (b) {
            static_deflate[offset] |= 0x1 << position; 
            #ifndef NDEBUG
            std::cerr << "Push bit 1" << std::endl; 
            #endif
        } 
        #ifndef NDEBUG 
        else {
            std::cerr << "Push bit 0 " << std::endl; 
        }
        #endif
        if (++position == 8) {
            #ifndef NDEBUG
            std::cerr << "Start a next byte. (index:" << (++index) << ")" << std::endl; 
            #endif
            position = 0; 
            static_deflate[++offset] = 0; 
        }
        return *this;
    }

    controller& operator() (std::vector<bool> const &value, bool reverse = false) {
        if (!reverse) {
            for (auto i: value) {
                (*this)(bool(i)); 
            }
        } else {
            for (auto i = value.crbegin(); i != value.crend(); ++i) {
                (*this)(bool(*i)); 
            }
        }
        return *this; 
    }

    controller& flush(auto& o) {
        o->write(reinterpret_cast<char*>(static_deflate), offset); 
        static_deflate[0] = static_deflate[offset]; 
        offset = 0; 
        return *this; 
    }

    controller& clear(auto& o) {
        this->flush(o); 
        if (position > 0) 
            o->put(static_deflate[0]); 
        position = 0; 
        static_deflate[0] = 0; 
        return *this; 
    }
} controller; 

void ziper::write_local_header(std::filesystem::path const &p, cutie_tag::stored_strategy_tag) {
    try {
        auto offset = this->out->tellp(); 
        auto name = p.native();  
        auto header_ptr = 
            std::make_unique<ZipCollection::LocalFileHeader>((ZipCollection::LocalFileHeader)
            {.version_need=20, .flag=0x0, .compress_method=0, 
                .modi_time=get_time(p), .modi_date=get_date(p), 
                .name_size=static_cast<u16>(name.size()) }); 
        std::ifstream file_stream (name, std::ios::binary); 
        u32 size {0}; 
        *this->out << *header_ptr; 
        this->out->write(name.data(), name.length()); 
        crc32_algorithm_machine machine{}; 
        while (file_stream) {
            file_stream.read(cache, LENGTH); 
            auto l = file_stream.gcount(); 
            size += l; 
            // std::cerr << size << std::endl; 
            this->out->write(cache, l); 
            machine(reinterpret_cast<uint8_t const*>(cache), l);
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
        FileHeader h {.version_made = 20, .version_need = header_ptr -> version_need, 
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
    try {
        auto offset = this->out->tellp(); 
        auto name = p.native();  
        auto header_ptr = 
            std::make_unique<ZipCollection::LocalFileHeader>((ZipCollection::LocalFileHeader)
            {.version_need=20, .flag=0x0, .compress_method=8, 
                .modi_time=get_time(p), .modi_date=get_date(p), 
                .name_size=static_cast<u16>(name.size()) }); 
        std::ifstream file_stream (name, std::ios::binary); 
        u32 uncompressed_size {0}, compressed_size{0}; 
        *this->out << *header_ptr; 
        this->out->write(name.data(), name.length()); 
        crc32_algorithm_machine crc{}; 
        while (file_stream) {
            file_stream.read(cache, LENGTH); 
            auto l = file_stream.gcount(); 
            crc(reinterpret_cast<u8*>(cache), std::uint32_t(l)); 
            uncompressed_size += l; 
            file_stream.peek(); 
            if (file_stream) {
                #ifndef NDEBUG
                std::cerr << "Find a block with another block behind. "<< std::endl; 
                #endif
                controller(false)(1, 2, true); 
                static_deflate_compress(reinterpret_cast<u8 const*>(cache), l, this->out); 
                compressed_size += controller.offset; 
                controller.flush(this->out); 
                // int head = file_stream ? 0 : 4; 
                // head |= 0x1; 
                // controller(head, 3); 
            } else {
                #ifndef NDEBUG
                std::cerr << "Find a end block. " << std::endl;
                #endif
                controller(true)(1, 2, true); 
                static_deflate_compress(reinterpret_cast<u8 const*>(cache), l, this->out); 
                compressed_size += controller.offset; 
                if (controller.position)
                    ++compressed_size;
                controller.clear(this->out);
            }
        }
        header_ptr->crc = crc; 
        header_ptr->compressed_size = compressed_size; 
        header_ptr->uncompressed_size = uncompressed_size; 
        this->file_lists.push_back({offset, name, std::move(header_ptr)}); 
    } catch (std::filesystem::filesystem_error &e) {
        std::cerr << e.what() << std::endl; 
    }
}

std::unordered_map<std::array<u8, 4>, std::uint32_t, decltype([](auto s) -> std::size_t {return s[0] | s[1] * 0x100UL | s[2] * 0x10000UL | s[3] * 0x1000000UL; }), 
    decltype([](auto x, auto y) -> bool {return x[0] == y[0] && x[1] == y[1] && x[2] == y[2] && x[3] == y[3]; })> map; 
std::uint32_t last[LENGTH]; 
std::uint32_t linkage[LENGTH]; 
std::uint32_t step[LENGTH]; 

bool contains(auto &c, auto &p) {
    return c.find(p) != c.end(); 
}

void static_deflate_compress(u8 const *p, std::uint32_t length, auto& o) {
    map.clear(); 
    std::uint32_t now_position = 0; 
    std::array<u8, 4> arrays; 
    #ifndef NDEBUG
    std::cerr << "Start a static deflate compression process. The block size is: " << length << std::endl; 
    #endif 
    for (std::uint32_t next_one = 0; now_position < length; ++now_position) {
        if (now_position + 3 < length) {
            arrays = {p[now_position], p[now_position + 1], p[now_position + 2], p[now_position + 3]}; 
            if (contains(map, arrays))
                last[now_position] = map[arrays]; 
            else 
                last[now_position] = now_position; 
            map[arrays] = now_position; 
        } else last[now_position] = now_position; 

        // Attempt to find the more match here. 
        if (now_position == next_one) {
            // calculate the information in linkage. 
            std::uint32_t pre = last[now_position]; 
            std::uint32_t len = 1; 
            std::uint32_t link = now_position; 
            if (pre != now_position)
                do {
                    if (now_position - pre > 32768)
                        break; 
                    std::uint32_t small_length = 0; 
                    {
                        for (;small_length < 256 && now_position + small_length < length && p[now_position+small_length] == p[pre+small_length];++small_length); 
                        if (len < small_length) {
                            len = small_length; 
                            link = pre; 
                        }
                    }
                    if (pre == last[pre])
                        break; 
                    pre = last[pre]; 
                } while (true); 
            linkage[now_position] = link; 
            step[now_position] = len; 
            next_one += len; 
        }
    }

    for (now_position = 0; now_position < length; now_position+=step[now_position]) {
        if (step[now_position] == 1) {
            #ifndef NDEBUG
            std::cerr << "Emit the information (literal = " << (int)(p[now_position]) << ")" << std::endl; 
            #endif
            auto [v, bw] = encode_with_static(p[now_position]); 
            controller(v, bw, false); 
        } else if (step[now_position] >= 4) {
            #ifndef NDEBUG
            std::cerr << "Emit the information (length = " << (step[now_position]) << ", dis = " 
                << (now_position - linkage[now_position]) << "). " << std::endl; 
            #endif
            // auto [v, bw, v2, bw2] = encode_length(now_position - linkage[now_position]); 
            auto [v, bw, v2, bw2] = encode_length(step[now_position]); 
            controller(v, bw, false)(v2, bw2, true); // true / false at the second place. 
            std::tie(v, bw, v2, bw2) = encode_distance(now_position - linkage[now_position]); 
            controller(v, bw, false)(v2, bw2, true); 
        } else assert (false); 
    }

    std::apply(controller, encode_with_static(256)); 
}

