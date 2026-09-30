#include "CameraController.h"
#include "Player.h"
#include "Vector3Util.h"
#include <algorithm>
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

	// 行列の更新
	camera_->UpdateMatrix();
}

void CameraController::Reset() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 追従対象とオフセットからカメラ座標を計算
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
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