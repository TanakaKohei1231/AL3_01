#include "BaseEnemy.h"
#include "Player.h"
#include "Ease.h"
#include <algorithm>

using namespace KamataEngine;

void BaseEnemy::OnFallDeath() {
	// 敵を死亡扱いにする
	isDead_ = true;

	// 当たり判定を無効にする
	isCollisionDisabled_ = true;
}

void BaseEnemy::OnCollisionGround() {
	// 地面に接触したら落下速度をリセット
	velocity_.y = 0.0f;
}

void BaseEnemy::OnCollisionCeiling() {
	// 天井に接触したら上昇速度をリセット
	velocity_.y = 0.0f;
}

void BaseEnemy::OnCollisionWall() {
	// 向きを反転
	if (lrDirection_ == LRDirection::kLeft) {
		lrDirection_ = LRDirection::kRight;
	} else {
		lrDirection_ = LRDirection::kLeft;
	}

	// 横移動速度を反転
	velocity_.x *= -1.0f;

	// 旋回先の角度
	float targetRotationY = 3.0f * kPi / 2.0f;

	if (lrDirection_ == LRDirection::kRight) {
		targetRotationY += kPi;
	}

	// 旋回開始
	StartTurn(targetRotationY);
}

void BaseEnemy::StartTurn(float targetRotationY) {
	// 現在の基本角度から旋回を開始
	turnStartRotationY_ = baseRotationY_;

	turnTargetRotationY_ = targetRotationY;

	turnTimer_ = 0.0f;
	isTurning_ = true;
}

void BaseEnemy::UpdateTurn() {
	if (!isTurning_) {
		return;
	}

	// タイマー更新
	turnTimer_ += 1.0f / 60.0f;

	// 補間割合
	float t = std::clamp(turnTimer_ / kTurnTime, 0.0f, 1.0f);

	// イージング
	float easedT = Ease::EaseInOut(t);

	// 基本角度を補間
	baseRotationY_ = std::lerp(turnStartRotationY_, turnTargetRotationY_, easedT);

	if (t >= 1.0f) {
		baseRotationY_ = turnTargetRotationY_;
		isTurning_ = false;
	}
}

void BaseEnemy::AfterBehaviorUpdate() {
	// 死亡演出中は通常処理を行わない
	if (isCollisionDisabled_) {
		return;
	}

	// 重力を適用
	velocity_.y -= kGravityAcceleration;
	velocity_.y = (std::max)(velocity_.y, -kLimitFallSpeed);

	// 縦方向の移動と衝突判定
	ResolveMapChipCollision({0.0f, velocity_.y, 0.0f});

	// 旋回更新
	UpdateTurn();

	// 基本の向きと歩行モーションを合成
	worldTransform_.rotation_.y = baseRotationY_ + walkMotionRotationY_;
}