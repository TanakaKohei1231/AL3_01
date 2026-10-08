#pragma once
#include "KamataEngine.h"
#include <memory>
#include "Fade.h"
#include "IScene.h"

class TitleScene : public IScene {
  public:
	// 初期化処理
	void Initialize() override;

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

  private:

	// シーンのフェーズ
	enum class Phase {
		kFadeIn,
		kMain,
		kFadeOut,
	};

	// 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

	// カメラ
	std::unique_ptr<KamataEngine::Camera> camera_;

	// タイトル用モデル
	std::unique_ptr<KamataEngine::Model> modelTitle_;

	// プレイヤー見た目用モデル
	std::unique_ptr<KamataEngine::Model> modelPlayer_;

	// タイトル用ワールド変換
	std::unique_ptr<KamataEngine::WorldTransform> worldTransformTitle_;

	// プレイヤー見た目用ワールド変換
	std::unique_ptr<KamataEngine::WorldTransform> worldTransformPlayer_;

	// アニメーション用タイマー
	float animationTimer_ = 0.0f;

	// フェード
	std::unique_ptr<Fade> fade_;
};
