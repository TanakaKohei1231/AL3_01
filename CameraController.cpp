#include "CameraController.h"
#include "Player.h"
#include "Vector3Util.h"
#include <algorithm>
#include <cmath>
#include "Ease.h"

using namespace KamataEngine;

void CameraController::Initialize(Camera* camera) {
	camera_ = camera;
	camera_->Initialize();

	movableArea_.left = 11.0f;
	movableArea_.right = 100.0f;
	movableArea_.bottom = 6.0f;
	movableArea_.top = 100.0f;
}



void CameraController::Update() {
	// 前フレームのシェイクを除去
	camera_->translation_ -= shakeOffset_;
	shakeOffset_ = {};

	switch (mode_) {
	// 追従カメラ(通常)
	case CameraMode::kFollow:
	default:
		UpdateFollow();
		break;

	// 強制スクロール
	case CameraMode::kForcedScroll:
		UpdateForcedScroll();
		break;
	}

	// シェイク適用とカメラ行列更新
	UpdateShake();
}

void CameraController::Reset() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 追従対象とオフセットからカメラ座標を計算
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;

	// シェイク状態をリセット
	shakeOffset_ = {};
	shakePower_ = 0.0f;
	shakeTimer_ = 0;
	shakeDuration_ = 0;
	shakeElapsedFrames_ = 0;
}

void CameraController::StartShake(float power, int32_t durationFrames) {
	// 不正な設定なら何もしない
	if (power <= 0.0f || durationFrames <= 0) {
		return;
	}

	// より強いシェイクが要求された場合は更新
	if (shakeTimer_ > 0 && power < shakePower_) {
		return;
	}

	shakePower_ = power;
	shakeDuration_ = durationFrames;
	shakeTimer_ = durationFrames;
	shakeElapsedFrames_ = 0;
}

void CameraController::UpdateShake() {
	// カメラシェイクを更新
	ApplyShake();

	// カメラの行列を更新
	camera_->UpdateMatrix();
}

void CameraController::ApplyShake() {
	// 前フレームの揺れを除去
	camera_->translation_ -= shakeOffset_;

	// オフセットをリセット
	shakeOffset_ = {};

	// シェイクしていなければ終了
	if (shakeTimer_ <= 0) {
		return;
	}

	// 残り時間に応じて揺れを弱くする
	float t = static_cast<float>(shakeTimer_) / static_cast<float>(shakeDuration_);

	float currentPower = shakePower_ * t;

	// 時間経過によって揺れの向きを変える
	float phase = static_cast<float>(shakeElapsedFrames_);

	shakeOffset_.x = std::sin(phase * 2.4f) * currentPower;
	shakeOffset_.y = std::cos(phase * 3.1f) * currentPower;

	// カメラに揺れを適用
	camera_->translation_ += shakeOffset_;

	// タイマー更新
	--shakeTimer_;
	++shakeElapsedFrames_;
}


void CameraController::UpdateFollow() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	targetCameraPosition_ = targetWorldTransform.translation_ + targetOffset_ + target_->GetVelocity() * kVelocityBias_;

	// 線形補完によりゆっくり追従
	camera_->translation_.x = Lerp(camera_->translation_.x, targetCameraPosition_.x, 0.05f);
	camera_->translation_.z = Lerp(camera_->translation_.z, targetCameraPosition_.z, 0.05f);

	// Yだけさらにゆっくり追従
	camera_->translation_.y = Lerp(camera_->translation_.y, targetCameraPosition_.y, 0.003f);

	// 追従対象が画面外に出ないように補正
	camera_->translation_.x = std::clamp(camera_->translation_.x, targetWorldTransform.translation_.x + kMargin_.left, targetWorldTransform.translation_.x + kMargin_.right);
	camera_->translation_.y = std::clamp(camera_->translation_.y, targetWorldTransform.translation_.y + kMargin_.bottom, targetWorldTransform.translation_.y + kMargin_.top);

	// カメラの移動範囲制限
	camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
	camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);
}


void CameraController::UpdateForcedScroll() {
	// カメラを右へ等速移動
	camera_->translation_.x += kForcedScrollSpeed;

	// カメラの移動範囲制限
	camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
	camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);
}