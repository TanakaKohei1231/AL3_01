#pragma once
#include "KamataEngine.h"

namespace KamataEngine {

// 行列の加算
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);

// 行列の減算
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

// 行列の乗算
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

// 転置行列
Matrix4x4 Transpose(const Matrix4x4& m);

// 単位行列の生成
Matrix4x4 MakeIdentity4x4();

// 座標変換
Vector3 Transform(const Vector3& v, const Matrix4x4& m);

// 平行移動行列の生成
Matrix4x4 MakeTranslateMatrix(const Vector3& translation);

// 拡縮行列の生成
Matrix4x4 MakeScaleMatrix(const Vector3& scale);

// X軸回転行列の生成
Matrix4x4 MakeRotateXMatrix(float radian);

// Y軸回転行列の生成
Matrix4x4 MakeRotateYMatrix(float radian);

// Z軸回転行列の生成
Matrix4x4 MakeRotateZMatrix(float radian);

// 全回転行列の生成
Matrix4x4 MakeRotateXYZMatrix(const Vector3& radian);

// アフィン変換行列の生成
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

// 逆行列
Matrix4x4 Inverse(const Matrix4x4& m);

// ワールドトランスフォームの更新
void WorldTransformUpdate(WorldTransform& worldTransform);

Matrix4x4& operator+=(Matrix4x4& left, const Matrix4x4& right);

} // namespace KamataEngine