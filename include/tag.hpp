#pragma once 

namespace cutie_tag {
    struct stored_strategy_tag {}; 
    struct deflate_strategy_tag {}; 
}

inline cutie_tag::stored_strategy_tag stored_strategy_tag; 
inline cutie_tag::deflate_strategy_tag deflate_strategy_tag; 