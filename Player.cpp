#define NOMINMAX
#include"Player.h"
#include<assert.h>
#include"Matrix4x4Util.h"
#include<numbers>
#include "Vector3Util.h"
#include<algorithm>
#include"Ease.h"
#include"MapChipField.h"
#include "GlobalVariables.h"
#include "GameInput.h"
#include "ModelManager.h"
#include <memory>

using namespace KamataEngine;

void Player::TurnTo(LRDirection direction) {
	// すでに指定方向を向いているなら何もしない
	if (lrDirection_ == direction) {
		return;
	}

	// 向きを変更
	lrDirection_ = direction;

	// 新しい方向へ旋回
	if (direction == LRDirection::kRight) {
		TurnToAngle(kPi / 2.0f);
	} else {
		TurnToAngle(kPi * 3.0f / 2.0f);
	}
}

void Player::TurnToAngle(float targetRotationY) {
	// 現在角度を取得
	float currentRotationY = worldTransform_.rotation_.y;

	// 目標角度との差分を求める
	float difference = targetRotationY - currentRotationY;

	// 差分を -π ～ π の範囲に収める
	difference = std::remainder(difference, 2.0f * kPi);

	// 現在角度を旋回開始角度として保存
	turnFirstRotationY_ = currentRotationY;

	// 最短経路の目標角度を設定
	turnTargetRotationY_ = currentRotationY + difference;

	// 旋回タイマーを設定
	turnTimer_ = kTurnTime;
}

void Player::TurnToLadder() {
	// 背面を向く
	TurnToAngle(kLadderRotationY);
}

void Player::TurnFromLadder() {
	// はしごを掴む前の左右方向に戻す
	if (lrDirection_ == LRDirection::kRight) {
		TurnToAngle(kPi / 2.0f);
	} else {
		TurnToAngle(kPi * 3.0f / 2.0f);
	}
}

bool Player::IsOnLadder() const {
	if (!mapChipField_) {
		return false;
	}

	// はしご判定用の座標
	const Vector3 center = worldTransform_.translation_;

	const Vector3 checkPositions[] = {
	    center,
	    {center.x, center.y + height_ * 0.35f,                     center.z},
	    {center.x, center.y - height_ * 0.35f,                     center.z},

	    // はしごの最上段に立った状態から降りるための判定
	    {center.x, center.y - height_ / 2.0f - kGroundCheckOffset, center.z}
	};

	// いずれかの点がはしごの範囲内にあるか
	for (const Vector3& position : checkPositions) {
		IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(position);

		MapChipType type = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

		if (type == MapChipType::kLadder) {
			return true;
		}
	}

	return false;
}

void Player::InitializeInternal() {

	// モデルマネージャーからモデルを取得
	model_ = ModelManager::GetInstance()->Load("player", true);
	modelAttack_ = ModelManager::GetInstance()->Load("hit_effect", true);

	assert(model_ != nullptr);
	assert(modelAttack_ != nullptr);

	// 死亡フラグを初期化
	isDead_ = false;
	deathType_ = DeathType::kNone;
	isDeathMotionFinished_ = false;

	// モデルの向きに合わせてY軸回りに90度回転させる
	worldTransform_.rotation_.y = kPi / 2.0f;

	turnTargetRotationY_ = worldTransform_.rotation_.y;
	turnFirstRotationY_ = worldTransform_.rotation_.y;
	turnTimer_ = 0.0f;

	worldTransformAttack_.Initialize();
	worldTransformAttack_.translation_ = worldTransform_.translation_;

	// 振る舞いの初期化
	RequestBehaviorChange(std::make_unique<PlayerRootBehavior>(this));

	// 攻撃エフェクトを非表示に初期化
	isAttackEffectVisible_ = false;

	// ノックバック無敵状態の初期化
	isKnockbackInvincible_ = false;

	// 外部からのノックバック要求を初期化
	isKnockbackRequested_ = false;

	width_ = 0.8f;
	height_ = 0.8f;
}

void Player::BeforeBehaviorUpdate() {
	// 外部からのノックバック要求を処理
	if (isKnockbackRequested_) {
		if (!isDead_) {
			// ノックバックへの切り替えを要求
			RequestBehaviorChange(std::make_unique<PlayerKnockbackBehavior>(this));
		}

		isKnockbackRequested_ = false;
	}
}

void Player::AfterBehaviorUpdate() {
	// 強制スクロール中かつ生存中は画面端補正と挟まれ死亡判定を行う
	if (!isDead_) {
		if (isScreenSqueezeDeathEnabled_) {
			// 画面端からはみ出さないように補正
			CorrectPositionInScreen();

			// 画面端とブロックに挟まれていたら死亡
			CheckScreenSqueezeDeath();
		}
	}

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		// 旋回タイマーを更新
		turnTimer_ -= 1.0f / 60.0f;

		// 補間割合
		float t = 1.0f - std::clamp(turnTimer_ / kTurnTime, 0.0f, 1.0f);

		// イージング
		float easedT = Ease::EaseInOut(t);

		// 目標角度へ補間
		worldTransform_.rotation_.y = std::lerp(turnFirstRotationY_, turnTargetRotationY_, easedT);

		// 完了したら正確に目標角度へ
		if (turnTimer_ <= 0.0f) {
			worldTransform_.rotation_.y = turnTargetRotationY_;
			turnTimer_ = 0.0f;
		}
	}
}

void Player::AfterTransformUpdate() {
	// 攻撃エフェクト用ワールド変換の更新
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	WorldTransformUpdate(worldTransformAttack_);
}

void Player::Draw() {
	// 描画すべきでないなら描画しない
	if (!ShouldDraw()) {
		return;
	}

	Model::PreDraw();

	model_->Draw(worldTransform_, *camera_, &objectColor_);

	// 攻撃エフェクトの描画
	if (IsAttack()) {
		if (isAttackEffectVisible_) {
			modelAttack_->Draw(worldTransformAttack_, *camera_);
		}
	}


	Model::PostDraw();
}

void Player::OnCollisionCeiling() {
	// 天井に当たったら上昇速度をリセット
	velocity_.y = 0.0f;
}

void Player::OnCollisionWall() {
	// 壁に接触した際の横速度減衰
	velocity_.x *= (1.0f - kAttenuationWall);
}

void Player::AfterMapChipCollision(const CollisionMapInfo& info) {
	// プレイヤーの接地状態を更新
	ChangeGroundState(info);
}

// 地面に接触してる場合の処理
void Player::HandleGroundCollision(const CollisionMapInfo& info) {
	if (info.onGround) {
		// 着地状態に移行
		isGround_ = true;

		// 接地時にx速度を減衰
		velocity_.x *= (1.0f - kAttenuationLanding);

		velocity_.y = 0.0f;
	}
}

// 接地状態の切り替え処理
void Player::ChangeGroundState(const CollisionMapInfo& info) {
	if (isGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			isGround_ = false;
			return;
		}

		// 落下開始判定
		MapChipType mapChipType;
		bool isHit = false;

		// 左下点の判定
		Vector3 leftBottom = CornerPosition(worldTransform_.translation_, kLeftBottom);
		leftBottom += Vector3(0.0f, -kGroundCheckOffset, 0.0f);

		IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);
		mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

		if (mapChipType == MapChipType::kBlock || mapChipField_->IsLadderTop(indexSet.xIndex, indexSet.yIndex)) {
			isHit = true;
		}

		// 右下点の判定
		Vector3 rightBottom = CornerPosition(worldTransform_.translation_, kRightBottom);
		rightBottom += Vector3(0.0f, -kGroundCheckOffset, 0.0f);

		indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);
		mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

		if (mapChipType == MapChipType::kBlock) {
			isHit = true;
		}

		// 床がなくなったら空中状態に移行
		if (!isHit) {
			isGround_ = false;
		}

	} else {
		// 空中から地面に接地する際の処理
		HandleGroundCollision(info);
	}
}

const WorldTransform& Player::GetWorldTransform() {
	return worldTransform_;
}

void Player::OnCollision(const BaseEnemy* enemy) {
	// すでに死んでいたら早期リターン
	if (isDead_) {
		return;
	}

	// 現状は敵に当たった時、敵に対して行いたい処理がない
	// そのため、警告を抑制する意味で無意味な処理を書く
	(void)enemy;

	// 攻撃中はダメージ無効
	if (IsAttack()) {
		return;
	}

	// ノックバック初めの時間に無敵でないと、そのまま敵にめり込んで死ぬ場合があるので、吹っ飛ばされている間は無敵
	if (IsKnockbackInvincible()) {
		return;
	}

	// 通常死亡状態にする
	deathType_ = DeathType::kNormal;
	isDead_ = true;
}

void Player::CorrectPositionInScreen() {
	// カメラが未設定なら何もしない
	if (!camera_) {
		return;
	}

	// カメラ位置を基準に画面端のワールド座標を計算
	const float leftLimit = camera_->translation_.x + kScreenLeftLimit + width_ / 2.0f;
	const float rightLimit = camera_->translation_.x + kScreenRightLimit - width_ / 2.0f;
	const float bottomLimit = camera_->translation_.y + kScreenBottomLimit + height_ / 2.0f;
	const float topLimit = camera_->translation_.y + kScreenTopLimit - height_ / 2.0f;

	// 画面外に出ないように座標を補正
	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, leftLimit, rightLimit);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, bottomLimit, topLimit);
}

void Player::CheckScreenSqueezeDeath() {
	// 強制スクロール中でなければ判定しない
	if (!isScreenSqueezeDeathEnabled_) {
		return;
	}

	// すでに死亡しているなら何もしない
	if (isDead_) {
		return;
	}

	// 画面左端にいなければ挟まれていない
	if (!IsTouchingLeftScreenEdge()) {
		return;
	}

	// 右側にブロックがあれば、画面端とブロックに挟まれて死亡
	if (IsBlockOnRightSide()) {
		StartSqueezeDeath();
	}
}

bool Player::IsTouchingLeftScreenEdge() {
	if (!camera_) {
		return false;
	}

	// プレイヤー中心座標として許される左端
	const float leftLimit = camera_->translation_.x + kScreenLeftLimit + width_ / 2.0f;

	return worldTransform_.translation_.x <= leftLimit + kScreenEdgeCheckTolerance;
}

bool Player::IsBlockOnRightSide() {
	if (!mapChipField_) {
		return false;
	}

	// 右上と右下を少しだけ右側にずらして、壁があるか調べる
	KamataEngine::Vector3 rightTop = {
		worldTransform_.translation_.x + width_ / 2.0f + kScreenSqueezeCheckOffset,
		worldTransform_.translation_.y + height_ / 2.0f - kScreenSqueezeCheckOffset,
		worldTransform_.translation_.z
	};

	KamataEngine::Vector3 rightBottom = {
		worldTransform_.translation_.x + width_ / 2.0f + kScreenSqueezeCheckOffset,
		worldTransform_.translation_.y - height_ / 2.0f + kScreenSqueezeCheckOffset,
		worldTransform_.translation_.z
	};

	IndexSet indexSet{};

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightTop);

	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		return true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);

	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		return true;
	}

	return false;
}

void Player::StartSqueezeDeath() {
	deathType_ = DeathType::kSqueeze;
	isDead_ = true;
	isDeathMotionFinished_ = false;

	RequestBehaviorChange(std::make_unique<PlayerSqueezeDeathBehavior>(this));

	// 通常移動の速度を一旦止める
	velocity_ = {0.0f, 0.0f, 0.0f};
}

bool Player::ShouldDraw() const {
	// 生きているなら描画する
	if (!isDead_) {
		return true;
	}

	// 挟まれ死亡中はプレイヤー本体を描画する
	if (deathType_ == DeathType::kSqueeze) {
		return true;
	}

	// 通常死亡では描画しない
	return false;
}

void Player::OnFallDeath() {
	// 通常死亡として処理
	deathType_ = DeathType::kNormal;
	isDead_ = true;
}


bool Player::TryFinishLadderClimb() {
	if (!mapChipField_) {
		return false;
	}

	// 足元の座標を取得
	Vector3 footPosition = worldTransform_.translation_;

	// 足元より少し下のはしごを探す
	footPosition.y -= height_ / 2.0f + 0.3f;

	// 足元にあるマップチップを取得
	IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(footPosition);

	// はしごの最上段でなければ対象外
	if (!mapChipField_->IsLadderTop(indexSet.xIndex, indexSet.yIndex)) {
		return false;
	}

	// 最上段の上面を取得
	Rect rect = mapChipField_->RectByIndex(indexSet.xIndex, indexSet.yIndex);

	// 登り切り時に補正してよい高さか確認
	float targetPositionY = rect.top + height_ / 2.0f + kBlank;

	// プレイヤーの足が上端に近づくまでは乗り移らない
	const float climbFinishDistance = 0.2f;

	float distanceToTop = targetPositionY - worldTransform_.translation_.y;

	if (distanceToTop < 0.0f || distanceToTop > climbFinishDistance) {
		return false;
	}

	// 乗り移り先にブロックがないことを確認する
	const Vector3 targetCenter{worldTransform_.translation_.x, targetPositionY, worldTransform_.translation_.z};

	const Vector3 checkPositions[] = {targetCenter, CornerPosition(targetCenter, kLeftTop), CornerPosition(targetCenter, kRightTop)};

	for (const Vector3& position : checkPositions) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(position);

		if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
			return false;
		}
	}

	// はしご上面に乗せる
	worldTransform_.translation_.y = targetPositionY;

	// 接地状態へ移行
	velocity_ = {0.0f, 0.0f, 0.0f};
	isGround_ = true;

	return true;
}