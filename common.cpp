#include "common.h"

void rev_comp_inplace(std::string & s) {
    std::string tmp = s; size_t l = s.length();
    for(size_t i = 0; i < l; ++i) {
        s[i] = COMPLEMENT[(int)tmp[l-i-1]];
    }
}
