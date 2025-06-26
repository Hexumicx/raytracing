#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <chrono>
#include <random>
#include <thread>

#include "color.hpp"
#include "constants.hpp"
#include "hittable.hpp"
#include "interval.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "utils.hpp"

class camera {
   public:
    double aspect_ratio = 1;
    int image_width = 100;
    int samples_per_pixel = 10;
    int max_depth = 10;
    const int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    double vfov = 90;
    vec3 lookfrom = vec3(0, 0, 0);
    vec3 lookat = vec3(0, 0, -1);
    vec3 vup = vec3(0, 1, 0);

    double defocus_angle = 0.0;
    double focus_dist = 10.0;

    void render(const hittable& world) {
        initialize();

        std::vector<color> frame_buffer(image_width * image_height);
        auto render_rows = [&](int start_row, int end_row, int thread_id) {
            for (int j = start_row; j < end_row; j++) {
                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0, 0, 0);
                    for (int sample = 0; sample < samples_per_pixel; sample++) {
                        ray r = get_ray(i, j, thread_id);
                        pixel_color += ray_color(r, max_depth, world);
                    }
                    frame_buffer[j * image_width + i] = pixel_color;
                }
            }
            std::clog << "Thread " << thread_id << " finished rendering rows "
                      << start_row << " to " << end_row - 1 << std::endl;
        };
        
        // Ensure threads hold unique rows of the image
        int rows_per_thread = image_height / num_threads;
        std::clog << "Using " << num_threads << " threads, each rendering "
                  << rows_per_thread << " rows of the image." << std::endl;
        for (int t = 0; t < num_threads; t++) {
            int start_row = t * rows_per_thread;
            int end_row = (t == num_threads - 1) ? image_height
                                                 : start_row + rows_per_thread;
            threads.emplace_back(render_rows, start_row, end_row, t);
        }

        for (int i = 0; i < num_threads; i++) {
            std::clog << "Joining thread " << i << std::endl;
            threads[i].join();
        }

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int j = 0; j < image_height; j++) {
            for (int i = 0; i < image_width; i++) {
                color pixel_color = frame_buffer[j * image_width + i];
                write_color(std::cout, pixel_color * pixel_samples_scale);
            }
        }
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
        image_height =
            image_height < 1 ? 1 : image_height;  // Ensure height is at least 1

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Camera
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width =
            viewport_height * static_cast<double>(image_width) / image_height;

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
        auto viewport_upper_left =
            center - (focus_dist * w) - (viewport_u / 2) - (viewport_v / 2);
        pixel00_loc =
            viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate defocus disk vectors
        auto defocus_radius =
            focus_dist * std::tan(degrees_to_radians(defocus_angle) / 2);
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
                return attenuation * ray_color(scattered, depth - 1, world);
            }
            return color(0, 0, 0);  // No scatter, return black
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }

    ray get_ray(int i, int j, int thread_id = 0) const {
        // Create a random ray from defocus disk
        auto offset = sample_square(thread_id);
        auto pixel_sample = pixel00_loc + (i + offset.x()) * pixel_delta_u +
                            (j + offset.y()) * pixel_delta_v;

        auto ray_origin = defocus_angle < 0 ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 defocus_disk_sample() const {
        auto disk_sample = random_in_unit_disk();
        return center + defocus_disk_u * disk_sample.x() +
               defocus_disk_v * disk_sample.y();
    }

    vec3 sample_square(int thread_id = 0) const {
        thread_local std::mt19937 rng(
            thread_id +
            std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<double> dist(-0.5, 0.5);
        return vec3(dist(rng), dist(rng), 0);
    }
};

#endif  // CAMERA_HPP