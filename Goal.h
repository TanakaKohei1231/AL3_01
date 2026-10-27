#pragma once
#include "Collision.h"
#include "KamataEngine.h"

class Goal {
public:
	// 初期化
	void Initialize(KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// あたり判定
	AABB GetAABB() const;

	// 取得する
	void Collect() { isCollected_ = true; }

	// 取得済みか
	bool IsCollected() const { return isCollected_; }

	// 座標
	KamataEngine::Vector3 GetWorldPosition() const;

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	// 基準位置
	KamataEngine::Vector3 basePosition_ = {};

	// 演出用タイマー
	float animationTimer_ = 0.0f;

	// 取得済み
	bool isCollected_ = false;

	// あたり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// 仮モデルのサイズ
	static inline const float kScale = 0.6f;

	// 上下移動量
	static inline const float kFloatAmplitude = 0.15f;
};
