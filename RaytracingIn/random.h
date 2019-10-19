#ifndef RANDOMH
#define RANDOMH

#include <cstdlib>

inline double random_double() {
	return rand() / (RAND_MAX + 1.0);
}

inline vec3 random_in_unit_disk() {
	vec3 p;
	do {
		p = 2.0f * vec3(random_double(), random_double(), 0.0f) - vec3(1.0f, 1.0f, 0.0f);
	} while (dot(p, p) >= 1.0f);
	return p;
}

inline vec3 random_in_unit_sphere() {
	vec3 p;
	do {
		p = 2.0f * vec3(random_double(), random_double(), random_double()) - vec3(1.0f, 1.0f, 1.0f);
	} while (p.squared_length() >= 1.0f);
	return p;
}

#endif
