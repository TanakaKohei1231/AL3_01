
#include "CharacterEnemyBehavior.h"

#include "BaseEnemy.h"
#include "Ease.h"
#include "ShieldEnemy.h"
#include "Vector3Util.h"

#include <algorithm>
#include <cmath>
#include <memory>

using namespace KamataEngine;

namespace {

// 通常敵・盾敵の歩行速度
constexpr float kWalkSpeed = 0.01f;

// 歩行モーションの周期
constexpr float kWalkMotionTime = 2.0f;

// 通常敵の歩行回転角
const float kNormalWalkAngle = kPi / 4.0f;

// 盾敵の歩行回転角
const float kShieldWalkAngle = kPi / 5.0f;

// モデルの初期Y回転角
const float kDefaultEnemyAngle = 3.0f * kPi / 2.0f;

// 死亡演出
constexpr float kDeathMotionTime = 1.0f;
const float kDeathMotionAngle = -kPi / 2.0f;
const float kDeathSpinSpeed = kPi * 6.0f;

// ガード演出
constexpr float kGuardMotionTime = 0.5f;
const float kGuardMotionAngle = kPi / 6.0f;

} // namespace

//============================================================
// 通常敵の歩行
//============================================================

NormalEnemyWalk::NormalEnemyWalk(BaseEnemy* enemy) : enemy_(enemy) {}

void NormalEnemyWalk::Initialize() {
	walkTimer_ = 0.0f;
	enemy_->SetVelocity({-kWalkSpeed, 0.0f, 0.0f});
}

void NormalEnemyWalk::Update() {
	// 横方向の速度を取得
	const Vector3& velocity = enemy_->GetVelocity();

	// 横方向のみ移動する
	enemy_->ResolveMapChipCollision({velocity.x, 0.0f, 0.0f});
}

void NormalEnemyWalk::UpdateMotion() {
	walkTimer_ += 1.0f / 60.0f;

	if (walkTimer_ >= kWalkMotionTime) {
		walkTimer_ -= kWalkMotionTime;
	}

	float parameter = std::sin(2.0f * kPi * walkTimer_ / kWalkMotionTime);

	float interpolation = (parameter + 1.0f) / 2.0f;

	float angle = Lerp(kNormalWalkAngle, -kNormalWalkAngle, interpolation);

	enemy_->SetRotationX(angle);
}

//============================================================
// 盾敵の歩行
//============================================================

ShieldEnemyWalk::ShieldEnemyWalk(BaseEnemy* enemy) : enemy_(enemy) {}

void ShieldEnemyWalk::Initialize() {
	// 歩行モーションを初期化
	walkTimer_ = 0.0f;
	enemy_->SetWalkMotionY(0.0f);

	// 現在の向きに応じて歩行速度を設定
	float moveSpeed = kWalkSpeed;

	if (enemy_->GetLRDirection() == LRDirection::kLeft) {
		moveSpeed = -kWalkSpeed;
	}

	enemy_->SetVelocity({moveSpeed, 0.0f, 0.0f});
}

void ShieldEnemyWalk::Update() {
	// 横方向の速度を取得
	const Vector3& velocity = enemy_->GetVelocity();

	// 横方向のみ移動する
	enemy_->ResolveMapChipCollision({velocity.x, 0.0f, 0.0f});
}

void ShieldEnemyWalk::UpdateMotion() {
	walkTimer_ += 1.0f / 60.0f;

	if (walkTimer_ >= kWalkMotionTime) {
		walkTimer_ -= kWalkMotionTime;
	}

	float parameter = std::sin(2.0f * kPi * walkTimer_ / kWalkMotionTime);

	float interpolation = (parameter + 1.0f) / 2.0f;

	float angle = Lerp(kShieldWalkAngle, -kShieldWalkAngle, interpolation);

	enemy_->SetWalkMotionY(angle);
}

//============================================================
// 敵共通の死亡演出
//============================================================

EnemyDeathBehavior::EnemyDeathBehavior(BaseEnemy* enemy) : enemy_(enemy) {}

void EnemyDeathBehavior::Initialize() {
	deathTimer_ = 0.0f;
	deathStartRotationX_ = enemy_->GetRotationX();
}

void EnemyDeathBehavior::Update() {
	// 死亡演出中は移動しない
}

void EnemyDeathBehavior::UpdateMotion() {
	deathTimer_ += 1.0f / 60.0f;

	float interpolation = std::clamp(deathTimer_ / kDeathMotionTime, 0.0f, 1.0f);

	enemy_->SetRotationX(Lerp(deathStartRotationX_, kDeathMotionAngle, Ease::EaseInOut(interpolation)));

	enemy_->SetRotationY(enemy_->GetRotationY() + kDeathSpinSpeed * (1.0f / 60.0f));

	if (deathTimer_ >= kDeathMotionTime) {
		enemy_->SetDead(true);
	}
}

//============================================================
// 盾敵のガード
//============================================================

ShieldEnemyGuard::ShieldEnemyGuard(ShieldEnemy* enemy) : enemy_(enemy) {}

void ShieldEnemyGuard::Initialize() {
	// ガードの経過時間を初期化
	guardTimer_ = 0.0f;

	// 歩行モーションのY軸回転量をリセット
	enemy_->SetWalkMotionY(0.0f);

	// ガード開始時のZ回転を保存
	guardStartRotationZ_ = enemy_->GetRotationZ();

	// ガード中は移動しない
	enemy_->SetVelocity({0.0f, 0.0f, 0.0f});
}

void ShieldEnemyGuard::Update() {
	// ガード中は移動しない
}

void ShieldEnemyGuard::UpdateMotion() {
	guardTimer_ += 1.0f / 60.0f;

	float interpolation = std::clamp(guardTimer_ / kGuardMotionTime, 0.0f, 1.0f);

	float motion = std::sin(interpolation * kPi);

	float guardAngle = kGuardMotionAngle;

	if (enemy_->GetLRDirection() == LRDirection::kLeft) {
		guardAngle = -kGuardMotionAngle;
	}

	enemy_->SetRotationZ(guardStartRotationZ_ + guardAngle * motion);

	if (guardTimer_ >= kGuardMotionTime) {
		enemy_->SetRotationZ(0.0f);

		enemy_->RequestBehaviorChange(std::make_unique<ShieldEnemyWalk>(enemy_));
	}
}
