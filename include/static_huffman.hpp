#include <cstdint> 
#include <tuple> 

inline std::pair<std::uint32_t, int> encode_with_static(int origincode) {
    assert (origincode >= 0); 
    assert (origincode <= 287); 
    if (origincode <= 143) {
        return {origincode + 0b110000, 8}; 
    } else if (origincode <= 255) 
        return {origincode - 144 + 0b110010000, 9}; 
    else if (origincode <= 279) 
        return {origincode - 256, 7}; 
    else 
        return {origincode - 280 + 0b11000000, 8}; 
}

namespace length {
    inline int length_array [256]; 
    inline int end_fix [256]; 
    inline int length_bound [] {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258}; 
    inline int bit_bound [] {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0}; 

    inline constexpr int number = sizeof length_bound / sizeof (int); 
    inline constexpr int number_ = sizeof bit_bound / sizeof (int); 
    static_assert (number == number_); 

    inline std::tuple<std::uint32_t, int, std::uint32_t, int> encode_length(int origin_length) {
        static bool init_operation = []() {
            int k = 0; 
            for (int i = 0; i < 256; ++i) {
                // the actual value: i + 3 
                if (k + 1 < number && length_bound[k+1] == i + 3) 
                    ++k; 
                // actual length array = k + 257 
                length_array[i] = k; 
                end_fix[i] = i + 3 - length_bound[k]; 
                assert (end_fix[i] >= 0); 
            }
            return true; 
        }(); 
        assert (origin_length >= 3); 
        assert (origin_length <= 258); 
        return std::tuple_cat(encode_with_static(length_array[origin_length - 3] + 257), 
            std::make_pair(end_fix[origin_length - 3], bit_bound[length_array[origin_length - 3]])); 
    }
}

using length::encode_length; 

namespace distance {
    inline int dis[32768]; // The code index of the dis x+1. 
    inline int end_fix[32768]; // The endfix of the value x+1. 
    inline int length_bound [] {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 
        2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577}; 
    inline int bit_bound [] {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13}; 
    inline constexpr int number = sizeof length_bound / sizeof (int); 
    inline constexpr int number_ = sizeof bit_bound / sizeof (int); 
    static_assert (number == number_); 
    inline std::tuple<std::uint32_t, int, std::uint32_t, int> encode_distance(int dis) {
        static bool init_operation = []() {
            int k = 0; 
            for (int i = 0; i < 32768; ++i) {
                // the actual value: i + 1 
                if (k + 1 < number && length_bound[k+1] == i + 1) 
                    ++k; 
                // actual length array = k + 257 
                distance::dis[i] = k; 
                end_fix[i] = i + 1 - length_bound[k]; 
                assert (end_fix[i] >= 0); 
            }
            return true; 
        }(); 
        assert (dis >= 1); 
        assert (dis <= 32768);  
        return std::tuple_cat(std::make_pair(distance::dis[dis-1], 5), 
            std::make_pair(end_fix[dis-1], bit_bound[distance::dis[dis-1]])); 
    }
}

using distance::encode_distance; 
