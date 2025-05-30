#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "hittable.hpp"
#include "ray.hpp"
#include "color.hpp"
#include "interval.hpp"
#include "material.hpp"
#include "constants.hpp"
#include "utils.hpp"

class camera {
    public:
        double aspect_ratio = 1;
        int image_width = 100;
        int samples_per_pixel = 10;
        int max_depth = 10;

        double vfov = 90;
        vec3 lookfrom = vec3(0, 0, 0);
        vec3 lookat = vec3(0, 0, -1);
        vec3 vup = vec3(0, 1, 0);

        double defocus_angle = 0.0;
        double focus_dist = 10.0;  

        void render(const hittable& world) {
            initialize();

            std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

            for (int j = 0; j < image_height; j++) {
                std::clog << "\rScanlines remaining: " << image_height - j << ' ' << std::flush;
                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0, 0, 0);
                    for (int sample = 0; sample < samples_per_pixel; sample++) {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }
                    write_color(std::cout, pixel_samples_scale * pixel_color);
                }
            }
            std::clog << "\rDone\n";
        }

    private: 
        int image_height;
        double pixel_samples_scale;
        point3 center;
        point3 pixel00_loc;
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;
        vec3 u, v, w;
        vec3 defocus_disk_u;
        vec3 defocus_disk_v;

        void initialize() {
            image_height = static_cast<int>(image_width / aspect_ratio);
            image_height = image_height < 1 ? 1 : image_height; // Ensure height is at least 1

            pixel_samples_scale = 1.0 / samples_per_pixel;

            center = lookfrom;
            
            // Camera
            auto theta = degrees_to_radians(vfov);
            auto h = std::tan(theta / 2);
            auto viewport_height = 2 * h * focus_dist;
            auto viewport_width = viewport_height * static_cast<double>(image_width) / image_height;

            // Determin u, v, w
            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);

            // Viewport vectors
            auto viewport_u = viewport_width * u;
            auto viewport_v = viewport_height * -v;

            // Pixel delta
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            // Upper left corner pixel location
            auto viewport_upper_left = center - (focus_dist * w) - (viewport_u / 2) - (viewport_v / 2);
            pixel00_loc = viewport_upper_left + 0.5*(pixel_delta_u + pixel_delta_v);

            // Calculate defocus disk vectors
            auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle) / 2);
            defocus_disk_u = defocus_radius * u;
            defocus_disk_v = defocus_radius * v;
        }

        color ray_color(const ray& r, int depth, const hittable& world) {
            if (depth <= 0) {
                return color(0, 0, 0);
            }
            hit_record rec;
            if (world.hit(r, interval(0.001, infinity), rec)) {
                ray scattered;
                color attenuation;
                if (rec.mat->scatter(r, rec, attenuation, scattered)) {
                    return attenuation * ray_color(scattered, depth-1, world);
                }
                return color(0, 0, 0); // No scatter, return black
            }

            vec3 unit_direction = unit_vector(r.direction());
            auto a = 0.5 * (unit_direction.y() + 1.0);
            return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
        }

        ray get_ray(int i, int j) const {
            // Create a random ray from defocus disk
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc + (i+offset.x()) * pixel_delta_u + (j+offset.y()) * pixel_delta_v;

            auto ray_origin = defocus_angle < 0 ? center : defocus_disk_sample();
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);
        }

        vec3 defocus_disk_sample() const {
            auto disk_sample = random_in_unit_disk();
            return center + defocus_disk_u * disk_sample.x() + defocus_disk_v * disk_sample.y();
        }
        
        vec3 sample_square() const {
            return vec3(random_double()-0.5, random_double()-0.5, 0);
        }
};

#endif // CAMERA_HPP