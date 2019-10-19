#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <omp.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "sphere.h"
#include "hitablelist.h"
#include "camera.h"
#include "material.h"
#include "random.h"

using namespace std;

vec3 color(const ray& r, hitable* world, int const depth) {
	hit_record rec;
	if (world->hit(r, 0.001f, numeric_limits<float>::max(), rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation * color(scattered, world, depth + 1);
		}
		else {
			return vec3(0.0f, 0.0f, 0.0f);
		}
	}
	else {
		vec3 unit_direction{ unit_vector(r.direction()) };
		float const t{ 0.5f * (unit_direction.y() + 1.0f) };
		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
	}

}

hitable* random_scene() {
	int n = 500;
	hitable** list = new hitable * [n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = random_double();
			vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {  // diffuse
					list[i++] = new sphere(
						center, 0.2,
						new lambertian(vec3(random_double() * random_double(),
							random_double() * random_double(),
							random_double() * random_double()))
					);
				}
				else if (choose_mat < 0.95) { // metal
					list[i++] = new sphere(
						center, 0.2,
						new metal(vec3(0.5 * (1 + random_double()),
							0.5 * (1 + random_double()),
							0.5 * (1 + random_double())),
							0.5 * random_double())
					);
				}
				else {  // glass
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new hitable_list(list, i);
}

int main() {
	streambuf* coutbuf = std::cout.rdbuf();
	ofstream out("out.ppm");
	cout.rdbuf(out.rdbuf());

	int nx = 1000;
	int ny = 600;
	int ns = 1000;
	std::cout << "P3\n" << nx << " " << ny << "\n255\n";
	hitable* world = random_scene();

	vec3 lookfrom(13.0f, 2.0f, 3.0f);
	vec3 lookat(0.0f, 0.0f, 0.0f);
	float dist_to_focus = 10.0f;
	float aperture = 0.1f;
	float const M_NX { 1.0f / static_cast<float>(nx)};
	float const M_NY { 1.0f / static_cast<float>(ny)};
	float const M_NS { 1.0f / static_cast<float>(ns)};
	camera cam(lookfrom, lookat, vec3(0.0f, 1.0f, 0.0f), 20.0f, float(nx) / float(ny), aperture, dist_to_focus);
	vector<vector<vector<int>>> image;
	image.resize(ny);
	
	#pragma omp parallel for
	for (int j{ 0 }; j < ny; ++j) {
		image[j].resize(nx);
		for (int i{ 0 }; i < nx; ++i) {
			image[j][i].resize(3);
		}
	}

	#pragma omp parallel for
	for (int j{ 0 }; j < ny; ++j) {
		float u, v;
		ray r;
		float const floatJ{ static_cast<float>(j) };
		vec3 col(0.0f, 0.0f, 0.0f);
		for (int i{ 0 }; i < nx; ++i) {
			float const floatI{ static_cast<float>(i) };
			col[0] = 0.0f; 
			col[1] = 0.0f; 
			col[2] = 0.0f;
			for (int s = 0; s < ns; ++s) {
				u = float(floatI + random_double()) * M_NX;
				v = float(floatJ + random_double()) * M_NY;
				r = cam.get_ray(u, v);
				col += color(r, world, 0);
			}
			col *= M_NS;
			col[0] = sqrtf(col[0]); 
			col[1] = sqrtf(col[1]); 
			col[2] = sqrtf(col[2]);
			image[j][i][0] = int(255.99f * col[0]);
			image[j][i][1] = int(255.99f * col[1]);
			image[j][i][2] = int(255.99f * col[2]);

		}
	}

	for (int j{ ny - 1 }; j >= 0; --j) {
		for (int i{ 0 }; i < nx; ++i) {
			cout << image[j][i][0] << " " << image[j][i][1] << " " << image[j][i][2] << "\n";
		}
	}

	cout.rdbuf(coutbuf);
}