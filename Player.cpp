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

using namespace KamataEngine;

void Player::Initialize(Model* model, Model* modelAttack, Camera* camera, Vector3 position) {

	assert(model);
	assert(camera);
	assert(modelAttack);

	// 引数で渡されたモデルをメンバ変数に保存
	model_ = model;
	modelAttack_ = modelAttack;

	// 死亡フラグを初期化
	isDead_ = false;
	deathType_ = DeathType::kNone;
	isDeathMotionFinished_ = false;

	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 初期配置座標
	worldTransform_.translation_ = position;

	// モデルの向きに合わせてY軸回りに90度回転させる
	worldTransform_.rotation_.y = kPi / 2.0f;

	worldTransformAttack_.Initialize();
	worldTransformAttack_.translation_ = position;

	// 引数で渡されたカメラをメンバ変数に保存
	camera_ = camera;

	// 振る舞いの初期化
	behavior_ = Behavior::kRoot;
	behaviorRequest_ = Behavior::kUnknown;

	BehaviorRootInitialize();

	attackParameter_ = 0.0f;
	attackPhase_ = AttackPhase::kBefore;
}

void Player::Update() {
	// 外部からのノックバック要求を処理
	if (isKnockbackRequested_) {
		// 攻撃中のスケールが残らないように戻す
		worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

		behaviorRequest_ = Behavior::kKnockback;
		isKnockbackRequested_ = false;
	}

	// 振る舞い変更のリクエストが来ていた場合、振る舞いを変更する
	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振る舞い変更の反映
		behavior_ = behaviorRequest_;

		// 振る舞いごとの初期化
		switch (behavior_) {
			// kRootとdefaultは同じ処理
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;

		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;

		case Behavior::kKnockback:
			BehaviorKnockInitialize();
			break;

		case Behavior::kSqueezeDeath:
			BehaviorSqueezeDeathInitialize();
			break;
		}

		// 振る舞いリクエストの初期化
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
		// kRootとdefaultは同じ処理
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;

	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;

	case Behavior::kKnockback:
		BehaviorKnockUpdate();
		break;

	case Behavior::kSqueezeDeath:
		BehaviorSqueezeDeathUpdate();
		break;
	}

	// 強制スクロール中かつ生存中は画面端補正と挟まれ死亡判定を行う
	if (!isDead_) {
		if (isScreenSqueezeDeathEnabled_ && !isDead_) {
			// 画面端からはみ出さないように補正
			CorrectPositionInScreen();

			// 画面端とブロックに挟まれていたら死亡
			CheckScreenSqueezeDeath();
		}
	}

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		// 旋回タイマーを1/60秒カウントダウンする
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = {
		    kPi / 2.0f,        // 右
		    kPi * 3.0f / 2.0f, // 左
		};

		// 状態に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// 0.0f から 1.0f に進む補間割合
		float t = 1.0f - std::clamp(turnTimer_ / kTurnTime, 0.0f, 1.0f);

		// イージングをかける
		float easedT = Ease::EaseInOut(t);

		// 自キャラの角度を設定する
		worldTransform_.rotation_.y = std::lerp(turnFirstRotationY_, destinationRotationY, easedT);
	}

	// ワールド変換の更新
	WorldTransformUpdate(worldTransform_);

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

	if (behavior_ == Behavior::kAttack) {
		if (attackPhase_ == AttackPhase::kAttack) {
			modelAttack_->Draw(worldTransformAttack_, *camera_);
		}
	}

	model_->Draw(worldTransform_, *camera_);

	Model::PostDraw();
}

void Player::BehaviorRootInitialize() {}

void Player::BehaviorRootUpdate() {
	// 移動入力
	// 接地状態
	if (isGround_) {
		// 左右移動操作
		// 左右を同時に入力されている場合、入力無しと扱う（入力優先順位をつけないようにしたり、摩擦の処理との整合性を保つため）
		if (GameInput::IsPress(GameAction::kMoveRight) ^ GameInput::IsPress(GameAction::kMoveLeft)) {
			// 左右加速
			Vector3 acceleration{};

			// 右入力
			if (GameInput::IsPress(GameAction::kMoveRight)) {
				// 向きの更新
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;

					// 旋回開始時の角度を記録する
					turnFirstRotationY_ = worldTransform_.rotation_.y;

					// 旋回タイマーに時間を設定する
					turnTimer_ = kTurnTime;
				}

				// 速度と逆方向に入力中は急ブレーキ
				if (velocity_.x < 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x += kAcceleration;
			}

			// 左入力
			if (GameInput::IsPress(GameAction::kMoveLeft)) {
				// 向きの更新
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;

					// 旋回開始時の角度を記録する
					turnFirstRotationY_ = worldTransform_.rotation_.y;

					// 旋回タイマーに時間を設定する
					turnTimer_ = kTurnTime;
				}

				// 速度と逆方向に入力中は急ブレーキ
				if (velocity_.x > 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x -= kAcceleration;
			}

			// 加速を反映
			velocity_ += acceleration;

			// 速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {
			// 左右移動操作がない場合は減速
			velocity_.x *= (1.0f - kAttenuation);
		}

		// ジャンプ操作
		if (GameInput::IsPress(GameAction::kJump)) {
			velocity_ += Vector3(0.0f, kJumpAcceleration, 0.0f);
		}

		// 空中
	} else {
		// 左右を同時に入力されている場合、入力無しと扱う
		if (GameInput::IsPress(GameAction::kMoveRight) ^ GameInput::IsPress(GameAction::kMoveLeft)) {
			// 空中での左右加速
			Vector3 acceleration{};

			// 右入力
			if (GameInput::IsPress(GameAction::kMoveRight)) {
				// 向きの更新
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;

					// 旋回開始時の角度を記録する
					turnFirstRotationY_ = worldTransform_.rotation_.y;

					// 旋回タイマーに時間を設定する
					turnTimer_ = kTurnTime;
				}

				acceleration.x += kAcceleration * kAirAccelerationRate;
			}

			// 左入力
			if (GameInput::IsPress(GameAction::kMoveLeft)) {
				// 向きの更新
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;

					// 旋回開始時の角度を記録する
					turnFirstRotationY_ = worldTransform_.rotation_.y;

					// 旋回タイマーに時間を設定する
					turnTimer_ = kTurnTime;
				}

				acceleration.x -= kAcceleration * kAirAccelerationRate;
			}

			// 加速を反映
			velocity_ += acceleration;

			// 速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		}

		// 落下速度
		velocity_ += Vector3(0.0f, -kGravityAcceleration, 0.0f);

		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}

	if (GameInput::IsTrigger(GameAction::kNormalAttack)) {
		behaviorRequest_ = Behavior::kAttack;
	}

	ResolveMapChipCollision(velocity_);
}

void Player::BehaviorAttackInitialize() {
	// 攻撃の時間を初期化
	attackParameter_ = 0.0f;

	// 攻撃のフェーズを初期化
	attackPhase_ = AttackPhase::kBefore;

	// 攻撃開始時に速度をリセット
	velocity_ = {0.0f, 0.0f, 0.0f};
}

void Player::BehaviorAttackUpdate() {
	attackParameter_ += 1.0f / 60.0f;

	// 攻撃動作用の速度
	Vector3 attackVelocity{};

	switch (attackPhase_) {
	case AttackPhase::kBefore: {
		// 溜め動作
		float t = std::clamp(attackParameter_ / kAttackBeforeTime, 0.0f, 1.0f);

		worldTransform_.scale_.z = Lerp(1.0f, 0.3f, Ease::EaseOut(t));
		worldTransform_.scale_.y = Lerp(1.0f, 1.6f, Ease::EaseOut(t));

		velocity_ = {0.0f, 0.0f, 0.0f};

		if (attackParameter_ >= kAttackBeforeTime) {
			attackPhase_ = AttackPhase::kAttack;
			attackParameter_ = 0.0f;
		}
		break;
	}

	case AttackPhase::kAttack: {
		// 突進動作
		float t = std::clamp(attackParameter_ / kAttackTime, 0.0f, 1.0f);

		worldTransform_.scale_.z = Lerp(0.3f, 1.3f, Ease::EaseOut(t));
		worldTransform_.scale_.y = Lerp(1.6f, 0.7f, Ease::EaseIn(t));

		if (lrDirection_ == LRDirection::kRight) {
			attackVelocity.x = kAttackMoveSpeed;
		} else {
			attackVelocity.x = -kAttackMoveSpeed;
		}

		if (attackParameter_ >= kAttackTime) {
			attackPhase_ = AttackPhase::kAfter;
			attackParameter_ = 0.0f;
		}
		break;
	}

	case AttackPhase::kAfter: {
		// 余韻動作
		float t = std::clamp(attackParameter_ / kAttackAfterTime, 0.0f, 1.0f);

		worldTransform_.scale_.z = Lerp(1.3f, 1.0f, Ease::EaseOut(t));
		worldTransform_.scale_.y = Lerp(0.7f, 1.0f, Ease::EaseOut(t));

		attackVelocity.x = 0.0f;

		if (attackParameter_ >= kAttackAfterTime) {
			worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
			behaviorRequest_ = Behavior::kRoot;
		}
		break;
	}
	}

	ResolveMapChipCollision(attackVelocity);
}

void Player::BehaviorKnockInitialize () {
	// 攻撃中の変形を戻す
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 攻撃状態を初期化
	attackParameter_ = 0.0f;
	attackPhase_ = AttackPhase::kBefore;

	// ノックバック状態を初期化
	knockbackPhase_ = KnockbackPhase::kKnockback;
	knockbackParameter_ = 0.0f;

	// 向いている方向と逆方向へ吹っ飛ばす
	float knockbackDirection = 0.0f;

	if (lrDirection_ == LRDirection::kRight) {
		knockbackDirection = -1.0f;
	} else {
		knockbackDirection = 1.0f;
	}

	velocity_ = {knockbackDirection * kKnockbackSpeed, 0.0f, 0.0f};
}

void Player::BehaviorKnockUpdate () {
	knockbackParameter_ += 1.0f / 60.0f;

	switch (knockbackPhase_) {
	case KnockbackPhase::kKnockback:
		// 真横ノックバックなのでY方向には動かさない
		velocity_.y = 0.0f;

		// ノックバック移動
		ResolveMapChipCollision(velocity_);

		if (knockbackParameter_ >= kKnockbackTime) {
			knockbackPhase_ = KnockbackPhase::kAfter;
			knockbackParameter_ = 0.0f;
		}

		break;

	case KnockbackPhase::kAfter:
		// 横移動を減速
		velocity_.x *= (1.0f - kKnockbackAttenuation);

		// 真横ノックバックなのでY方向には動かさない
		velocity_.y = 0.0f;

		ResolveMapChipCollision(velocity_);

		if (knockbackParameter_ >= kKnockbackAfterTime) {
			velocity_ = {0.0f, 0.0f, 0.0f};
			behaviorRequest_ = Behavior::kRoot;
		}

		break;
	}
}

void Player::ResolveMapChipCollision(const Vector3& moveDistance) {
	// 衝突判定の初期化
	CollisionMapInfo collisionMapInfo{};

	// 移動量のコピー
	collisionMapInfo.moveDistance = moveDistance;

	// マップ衝突チェック
	MapChipCollision(collisionMapInfo);

	// 移動
	ApplyCollisionResult(collisionMapInfo);

	// 天井に接触している時の処理
	HandleCeilingCollision(collisionMapInfo);

	// 壁に接触している時の処理
	HandleWallCollision(collisionMapInfo);

	// 接地状態の切り替え処理
	ChangeGroundState(collisionMapInfo);
}

void Player::MapChipCollision(CollisionMapInfo& info) {
	MapChipCollisionTop(info);
	MapChipCollisionBottom(info);
	MapChipCollisionLeft(info);
	MapChipCollisionRight(info);
}

void Player::MapChipCollisionTop(CollisionMapInfo& info) {
	// 上昇あり？
	if (info.moveDistance.y <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew{};

	for (uint32_t i = 0; i < positionsNew.size(); i++) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveDistance, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 真上の判定を行う
	bool isHit = false;
	IndexSet indexSet{};
	IndexSet hitIndexSet{};

	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	if (isHit) {
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));

		// 移動前と移動後でY方向のセル番号が変わった場合だけ、天井として処理する
		if (indexSetNow.yIndex != hitIndexSet.yIndex) {
			Rect rect = mapChipField_->RectByIndex(hitIndexSet.xIndex, hitIndexSet.yIndex);

			info.moveDistance.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - kHeight / 2.0f - kBlank);

			info.isHitCeiling = true;
		}
	}
}

void Player::MapChipCollisionBottom(CollisionMapInfo& info) {
	// 下降あり？
	if (info.moveDistance.y >= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew{};

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveDistance, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 真下の当たり判定を行う
	bool isHit = false;
	IndexSet indexSet{};
	IndexSet hitIndexSet{};

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	if (isHit) {
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftBottom));

		// 移動前と移動後でY方向のセル番号が変わった場合だけ、地面として処理する
		if (indexSetNow.yIndex != hitIndexSet.yIndex) {
			Rect rect = mapChipField_->RectByIndex(hitIndexSet.xIndex, hitIndexSet.yIndex);

			info.moveDistance.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + kHeight / 2.0f + kBlank);

			info.onGround = true;
		}
	}
}

void Player::MapChipCollisionLeft(CollisionMapInfo& info) {
	// 左移動あり？
	if (info.moveDistance.x >= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew{};

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveDistance, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 真左の当たり判定を行う
	bool isHit = false;
	IndexSet indexSet{};
	IndexSet hitIndexSet{};

	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);

	// 隣接セルがどちらもブロックなら床や天井扱いなので、壁としては扱わない
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);

	// 隣接セルがどちらもブロックなら床や天井扱いなので、壁としては扱わない
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	if (isHit) {
		// 現在座標が属するマップチップ番号
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));

		// 移動前と移動後でX方向のセル番号が変わった場合だけ、壁として処理する
		if (indexSetNow.xIndex != hitIndexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			Rect rect = mapChipField_->RectByIndex(hitIndexSet.xIndex, hitIndexSet.yIndex);

			// めり込みを排除する方向に移動量を設定する
			info.moveDistance.x = std::min(0.0f, rect.right - worldTransform_.translation_.x + kWidth / 2.0f + kBlank);

			// 壁に当たったことを記録する
			info.isHitWall = true;
		}
	}
}

void Player::MapChipCollisionRight(CollisionMapInfo& info) {
	// 右移動あり？
	if (info.moveDistance.x <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew{};

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveDistance, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 真右の当たり判定を行う
	bool isHit = false;
	IndexSet indexSet{};
	IndexSet hitIndexSet{};

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);

	// 隣接セルがどちらもブロックなら床や天井扱いなので、壁としては扱わない
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);

	// 隣接セルがどちらもブロックなら床や天井扱いなので、壁としては扱わない
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	if (isHit) {
		// 現在座標が属するマップチップ番号
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kRightTop));

		// 移動前と移動後でX方向のセル番号が変わった場合だけ、壁として処理する
		if (indexSetNow.xIndex != hitIndexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			Rect rect = mapChipField_->RectByIndex(hitIndexSet.xIndex, hitIndexSet.yIndex);

			// めり込みを排除する方向に移動量を設定する
			info.moveDistance.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - kWidth / 2.0f - kBlank);

			// 壁に当たったことを記録する
			info.isHitWall = true;
		}
	}
}

void Player::ApplyCollisionResult(const CollisionMapInfo& info) {
	worldTransform_.translation_ += info.moveDistance;
}

void Player::HandleCeilingCollision(const CollisionMapInfo& info) {
	if (info.isHitCeiling) {
		velocity_.y = 0.0f;
	}
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

void Player::HandleWallCollision(const CollisionMapInfo& info) {
	if (info.isHitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
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

		if (mapChipType == MapChipType::kBlock) {
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

Vector3 Player::GetWorldPosition() const{
	Vector3 worldPosition{};
	worldPosition.x = worldTransform_.matWorld_.m[3][0];
	worldPosition.y = worldTransform_.matWorld_.m[3][1];
	worldPosition.z = worldTransform_.matWorld_.m[3][2];
	return worldPosition;
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb{};
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
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

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[static_cast<uint32_t>(Corner::kNumCorner)] = {
		Vector3(kWidth / 2.0f,	 -kHeight / 2.0f,	 0.0f),  // 右下
		Vector3(-kWidth / 2.0f,	 -kHeight / 2.0f,	 0.0f), // 左下
		Vector3(kWidth / 2.0f,	 kHeight / 2.0f,	 0.0f),   // 右上
		Vector3(-kWidth / 2.0f,	 kHeight / 2.0f,	 0.0f)   // 左上
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::RegisterGlobalVariables() {
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();
	const char* groupName = "Player";

	globalVariables->CreateGroup(groupName);

	globalVariables->AddItem(groupName, "Acceleration", kAcceleration);
	globalVariables->AddItem(groupName, "AirAccelerationRate", kAirAccelerationRate);
	globalVariables->AddItem(groupName, "Attenuation", kAttenuation);
	globalVariables->AddItem(groupName, "LimitRunSpeed", kLimitRunSpeed);
	globalVariables->AddItem(groupName, "AttenuationLanding", kAttenuationLanding);
	globalVariables->AddItem(groupName, "AttenuationWall", kAttenuationWall);
	globalVariables->AddItem(groupName, "TurnTime", kTurnTime);
	globalVariables->AddItem(groupName, "JumpAcceleration", kJumpAcceleration);

	globalVariables->AddItem(groupName, "KnockbackTime", kKnockbackTime);
	globalVariables->AddItem(groupName, "KnockbackAfterTime", kKnockbackAfterTime);
	globalVariables->AddItem(groupName, "KnockbackSpeed", kKnockbackSpeed);
	globalVariables->AddItem(groupName, "KnockbackAttenuation", kKnockbackAttenuation);

	globalVariables->AddItem(groupName, "AttackBeforeTime", kAttackBeforeTime);
	globalVariables->AddItem(groupName, "AttackTime", kAttackTime);
	globalVariables->AddItem(groupName, "AttackAfterTime", kAttackAfterTime);
	globalVariables->AddItem(groupName, "AttackMoveSpeed", kAttackMoveSpeed);
}

void Player::ApplyGlobalVariables() {
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();
	const char* groupName = "Player";

	kAcceleration = globalVariables->GetFloatValue(groupName, "Acceleration");
	kAirAccelerationRate = globalVariables->GetFloatValue(groupName, "AirAccelerationRate");
	kAttenuation = globalVariables->GetFloatValue(groupName, "Attenuation");
	kLimitRunSpeed = globalVariables->GetFloatValue(groupName, "LimitRunSpeed");
	kAttenuationLanding = globalVariables->GetFloatValue(groupName, "AttenuationLanding");
	kAttenuationWall = globalVariables->GetFloatValue(groupName, "AttenuationWall");
	kTurnTime = globalVariables->GetFloatValue(groupName, "TurnTime");
	kJumpAcceleration = globalVariables->GetFloatValue(groupName, "JumpAcceleration");

	kKnockbackTime = globalVariables->GetFloatValue(groupName, "KnockbackTime");
	kKnockbackAfterTime = globalVariables->GetFloatValue(groupName, "KnockbackAfterTime");
	kKnockbackSpeed = globalVariables->GetFloatValue(groupName, "KnockbackSpeed");
	kKnockbackAttenuation = globalVariables->GetFloatValue(groupName, "KnockbackAttenuation");

	kAttackBeforeTime = globalVariables->GetFloatValue(groupName, "AttackBeforeTime");
	kAttackTime = globalVariables->GetFloatValue(groupName, "AttackTime");
	kAttackAfterTime = globalVariables->GetFloatValue(groupName, "AttackAfterTime");
	kAttackMoveSpeed = globalVariables->GetFloatValue(groupName, "AttackMoveSpeed");
}

void Player::CorrectPositionInScreen() {
	// カメラが未設定なら何もしない
	if (!camera_) {
		return;
	}

	// カメラ位置を基準に画面端のワールド座標を計算
	const float leftLimit = camera_->translation_.x + kScreenLeftLimit + kWidth / 2.0f;
	const float rightLimit = camera_->translation_.x + kScreenRightLimit - kWidth / 2.0f;
	const float bottomLimit = camera_->translation_.y + kScreenBottomLimit + kHeight / 2.0f;
	const float topLimit = camera_->translation_.y + kScreenTopLimit - kHeight / 2.0f;

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
	const float leftLimit = camera_->translation_.x + kScreenLeftLimit + kWidth / 2.0f;

	return worldTransform_.translation_.x <= leftLimit + kScreenEdgeCheckTolerance;
}

bool Player::IsBlockOnRightSide() {
	if (!mapChipField_) {
		return false;
	}

	// 右上と右下を少しだけ右側にずらして、壁があるか調べる
	KamataEngine::Vector3 rightTop = {
		worldTransform_.translation_.x + kWidth / 2.0f + kScreenSqueezeCheckOffset,
		worldTransform_.translation_.y + kHeight / 2.0f - kScreenSqueezeCheckOffset,
		worldTransform_.translation_.z
	};

	KamataEngine::Vector3 rightBottom = {
		worldTransform_.translation_.x + kWidth / 2.0f + kScreenSqueezeCheckOffset,
		worldTransform_.translation_.y - kHeight / 2.0f + kScreenSqueezeCheckOffset,
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

	behaviorRequest_ = Behavior::kSqueezeDeath;

	// 通常移動の速度を一旦止める
	velocity_ = {0.0f, 0.0f, 0.0f};
}

void Player::BehaviorSqueezeDeathInitialize() {
	// 攻撃中の変形が残らないように戻す
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 演出タイマー初期化
	squeezeDeathTimer_ = 0.0f;

	// 少し上へ飛ばす
	velocity_ = {0.0f, kSqueezeDeathJumpVelocity, 0.0f};

	// 演出終了フラグを戻す
	isDeathMotionFinished_ = false;
}

void Player::BehaviorSqueezeDeathUpdate() {
	squeezeDeathTimer_ += 1.0f / 60.0f;

	// 重力で落下
	velocity_.y -= kSqueezeDeathGravity;

	// 移動
	worldTransform_.translation_ += velocity_;

	// X回転しながら吹っ飛ぶ
	worldTransform_.rotation_.x += kSqueezeDeathRotationSpeed * (1.0f / 60.0f);

	// 一定時間経過、または画面下の少し下まで落ちたら演出終了
	const float bottomLimit = camera_->translation_.y + kScreenBottomLimit - 2.0f;

	if (squeezeDeathTimer_ >= kSqueezeDeathMotionTime || worldTransform_.translation_.y < bottomLimit) {
		isDeathMotionFinished_ = true;
	}
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