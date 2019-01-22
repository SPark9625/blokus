#ifndef utils_hpp
#define utils_hpp

#include <sstream>
#include <string>

namespace util {

inline std::stringstream get_text() {
std::string raw_text =
R"({0
    14
    1111
}
{1
    24
    1111
    0001
}
{2
    24
    1111
    0010
}
{3
    32
    10
    10
    11
}
{4
    32
    11
    11
    10
}
{5
    33
    100
    100
    111
}
{6
    33
    110
    010
    011
}
{7
    33
    010
    111
    010
}
{8
    32
    10
    11
    10
}
{9
    32
    11
    10
    11
}
{10
    33
    011
    110
    010
}
{11
    33
    111
    010
    010
}
{12
    22
    11
    11
}
{13
    23
    110
    011
}
{14
    22
    10
    11
}
{15
    33
    100
    110
    011
}
{16
    11
    1
}
{17
    15
    11111
}
{18
    24
    1110
    0011
}
{19
    13
    111
}
{20
    12
    11
})";

    return std::stringstream(raw_text);
}

inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

} // namespace util

#endif // utils_hpp
