#pragma once
#include"BaseEffect.h"
#include"KamataEngine.h"
#include <array>

class GuardEffect : public BaseEffect {
  public:
	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	// 死亡状態か
	bool IsDead() const override { return state_ == State::kDead; }

	// インスタンス生成と初期化
	static GuardEffect* Create(const KamataEngine::Vector3& position);

	static void SetModel(KamataEngine::Model* model) { model_ = model; }


  private:
	// 初期化の個別処理
	void InitializeInternal(const KamataEngine::Vector3& position) override;

	// 状態
	enum class State {
		kSpread,  // 拡大中
		kFadeOut, // フェードアウト中
		kDead,    // 終了
	};

	static KamataEngine::Model* model_;

	// 拡大時間
	static inline const float kSpreadDuration = 0.1f;

	// フェード時間
	static inline const float kFadeDuration = 0.1f;

	// 円の開始スケール
	static inline const float kCircleStartScale = 0.2f;

	// 円の終了スケール
	static inline const float kCircleEndScale = 2.0f;

	// 現在の状態
	State state_ = State::kSpread;

	// 円のワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;
};
