#pragma once
#include "KamataEngine.h"
#include <numbers>

namespace KamataEngine {

// ベクトルの加算
Vector3 Add(const Vector3& v1, const Vector3& v2);

// ベクトルの減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2);

// ベクトルのスカラー倍
Vector3 Multiply(float scalar, const Vector3& v);

// ベクトルの内積
float Dot(const Vector3& v1, const Vector3& v2);

// ベクトルの長さ
float Length(const Vector3& v);

// ベクトルの正規化
Vector3 Normalize(const Vector3& v);

// クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2);

inline Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	Vector3 result = Add(v1, v2);
	return result;
}

inline Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	Vector3 result = Subtract(v1, v2);
	return result;
}

inline Vector3 operator*(float scalar, const Vector3& v) {
	Vector3 result = Multiply(scalar, v);
	return result;
}

inline Vector3 operator*(const Vector3& v, float scalar) {
	Vector3 result = Multiply(scalar, v);
	return result;
}

inline Vector3& operator+=(Vector3& left, const Vector3& right) {
	left = left + right;
	return left;
}

inline Vector3& operator-=(Vector3& left, const Vector3& right) {
	left = left - right;
	return left;
}

inline Vector3 operator/(const Vector3& v, const float scalar) {
	return Multiply(1.0f / scalar, v);
}

inline float Lerp(float start, float end, float t) { return start + (end - start) * t; }
inline Vector3 Vector3Lerp(const Vector3& start, const Vector3& end, float t) { return start + (end - start) * t; }

} // namespace KamataEngine

static inline const float kPi = std::numbers::pi_v<float>;