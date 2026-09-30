#pragma once
#include<KamataEngine.h>
#include "Vector3Util.h"
#include "Matrix4x4Util.h"

// 直線
struct Line {
	KamataEngine::Vector3 origin; //!< 始点
	KamataEngine::Vector3 diff;   //!< 終点への差分ベクトル
};

// 半直線
struct Ray {
	KamataEngine::Vector3 origin; //!< 始点
	KamataEngine::Vector3 diff;   //!< 終点への差分ベクトル
};

// 線分
struct Segment {
	KamataEngine::Vector3 origin; //!< 始点
	KamataEngine::Vector3 diff;   //!< 終点への差分ベクトル
};

// 平面
struct Plane {
	KamataEngine::Vector3 normal; //!< 法線
	float distance; //!< 距離
};

// 三角形
struct Triangle {
	KamataEngine::Vector3 vertices[3]; //!< 頂点
};

struct Sphere {
	KamataEngine::Vector3 center;
	float radius;
};

// AABB
struct AABB {
	KamataEngine::Vector3 min; //!< 最小点
	KamataEngine::Vector3 max; //!< 最大点

	void NormalizeAABB();
};

struct OBB {
	KamataEngine::Vector3 center;          //!< 中心点
	KamataEngine::Vector3 orientations[3]; //!< 座標軸。正規化・直交必須
	KamataEngine::Vector3 size;            //!< 座標軸方向の長さの半分。中心から面までの距離

	// OBBのワールド行列を作成する
	KamataEngine::Matrix4x4 MakeWorldMatrix() const;

	// OBBの座標軸を更新する
	void UpdateOBBOrientations(const KamataEngine::Vector3& rotate);
};

KamataEngine::Vector3 Project(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

KamataEngine::Vector3 ClosestPoint(const KamataEngine::Vector3& point, const Segment& segment);

// 球同士の衝突判定
bool IsCollision(const Sphere &s1, const Sphere &s2);

// 球と平面の衝突判定
bool IsCollision(const Sphere &sphere, const Plane &plane);

// 線分と平面の衝突判定
bool IsCollision(const Line &line, const Plane &plane);

// 半直線と平面の衝突判定
bool IsCollision(const Ray &ray, const Plane &plane);

// 線分と平面の衝突判定
bool IsCollision(const Segment &segment, const Plane &plane);

// 線分と三角形の衝突判定
bool IsCollision(const Segment &segment, const Triangle &triangle);

// AABB同士の衝突判定
bool IsCollision(const AABB &aabb1, const AABB &aabb2);

// 球とAABBの衝突判定
bool IsCollision(const Sphere &sphere, const AABB &aabb);

// 線分とAABBの衝突判定
bool IsCollision(const AABB &aabb, const Segment &segment);

// 半直線とAABBの衝突判定
bool IsCollision(const AABB &aabb, const Ray &ray);

// 直線とAABBの衝突判定
bool IsCollision(const AABB &aabb, const Line &line);

// OBBと球の衝突判定
bool IsCollision(const OBB &obb, const Sphere &sphere);

// 線分とOBBの衝突判定
bool IsCollision(const Segment &segment, const OBB &obb);

// 半直線とOBBの衝突判定
bool IsCollision(const Ray &ray, const OBB &obb);

// 直線とOBBの衝突判定
bool IsCollision(const Line &line, const OBB &obb);

// AABBとOBBの衝突判定
bool IsCollision(const AABB &aabb, const OBB &obb);

// OBB同士の衝突判定
bool IsCollision(const OBB &obb1, const OBB &obb2);