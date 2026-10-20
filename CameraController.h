#pragma once
#include "KamataEngine.h"
#include"Rect.h"

// 前方宣言
class Player;

enum class CameraMode {
	kFollow,       // プレイヤー追従
	kForcedScroll, // 強制スクロール
};

class CameraController {
public:
	void Initialize(KamataEngine::Camera* camera_);
	void Update();
	void Reset();

	// カメラシェイク開始
	void StartShake(float power, int32_t durationFrames);

	// カメラシェイクだけ更新
	void UpdateShake();


	//============================================================
	// ゲッター・セッター
	//============================================================

	CameraMode GetMode() const { return mode_; }

	void SetTarget(Player* target) { target_ = target; }
	void SetMode(CameraMode mode) { mode_ = mode; }
	void SetMovableArea(Rect area) { movableArea_ = area; }

private:
	// カメラ
	KamataEngine::Camera *camera_ = nullptr;

	// 追従対象のプレイヤー
	Player* target_ = nullptr;

	// カメラモード
	CameraMode mode_ = CameraMode::kFollow;

	// 追従カメラ更新
	void UpdateFollow();

	// 追従対象とカメラの座標の差（オフセット）
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};

	// カメラ移動範囲
	Rect movableArea_ = {};

	// カメラの目標座標
	KamataEngine::Vector3 targetCameraPosition_ = {};

	//速度掛け算
	static inline const float kVelocityBias_ = 30.0f;

	// 追従対象の各方向へのカメラ移動範囲
	static inline const Rect kMargin_ = {-10.0f, 10.0f, -5.5f, 5.5f};


	//============================================================
	// 強制スクロール
	//============================================================

	// 強制スクロール更新
	void UpdateForcedScroll();

	// 強制スクロール速度
	static inline const float kForcedScrollSpeed = 0.05f;

	//============================================================
	// カメラシェイク
	//============================================================

	// シェイク処理内部
	void ApplyShake();

	// 前フレームで適用した揺れの移動量
	KamataEngine::Vector3 shakeOffset_ = {};

	// シェイクの強さ
	float shakePower_ = 0.0f;

	// シェイクの残りフレーム数
	int32_t shakeTimer_ = 0;

	// シェイクの持続フレーム数
	int32_t shakeDuration_ = 0;

	// シェイク用の経過フレーム数
	int32_t shakeElapsedFrames_ = 0;
};
