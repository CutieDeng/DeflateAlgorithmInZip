#pragma once 

#include <iosfwd>
#include <exception>
#include <sstream> 
#include <memory> 
#include <type_traits>
#include <filesystem> 

namespace {
    typedef std::uint8_t u8; 
    typedef std::uint16_t u16; 
    typedef std::uint32_t u32; 
    typedef std::uint64_t u64; 
}

namespace ZipCollection {

    struct ZipDate {
        u16 d: 5; // Day of a month: [1, 31]. 
        u16 m: 4; // Month: [1, 12] 
        u16 y: 7; // Year: [1980 - 2107] 
    }; 

    struct ZipTime {
        u16 s: 5; // Second - [0, 59] 
        u16 m: 6; // Minute - [0, 59]
        u16 h: 5; // Hour - [0, 23] 
    }; 

    inline std::ostream &operator << (std::ostream &o, ZipDate const &d) {
        std::stringstream str; 
        str << d.y + 1980 << " "; 
        if (d.m < 10) 
            str << '0'; 
        str << d.m << " "; 
        if (d.d < 10) 
            str << '0';
        str << d.d; 
        return o << str.str(); 
    }

    inline std::ostream &operator << (std::ostream &o, ZipTime const &t) {
        std::stringstream str; 
        if (t.h < 10)
            str << '0'; 
        str << t.h << ":"; 
        if (t.m < 10) 
            str << '0'; 
        str << t.m << ":";
        if (t.s < 5) 
            str << '0'; 
        str << (t.s << 1); 
        return o << str.str(); 
    }

    inline namespace local_file_header {

        #pragma pack(2)
        struct LocalFileHeader { 
            constexpr static u32 MAGIC{0x04034b50}; 
            u32 magic {MAGIC}; // The magic number. 
            u16 version_need; // The version needed to extract. 
            u16 flag; // The general purpose bit flag. 
            u16 compress_method; // 0: store, 8: deflate 
            ZipTime modi_time; // Last modifiled time. 
            ZipDate modi_date; // Last modifiled date. 
            u32 crc; // CRC-32 verification code. 
            u32 compressed_size; // The data size after compressed, with byte unit. 
            u32 uncompressed_size; // The data size before compressed. 
            u16 name_size; // File name length. 
            u16 extra_size; // Extra field length 
            inline operator std::string() const ; 
        }; 
        #pragma pack()

        inline LocalFileHeader::operator std::string() const {
            std::stringstream str; 
            str.setf(std::ios::showbase); 
            str << "Zip file header magic number: " << std::hex << magic << "\n" 
                << "Version needed to extract: " << std::dec << version_need << "\n"
                << "General Flag: " << std::hex << flag << "\n"
                << "Compress Method: " << std::hex << compress_method << "\n" 
                << "Modified time: " << modi_time << "\n" 
                << "Modified date: " << modi_date << "\n"
                << "CRC-32: " << std::hex << crc << "\n"
                << "Compressed size: " << std::dec << compressed_size << " bytes. " << "\n"
                << "Uncompressed size: " << std::dec << uncompressed_size << " bytes. " << "\n" 
                << "The file name size: " << std::dec << name_size << "\n" 
                << "The extra field size: " << std::dec << extra_size << "\n"; 
            return str.str(); 
        }

    }

    inline namespace file_header {

        #pragma pack(2)
        struct FileHeader {
            constexpr static u32 MAGIC {0x02014b50}; 
            u32 magic {MAGIC}; 
            u16 version_made; 
            u16 version_need; 
            u16 flag; 
            u16 compressing_method; 
            ZipTime modi_time; 
            ZipDate modi_date; 
            u32 crc; 
            u32 compressed_size; 
            u32 uncompressed_size; 
            u16 name_size; 
            u16 extra_size; 
            u16 comment_size; 
            u16 disk_number; 
            u16 internal_file_attribute; 
            u32 external_file_attribute; 
            u32 relative_offset; 
            inline operator std::string() const; 
        }; 
        #pragma pack() 

        inline FileHeader::operator std::string() const {
            std::stringstream str; 
            str.setf(std::ios::showbase); 
            str << "Zip Central Directory Entry Magic Number: " << std::hex << this -> magic << "\n" 
                << "The Version made: " << std::dec << version_made << "\n" 
                << "The version needed: " << std::dec << version_need << "\n" 
                << "Flag: " << std::hex << flag << "\n" 
                << "Compressing method: " << std::hex << compressing_method << "\n" 
                << "Last Modified Time: " << modi_time << "\n" 
                << "Last Modified Date: " << modi_date << "\n" 
                << "CRC-32: " << std::hex << "\n" 
                << "Compressed size: " << compressed_size << "\n" 
                << "Uncompressed size: " << uncompressed_size << "\n" 
                << "Name Length: " << name_size << "\n" 
                << "Extra Field Size: " << extra_size << "\n" 
                << "Comment Field Size: " << comment_size << "\n"; 
            return str.str(); 
        }

    }

    class unmatch_exception: std::runtime_error {
    }; 

    // The template operator method would read for the specific structure. 
    // When it meets a failure reading: it would attempt to recover the reading situation and throw an runtime_error for reading fail. 
    template <typename T> 
    requires requires (T t) {
        T::MAGIC; 
        std::is_pod<T>::value; 
    }
    inline std::istream &operator >> (std::istream &i, T &z) {
        constexpr static size_t LENGTH {sizeof z}; 
        i.read(reinterpret_cast<char *>(&z), LENGTH); 
        if (auto l = i.gcount(); l < LENGTH || z.magic != T::MAGIC) {
            i.seekg(-l, std::ios_base::cur); 
        }
        return i; 
    }

    template <typename T> 
    requires requires (T t) {
        T::MAGIC; 
    }
    inline std::ostream &operator << (std::ostream &o, T const &t) {
        constexpr static size_t LENGTH {sizeof t}; 
        return o.write(reinterpret_cast<char const *>(&t), LENGTH); 
    }

    template  
    std::ostream &operator<< (std::ostream&, LocalFileHeader const&); 

    template  
    std::ostream &operator<< (std::ostream&, FileHeader const&); 

    template <typename T> 
    requires requires (T t) {
        std::is_same<decltype(t.modi_time), ZipTime>::value; 
        std::is_same<decltype(t.modi_date), ZipDate>::value; 
    }
    inline void set_modi_time(T &e, std::filesystem::file_time_type const &t) {
        std::time_t t_express = std::chrono::file_clock::to_time_t(t); 
        auto tp = std::unique_ptr<std::tm>(std::localtime(&t_express)); 
        e.modi_time = {.h = (u16)tp->tm_hour, .m = (u16)tp->tm_min, .s = (u16)tp->tm_sec }; 
        e.modi_date = {.y = (u16)(tp->tm_year - 80), .m = (u16)tp->tm_mon, .d = (u16)tp->tm_yday };
    }

    inline namespace end_directory_record {

        #pragma pack(2)
        struct EndDirectoryRecord {
            constexpr static u32 MAGIC {0x06054b50}; 
            u32 magic {MAGIC}; // End of central directory signature = 0x06054b50
            u16 disk_number; // Number of this disk (or 0xffff for ZIP64)
            u16 central_starts; // Disk where central directory starts (or 0xffff for ZIP64)
            u16 directory_records; // Number of central directory records on this disk (or 0xffff for ZIP64)
            u16 total_records; // Total number of central directory records (or 0xffff for ZIP64)
            u32 central_size; // Size of central directory (bytes) (or 0xffffffff for ZIP64)
            u32 central_offset; // Offset of start of central directory, relative to start of archive (or 0xffffffff for ZIP64)
            u16 comment_size; // Comment length (n)
            inline operator std::string() const; 
        }; 
        #pragma pack() 

        inline EndDirectoryRecord::operator std::string() const {
            std::stringstream str; 
            str.setf(std::ios::showbase); 
            str << "Zip End of Central Directory Record Magic Number: " << std::hex << this -> magic << "\n" 
                << "Number of this disk: " << std::hex << disk_number << "\n" 
                << "Central Directory starts: " << std::dec << central_starts << "\n" 
                << "Directory Records: " << std::dec << directory_records << "\n" 
                << "Total directory records: " << std::dec << total_records << "\n" 
                << "Central directory size: " << std::dec << central_size << "\n" 
                << "Central directory offset: " << std::hex << central_offset << "\n" 
                << "Comment size: " << std::dec << comment_size << "\n"; 
            return str.str(); 
        }
        
    }

}
