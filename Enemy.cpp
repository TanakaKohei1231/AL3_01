#include "Enemy.h"
#include "MapChipField.h"
#include "Vector3Util.h"
#include "Matrix4x4Util.h"
#include<numbers>
#include<cmath>
#include "Player.h"
#include <algorithm>
#include "Ease.h"
#include"GameScene.h"

using namespace KamataEngine;

void Enemy::InitializeInternal() {
	// モデルの向きに合わせてY軸回りに270度回転させる
	worldTransform_.rotation_.y = kDefaultAngle;

	velocity_.x = -kWalkSpeed;

	walkTimer_ = 0.0f;
	deathTimer_ = 0.0f;

	behavior_ = Behavior::kWalk;
	behaviorRequest_ = Behavior::kUnknown;
}

void Enemy::Update() {
	// 振る舞い変更のリクエストが来ていた場合、振る舞いを変更する
	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振る舞い変更の反映
		behavior_ = behaviorRequest_;

		// 振る舞いごとの初期化
		switch (behavior_) {
			// kRootとdefaultは同じ処理
		case Behavior::kWalk:
		default:
			BehaviorWalkInitialize();
			break;

		case Behavior::kDeathDirection:
			BehaviorDeathDirectionInitialize();
			break;
		}

		// 振る舞いリクエストの初期化
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Behavior::kWalk:
	default:
		BehaviorWalkUpdate();
		break;
	case Behavior::kDeathDirection:
		BehaviorDeathDirectionUpdate();
		break;
	}

	// ワールド変換の更新
	WorldTransformUpdate(worldTransform_);
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb{};
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Enemy::OnCollision(Player* player) {
	// 死んでいるなら何もしない
	if (behavior_ == Behavior::kDeathDirection) {
		return;
	}

	// プレイヤーが攻撃中なら死ぬ
	if (player->IsAttack()) {
		// 振る舞いをデス演出に変更
		behaviorRequest_ = Behavior::kDeathDirection;

		// あたり判定無効フラグを立てる
		isCollisionDisabled_ = true;

		// 敵と自キャラの中間位置にエフェクトを生成
		Vector3 effectPosition = (GetWorldPosition() + player->GetWorldPosition()) / 2.0f;

		if (gameScene_) {
			gameScene_->CreateHitEffect(effectPosition);
		}
	}
}

void Enemy::BehaviorWalkInitialize() {
	velocity_.x = -kWalkSpeed;

	walkTimer_ = 0.0f;
}

void Enemy::BehaviorWalkUpdate() {
	walkTimer_ += 1.0f / 60.0f;

	// タイマーが大きくなりすぎないようにする
	if (walkTimer_ >= kWalkMotionTime) {
		walkTimer_ -= kWalkMotionTime;
	}

	// -1.0f ～ 1.0f を周期的に繰り返す値
	float param = std::sin(2.0f * kPi * walkTimer_ / kWalkMotionTime);

	// -1.0f ～ 1.0f を 0.0f ～ 1.0f に変換
	float t = (param + 1.0f) / 2.0f;

	// 最初の角度から最後の角度まで補間
	float angle = kWalkMotionAngleStart + (kWalkMotionAngleEnd - kWalkMotionAngleStart) * t;

	// X軸周りに回転アニメーション
	worldTransform_.rotation_.x = angle;

	worldTransform_.translation_ += velocity_;
}

void Enemy::BehaviorDeathDirectionInitialize() {
	deathTimer_ = 0.0f;

	// 死亡演出開始時の角度を保存
	deathStartRotationX_ = worldTransform_.rotation_.x;
}

void Enemy::BehaviorDeathDirectionUpdate() {
	deathTimer_+= 1.0f / 60.0f;

	// デスタイマーを 0.0f ～ 1.0f に変換
	float t = std::clamp(deathTimer_ / kDeathMotionTime, 0.0f, 1.0f);

	// X軸回転で倒れる
	worldTransform_.rotation_.x = Lerp(deathStartRotationX_, kDeathMotionAngleEnd, Ease::EaseInOut(t));

	// Y軸回転でぐるぐる回る
	worldTransform_.rotation_.y += kDeathSpinSpeed * (1.0f / 60.0f);


	if (deathTimer_ >= kDeathMotionTime) {
		isDead_ = true;
	}
}
