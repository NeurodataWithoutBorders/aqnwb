#pragma once

#include <vector>

class Types {
public:
    enum Status {Success = 0, Failure = -1};
    using SizeType = size_t;
    using SizeArray = std::vector<size_t>;
};