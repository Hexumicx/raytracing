#ifndef UTILS_HPP
#define UTILS_HPP

#include "constants.hpp"

#include <cstdlib>

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

#endif // UTILS_HPP