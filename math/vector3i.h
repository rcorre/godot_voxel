#ifndef VOXEL_VECTOR3I_H
#define VOXEL_VECTOR3I_H

#include "../util/utility.h"
#include <core/hashfuncs.h>
#include <core/math/vector3.h>

struct Vector3i {

	union {
		struct {
			int x;
			int y;
			int z;
		};
		int coords[3];
	};

	_FORCE_INLINE_ Vector3i() :
			x(0),
			y(0),
			z(0) {}

	_FORCE_INLINE_ Vector3i(int xyz) :
			x(xyz),
			y(xyz),
			z(xyz) {}

	_FORCE_INLINE_ Vector3i(int px, int py, int pz) :
			x(px),
			y(py),
			z(pz) {}

	_FORCE_INLINE_ Vector3i(const Vector3i &other) {
		*this = other;
	}

	_FORCE_INLINE_ Vector3i(const Vector3 &f) {
		x = Math::floor(f.x);
		y = Math::floor(f.y);
		z = Math::floor(f.z);
	}

	_FORCE_INLINE_ Vector3 to_vec3() const {
		return Vector3(x, y, z);
	}

	_FORCE_INLINE_ int volume() const {
		return x * y * z;
	}

	_FORCE_INLINE_ int length_sq() const {
		return x * x + y * y + z * z;
	}

	_FORCE_INLINE_ real_t length() const {
		return Math::sqrt((real_t)length_sq());
	}

	_FORCE_INLINE_ int distance_sq(const Vector3i &other) const;

	_FORCE_INLINE_ Vector3i &operator=(const Vector3i &other) {
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	_FORCE_INLINE_ void operator+=(const Vector3i &other) {
		x += other.x;
		y += other.y;
		z += other.z;
	}

	_FORCE_INLINE_ void operator-=(const Vector3i &other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
	}

	_FORCE_INLINE_ Vector3i operator-() const {
		return Vector3i(-x, -y, -z);
	}

	_FORCE_INLINE_ int &operator[](unsigned int i) {
		return coords[i];
	}

	_FORCE_INLINE_ const int &operator[](unsigned int i) const {
		return coords[i];
	}

	void clamp_to(const Vector3i min, const Vector3i max) {
		if (x < min.x) x = min.x;
		if (y < min.y) y = min.y;
		if (z < min.z) z = min.z;

		// TODO Not sure it should clamp like that...
		if (x >= max.x) x = max.x - 1;
		if (y >= max.y) y = max.y - 1;
		if (z >= max.z) z = max.z - 1;
	}

	_FORCE_INLINE_ bool is_contained_in(const Vector3i &min, const Vector3i &max) {
		return x >= min.x && y >= min.y && z >= min.z && x < max.x && y < max.y && z < max.z;
	}

	static void sort_min_max(Vector3i &a, Vector3i &b) {
		sort_min_max(a.x, b.x);
		sort_min_max(a.y, b.y);
		sort_min_max(a.z, b.z);
	}

	inline Vector3i udiv(const Vector3i d) const {
		return Vector3i(::udiv(x, d.x), ::udiv(y, d.y), ::udiv(z, d.z));
	}

	inline Vector3i wrap(const Vector3i d) const {
		return Vector3i(::wrap(x, d.x), ::wrap(y, d.y), ::wrap(z, d.z));
	}

	inline unsigned int get_zxy_index(const Vector3i area_size) const {
		return y + area_size.y * (x + area_size.x * z); // ZXY
	}

	static inline Vector3i from_zxy_index(unsigned int i, const Vector3i area_size) {
		Vector3i pos;
		pos.y = i % area_size.y;
		pos.x = (i / area_size.y) % area_size.x;
		pos.z = i / (area_size.y * area_size.x);
		return pos;
	}

private:
	static _FORCE_INLINE_ void sort_min_max(int &a, int &b) {
		if (a > b) {
			int temp = a;
			a = b;
			b = temp;
		}
	}
};

_FORCE_INLINE_ Vector3i operator+(const Vector3i a, const Vector3i &b) {
	return Vector3i(a.x + b.x, a.y + b.y, a.z + b.z);
}

_FORCE_INLINE_ Vector3i operator-(const Vector3i &a, const Vector3i &b) {
	return Vector3i(a.x - b.x, a.y - b.y, a.z - b.z);
}

_FORCE_INLINE_ Vector3i operator*(const Vector3i &a, const Vector3i &b) {
	return Vector3i(a.x * b.x, a.y * b.y, a.z * b.z);
}

_FORCE_INLINE_ Vector3i operator*(const Vector3i &a, int n) {
	return Vector3i(a.x * n, a.y * n, a.z * n);
}

_FORCE_INLINE_ Vector3i operator*(int n, const Vector3i &a) {
	return Vector3i(a.x * n, a.y * n, a.z * n);
}

_FORCE_INLINE_ Vector3i operator/(const Vector3i &a, int n) {
	return Vector3i(a.x / n, a.y / n, a.z / n);
}

_FORCE_INLINE_ bool operator==(const Vector3i &a, const Vector3i &b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

_FORCE_INLINE_ bool operator!=(const Vector3i &a, const Vector3i &b) {
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

_FORCE_INLINE_ Vector3i operator<<(const Vector3i &a, int b) {
	return Vector3i(a.x << b, a.y << b, a.z << b);
}

_FORCE_INLINE_ Vector3i operator>>(const Vector3i &a, int b) {
	return Vector3i(a.x >> b, a.y >> b, a.z >> b);
}

_FORCE_INLINE_ bool operator<(const Vector3i &a, const Vector3i &b) {

	if (a.x == b.x) {
		if (a.y == b.y) {
			return a.z < b.z;
		} else {
			return a.y < b.y;
		}
	} else {
		return a.x < b.x;
	}
}

_FORCE_INLINE_ int Vector3i::distance_sq(const Vector3i &other) const {
	return (other - *this).length_sq();
}

struct Vector3iHasher {
	static _FORCE_INLINE_ uint32_t hash(const Vector3i &v) {
		uint32_t hash = hash_djb2_one_32(v.x);
		hash = hash_djb2_one_32(v.y, hash);
		return hash_djb2_one_32(v.z, hash);
	}
};

#endif // VOXEL_VECTOR3I_H