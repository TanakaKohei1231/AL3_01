#include "ShieldEnemy.h"
#include "Player.h"
#include "GameScene.h"
#include "Vector3Util.h"
#include "Matrix4x4Util.h"
#include "Ease.h"
#include<numbers>
#include<cmath>
#include <algorithm>

using namespace KamataEngine;

void ShieldEnemy::InitializeInternal () {
	// モデルの向きに合わせてY軸回りに回転させる
	worldTransform_.rotation_.y = kDefaultAngle;

	velocity_.x = -kWalkSpeed;

	walkTimer_ = 0.0f;
	deathTimer_ = 0.0f;

	lrDirection_ = LRDirection::kLeft;

	behavior_ = Behavior::kWalk;
	behaviorRequest_ = Behavior::kUnknown;
}

void ShieldEnemy::Update () {
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

		case Behavior::kGuard:
			BehaviorGuardInitialize();
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

	case Behavior::kGuard:
		BehaviorGuardUpdate();
		break;
	}

	// ワールド変換の更新
	WorldTransformUpdate(worldTransform_);
}

void ShieldEnemy::OnCollision(Player* player) {
	// 死んでいるなら何もしない
	if (behavior_ == Behavior::kDeathDirection) {
		return;
	}

	// プレイヤーが攻撃中なら死ぬ
	if (player->IsAttack()) {

		// プレイヤーと向き合っている(向きが違う)時はガード成功
		if (lrDirection_ != player->GetLRDirection()) {
			// ガードエフェクトの生成
			gameScene_->CreateGuardEffect(GetWorldPosition());

			// ガード演出へ移行
			behaviorRequest_ = Behavior::kGuard;

			// プレイヤーのノックバックを要求
			player->RequestKnockback();

			return;
		}

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

AABB ShieldEnemy::GetAABB () {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb{};
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void ShieldEnemy::BehaviorWalkInitialize() {
	velocity_.x = -kWalkSpeed;
}

void ShieldEnemy::BehaviorWalkUpdate() {
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

	// Y軸周りに回転アニメーション
	worldTransform_.rotation_.y = kDefaultAngle + angle;

	worldTransform_.translation_ += velocity_;
}


void ShieldEnemy::BehaviorDeathDirectionInitialize() {
	deathTimer_ = 0.0f;

	// 死亡演出開始時の角度を保存
	deathStartRotationX_ = worldTransform_.rotation_.x;
}


void ShieldEnemy::BehaviorDeathDirectionUpdate() {
	deathTimer_ += 1.0f / 60.0f;

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


void ShieldEnemy::BehaviorGuardInitialize() {
	guardTimer_ = 0.0f;

	// ガード開始時のZ回転を保存
	guardStartRotationZ_ = worldTransform_.rotation_.z;

	// ガード中は移動しない
	velocity_ = {0.0f, 0.0f, 0.0f};
}

void ShieldEnemy::BehaviorGuardUpdate() {
	guardTimer_ += 1.0f / 60.0f;

	// 0.0f ～ 1.0f
	float t = std::clamp(guardTimer_ / kGuardMotionTime, 0.0f, 1.0f);

	// 0 → 1 → 0 の値
	float motion = std::sin(t * kPi);

	// のけぞる向き
	float guardAngle = 0.0f;

	if (lrDirection_ == LRDirection::kLeft) {
		guardAngle = -kGuardMotionAngle;
	} else {
		guardAngle = kGuardMotionAngle;
	}

	// Z軸回転で一瞬のけぞって戻る
	worldTransform_.rotation_.z = guardStartRotationZ_ + guardAngle * motion;

	// ガード中は移動しない
	velocity_ = {0.0f, 0.0f, 0.0f};

	if (guardTimer_ >= kGuardMotionTime) {
		guardTimer_ = 0.0f;

		// 念のためZ回転を戻す
		worldTransform_.rotation_.z = 0.0f;

		behaviorRequest_ = Behavior::kWalk;
	}
}