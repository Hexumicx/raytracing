#ifndef INTERVAL_HPP
#define INTERVAL_HPP

#include "constants.hpp"

class interval {
    public:
        double min;
        double max;

        interval() : min(+infinity), max(-infinity) {}

        interval(double min, double max) : min(min), max(max) {}

        double size() const {
            return max - min;
        }

        bool contains(double value) const {
            return min <= value && value <= max;
        }

        bool surrounds(double value) const {
            return min < value && value < max;
        }
        
        double clamp(double value) const {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }
        
        static const interval empty, universe;
};

const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

#endif // INTERVAL_HPP