#pragma once
#include "KamataEngine.h"
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

	~TitleScene();

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
	KamataEngine::Camera camera_;

	// タイトル用モデル
	KamataEngine::Model* modelTitle_ = nullptr;

	// プレイヤー見た目用モデル
	KamataEngine::Model* modelPlayer_ = nullptr;

	// タイトル用ワールド変換
	KamataEngine::WorldTransform worldTransformTitle_;

	// プレイヤー見た目用ワールド変換
	KamataEngine::WorldTransform worldTransformPlayer_;

	// アニメーション用タイマー
	float animationTimer_ = 0.0f;

	// フェード
	Fade* fade_ = nullptr;
};
