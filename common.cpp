#include "common.h"

void rev_comp_inplace(std::string & s) {
    std::string tmp = s; int l = s.length();
    for(int i = 0; i < l; ++i) {
        s[i] = COMPLEMENT[(int)tmp[l-i-1]];
    }
}
