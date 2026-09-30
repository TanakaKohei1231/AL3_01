#include "Collision.h"
#include <cmath>
#include <algorithm>
#include <limits>

using namespace KamataEngine;

Vector3 Project(const Vector3 &v1, const Vector3 &v2) {
	float dot = Dot(v1, v2);
	float lengthSquared = Dot(v2, v2);

	if (lengthSquared == 0.0f) {
		return {};
	}

	float t = dot / lengthSquared;

	return Multiply(t, v2);
}

Vector3 ClosestPoint(const Vector3 &point, const Segment &segment) {
	Vector3 originToPoint = Subtract(point, segment.origin);

	float dot = Dot(originToPoint, segment.diff);
	float lengthSquared = Dot(segment.diff, segment.diff);

	if (lengthSquared == 0.0f) {
		return segment.origin;
	}

	float t = dot / lengthSquared;

	if (t < 0.0f) {
		t = 0.0f;
	}

	if (t > 1.0f) {
		t = 1.0f;
	}

	return Add(segment.origin, Multiply(t, segment.diff));
}

bool IsCollision(const Sphere &s1, const Sphere &s2) {
	// 2つの球の中心点間の距離を求める
	float distance = Length(Subtract(s2.center, s1.center));

	// 半径の合計よりも短ければ衝突
	if (distance <= s1.radius + s2.radius) {
		return true;
	}

	return false;
}

bool IsCollision(const Sphere &sphere, const Plane &plane) {
	// 平面と球の中心点との符号付き距離を求める
	float signedDistance = Dot(plane.normal, sphere.center) - plane.distance;

	// 距離は絶対値で扱う
	float distance = std::abs(signedDistance);

	// 平面と球の中心点との距離が球の半径以下なら衝突
	if (distance <= sphere.radius) {
		return true;
	}

	return false;
}

bool IsCollision(const Line &line, const Plane &plane) {
	// 法線と線の内積を求める
	float dot = Dot(plane.normal, line.diff);

	// 内積が垂直ということは線は平面と平行なので衝突しない
	if (dot == 0.0f) {
		return false;
	}
	
	// 直線は無限に続くため平行でなければ衝突する
	return true;
}

bool IsCollision(const Ray &ray, const Plane &plane) {
	// 法線と線の内積を求める
	float dot = Dot(plane.normal, ray.diff);

	// 内積が垂直ということは線は平面と平行なので衝突しない
	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distance - Dot(plane.normal, ray.origin)) / dot;

	// tが0以上なら半直線と平面は衝突する
	if (t >= 0.0f) {
		return true;
	}

	return false;
}

bool IsCollision(const Segment &segment, const Plane &plane) {
	// 法線と線の内積を求める
	float dot = Dot(plane.normal, segment.diff);

	// 内積が垂直ということは線は平面と平行なので衝突しない
	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distance - Dot(plane.normal, segment.origin)) / dot;

	// tが0から1の範囲にある場合、線分と平面は衝突する
	if (t >= 0.0f && t <= 1.0f) {
		return true;
	}

	return false;
}

bool IsCollision(const Segment &segment, const Triangle &triangle) {
	// 三角形の2辺を求める
	Vector3 v01 = triangle.vertices[1] - triangle.vertices[0];
	Vector3 v12 = triangle.vertices[2] - triangle.vertices[1];

	// 三角形の法線を求める
	Vector3 cross = Cross(v01, v12);
	Vector3 normal = Normalize(cross);

	// 三角形の存在する平面を求める
	Plane plane;
	plane.normal = normal;
	plane.distance = Dot(normal, triangle.vertices[0]);

	// 線分と平面の衝突判定を行い、偽なら衝突しない
	if (!IsCollision(segment, plane)) {
		return false;
	}

	// 法線と線の内積を求める
	float dot = Dot(plane.normal, segment.diff);

	// tを求める
	float t = (plane.distance - Dot(plane.normal, segment.origin)) / dot;

	// 衝突点を求める
	Vector3 collisionPoint = segment.origin + Multiply(t, segment.diff);

	// 各辺を結んだベクトルと、頂点と衝突点pを結んだベクトルのクロス積を取る
	Vector3 v20 = triangle.vertices[0] - triangle.vertices[2];

	Vector3 v0p = collisionPoint - triangle.vertices[0];
	Vector3 v1p = collisionPoint - triangle.vertices[1];
	Vector3 v2p = collisionPoint - triangle.vertices[2];

	Vector3 cross01 = Cross(v01, v0p);
	Vector3 cross12 = Cross(v12, v1p);
	Vector3 cross20 = Cross(v20, v2p);

	// すべての小三角形のクロス積と法線が同じ方向を向いていたら衝突
	if (Dot(cross01, normal) >= 0.0f && Dot(cross12, normal) >= 0.0f && Dot(cross20, normal) >= 0.0f) {
		return true;
	}

	return false;
}

bool IsCollision(const AABB &aabb1, const AABB &aabb2) {
	if (aabb1.max.x < aabb2.min.x || aabb2.max.x < aabb1.min.x) {
		return false;
	}

	if (aabb1.max.y < aabb2.min.y || aabb2.max.y < aabb1.min.y) {
		return false;
	}

	if (aabb1.max.z < aabb2.min.z || aabb2.max.z < aabb1.min.z) {
		return false;
	}

	return true;
}

bool IsCollision(const Sphere &sphere, const AABB &aabb) {
	// 最近点を求める
	Vector3 closestPoint{
		std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
		std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
		std::clamp(sphere.center.z, aabb.min.z, aabb.max.z)
	};

	// 最近点と球の中心点との距離を求める
	float distance = Length(closestPoint - sphere.center);

	// 距離が球の半径よりも小さければ衝突
	if (distance <= sphere.radius) {
		return true;
	}

	return false;

}

bool IsCollision(const AABB &aabb, const Segment &segment) {
	float tMin = 0.0f;
	float tMax = 1.0f;

	// X軸
	if (segment.diff.x == 0.0f) {
		if (segment.origin.x < aabb.min.x || segment.origin.x > aabb.max.x) {
			return false;
		}
	} else {
		float t1 = (aabb.min.x - segment.origin.x) / segment.diff.x;
		float t2 = (aabb.max.x - segment.origin.x) / segment.diff.x;

		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);

		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);

		if (tMin > tMax) {
			return false;
		}
	}

	// Y軸
	if (segment.diff.y == 0.0f) {
		if (segment.origin.y < aabb.min.y || segment.origin.y > aabb.max.y) {
			return false;
		}
	} else {
		float t1 = (aabb.min.y - segment.origin.y) / segment.diff.y;
		float t2 = (aabb.max.y - segment.origin.y) / segment.diff.y;

		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);

		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);

		if (tMin > tMax) {
			return false;
		}
	}

	// Z軸
	if (segment.diff.z == 0.0f) {
		if (segment.origin.z < aabb.min.z || segment.origin.z > aabb.max.z) {
			return false;
		}
	} else {
		float t1 = (aabb.min.z - segment.origin.z) / segment.diff.z;
		float t2 = (aabb.max.z - segment.origin.z) / segment.diff.z;

		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);

		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);

		if (tMin > tMax) {
			return false;
		}
	}

	return true;
}

namespace {
// AABBと直線の衝突判定の内部処理
// 直線・半直線・線分の衝突判定の共通部分
bool IsCollisionAABBLineInternal(const AABB &aabb, const Vector3 &origin, const Vector3 &diff, float tMin, float tMax) {
	// X軸
	if (diff.x == 0.0f) {
		if (origin.x < aabb.min.x || origin.x > aabb.max.x) {
			return false;
		}
	} else {
		float t1 = (aabb.min.x - origin.x) / diff.x;
		float t2 = (aabb.max.x - origin.x) / diff.x;

		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);

		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);

		if (tMin > tMax) {
			return false;
		}
	}

	// Y軸
	if (diff.y == 0.0f) {
		if (origin.y < aabb.min.y || origin.y > aabb.max.y) {
			return false;
		}
	} else {
		float t1 = (aabb.min.y - origin.y) / diff.y;
		float t2 = (aabb.max.y - origin.y) / diff.y;

		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);

		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);

		if (tMin > tMax) {
			return false;
		}
	}

	// Z軸
	if (diff.z == 0.0f) {
		if (origin.z < aabb.min.z || origin.z > aabb.max.z) {
			return false;
		}
	} else {
		float t1 = (aabb.min.z - origin.z) / diff.z;
		float t2 = (aabb.max.z - origin.z) / diff.z;

		float tNear = (std::min)(t1, t2);
		float tFar = (std::max)(t1, t2);

		tMin = (std::max)(tMin, tNear);
		tMax = (std::min)(tMax, tFar);

		if (tMin > tMax) {
			return false;
		}
	}

	return true;
}

bool IsSeparatedOnAxis(const OBB &obb1, const OBB &obb2, const Vector3 &axis) {
	float axisLengthSquared = Dot(axis, axis);

	if (axisLengthSquared <= 0.000001f) {
		return false;
	}

	Vector3 normalizedAxis = Normalize(axis);

	float center1 = Dot(obb1.center, normalizedAxis);
	float center2 = Dot(obb2.center, normalizedAxis);

	float radius1 = obb1.size.x * std::abs(Dot(obb1.orientations[0], normalizedAxis)) +
	                obb1.size.y * std::abs(Dot(obb1.orientations[1], normalizedAxis)) +
	                obb1.size.z * std::abs(Dot(obb1.orientations[2], normalizedAxis));

	float radius2 = obb2.size.x * std::abs(Dot(obb2.orientations[0], normalizedAxis)) +
	                obb2.size.y * std::abs(Dot(obb2.orientations[1], normalizedAxis)) +
	                obb2.size.z * std::abs(Dot(obb2.orientations[2], normalizedAxis));

	float distance = std::abs(center2 - center1);
	float sumRadius = radius1 + radius2;

	if (distance > sumRadius) {
		return true;
	}

	return false;
}

}

bool IsCollision(const AABB &aabb, const Ray &ray) {
	return IsCollisionAABBLineInternal(
		aabb, ray.origin, ray.diff, 0.0f, std::numeric_limits<float>::infinity()
	);
}

bool IsCollision(const AABB &aabb, const Line &line) {
	return IsCollisionAABBLineInternal(
		aabb, line.origin, line.diff, -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()
	);
}


bool IsCollision(const OBB &obb, const Sphere &sphere) {
	Matrix4x4 obbWorldMatrix = obb.MakeWorldMatrix();

	// OBBのワールド行列の逆行列を求める事で、トランスフォームを打ち消して原点上のAABBに変換する
	Matrix4x4 obbWorldMatrixInverse = Inverse(obbWorldMatrix);

	// OBBローカル空間上の球中心座標
	Vector3 centerInOBBLocalSpace = Transform(sphere.center, obbWorldMatrixInverse);

	AABB aabbOBBLocal{.min{-obb.size.x, -obb.size.y, -obb.size.z}, .max{obb.size.x, obb.size.y, obb.size.z}};

	// OBBローカル空間上の球
	Sphere sphereOBBLocal{.center = centerInOBBLocalSpace, .radius = sphere.radius};

	return IsCollision(sphereOBBLocal, aabbOBBLocal);
}



bool IsCollision(const Segment &segment, const OBB &obb) {
	Matrix4x4 obbWorldMatrix = obb.MakeWorldMatrix();
	Matrix4x4 obbInverse = Inverse(obbWorldMatrix);

	Vector3 localOrigin = Transform(segment.origin, obbInverse);
	Vector3 localEnd = Transform(segment.origin + segment.diff, obbInverse);

	AABB localAABB{.min{-obb.size.x, -obb.size.y, -obb.size.z}, .max{obb.size.x, obb.size.y, obb.size.z}};

	Segment localSegment{};
	localSegment.origin = localOrigin;
	localSegment.diff = localEnd - localOrigin;

	return IsCollision(localAABB, localSegment);
}

bool IsCollision(const Ray &ray, const OBB &obb) {
	Matrix4x4 obbWorldMatrix = obb.MakeWorldMatrix();
	Matrix4x4 obbInverse = Inverse(obbWorldMatrix);

	Vector3 localOrigin = Transform(ray.origin, obbInverse);
	Vector3 localEnd = Transform(ray.origin + ray.diff, obbInverse);

	AABB localAABB{.min{-obb.size.x, -obb.size.y, -obb.size.z}, .max{obb.size.x, obb.size.y, obb.size.z}};

	Ray localRay{};
	localRay.origin = localOrigin;
	localRay.diff = localEnd - localOrigin;

	return IsCollision(localAABB, localRay);
}

bool IsCollision(const Line &line, const OBB &obb) {
	Matrix4x4 obbWorldMatrix = obb.MakeWorldMatrix();
	Matrix4x4 obbInverse = Inverse(obbWorldMatrix);

	Vector3 localOrigin = Transform(line.origin, obbInverse);
	Vector3 localEnd = Transform(line.origin + line.diff, obbInverse);

	AABB localAABB{.min{-obb.size.x, -obb.size.y, -obb.size.z}, .max{obb.size.x, obb.size.y, obb.size.z}};

	Line localLine{};
	localLine.origin = localOrigin;
	localLine.diff = localEnd - localOrigin;

	return IsCollision(localAABB, localLine);
}

bool IsCollision(const AABB &aabb, const OBB &obb) {
	OBB aabbOBB{};

	aabbOBB.center = (aabb.min + aabb.max) * 0.5f;
	aabbOBB.orientations[0] = {1.0f, 0.0f, 0.0f};
	aabbOBB.orientations[1] = {0.0f, 1.0f, 0.0f};
	aabbOBB.orientations[2] = {0.0f, 0.0f, 1.0f};
	aabbOBB.size = (aabb.max - aabb.min) * 0.5f;

	return IsCollision(aabbOBB, obb);
}

bool IsCollision(const OBB &obb1, const OBB &obb2) {
	// OBB1の面法線3本
	for (int32_t index = 0; index < 3; ++index) {
		if (IsSeparatedOnAxis(obb1, obb2, obb1.orientations[index])) {
			return false;
		}
	}

	// OBB2の面法線3本
	for (int32_t index = 0; index < 3; ++index) {
		if (IsSeparatedOnAxis(obb1, obb2, obb2.orientations[index])) {
			return false;
		}
	}

	// 各辺の組み合わせのクロス積9本
	for (int32_t index1 = 0; index1 < 3; ++index1) {
		for (int32_t index2 = 0; index2 < 3; ++index2) {
			Vector3 crossAxis = Cross(obb1.orientations[index1], obb2.orientations[index2]);

			if (IsSeparatedOnAxis(obb1, obb2, crossAxis)) {
				return false;
			}
		}
	}

	return true;
}



void AABB::NormalizeAABB() {
	const Vector3 min_ = this->min;
	const Vector3 max_ = this->max;

	this->min.x = (std::min)(min_.x, max_.x);
	this->max.x = (std::max)(min_.x, max_.x);

	this->min.y = (std::min)(min_.y, max_.y);
	this->max.y = (std::max)(min_.y, max_.y);

	this->min.z = (std::min)(min_.z, max_.z);
	this->max.z = (std::max)(min_.z, max_.z);
}

Matrix4x4 OBB::MakeWorldMatrix() const {
	Matrix4x4 worldMatrix = MakeIdentity4x4();

	worldMatrix.m[0][0] = orientations[0].x;
	worldMatrix.m[0][1] = orientations[0].y;
	worldMatrix.m[0][2] = orientations[0].z;

	worldMatrix.m[1][0] = orientations[1].x;
	worldMatrix.m[1][1] = orientations[1].y;
	worldMatrix.m[1][2] = orientations[1].z;

	worldMatrix.m[2][0] = orientations[2].x;
	worldMatrix.m[2][1] = orientations[2].y;
	worldMatrix.m[2][2] = orientations[2].z;

	worldMatrix.m[3][0] = center.x;
	worldMatrix.m[3][1] = center.y;
	worldMatrix.m[3][2] = center.z;
	worldMatrix.m[3][3] = 1.0f;

	return worldMatrix;
}

void OBB::UpdateOBBOrientations(const Vector3 &rotate) {
	// rotateからOBBの3軸を更新
	Matrix4x4 rotateMatrix = MakeRotateXYZMatrix(rotate);

	orientations[0] = {rotateMatrix.m[0][0], rotateMatrix.m[0][1], rotateMatrix.m[0][2]};

	orientations[1] = {rotateMatrix.m[1][0], rotateMatrix.m[1][1], rotateMatrix.m[1][2]};

	orientations[2] = {rotateMatrix.m[2][0], rotateMatrix.m[2][1], rotateMatrix.m[2][2]};
}
