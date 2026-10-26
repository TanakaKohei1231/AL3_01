#define NOMINMAX
#include "PlayerBehavior.h"
#include "Player.h"
#include "GameInput.h"
#include "Vector3Util.h"
#include <algorithm>
#include <memory>
#include "Ease.h"

using namespace KamataEngine;

//============================================================
// 通常状態
//============================================================

PlayerRootBehavior::PlayerRootBehavior(Player* player) : player_(player) {}

void PlayerRootBehavior::Initialize() {
}


void PlayerRootBehavior::Update() {
	GameInput* input = GameInput::GetInstance();

	// 現在の移動速度を取得
	Vector3 velocity = player_->GetVelocity();

	// 接地状態
	if (player_->IsGround()) {
		// 左右同時押しは入力なしとして扱う
		if (input->IsPress(GameAction::kMoveRight) ^ input->IsPress(GameAction::kMoveLeft)) {
			Vector3 acceleration{};

			// 右入力
			if (input->IsPress(GameAction::kMoveRight)) {
				player_->TurnTo(LRDirection::kRight);

				// 速度と反対方向の入力なら急ブレーキ
				if (velocity.x < 0.0f) {
					velocity.x *= (1.0f - kAttenuation);
				}

				acceleration.x += kAcceleration;
			}

			// 左入力
			if (input->IsPress(GameAction::kMoveLeft)) {
				player_->TurnTo(LRDirection::kLeft);

				// 速度と反対方向の入力なら急ブレーキ
				if (velocity.x > 0.0f) {
					velocity.x *= (1.0f - kAttenuation);
				}

				acceleration.x -= kAcceleration;
			}

			// 加速を反映
			velocity += acceleration;

			// 速度制限
			velocity.x = std::clamp(velocity.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {
			// 入力がない場合は減速
			velocity.x *= (1.0f - kAttenuation);
		}

		// ジャンプ
		if (input->IsPress(GameAction::kJump)) {
			velocity += Vector3(0.0f, kJumpAcceleration, 0.0f);
		}

	} else {
		// 空中での左右移動
		if (input->IsPress(GameAction::kMoveRight) ^ input->IsPress(GameAction::kMoveLeft)) {
			Vector3 acceleration{};

			// 右入力
			if (input->IsPress(GameAction::kMoveRight)) {
				player_->TurnTo(LRDirection::kRight);

				acceleration.x += kAcceleration * kAirAccelerationRate;
			}

			// 左入力
			if (input->IsPress(GameAction::kMoveLeft)) {
				player_->TurnTo(LRDirection::kLeft);

				acceleration.x -= kAcceleration * kAirAccelerationRate;
			}

			// 加速を反映
			velocity += acceleration;

			// 速度制限
			velocity.x = std::clamp(velocity.x, -kLimitRunSpeed, kLimitRunSpeed);
		}

		// 重力加速度
		velocity += Vector3(0.0f, -Player::kGravityAcceleration, 0.0f);

		// 落下速度制限
		velocity.y = std::max(velocity.y, -Player::kLimitFallSpeed);
	}

	// 攻撃入力
	if (input->IsTrigger(GameAction::kAttack)) {
		player_->RequestBehaviorChange(std::make_unique<PlayerAttackBehavior>(player_));
	}

	// 計算した速度をプレイヤーへ反映
	player_->SetVelocity(velocity);

	// マップチップとの衝突処理
	player_->ResolveMapChipCollision(velocity);
}


void PlayerRootBehavior::UpdateMotion() {}

//============================================================
// 攻撃状態
//============================================================

PlayerAttackBehavior::PlayerAttackBehavior(Player* player) : player_(player) {}

void PlayerAttackBehavior::Initialize() {
	// 攻撃の経過時間を初期化
	attackParameter_ = 0.0f;

	// 攻撃段階を初期化
	attackPhase_ = AttackPhase::kBefore;

	// 攻撃エフェクトを非表示
	player_->isAttackEffectVisible_ = false;

	// 攻撃開始時に速度をリセット
	player_->SetVelocity({0.0f, 0.0f, 0.0f});
}


void PlayerAttackBehavior::Update() {
	// 攻撃の経過時間を更新
	attackParameter_ += 1.0f / 60.0f;

	// 攻撃動作用の移動量
	Vector3 attackVelocity{};

	switch (attackPhase_) {
	case AttackPhase::kBefore: {
		// 前隙
		float t = std::clamp(attackParameter_ / kAttackBeforeTime, 0.0f, 1.0f);

		player_->worldTransform_.scale_.z = Lerp(1.0f, 0.3f, Ease::EaseOut(t));

		player_->worldTransform_.scale_.y = Lerp(1.0f, 1.6f, Ease::EaseOut(t));

		player_->SetVelocity({0.0f, 0.0f, 0.0f});

		if (attackParameter_ >= kAttackBeforeTime) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0.0f;

			// 攻撃エフェクトを表示
			player_->isAttackEffectVisible_ = true;
		}

		break;
	}

	case AttackPhase::kAttack: {
		// 攻撃中
		float t = std::clamp(attackParameter_ / kAttackTime, 0.0f, 1.0f);

		player_->worldTransform_.scale_.z = Lerp(0.3f, 1.3f, Ease::EaseOut(t));

		player_->worldTransform_.scale_.y = Lerp(1.6f, 0.7f, Ease::EaseIn(t));

		if (player_->GetLRDirection() == LRDirection::kRight) {
			attackVelocity.x = kAttackMoveSpeed;
		} else {
			attackVelocity.x = -kAttackMoveSpeed;
		}

		if (attackParameter_ >= kAttackTime) {
			attackPhase_ = AttackPhase::kAfter;
			attackParameter_ = 0.0f;

			// 攻撃エフェクトを非表示
			player_->isAttackEffectVisible_ = false;
		}

		break;
	}

	case AttackPhase::kAfter: {
		// 後隙
		float t = std::clamp(attackParameter_ / kAttackAfterTime, 0.0f, 1.0f);

		player_->worldTransform_.scale_.z = Lerp(1.3f, 1.0f, Ease::EaseOut(t));

		player_->worldTransform_.scale_.y = Lerp(0.7f, 1.0f, Ease::EaseOut(t));

		attackVelocity.x = 0.0f;

		if (attackParameter_ >= kAttackAfterTime) {
			// 攻撃で変形したスケールを元に戻す
			player_->worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

			player_->isAttackEffectVisible_ = false;

			// 通常状態への復帰を要求
			player_->RequestBehaviorChange(std::make_unique<PlayerRootBehavior>(player_));
		}

		break;
	}
	}

	// マップチップとの衝突処理
	player_->ResolveMapChipCollision(attackVelocity);
}


void PlayerAttackBehavior::UpdateMotion() {}

//============================================================
// ノックバック状態
//============================================================

PlayerKnockbackBehavior::PlayerKnockbackBehavior(Player* player) : player_(player) {}

void PlayerKnockbackBehavior::Initialize() {
	// 攻撃中の変形を戻す
	player_->worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 攻撃エフェクトを非表示
	player_->isAttackEffectVisible_ = false;

	// ノックバックの進行段階を初期化
	knockbackPhase_ = KnockbackPhase::kKnockback;
	knockbackParameter_ = 0.0f;

	// 吹っ飛ばされている間は無敵
	player_->SetKnockbackInvincible(true);

	// 向いている方向と逆方向へ吹っ飛ばす
	float knockbackDirection = 0.0f;

	if (player_->GetLRDirection() == LRDirection::kRight) {
		knockbackDirection = -1.0f;
	} else {
		knockbackDirection = 1.0f;
	}

	player_->SetVelocity({knockbackDirection * kKnockbackSpeed, 0.0f, 0.0f});
}


void PlayerKnockbackBehavior::Update() {
	// 経過時間を更新
	knockbackParameter_ += 1.0f / 60.0f;

	// 現在の速度を取得
	Vector3 velocity = player_->GetVelocity();

	switch (knockbackPhase_) {
	case KnockbackPhase::kKnockback: {
		// 真横にノックバックする
		velocity.y = 0.0f;

		// 現在の速度を反映
		player_->SetVelocity(velocity);

		// マップチップとの衝突を解決
		player_->ResolveMapChipCollision(velocity);

		// 吹っ飛ばされる時間が終了
		if (knockbackParameter_ >= kKnockbackTime) {
			knockbackPhase_ = KnockbackPhase::kAfter;
			knockbackParameter_ = 0.0f;

			// 後隙では無敵を解除
			player_->SetKnockbackInvincible(false);
		}

		break;
	}

	case KnockbackPhase::kAfter: {
		// 横移動を減速
		velocity.x *= (1.0f - kKnockbackAttenuation);

		// Y方向には移動しない
		velocity.y = 0.0f;

		// 現在の速度を反映
		player_->SetVelocity(velocity);

		// マップチップとの衝突を解決
		player_->ResolveMapChipCollision(velocity);

		// 後隙が終了
		if (knockbackParameter_ >= kKnockbackAfterTime) {
			player_->SetVelocity({0.0f, 0.0f, 0.0f});

			// 念のため無敵解除
			player_->SetKnockbackInvincible(false);

			// 通常状態へ復帰
			player_->RequestBehaviorChange(std::make_unique<PlayerRootBehavior>(player_));
		}

		break;
	}
	}
}


void PlayerKnockbackBehavior::UpdateMotion() {}

//============================================================
// 挟まれ死亡演出
//============================================================

PlayerSqueezeDeathBehavior::PlayerSqueezeDeathBehavior(Player* player) : player_(player) {}

void PlayerSqueezeDeathBehavior::Initialize() {
	// 攻撃中の変形が残らないように戻す
	player_->worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 攻撃エフェクトを非表示
	player_->isAttackEffectVisible_ = false;

	// 演出タイマーを初期化
	squeezeDeathTimer_ = 0.0f;

	// 少し上へ飛ばす
	player_->SetVelocity({0.0f, kSqueezeDeathJumpVelocity, 0.0f});

	// 演出終了フラグを初期化
	player_->isDeathMotionFinished_ = false;

	// ノックバック中に挟まれても無敵を残さなくする
	player_->SetKnockbackInvincible(false);
}

void PlayerSqueezeDeathBehavior::Update() {
	// 演出の経過時間を更新
	squeezeDeathTimer_ += 1.0f / 60.0f;

	// 現在の速度を取得
	Vector3 velocity = player_->GetVelocity();

	// 重力で落下
	velocity.y -= kSqueezeDeathGravity;

	// 速度を反映
	player_->SetVelocity(velocity);

	// 死亡演出中は地形衝突を無視して移動
	player_->Move(velocity);

	// X軸回転しながら吹っ飛ぶ
	player_->SetRotationX(player_->GetRotationX() + kSqueezeDeathRotationSpeed * (1.0f / 60.0f));

	// 画面下の少し下を終了位置とする
	const float bottomLimit = player_->camera_->translation_.y + Player::kScreenBottomLimit - 2.0f;

	// 経過時間または落下位置から終了を判定
	if (squeezeDeathTimer_ >= kSqueezeDeathMotionTime || player_->worldTransform_.translation_.y < bottomLimit) {

		player_->isDeathMotionFinished_ = true;
	}
}

void PlayerSqueezeDeathBehavior::UpdateMotion() {}
