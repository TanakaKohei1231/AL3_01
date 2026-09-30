#pragma once
#include "KamataEngine.h"

/// <summary>
/// 天球
/// </summary>
class Skydome {
  public:
	// 初期化処理
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

  private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;
};
