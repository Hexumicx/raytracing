#ifndef COLOR_HPP
#define COLOR_HPP

#include <iostream>

#include "vec3.hpp"
#include "interval.hpp"

using color = vec3;

inline double linear_to_gamma(double linear_component) {
    if (linear_component > 0){
        return std::sqrt(linear_component);
    }
    return 0;
}

void write_color(std::ostream &out, color pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);
    
    //scale color to [0, 255]
    static const interval intensity(0.0, 0.999);
    auto rbyte = static_cast<int>(255.999 * intensity.clamp(r));
    auto gbyte = static_cast<int>(255.999 * intensity.clamp(g));
    auto bbyte = static_cast<int>(255.999 * intensity.clamp(b));

    //write the translated [0,255] value of each color component
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif // COLOR_H