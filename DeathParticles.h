#pragma once
#include "KamataEngine.h"
#include <array>
#include "Vector3Util.h"

/// <summary>
/// デス演出用パーティクル
/// </summary>
class DeathParticles {
public:
	// 初期化処理
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// 終了したか
	bool IsFinished() const { return isFinished_; }

private:
	// パーティクルの個数
	static inline const uint32_t kNumParticles = 8;

	// ワールド変換データ
	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 色変更オブジェクト
	KamataEngine::ObjectColor objectColor_;

	// 色
	KamataEngine::Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	// 移動の速さ
	static inline const float kSpeed = 0.08f;

	// 分割した1個分の角度
	static inline const float kAngleUnit = 2.0f * kPi / static_cast<float>(kNumParticles);

	// 存続時間
	static inline const float kDuration = 1.0f;

	// 終了フラグ
	bool isFinished_ = false;

	// 経過時間
	float counter_ = 0.0f;
};
