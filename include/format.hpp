#pragma once 

namespace cutie {
    template <typename ...Args>
    inline void inner_format(std::stringstream &a, std::string_view s, int, Args const &...args); 

    template <typename T> 
    inline void to_string(std::ostream &, T const &); 

    template <> 
    inline void inner_format(std::stringstream &a, std::string_view s, int offset) {
        while (s[offset]) {
            a << s[offset++]; 
        }
    }

    template <typename T, typename ...Args>
    inline void inner_format(std::stringstream &a, std::string_view s, int offset, T const &arg, Args const &...args) {
        bool flag = false; 
        bool ignore = false; 
        while (s[offset]) {
            switch (s[offset]) {
                case '{': {
                    if (ignore) 
                        throw std::runtime_error("Invalid '}{' matching. "); 
                    if (flag) 
                        a << '{'; 
                    flag = !flag; 
                    break; 
                }
                case '}': {
                    if (flag) {
                        cutie::to_string(a, arg);
                        inner_format(a, s, offset + 1, args...); 
                        return ; 
                    } else {
                        if (ignore)
                            a << '}'; 
                        ignore = !ignore; 
                    }
                }
                default: {
                    a << s[offset];
                    flag = false; 
                    ignore = false; 
                }
            }
            ++offset;
        }
    }

    template <typename T>
    inline void to_string(std::ostream &o, T const &a) {
        o << a; 
        return ; 
    }

    template <> 
    inline void to_string(std::ostream &o, tree_node const &t) {
        o << "[tree_node value = " << t.data << ", n_count = " << t.node_count << 
            ", t_count = " << t.tree_count << ". Relation(left: "; 
        if (!t.l_child)  
            o << "un"; o << "exists, right: "; 
        if (!t.r_child)
            o << "un"; o << "exists, father: "; 
        if (!t.father)
            o << "un"; o << "exists.]"; 
        return ; 
    }
}

inline namespace {
    template <typename ...Args> 
    inline std::string format(std::string_view a, Args const & ...b)  {
        std::stringstream s; 
        cutie::inner_format(s, a, 0, b...); 
        return s.str(); 
    }
}